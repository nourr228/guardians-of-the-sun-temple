#include "combat/CombatSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "world/Battlefield.h"

namespace {
constexpr float jumpDuration = 0.7f;
constexpr float attackPoseDuration = 0.6f;
constexpr float projectileRadius = 7.0f;
constexpr float impactDuration = 0.18f;

bool sameLane(int a, int b) { return a == b; }
bool sweptHit(float from, float to, float center, float halfWidth) {
    return center + halfWidth + projectileRadius >= std::min(from, to) &&
           center - halfWidth - projectileRadius <= std::max(from, to);
}
bool dodges(const Guardian& guardian, AttackHeight height) {
    return (height == AttackHeight::High &&
            guardian.action == GuardianAction::Ducking) ||
           (height == AttackHeight::Low &&
            guardian.action == GuardianAction::Jumping);
}
}

AttackSpec attackSpec(AttackKind kind) {
    switch (kind) {
    case AttackKind::KhopeshStrike: return {1, 45.0f, 0.8f, AttackHeight::Middle, 0.0f, TargetFaction::Enemies};
    case AttackKind::SunArrow: return {1, 1000.0f, 1.2f, AttackHeight::Middle, 290.0f, TargetFaction::Enemies};
    case AttackKind::ScarabCurse: return {1, 1000.0f, 1.65f, AttackHeight::High, -195.0f, TargetFaction::Guardians};
    case AttackKind::SandBlast: return {1, 1000.0f, 1.9f, AttackHeight::Low, -195.0f, TargetFaction::Guardians};
    case AttackKind::PharaohStrike: return {2, 48.0f, 1.05f, AttackHeight::Middle, 0.0f, TargetFaction::Guardians};
    }
    return {0, 0.0f, 1.0f, AttackHeight::Middle, 0.0f, TargetFaction::Enemies};
}

CombatSystem::CombatSystem(const Battlefield& battlefield)
        : battlefield_(battlefield) {}

void CombatSystem::addImpact(float x, float y, AttackKind kind) {
    impacts_.push_back({x, y, kind, impactDuration});
    effectEvents_.push_back({kind == AttackKind::KhopeshStrike ||
                             kind == AttackKind::SunArrow
                             ? EffectEventKind::GoldImpact
                             : EffectEventKind::CurseImpact, x, y});
}

Guardian* CombatSystem::protectingSentinel(
        std::vector<Guardian>& guardians, int laneId,
        float attackerX, float targetX) {
    Guardian* shield = nullptr;
    for (Guardian& guardian : guardians) {
        if (!guardian.alive || guardian.type != GuardianType::Sentinel ||
            !sameLane(guardian.laneId, laneId) ||
            guardian.position.x <= targetX ||
            guardian.position.x >= attackerX) continue;
        if (shield == nullptr || guardian.position.x > shield->position.x)
            shield = &guardian;
    }
    return shield;
}

void CombatSystem::advanceTimers(float dt, const GuardianControls& controls,
                                 GuardianManager& guardians,
                                 EnemyManager& enemies) {
    for (Guardian& guardian : guardians.guardians()) {
        if (!guardian.alive) continue;
        guardian.attackCooldown = std::max(0.0f, guardian.attackCooldown - dt);
        guardian.attackRemaining = std::max(0.0f, guardian.attackRemaining - dt);
        guardian.damageRemaining = std::max(0.0f, guardian.damageRemaining - dt);
        guardian.actionRemaining = std::max(0.0f, guardian.actionRemaining - dt);
        if (guardian.type == GuardianType::Sentinel) continue;
        const bool selected = std::find(controls.selectedLanes.begin(),
                controls.selectedLanes.end(), guardian.laneId) !=
                controls.selectedLanes.end();
        if (selected && controls.jumpPressed) {
            guardian.action = GuardianAction::Jumping;
            guardian.actionRemaining = jumpDuration;
        } else if (guardian.actionRemaining <= 0.0f) {
            guardian.action = selected && controls.duckHeld
                    ? GuardianAction::Ducking : GuardianAction::Normal;
        }
    }
    for (Enemy& enemy : enemies.enemies()) {
        if (!enemy.alive) continue;
        enemy.attackCooldown = std::max(0.0f, enemy.attackCooldown - dt);
        enemy.attackRemaining = std::max(0.0f, enemy.attackRemaining - dt);
        enemy.damageRemaining = std::max(0.0f, enemy.damageRemaining - dt);
    }
    for (ImpactEffect& effect : impacts_)
        effect.remaining -= dt;
    std::erase_if(impacts_, [](const ImpactEffect& effect) {
        return effect.remaining <= 0.0f;
    });
}

