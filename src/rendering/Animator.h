#pragma once

#include "rendering/Animation.h"

// Visual timing only. The gameplay layer explicitly requests transitions.
class Animator {
public:
    explicit Animator(CharacterType type);

    bool setState(AnimationState state, bool restart = false);
    void update(float deltaTime);

    [[nodiscard]] CharacterType type() const { return type_; }
    [[nodiscard]] AnimationState state() const { return state_; }
    [[nodiscard]] int currentFrame() const;
    [[nodiscard]] float frameProgress() const;
    [[nodiscard]] float progress() const;
    [[nodiscard]] bool complete() const { return complete_; }
    [[nodiscard]] AnimationClip clip() const { return clip_; }

private:
    CharacterType type_;
    AnimationState state_ = AnimationState::Idle;
    AnimationClip clip_{};
    float elapsed_ = 0.0f;
    bool complete_ = false;
};
