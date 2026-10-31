#include "cutscene/CutsceneManager.h"

#include <algorithm>
#include <cmath>

namespace {
float smooth(float value) {
    const float t = std::clamp(value, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float between(float time, float from, float to) {
    return smooth((time - from) / (to - from));
}

float lerp(float from, float to, float t) {
    return from + (to - from) * t;
}
}

CutsceneManager::CutsceneManager()
        : actors_{CutsceneActor{CharacterType::KhopeshGuardian},
                  CutsceneActor{CharacterType::CursedPharaoh},
                  CutsceneActor{CharacterType::CursedScarab},
                  CutsceneActor{CharacterType::CursedScarab},
                  CutsceneActor{CharacterType::SunArcher},
                  CutsceneActor{CharacterType::StoneSentinel}} {
    evaluate(0.0f);
}

void CutsceneManager::start() {
    time_ = 0.0f;
    for (CutsceneActor& actor : actors_)
        actor.animator = Animator(actor.type);
    evaluate(0.0f);
}

void CutsceneManager::update(float deltaTime) {
    if (complete()) return;
    const float step = std::max(0.0f, deltaTime);
    time_ = std::min(duration, time_ + step);
    evaluate(step);
}

void CutsceneManager::skip() {
    time_ = duration;
    evaluate(0.0f);
}

void CutsceneManager::seek(float time) {
    start();
    time_ = std::clamp(time, 0.0f, duration);
    evaluate(0.12f);
}

void CutsceneManager::evaluate(float animationDelta) {
    const float t = time_;
    auto& guardian = actors_[static_cast<int>(CutsceneRole::Guardian)];
    guardian.visible = true;
    guardian.feet = {lerp(265.0f, 414.0f, between(t, 8.0f, 9.45f)), 487.0f};
    guardian.facing = 1.0f;
    guardian.scale = 1.45f;
    const AnimationState guardianPose = t >= 9.4f && t < 12.2f
            ? AnimationState::Attacking
            : (t >= 7.9f && t < 9.45f) ? AnimationState::Moving
            : AnimationState::Idle;
    guardian.animator.setState(guardianPose);
    guardian.animator.update(animationDelta);

    auto& pharaoh = actors_[static_cast<int>(CutsceneRole::Pharaoh)];
    pharaoh.visible = t >= 4.2f;
    pharaoh.feet = {lerp(1000.0f, 695.0f, between(t, 4.2f, 7.8f)) -
                    44.0f * between(t, 8.1f, 9.7f), 487.0f};
    pharaoh.facing = -1.0f;
    pharaoh.scale = 1.50f;
    pharaoh.animator.setState(t >= 8.3f && t < 12.1f
            ? AnimationState::Attacking : AnimationState::Moving);
    pharaoh.animator.update(animationDelta);

    for (int i = 0; i < 2; ++i) {
        auto& scarab = actors_[static_cast<int>(CutsceneRole::ScarabA) + i];
        scarab.visible = t >= 5.0f;
        scarab.feet = {lerp(1020.0f + i * 65.0f,
                            800.0f + i * 115.0f, between(t, 5.0f, 8.0f)) -
                       80.0f * between(t, 12.0f, 15.0f),
                       345.0f + i * 58.0f + 9.0f * std::sin(t * 5.0f + i)};
        scarab.facing = -1.0f;
        scarab.scale = 1.02f;
        scarab.animator.setState(AnimationState::Moving);
        scarab.animator.update(animationDelta);
    }

    auto& archer = actors_[static_cast<int>(CutsceneRole::Archer)];
    archer.visible = t >= 12.0f;
    archer.feet = {lerp(178.0f, 327.0f, between(t, 12.0f, 13.3f)), 488.0f};
    archer.facing = 1.0f;
    archer.scale = 1.38f;
    archer.animator.setState(t < 13.2f ? AnimationState::Moving
                                       : AnimationState::Attacking);
    archer.animator.update(animationDelta);

    auto& sentinel = actors_[static_cast<int>(CutsceneRole::Sentinel)];
    sentinel.visible = t >= 12.4f;
    sentinel.feet = {250.0f, 489.0f};
    sentinel.facing = 1.0f;
    sentinel.scale = 1.36f;
    sentinel.animator.setState(t < 12.8f ? AnimationState::Idle
                                         : AnimationState::Attacking);
    sentinel.animator.update(animationDelta);
}

CutsceneFrame CutsceneManager::frame() const {
    const float t = time_;
    CutsceneFrame result;
    result.time = t;
    result.scene = t < 4.0f ? 1 : t < 8.0f ? 2 : t < 12.0f ? 3 : 4;
    result.crystalGlow = (0.50f + 0.12f * std::sin(t * 4.2f)) *
                         (1.0f - 0.32f * between(t, 4.0f, 7.0f));
    result.storm = between(t, 4.0f, 6.2f) *
                   (1.0f - 0.65f * between(t, 7.4f, 9.2f));
    result.darkness = 0.08f + 0.37f * between(t, 4.0f, 7.0f);
    result.shake = 5.0f * (between(t, 4.05f, 4.35f) -
                           between(t, 5.25f, 5.7f)) * std::sin(t * 31.0f);
    result.transition = between(t, 15.15f, duration);
    result.curseVisible = t >= 10.35f && t <= 11.55f;
    result.cursePosition = {lerp(615.0f, 439.0f,
                                 between(t, 10.35f, 11.55f)), 405.0f};
    result.clashVisible = t >= 11.24f && t <= 11.85f;
    result.arrowVisible = t >= 13.35f && t <= 14.75f;
    result.arrowPosition = {lerp(348.0f, 825.0f,
                                 between(t, 13.35f, 14.75f)), 397.0f};
    if (t < 3.8f) {
        result.speaker = "NARRATOR";
        result.dialogue = "The Heart of the Sun protects Ra's kingdom.";
    } else if (t < 7.8f) {
        result.speaker = "NARRATOR";
        result.dialogue = "A curse awakens beneath the desert.";
    } else if (t < 10.05f) {
        result.speaker = "CURSED PHARAOH";
        result.dialogue = "The Heart of the Sun is mine!";
    } else if (t < 12.05f) {
        result.speaker = "KHOPESH GUARDIAN";
        result.dialogue = "Then you must face its guardians.";
    } else if (t < 15.5f) {
        result.speaker = "SUN ARCHER";
        result.dialogue = "Defend the sacred chamber!";
    }
    return result;
}
