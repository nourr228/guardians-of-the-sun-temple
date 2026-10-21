#include "rendering/Animator.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

bool supportsAnimation(CharacterType type, AnimationState state) {
    switch (type) {
        case CharacterType::KhopeshGuardian:
        case CharacterType::SunArcher:
            return true;
        case CharacterType::StoneSentinel:
            return state == AnimationState::Idle ||
                   state == AnimationState::Attacking ||
                   state == AnimationState::TakingDamage ||
                   state == AnimationState::Dying;
        case CharacterType::CursedScarab:
        case CharacterType::SandSerpent:
        case CharacterType::CursedPharaoh:
            return state != AnimationState::Jumping &&
                   state != AnimationState::Ducking;
    }
    return false;
}

AnimationClip animationClip(CharacterType type, AnimationState state) {
    if (!supportsAnimation(type, state)) {
        throw std::invalid_argument("Animation is unsupported for character");
    }
    switch (state) {
        case AnimationState::Idle: return {4, 0.24f, true};
        case AnimationState::Moving: return {6, 0.10f, true};
        case AnimationState::Attacking: return {7, 0.085f, false};
        case AnimationState::Jumping: return {8, 0.09f, false};
        case AnimationState::Ducking: return {4, 0.14f, true};
        case AnimationState::TakingDamage: return {4, 0.08f, false};
        case AnimationState::Dying: return {9, 0.11f, false};
    }
    throw std::invalid_argument("Unknown animation state");
}

const char* animationName(AnimationState state) {
    switch (state) {
        case AnimationState::Idle: return "Idle";
        case AnimationState::Moving: return "Moving";
        case AnimationState::Attacking: return "Attacking";
        case AnimationState::Jumping: return "Jumping";
        case AnimationState::Ducking: return "Ducking";
        case AnimationState::TakingDamage: return "Taking damage";
        case AnimationState::Dying: return "Dying";
    }
    return "Unknown";
}

const char* characterName(CharacterType type) {
    switch (type) {
        case CharacterType::KhopeshGuardian: return "Khopesh Guardian";
        case CharacterType::StoneSentinel: return "Stone Sentinel";
        case CharacterType::SunArcher: return "Sun Archer";
        case CharacterType::CursedScarab: return "Cursed Scarab";
        case CharacterType::SandSerpent: return "Sand Serpent";
        case CharacterType::CursedPharaoh: return "Cursed Pharaoh";
    }
    return "Unknown";
}

Animator::Animator(CharacterType type) : type_(type) {
    clip_ = animationClip(type_, state_);
}

bool Animator::setState(AnimationState state, bool restart) {
    if (!supportsAnimation(type_, state)) return false;
    if (state == state_ && !restart) return true;
    state_ = state;
    clip_ = animationClip(type_, state);
    elapsed_ = 0.0f;
    complete_ = false;
    return true;
}

void Animator::update(float deltaTime) {
    const float step = std::max(0.0f, deltaTime);
    if (complete_) return;
    elapsed_ += step;
    if (clip_.loops) {
        elapsed_ = std::fmod(elapsed_, clip_.duration());
    } else if (elapsed_ >= clip_.duration()) {
        elapsed_ = clip_.duration();
        complete_ = true;
    }
}

int Animator::currentFrame() const {
    if (complete_) return clip_.frameCount - 1;
    return std::min(static_cast<int>(elapsed_ / clip_.frameDuration),
                    clip_.frameCount - 1);
}

float Animator::frameProgress() const {
    if (complete_) return 1.0f;
    return std::fmod(elapsed_, clip_.frameDuration) / clip_.frameDuration;
}

float Animator::progress() const {
    return complete_ ? 1.0f : elapsed_ / clip_.duration();
}