void CombatSystem::moveAndEngage(float dt, GuardianManager& guardians,
                                 EnemyManager& enemies) {
    for (Guardian& guardian : guardians.guardians()) {
        guardian.blockedByEnemy = false;
        if (!guardian.alive || guardian.type == GuardianType::Sentinel) continue;
        const float half = guardianStats(guardian.type).halfWidth;
        const float nextX = std::min(Battlefield::playableRight - half,
                                     guardian.position.x + guardian.speed * dt);
        float limit = nextX;
        for (const Enemy& enemy : enemies.enemies()) {
            if (!enemy.alive || !sameLane(guardian.laneId, enemy.laneId) ||
                enemy.position.x < guardian.position.x - 1.0f) continue;
            limit = std::min(limit, enemy.position.x - enemy.halfWidth - half);
        }
        if (limit < nextX) guardian.blockedByEnemy = true;
        guardian.position.x = std::max(Battlefield::playableLeft, limit);
    }
    for (Enemy& enemy : enemies.enemies()) {
        if (!enemy.alive || enemy.type == EnemyType::TrainingDummy) continue;
        const float nextX = enemy.position.x - enemy.speed * dt;
        float limit = nextX;
        for (const Guardian& guardian : guardians.guardians()) {
            if (!guardian.alive || !sameLane(enemy.laneId, guardian.laneId))
                continue;
            const float half = guardianStats(guardian.type).halfWidth;
            if (guardian.position.x - half > enemy.position.x + enemy.halfWidth)
                continue;
            limit = std::max(limit,
                    std::min(enemy.position.x,
                             guardian.position.x + half + enemy.halfWidth));
        }
        enemy.position.x = limit;
        if (enemy.position.x - enemy.halfWidth <= Battlefield::homeBoundary)
            enemies.markHomeBreach();
    }
}

Enemy* CombatSystem::contactEnemy(const Guardian& guardian,
                                   EnemyManager& enemies) const {
    Enemy* nearest = nullptr;
    float smallestX = std::numeric_limits<float>::max();
    for (Enemy& enemy : enemies.enemies()) {
        if (!enemy.alive || !sameLane(guardian.laneId, enemy.laneId) ||
            enemy.position.x < guardian.position.x - 1.0f) continue;
        const float gap = enemy.position.x - enemy.halfWidth -
                          guardian.position.x -
                          guardianStats(guardian.type).halfWidth;
        if (gap <= 1.5f && enemy.position.x < smallestX) {
            nearest = &enemy;
            smallestX = enemy.position.x;
        }
    }
    return nearest;
}

Guardian* CombatSystem::contactGuardian(const Enemy& enemy,
                                         GuardianManager& guardians) const {
    Guardian* nearest = nullptr;
    float rightmost = -1.0f;
    for (Guardian& guardian : guardians.guardians()) {
        if (!guardian.alive || !sameLane(enemy.laneId, guardian.laneId) ||
            guardian.position.x - guardianStats(guardian.type).halfWidth >
                    enemy.position.x + enemy.halfWidth) continue;
        const float gap = enemy.position.x - enemy.halfWidth -
                          guardian.position.x -
                          guardianStats(guardian.type).halfWidth;
        if (gap <= 1.5f && guardian.position.x > rightmost) {
            nearest = &guardian;
            rightmost = guardian.position.x;
        }
    }
    return nearest;
}

