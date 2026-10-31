#include "audio/AudioManager.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <mutex>
#include <vector>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace {
constexpr int sampleRate = 44100;
constexpr float tau = 6.28318530718f;
constexpr std::size_t cueCount = 8;

float noise(std::uint32_t index) {
    index ^= index >> 16;
    index *= 0x7feb352du;
    index ^= index >> 15;
    index *= 0x846ca68bu;
    index ^= index >> 16;
    return static_cast<float>(index & 0xffffu) / 32767.5f - 1.0f;
}

std::vector<float> makeMusic() {
    constexpr std::array<float, 16> notes{
            220.0f, 277.2f, 293.7f, 349.2f,
            330.0f, 293.7f, 233.1f, 220.0f,
            261.6f, 293.7f, 392.0f, 349.2f,
            330.0f, 277.2f, 233.1f, 220.0f};
    std::vector<float> result(sampleRate * 8);
    for (std::size_t i = 0; i < result.size(); ++i) {
        const float t = static_cast<float>(i) / sampleRate;
        const int noteIndex = std::min(15, static_cast<int>(t * 2.0f));
        const float noteTime = t - noteIndex * 0.5f;
        const float flute = (std::sin(tau * notes[noteIndex] * t) +
                             0.18f * std::sin(tau * notes[noteIndex] * 2 * t)) *
                            std::exp(-2.6f * noteTime);
        const float drone = 0.35f * std::sin(tau * 110.0f * t) +
                            0.14f * std::sin(tau * 165.0f * t);
        const float beatTime = std::fmod(t, 0.5f);
        const float drum = std::sin(tau * (75.0f - 30.0f * beatTime) * beatTime) *
                           std::exp(-27.0f * beatTime);
        const float edge = std::min({1.0f, t * 12.0f, (8.0f - t) * 12.0f});
        result[i] = (0.095f * flute + 0.035f * drone + 0.075f * drum) * edge;
    }
    return result;
}

std::vector<float> makeCue(SoundCue cue) {
    float seconds = 0.20f;
    switch (cue) {
    case SoundCue::Select: seconds = 0.10f; break;
    case SoundCue::Purchase: seconds = 0.30f; break;
    case SoundCue::Arrow: seconds = 0.16f; break;
    case SoundCue::Curse: seconds = 0.23f; break;
    case SoundCue::Pickup: seconds = 0.34f; break;
    case SoundCue::Damage: seconds = 0.20f; break;
    case SoundCue::Victory: seconds = 0.85f; break;
    case SoundCue::Defeat: seconds = 0.85f; break;
    }
    std::vector<float> result(static_cast<std::size_t>(seconds * sampleRate));
    for (std::size_t i = 0; i < result.size(); ++i) {
        const float t = static_cast<float>(i) / sampleRate;
        const float p = t / seconds;
        const float envelope = std::sin(3.14159265f * p) *
                               std::exp(-1.3f * p);
        const float hiss = noise(static_cast<std::uint32_t>(i +
                            static_cast<int>(cue) * 5789));
        float signal = 0.0f;
        switch (cue) {
        case SoundCue::Select:
            signal = std::sin(tau * 660.0f * t) * std::exp(-22.0f * t);
            break;
        case SoundCue::Purchase:
            signal = 0.7f * std::sin(tau * (440.0f + 220.0f * p) * t) +
                     0.3f * std::sin(tau * 880.0f * t);
            break;
        case SoundCue::Arrow:
            signal = 0.5f * hiss +
                     0.5f * std::sin(tau * (800.0f - 500.0f * p) * t);
            break;
        case SoundCue::Curse:
            signal = 0.65f * std::sin(tau * (240.0f - 120.0f * p) * t) +
                     0.35f * hiss;
            break;
        case SoundCue::Pickup:
            signal = std::sin(tau * (650.0f + 600.0f * p) * t) +
                     0.3f * std::sin(tau * 1300.0f * t);
            break;
        case SoundCue::Damage:
            signal = 0.6f * hiss + 0.4f * std::sin(tau * 110.0f * t);
            break;
        case SoundCue::Victory:
            signal = std::sin(tau * (440.0f + 110.0f * std::floor(p * 4)) * t) +
                     0.25f * std::sin(tau * 880.0f * t);
            break;
        case SoundCue::Defeat:
            signal = std::sin(tau * (330.0f - 180.0f * p) * t) +
                     0.18f * hiss;
            break;
        }
        result[i] = 0.19f * envelope * signal;
    }
    return result;
}
}

struct AudioManager::Impl {
    struct Voice {
        const std::vector<float>* clip = nullptr;
        std::size_t cursor = 0;
    };

    ma_device device{};
    bool ready = false;
    AudioSettings settings{};
    std::mutex mutex;
    std::vector<float> music = makeMusic();
    std::array<std::vector<float>, cueCount> cues{};
    std::array<Voice, 24> voices{};
    std::size_t musicCursor = 0;

    Impl() {
        for (std::size_t i = 0; i < cues.size(); ++i)
            cues[i] = makeCue(static_cast<SoundCue>(i));
        ma_device_config config = ma_device_config_init(ma_device_type_playback);
        config.playback.format = ma_format_f32;
        config.playback.channels = 2;
        config.sampleRate = sampleRate;
        config.dataCallback = callback;
        config.pUserData = this;
        if (ma_device_init(nullptr, &config, &device) != MA_SUCCESS) return;
        if (ma_device_start(&device) != MA_SUCCESS) {
            ma_device_uninit(&device);
            return;
        }
        ready = true;
    }

    ~Impl() {
        if (ready) {
            ma_device_stop(&device);
            ma_device_uninit(&device);
        }
    }

    static void callback(ma_device* device, void* output,
                         const void*, ma_uint32 frameCount) {
        auto* self = static_cast<Impl*>(device->pUserData);
        auto* samples = static_cast<float*>(output);
        std::lock_guard lock(self->mutex);
        for (ma_uint32 i = 0; i < frameCount; ++i) {
            float mix = self->music[self->musicCursor++];
            if (self->musicCursor == self->music.size()) self->musicCursor = 0;
            for (Voice& voice : self->voices) {
                if (voice.clip == nullptr) continue;
                mix += (*voice.clip)[voice.cursor++];
                if (voice.cursor >= voice.clip->size()) voice = {};
            }
            mix = std::clamp(mix, -1.0f, 1.0f) *
                  (self->settings.muted ? 0.0f : self->settings.volume);
            samples[i * 2] = mix;
            samples[i * 2 + 1] = mix;
        }
    }
};

AudioManager::AudioManager() : impl_(std::make_unique<Impl>()) {}
AudioManager::~AudioManager() = default;

void AudioManager::play(SoundCue cue) {
    if (!impl_->ready) return;
    std::lock_guard lock(impl_->mutex);
    for (auto& voice : impl_->voices) {
        if (voice.clip == nullptr) {
            voice = {&impl_->cues[static_cast<std::size_t>(cue)], 0};
            return;
        }
    }
}

void AudioManager::clearEffects() {
    std::lock_guard lock(impl_->mutex);
    impl_->voices.fill({});
}

void AudioManager::adjustVolume(float amount) {
    std::lock_guard lock(impl_->mutex);
    impl_->settings.adjust(amount);
}

void AudioManager::toggleMute() {
    std::lock_guard lock(impl_->mutex);
    impl_->settings.toggleMute();
}

AudioSettings AudioManager::settings() const {
    std::lock_guard lock(impl_->mutex);
    return impl_->settings;
}

bool AudioManager::available() const { return impl_->ready; }
