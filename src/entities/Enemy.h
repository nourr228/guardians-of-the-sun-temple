#pragma once

#include <cstdint>

#include <glm/vec2.hpp>

#include "rendering/Animator.h"
#include "world/Battlefield.h"

enum class EnemyType { Scarab, Serpent, Pharaoh, TrainingDummy };
enum class EnemyAttackType { UpperRanged, LowerRanged, Melee, None };
enum class EnemyBehavior { Advancing, Engaged, Attacking, TakingDamage, Dying };

struct EnemyStats {
    int maxHealth;
    int damage;
    float speed;
    float attackCooldown;
    float halfWidth;
    EnemyAttackType attackType;
    CharacterType visual;
};

[[nodiscard]] EnemyStats enemyStats(EnemyType type);
[[nodiscard]] const char* enemyName(EnemyType type);

struct Enemy {
    std::uint64_t id;
    EnemyType type;
    int laneId;
    glm::vec2 position;
    float halfWidth;
    float speed;
    int health;
    int maxHealth;
    int damage;
    EnemyAttackType attackType;
    EnemyBehavior behavior = EnemyBehavior::Advancing;
    float attackCooldown = 0.0f;
    float attackRemaining = 0.0f;
    float damageRemaining = 0.0f;
    bool alive = true;
    Animator animator;

    Enemy(std::uint64_t id, EnemyType type, int laneId, glm::vec2 position);
    [[nodiscard]] bool takeDamage(int amount);
    [[nodiscard]] float healthRatio() const;
};
