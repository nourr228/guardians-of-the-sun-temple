#include "systems/EnemyManager.h"

#include <algorithm>
#include <stdexcept>

#include "world/Battlefield.h"

namespace {
constexpr float attackPoseDuration = 0.60f;
constexpr float projectileRadius = 7.0f;
}

EnemyManager::EnemyManager(const Battlefield& battlefield)
        : battlefield_(battlefield) {}

std::uint64_t EnemyManager::spawn(EnemyType type, int laneId) {
    if (laneId < 0 || laneId >= Battlefield::laneCount)
        throw std::out_of_range("Invalid enemy lane");
    const std::uint64_t id = nextId_++;
    enemies_.emplace_back(id, type, laneId, battlefield_.enemyEntry(laneId));
    return id;
}

std::vector<EnemyTarget> EnemyManager::targets() const {
    std::vector<EnemyTarget> result;
    for (const Enemy& enemy : enemies_)
        if (enemy.alive)
            result.push_back({enemy.id, enemy.laneId,
                              enemy.position.x, enemy.halfWidth});
    return result;
}

void EnemyManager::applyDamage(std::uint64_t id, int damage) {
    for (Enemy& enemy : enemies_) {
        if (enemy.id != id) continue;
        if (enemy.takeDamage(damage))
            defeats_.push_back({enemy.id, enemy.type, enemy.laneId,
                                enemy.position.x});
        return;
    }
}

std::vector<EnemyDefeat> EnemyManager::takeDefeats() {
    std::vector<EnemyDefeat> result;
    result.swap(defeats_);
    return result;
}

int EnemyManager::livingCount() const {
    return static_cast<int>(std::count_if(enemies_.begin(), enemies_.end(),
            [](const Enemy& enemy) { return enemy.alive; }));
}

Guardian* EnemyManager::contactFor(const Enemy& enemy, float nextX,
                                   std::vector<Guardian>& guardians) const {
    Guardian* contact = nullptr;
    float rightmost = -1.0f;
    for (Guardian& guardian : guardians) {
        if (!guardian.alive || guardian.laneId != enemy.laneId) continue;
        const float halfWidth = guardianStats(guardian.type).halfWidth;
        const float stopX = guardian.position.x + halfWidth + enemy.halfWidth;
        if (nextX <= stopX &&
            guardian.position.x - halfWidth <= enemy.position.x + enemy.halfWidth &&
            guardian.position.x > rightmost) {
            rightmost = guardian.position.x;
            contact = &guardian;
        }
    }
    return contact;
}

void EnemyManager::update(float deltaTime, std::vector<Guardian>& guardians) {
    if (breachedHome_) return;
    const float dt = std::clamp(deltaTime, 0.0f, 0.25f);
    for (Enemy& enemy : enemies_) updateEnemy(enemy, dt, guardians);
    if (!breachedHome_) updateProjectiles(dt, guardians);
    std::erase_if(enemies_, [](const Enemy& enemy) {
        return !enemy.alive && enemy.animator.complete();
    });
}

void EnemyManager::updateEnemy(Enemy& enemy, float dt,
                               std::vector<Guardian>& guardians) {
    if (!enemy.alive) {
        enemy.animator.update(dt);
        return;
    }
    const EnemyStats stats = enemyStats(enemy.type);
    enemy.attackCooldown = std::max(0.0f, enemy.attackCooldown - dt);
    enemy.attackRemaining = std::max(0.0f, enemy.attackRemaining - dt);
    enemy.damageRemaining = std::max(0.0f, enemy.damageRemaining - dt);

    const float nextX = enemy.position.x - enemy.speed * dt;
    Guardian* contact = contactFor(enemy, nextX, guardians);
    if (contact != nullptr) {
        const float stopX = contact->position.x +
                            guardianStats(contact->type).halfWidth +
                            enemy.halfWidth;
        // A guardian may already overlap the right-edge spawn point. Stop
        // there instead of pushing a newly spawned enemy outside the field.
        enemy.position.x = std::max(nextX,
                                    std::min(stopX, enemy.position.x));
    } else {
        enemy.position.x = nextX;
    }
    if (enemy.position.x - enemy.halfWidth <= Battlefield::homeBoundary) {
        breachedHome_ = true;
        return;
    }

    if (enemy.attackCooldown <= 0.0f) {
        if (enemy.attackType == EnemyAttackType::Melee) {
            // Swing on a regular rhythm even when no guardian is in reach;
            // only a contact target receives damage.
            if (contact != nullptr) contact->takeDamage(enemy.damage);
            enemy.attackRemaining = attackPoseDuration;
            enemy.attackCooldown = stats.attackCooldown;
        } else {
            const ProjectileLevel level = enemy.attackType ==
                    EnemyAttackType::UpperRanged
                    ? ProjectileLevel::High : ProjectileLevel::Low;
            projectiles_.push_back({enemy.id, enemy.laneId, level,
                    enemy.position.x - enemy.halfWidth - 8.0f,
                    battlefield_.projectilePosition(
                            enemy.laneId, enemy.position.x, level).y,
                    195.0f, enemy.damage});
            enemy.attackRemaining = attackPoseDuration;
            enemy.attackCooldown = stats.attackCooldown;
        }
    }

    AnimationState state = contact ? AnimationState::Idle : AnimationState::Moving;
    enemy.behavior = contact ? EnemyBehavior::Engaged : EnemyBehavior::Advancing;
    if (enemy.attackRemaining > 0.0f) {
        state = AnimationState::Attacking;
        enemy.behavior = EnemyBehavior::Attacking;
    }
    if (enemy.damageRemaining > 0.0f) {
        state = AnimationState::TakingDamage;
        enemy.behavior = EnemyBehavior::TakingDamage;
    }
    enemy.animator.setState(state);
    enemy.animator.update(dt);
}

void EnemyManager::updateProjectiles(float dt,
                                     std::vector<Guardian>& guardians) {
    for (EnemyProjectile& projectile : projectiles_) {
        const float previousX = projectile.x;
        projectile.x -= projectile.speed * dt;
        Guardian* firstHit = nullptr;
        float rightmost = -1.0f;
        for (Guardian& guardian : guardians) {
            if (!guardian.alive || guardian.laneId != projectile.laneId)
                continue;
            if (projectile.level == ProjectileLevel::High &&
                guardian.action == GuardianAction::Ducking) continue;
            if (projectile.level == ProjectileLevel::Low &&
                guardian.action == GuardianAction::Jumping) continue;
            const float halfWidth = guardianStats(guardian.type).halfWidth;
            if (guardian.position.x + halfWidth + projectileRadius < projectile.x ||
                guardian.position.x - halfWidth - projectileRadius > previousX ||
                guardian.position.x <= rightmost) continue;
            firstHit = &guardian;
            rightmost = guardian.position.x;
        }
        if (firstHit != nullptr) {
            firstHit->takeDamage(projectile.damage);
            projectile.x = Battlefield::homeBoundary - 1000.0f;
        }
    }
    std::erase_if(projectiles_, [](const EnemyProjectile& projectile) {
        return projectile.x < Battlefield::homeBoundary - 20.0f;
    });
}

void EnemyManager::clear() {
    enemies_.clear();
    projectiles_.clear();
    defeats_.clear();
    nextId_ = 1;
    breachedHome_ = false;
}
