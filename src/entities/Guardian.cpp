#include "entities/Guardian.h"

#include <algorithm>

GuardianStats guardianStats(GuardianType type) {
    switch (type) {
    case GuardianType::Khopesh:
        return {5, 10, 45.0f, 0.8f, 17.0f,
                CharacterType::KhopeshGuardian};
    case GuardianType::Sentinel:
        return {7, 30, 0.0f, 1.1f, 23.0f,
                CharacterType::StoneSentinel};
    case GuardianType::Archer:
        return {5, 20, 37.0f, 1.2f, 17.0f,
                CharacterType::SunArcher};
    }
    return {5, 10, 45.0f, 0.8f, 17.0f,
            CharacterType::KhopeshGuardian};
}

const char* guardianName(GuardianType type) {
    switch (type) {
    case GuardianType::Khopesh: return "Khopesh";
    case GuardianType::Sentinel: return "Sentinel";
    case GuardianType::Archer: return "Archer";
    }
    return "Unknown";
}

Guardian::Guardian(std::uint64_t idValue, GuardianType guardianType,
                   int lane, glm::vec2 entry)
        : id(idValue), type(guardianType), laneId(lane), position(entry),
          size(guardianStats(guardianType).halfWidth * 2.0f, 70.0f),
          speed(guardianStats(guardianType).speed),
          health(guardianStats(guardianType).maxHealth),
          maxHealth(guardianStats(guardianType).maxHealth),
          animator(guardianStats(guardianType).visual) {}

void Guardian::takeDamage(int amount) {
    if (!alive || amount <= 0) return;
    health = std::max(0, health - amount);
    if (health == 0) {
        alive = false;
        active = false;
        action = GuardianAction::Normal;
        animator.setState(AnimationState::Dying, true);
    } else {
        damageRemaining = 0.32f;
        animator.setState(AnimationState::TakingDamage, true);
    }
}

float Guardian::healthRatio() const {
    return static_cast<float>(health) / static_cast<float>(maxHealth);
}