void CombatSystem::launchAttacks(GuardianManager& guardians,
                                 EnemyManager& enemies) {
    for (Guardian& guardian : guardians.guardians()) {
        if (!guardian.alive || guardian.attackCooldown > 0.0f) continue;
        if (guardian.type == GuardianType::Sentinel) {
            guardian.attackCooldown = guardianStats(guardian.type).attackCooldown;
            guardian.attackRemaining = attackPoseDuration;
            continue;
        }
        if (guardian.type == GuardianType::Khopesh) {
            const AttackSpec spec = attackSpec(AttackKind::KhopeshStrike);
            guardian.attackCooldown = spec.cooldown;
            guardian.attackRemaining = attackPoseDuration;
            if (Enemy* enemy = contactEnemy(guardian, enemies)) {
                const float separation = enemy->position.x - guardian.position.x;
                if (separation <= spec.range) {
                    enemies.applyDamage(enemy->id, spec.damage);
                    addImpact(enemy->position.x,
                              battlefield_.lane(enemy->laneId).centerY,
                              AttackKind::KhopeshStrike);
                }
            }
        } else {
            const AttackSpec spec = attackSpec(AttackKind::SunArrow);
            guardian.attackCooldown = spec.cooldown;
            guardian.attackRemaining = attackPoseDuration;
            projectiles_.push_back({guardian.id, guardian.laneId,
                    spec.targetFaction, AttackKind::SunArrow,
                    guardian.position.x + guardianStats(guardian.type).halfWidth + 8.0f,
                    battlefield_.projectilePosition(guardian.laneId,
                            guardian.position.x, ProjectileLevel::Middle).y - 9.0f,
                    spec.projectileSpeed, spec.damage});
            effectEvents_.push_back({EffectEventKind::ArrowLaunch,
                                     guardian.position.x + 20.0f,
                                     battlefield_.lane(guardian.laneId).centerY});
        }
    }
    for (Enemy& enemy : enemies.enemies()) {
        if (!enemy.alive || enemy.attackCooldown > 0.0f ||
            enemy.type == EnemyType::TrainingDummy) continue;
        AttackKind kind = AttackKind::ScarabCurse;
        if (enemy.type == EnemyType::Serpent) kind = AttackKind::SandBlast;
        if (enemy.type == EnemyType::Pharaoh) kind = AttackKind::PharaohStrike;
        const AttackSpec spec = attackSpec(kind);
        enemy.attackCooldown = spec.cooldown;
        enemy.attackRemaining = attackPoseDuration;
        if (kind == AttackKind::PharaohStrike) {
            if (Guardian* target = contactGuardian(enemy, guardians)) {
                if (enemy.position.x - target->position.x <= spec.range) {
                    if (Guardian* shield = protectingSentinel(guardians.guardians(),
                            enemy.laneId, enemy.position.x, target->position.x))
                        target = shield;
                    target->takeDamage(spec.damage);
                    addImpact(target->position.x,
                              battlefield_.lane(target->laneId).centerY, kind);
                }
            }
        } else {
            const ProjectileLevel level = spec.height == AttackHeight::High
                    ? ProjectileLevel::High : ProjectileLevel::Low;
            projectiles_.push_back({enemy.id, enemy.laneId,
                    spec.targetFaction, kind,
                    enemy.position.x - enemy.halfWidth - 8.0f,
                    battlefield_.projectilePosition(enemy.laneId,
                            enemy.position.x, level).y,
                    spec.projectileSpeed, spec.damage});
            effectEvents_.push_back({EffectEventKind::CurseLaunch,
                                     enemy.position.x - 20.0f,
                                     battlefield_.lane(enemy.laneId).centerY});
        }
    }
}

void CombatSystem::moveProjectiles(float dt, GuardianManager& guardians,
                                   EnemyManager& enemies) {
    for (Projectile& projectile : projectiles_) {
        const float from = projectile.x;
        const float to = from + projectile.speed * dt;
        if (projectile.targetFaction == TargetFaction::Enemies) {
            Enemy* hit = nullptr;
            float firstX = std::numeric_limits<float>::max();
            for (Enemy& enemy : enemies.enemies()) {
                if (!enemy.alive || !sameLane(enemy.laneId, projectile.laneId) ||
                    !sweptHit(from, to, enemy.position.x, enemy.halfWidth) ||
                    enemy.position.x >= firstX) continue;
                hit = &enemy;
                firstX = enemy.position.x;
            }
            if (hit != nullptr) {
                enemies.applyDamage(hit->id, projectile.damage);
                addImpact(hit->position.x, projectile.y, projectile.kind);
                projectile.x = Battlefield::playableRight + 1000.0f;
            } else projectile.x = to;
        } else {
            Guardian* hit = nullptr;
            float rightmost = -1.0f;
            const AttackHeight height = attackSpec(projectile.kind).height;
            for (Guardian& guardian : guardians.guardians()) {
                if (!guardian.alive || !sameLane(guardian.laneId, projectile.laneId) ||
                    dodges(guardian, height) ||
                    !sweptHit(from, to, guardian.position.x,
                            guardianStats(guardian.type).halfWidth) ||
                    guardian.position.x <= rightmost) continue;
                hit = &guardian;
                rightmost = guardian.position.x;
            }
            if (hit != nullptr) {
                if (Guardian* shield = protectingSentinel(guardians.guardians(),
                        projectile.laneId, from, hit->position.x)) hit = shield;
                hit->takeDamage(projectile.damage);
                addImpact(hit->position.x, projectile.y, projectile.kind);
                projectile.x = Battlefield::homeBoundary - 1000.0f;
            } else projectile.x = to;
        }
    }
    std::erase_if(projectiles_, [](const Projectile& projectile) {
        return projectile.x > Battlefield::playableRight + 20.0f ||
               projectile.x < Battlefield::homeBoundary - 20.0f;
    });
}

