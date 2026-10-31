#include "effects/EffectManager.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float tau = 6.28318530718f;
}

void EffectManager::burst(glm::vec2 point, glm::vec4 color,
                          int count, float speed, float life) {
    if (particles_.size() > 450) return;
    for (int i = 0; i < count; ++i) {
        const float angle = tau * static_cast<float>(i) / count +
                            burstIndex_ * 0.41f;
        const float variation = 0.65f + ((i * 7 + burstIndex_) % 6) * 0.1f;
        const glm::vec2 velocity{std::cos(angle) * speed * variation,
                                  std::sin(angle) * speed * variation};
        const float lifetime = life * (0.75f + (i % 4) * 0.09f);
        particles_.push_back({point, velocity, color,
                              3.0f + static_cast<float>(i % 4) * 1.7f,
                              lifetime, lifetime});
    }
    ++burstIndex_;
}

void EffectManager::emit(EffectEvent event) {
    const glm::vec2 point{event.x, event.y};
    switch (event.kind) {
    case EffectEventKind::GoldImpact:
        burst(point, {1.0f, 0.84f, 0.29f, 0.9f}, 9, 86.0f, 0.32f);
        break;
    case EffectEventKind::CurseImpact:
        burst(point, {0.65f, 0.30f, 0.85f, 0.9f}, 9, 75.0f, 0.35f);
        break;
    case EffectEventKind::CoinPickup:
        burst(point, {1.0f, 0.93f, 0.45f, 1.0f}, 16, 105.0f, 0.55f);
        break;
    case EffectEventKind::EnemyDefeat:
        burst(point, {0.40f, 0.22f, 0.54f, 0.8f}, 12, 70.0f, 0.48f);
        break;
    case EffectEventKind::ArrowLaunch:
        burst(point, {1.0f, 0.79f, 0.29f, 0.58f}, 3, 32.0f, 0.22f);
        break;
    case EffectEventKind::CurseLaunch:
        burst(point, {0.51f, 0.25f, 0.72f, 0.58f}, 3, 30.0f, 0.25f);
        break;
    }
}

void EffectManager::setOutcome(EffectOutcome outcome) {
    if (outcome_ == outcome) return;
    outcome_ = outcome;
    outcomeTimer_ = 0.0f;
    if (outcome != EffectOutcome::None)
        burst({77.0f, 342.0f},
              outcome == EffectOutcome::Victory
                      ? glm::vec4{1.0f, 0.86f, 0.33f, 0.9f}
                      : glm::vec4{0.60f, 0.20f, 0.76f, 0.9f},
              32, 136.0f, 1.2f);
}

void EffectManager::update(float deltaTime) {
    const float dt = std::clamp(deltaTime, 0.0f, 0.25f);
    for (EffectParticle& particle : particles_) {
        particle.position += particle.velocity * dt;
        particle.velocity *= std::max(0.0f, 1.0f - 1.3f * dt);
        particle.life -= dt;
    }
    std::erase_if(particles_, [](const EffectParticle& particle) {
        return particle.life <= 0.0f;
    });
    if (outcome_ != EffectOutcome::None) {
        outcomeTimer_ += dt;
        while (outcomeTimer_ >= 0.16f) {
            outcomeTimer_ -= 0.16f;
            burst({77.0f, 342.0f},
                  outcome_ == EffectOutcome::Victory
                          ? glm::vec4{1.0f, 0.86f, 0.33f, 0.8f}
                          : glm::vec4{0.58f, 0.19f, 0.75f, 0.75f},
                  3, 45.0f, 0.75f);
        }
    }
}

void EffectManager::reset() {
    particles_.clear();
    outcome_ = EffectOutcome::None;
    outcomeTimer_ = 0.0f;
    burstIndex_ = 0;
}
