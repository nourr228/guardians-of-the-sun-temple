#pragma once

enum class CharacterType {
    KhopeshGuardian,
    StoneSentinel,
    SunArcher,
    CursedScarab,
    SandSerpent,
    CursedPharaoh
};

enum class AnimationState {
    Idle,
    Moving,
    Attacking,
    Jumping,
    Ducking,
    TakingDamage,
    Dying
};

struct AnimationClip {
    int frameCount = 1;
    float frameDuration = 0.1f;
    bool loops = true;

    [[nodiscard]] float duration() const { return frameCount * frameDuration; }
};

[[nodiscard]] bool supportsAnimation(CharacterType type, AnimationState state);
[[nodiscard]] AnimationClip animationClip(CharacterType type,
                                          AnimationState state);
[[nodiscard]] const char* animationName(AnimationState state);
[[nodiscard]] const char* characterName(CharacterType type);
