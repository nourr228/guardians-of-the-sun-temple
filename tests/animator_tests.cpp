#include <cmath>
#include <cstdlib>
#include <iostream>

#include "rendering/Animator.h"

namespace {
void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

bool near(float a, float b) { return std::abs(a - b) < 0.0001f; }
} // namespace

int main() {
    for (CharacterType type : {CharacterType::KhopeshGuardian,
                               CharacterType::StoneSentinel,
                               CharacterType::SunArcher,
                               CharacterType::CursedScarab,
                               CharacterType::SandSerpent,
                               CharacterType::CursedPharaoh}) {
        check(supportsAnimation(type, AnimationState::Idle), "idle supported");
        check(supportsAnimation(type, AnimationState::Attacking),
              "attack supported");
        check(supportsAnimation(type, AnimationState::TakingDamage),
              "damage supported");
        check(supportsAnimation(type, AnimationState::Dying), "death supported");
    }
    check(!supportsAnimation(CharacterType::StoneSentinel,
                             AnimationState::Jumping),
          "sentinel cannot jump");
    check(!supportsAnimation(CharacterType::SandSerpent,
                             AnimationState::Ducking),
          "serpent cannot duck");

    Animator many(CharacterType::KhopeshGuardian);
    Animator one(CharacterType::KhopeshGuardian);
    check(many.setState(AnimationState::Moving), "moving transition");
    check(one.setState(AnimationState::Moving), "moving transition 2");
    for (int i = 0; i < 23; ++i) many.update(0.01f);
    one.update(0.23f);
    check(many.currentFrame() == one.currentFrame(),
          "frame independent of update count");
    check(std::abs(many.progress() - one.progress()) < 0.001f,
          "progress independent of update count");
    one.update(0.6f);
    check(!one.complete() && one.currentFrame() == 2,
          "moving loops through frames");

    check(one.setState(AnimationState::Attacking), "attack transition");
    check(one.currentFrame() == 0 && near(one.progress(), 0),
          "transition resets frame");
    one.update(one.clip().duration() + 2.0f);
    check(one.complete() && one.currentFrame() == one.clip().frameCount - 1,
          "one-shot attack finishes on last frame");
    one.update(1.0f);
    check(one.complete() && near(one.progress(), 1),
          "completed animation holds final pose");
    check(one.setState(AnimationState::Attacking, true) && !one.complete(),
          "explicit restart replays same state");
    check(one.setState(AnimationState::Dying), "death transition");
    one.update(one.clip().duration());
    check(one.complete(), "death completes");
    check(one.setState(AnimationState::Idle), "can return to idle for preview");

    Animator sentinel(CharacterType::StoneSentinel);
    check(!sentinel.setState(AnimationState::Moving),
          "unsupported transition rejected");
    check(sentinel.state() == AnimationState::Idle,
          "rejected transition preserves state");
    sentinel.update(-1.0f);
    check(near(sentinel.progress(), 0), "negative delta ignored");

    std::cout << "Animator tests passed\n";
}