void CombatSystem::processDefeatsAndCoins(GuardianManager& guardians,
                                          EnemyManager& enemies) {
    for (const EnemyDefeat& defeat : enemies.takeDefeats()) {
        if (defeat.type == EnemyType::TrainingDummy) continue;
        ++defeatedCount_;
        defeats_.push_back(defeat);
        effectEvents_.push_back({EffectEventKind::EnemyDefeat, defeat.x,
                                 battlefield_.lane(defeat.laneId).centerY});
        const glm::vec2 place = battlefield_.collectiblePosition(
                defeat.laneId, defeat.x);
        coins_.push_back({nextCoinId_++, defeat.laneId, place.x, place.y});
    }
    for (auto coin = coins_.begin(); coin != coins_.end();) {
        Guardian* collector = nullptr;
        for (Guardian& guardian : guardians.guardians()) {
            if (!guardian.alive || !sameLane(guardian.laneId, coin->laneId))
                continue;
            if (std::abs(guardian.position.x - coin->x) <=
                    guardianStats(guardian.type).halfWidth + 8.0f) {
                collector = &guardian;
                break;
            }
        }
        if (collector != nullptr) {
            effectEvents_.push_back({EffectEventKind::CoinPickup,
                                     coin->x, coin->y});
            pickups_.push_back({coin->id, collector->id});
            ++collectedCount_;
            coin = coins_.erase(coin);
        } else ++coin;
    }
}

void CombatSystem::animateAndPrune(float dt, GuardianManager& guardians,
                                   EnemyManager& enemies) {
    for (Guardian& guardian : guardians.guardians()) {
        if (!guardian.alive) {
            guardian.animator.update(dt);
            continue;
        }
        AnimationState state = guardian.type == GuardianType::Sentinel
                ? AnimationState::Idle : AnimationState::Moving;
        if (guardian.attackRemaining > 0.0f) state = AnimationState::Attacking;
        if (guardian.action == GuardianAction::Ducking) state = AnimationState::Ducking;
        if (guardian.action == GuardianAction::Jumping) state = AnimationState::Jumping;
        if (guardian.damageRemaining > 0.0f) state = AnimationState::TakingDamage;
        guardian.animator.setState(state);
        guardian.animator.update(dt);
    }
    for (Enemy& enemy : enemies.enemies()) {
        if (!enemy.alive) {
            enemy.animator.update(dt);
            continue;
        }
        AnimationState state = enemy.type == EnemyType::TrainingDummy
                ? AnimationState::Idle : AnimationState::Moving;
        enemy.behavior = EnemyBehavior::Advancing;
        if (contactGuardian(enemy, guardians) != nullptr) {
            state = AnimationState::Idle;
            enemy.behavior = EnemyBehavior::Engaged;
        }
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
    std::erase_if(guardians.guardians(), [](const Guardian& guardian) {
        return !guardian.alive && guardian.animator.complete();
    });
    std::erase_if(enemies.enemies(), [](const Enemy& enemy) {
        return !enemy.alive && enemy.animator.complete();
    });
}

void CombatSystem::update(float deltaTime, const GuardianControls& controls,
                          GuardianManager& guardians, EnemyManager& enemies) {
    if (enemies.breachedHome()) return;
    const float dt = std::clamp(deltaTime, 0.0f, 0.25f);
    advanceTimers(dt, controls, guardians, enemies);
    moveAndEngage(dt, guardians, enemies);
    if (enemies.breachedHome()) return;
    launchAttacks(guardians, enemies);
    moveProjectiles(dt, guardians, enemies);
    processDefeatsAndCoins(guardians, enemies);
    animateAndPrune(dt, guardians, enemies);
}

std::vector<CoinPickup> CombatSystem::takePickups() {
    std::vector<CoinPickup> result;
    result.swap(pickups_);
    return result;
}

std::vector<EnemyDefeat> CombatSystem::takeEnemyDefeats() {
    std::vector<EnemyDefeat> result;
    result.swap(defeats_);
    return result;
}

std::vector<EffectEvent> CombatSystem::takeEffectEvents() {
    std::vector<EffectEvent> result;
    result.swap(effectEvents_);
    return result;
}

void CombatSystem::reset() {
    projectiles_.clear();
    impacts_.clear();
    coins_.clear();
    pickups_.clear();
    defeats_.clear();
    effectEvents_.clear();
    nextCoinId_ = 1;
    defeatedCount_ = 0;
    collectedCount_ = 0;
}
