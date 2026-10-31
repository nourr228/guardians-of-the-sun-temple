#pragma once

#include <algorithm>
#include <memory>

enum class SoundCue {
    Select, Purchase, Arrow, Curse, Pickup, Damage, Victory, Defeat
};

struct AudioSettings {
    float volume = 0.6f;
    bool muted = false;

    void adjust(float amount) {
        volume = std::clamp(volume + amount, 0.0f, 1.0f);
    }
    void toggleMute() { muted = !muted; }
};

class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    void play(SoundCue cue);
    void clearEffects();
    void adjustVolume(float amount);
    void toggleMute();
    [[nodiscard]] AudioSettings settings() const;
    [[nodiscard]] bool available() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
