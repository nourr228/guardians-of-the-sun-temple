#pragma once

#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "effects/EffectEvent.h"

enum class EffectOutcome { None, Victory, Defeat };

struct EffectParticle {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec4 color;
    float size;
    float life;
    float maxLife;
};

class EffectManager {
public:
    void emit(EffectEvent event);
    void setOutcome(EffectOutcome outcome);
    void update(float deltaTime);
    void reset();

    [[nodiscard]] const std::vector<EffectParticle>& particles() const {
        return particles_;
    }
    [[nodiscard]] EffectOutcome outcome() const { return outcome_; }

private:
    void burst(glm::vec2 point, glm::vec4 color,
               int count, float speed, float life);

    std::vector<EffectParticle> particles_;
    EffectOutcome outcome_ = EffectOutcome::None;
    float outcomeTimer_ = 0.0f;
    unsigned int burstIndex_ = 0;
};
