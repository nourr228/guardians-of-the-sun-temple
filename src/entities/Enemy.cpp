#include "entities/Enemy.h"

#include <algorithm>

EnemyStats enemyStats(EnemyType type) {
    switch (type) {
    case EnemyType::Scarab:
        return {5, 1, 31.0f, 1.65f, 18.0f,
                EnemyAttackType::UpperRanged, CharacterType::CursedScarab};
    case EnemyType::Serpent:
        return {5, 1, 31.0f, 1.9f, 22.0f,
                EnemyAttackType::LowerRanged, CharacterType::SandSerpent};
    case EnemyType::Pharaoh:
        return {7, 2, 57.0f, 1.05f, 19.0f,
                EnemyAttackType::Melee, CharacterType::CursedPharaoh};
    }
    return {5, 1, 31.0f, 1.65f, 18.0f,
            EnemyAttackType::UpperRanged, CharacterType::CursedScarab};
}

const char* enemyName(EnemyType type) {
    switch (type) {
    case EnemyType::Scarab: return "Cursed Scarab";
    case EnemyType::Serpent: return "Sand Serpent";
    case EnemyType::Pharaoh: return "Cursed Pharaoh";
    }
    return "Unknown";
}

Enemy::Enemy(std::uint64_t enemyId, EnemyType enemyType, int lane,
             glm::vec2 entry)
        : id(enemyId), type(enemyType), laneId(lane), position(entry),
          halfWidth(enemyStats(enemyType).halfWidth),
          speed(enemyStats(enemyType).speed),
          health(enemyStats(enemyType).maxHealth),
          maxHealth(enemyStats(enemyType).maxHealth),
          damage(enemyStats(enemyType).damage),
          attackType(enemyStats(enemyType).attackType),
          animator(enemyStats(enemyType).visual) {}

bool Enemy::takeDamage(int amount) {
    if (!alive || amount <= 0) return false;
    health = std::max(0, health - amount);
    if (health == 0) {
        alive = false;
        behavior = EnemyBehavior::Dying;
        animator.setState(AnimationState::Dying, true);
        return true;
    }
    damageRemaining = 0.32f;
    behavior = EnemyBehavior::TakingDamage;
    animator.setState(AnimationState::TakingDamage, true);
    return false;
}

float Enemy::healthRatio() const {
    return static_cast<float>(health) / static_cast<float>(maxHealth);
}
