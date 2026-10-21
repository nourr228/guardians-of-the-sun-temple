#include "demo/CharacterShowcase.h"

#include <algorithm>
#include <array>
#include <cmath>

#include <GLFW/glfw3.h>

#include "core/InputManager.h"
#include "rendering/Camera2D.h"
#include "rendering/Renderer2D.h"
#include "world/Battlefield.h"

namespace {
constexpr std::array<CharacterType, 6> types{
        CharacterType::KhopeshGuardian,
        CharacterType::StoneSentinel,
        CharacterType::SunArcher,
        CharacterType::CursedScarab,
        CharacterType::SandSerpent,
        CharacterType::CursedPharaoh};
constexpr std::array<AnimationState, 7> states{
        AnimationState::Idle,
        AnimationState::Moving,
        AnimationState::Attacking,
        AnimationState::Jumping,
        AnimationState::Ducking,
        AnimationState::TakingDamage,
        AnimationState::Dying};
constexpr std::array<float, 4> healthSteps{1.0f, 0.7f, 0.4f, 0.1f};
} // namespace

CharacterShowcase::CharacterShowcase(const Renderer2D& renderer)
        : renderer_(renderer), characters_(renderer),
          animators_{Animator(types[0]), Animator(types[1]),
                     Animator(types[2]), Animator(types[3]),
                     Animator(types[4]), Animator(types[5])} {}

void CharacterShowcase::showState(AnimationState state) {
    activeState_ = state;
    for (auto& animator : animators_) {
        if (!animator.setState(state, true)) {
            animator.setState(AnimationState::Idle, true);
        }
    }
}

void CharacterShowcase::openPreview(AnimationState state, int damageStep) {
    visible_ = true;
    autoplay_ = false;
    damageStep_ = std::clamp(damageStep, 0,
                             static_cast<int>(healthSteps.size()) - 1);
    showState(state);
}

void CharacterShowcase::handleInput(const InputManager& input) {
    if (input.keyPressed(GLFW_KEY_F3)) {
        visible_ = !visible_;
        if (visible_) {
            autoplay_ = true;
            autoTime_ = 0;
            autoStage_ = 0;
            showState(AnimationState::Idle);
        }
    }
    if (!visible_) return;
    if (input.keyPressed(GLFW_KEY_0)) {
        autoplay_ = true;
        autoTime_ = 0;
        autoStage_ = 0;
        showState(AnimationState::Idle);
    }
    for (int i = 0; i < static_cast<int>(states.size()); ++i) {
        if (input.keyPressed(GLFW_KEY_1 + i)) {
            autoplay_ = false;
            showState(states[i]);
        }
    }
    if (input.keyPressed(GLFW_KEY_H)) {
        damageStep_ = (damageStep_ + 1) % static_cast<int>(healthSteps.size());
    }
    if (input.keyPressed(GLFW_KEY_R)) damageStep_ = 0;
}

void CharacterShowcase::update(float deltaTime) {
    if (!visible_) return;
    if (autoplay_) {
        autoTime_ += deltaTime;
        const int nextStage = static_cast<int>(autoTime_ / 2.0f) %
                              static_cast<int>(states.size());
        if (nextStage != autoStage_) {
            autoStage_ = nextStage;
            showState(states[autoStage_]);
        }
    }
    for (auto& animator : animators_) {
        const bool wasComplete = animator.complete();
        if (autoplay_ && wasComplete &&
            activeState_ != AnimationState::Dying) {
            animator.setState(animator.state(), true);
        }
        animator.update(deltaTime);
    }
}

void CharacterShowcase::render(const Camera2D& camera,
                                const Battlefield& battlefield) const {
    if (!visible_) return;
    const float health = healthSteps[damageStep_];
    for (int i = 0; i < static_cast<int>(types.size()); ++i) {
        const int laneId = i % 3;
        const float x = i < 3 ? 345.0f : 706.0f;
        const float feetY = battlefield.lane(laneId).bottom - 9.0f;
        renderer_.drawRect(camera, {x - 35, feetY - 2}, {70, 5},
                           {0.17f, 0.09f, 0.15f, 0.23f});
        characters_.draw(camera, types[i], animators_[i], {x, feetY},
                         i < 3 ? 1.0f : -1.0f, 0.94f, health);
    }
}

std::string CharacterShowcase::statusText() const {
    if (!visible_) return "Guardians of the Sun Temple - Battlefield (F3: characters)";
    return std::string("Character Showcase: ") + animationName(activeState_) +
           (autoplay_ ? " [auto]" : " [manual]") +
           " | 1-7 state, 0 auto, H damage, R reset, F3 close";
}
