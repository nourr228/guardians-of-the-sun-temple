#pragma once

#include <array>
#include <string>

#include "rendering/Animator.h"
#include "rendering/CharacterRenderer.h"

class Battlefield;
class Camera2D;
class InputManager;
class Renderer2D;

// Visual inspection only. It does not spawn entities or alter gameplay.
class CharacterShowcase {
public:
    explicit CharacterShowcase(const Renderer2D& renderer);

    void handleInput(const InputManager& input);
    void openPreview(AnimationState state, int damageStep = 0);
    void update(float deltaTime);
    void render(const Camera2D& camera, const Battlefield& battlefield) const;

    [[nodiscard]] bool visible() const { return visible_; }
    [[nodiscard]] std::string statusText() const;

private:
    void showState(AnimationState state);

    const Renderer2D& renderer_;
    CharacterRenderer characters_;
    std::array<Animator, 6> animators_;
    bool visible_ = false;
    bool autoplay_ = true;
    AnimationState activeState_ = AnimationState::Idle;
    int autoStage_ = 0;
    float autoTime_ = 0.0f;
    int damageStep_ = 0;
};
