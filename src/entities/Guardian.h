#pragma once

#include <cstdint>

#include <glm/vec2.hpp>

#include "rendering/Animator.h"

enum class GuardianType { Khopesh, Sentinel, Archer };
enum class GuardianAction { Normal, Jumping, Ducking };

struct GuardianStats {
    int maxHealth;
    int cost;
    float speed;
    float attackCooldown;
    float halfWidth;
    CharacterType visual;
};

[[nodiscard]] GuardianStats guardianStats(GuardianType type);
[[nodiscard]] const char* guardianName(GuardianType type);

struct Guardian {
    std::uint64_t id;
    GuardianType type;
    int laneId;
    glm::vec2 position;
    glm::vec2 size;
    float speed;
    int health;
    int maxHealth;
    GuardianAction action = GuardianAction::Normal;
    float actionRemaining = 0.0f;
    float attackCooldown = 0.0f;
    float attackRemaining = 0.0f;
    float damageRemaining = 0.0f;
    bool active = true;
    bool alive = true;
    bool blockedByEnemy = false;
    Animator animator;

    Guardian(std::uint64_t id, GuardianType type, int laneId,
             glm::vec2 position);
    void takeDamage(int amount);
    [[nodiscard]] float healthRatio() const;
};
