#include "systems/GuardianManager.h"

#include <algorithm>
#include <cmath>

#include "world/Battlefield.h"

namespace {
constexpr float jumpDuration = 0.7f;
constexpr float attackPoseDuration = 0.60f;
constexpr float arrowHitRadius = 11.0f;
}

GuardianManager::GuardianManager(const Battlefield& battlefield)
        : battlefield_(battlefield) {}

PlacementResult GuardianManager::tryPlace(GuardianType type, int laneId,
                                           GuardianWallet& wallet) {
    if (laneId < 0 || laneId >= Battlefield::laneCount)
        return PlacementResult::InvalidLane;
    const GuardianStats stats = guardianStats(type);
    if (wallet.balance() < stats.cost)
        return PlacementResult::InsufficientCoins;
    const glm::vec2 entry = type == GuardianType::Sentinel
            ? battlefield_.defenderAnchor(laneId)
            : battlefield_.guardianEntry(laneId);
    guardians_.emplace_back(nextId_, type, laneId, entry);
    if (!wallet.spend(stats.cost)) {
        guardians_.pop_back();
        return PlacementResult::InsufficientCoins;
    }
    ++nextId_;
    return PlacementResult::Placed;
}

std::optional<EnemyTarget> GuardianManager::nearestContact(
        const Guardian& guardian, float nextX,
        const std::vector<EnemyTarget>& targets) const {
    std::optional<EnemyTarget> contact;
    float smallestX = Battlefield::playableRight + 100.0f;
    for (const EnemyTarget& target : targets) {
        if (target.laneId != guardian.laneId) continue;
        const float gap = target.x - target.halfWidth -
                          guardianStats(guardian.type).halfWidth;
        if (gap <= nextX && target.x >= guardian.position.x - 1.0f &&
            target.x < smallestX) {
            smallestX = target.x;
            contact = target;
        }
    }
    return contact;
}

void GuardianManager::update(float deltaTime,
                              const GuardianControls& controls,
                              EnemyTargets& enemies) {
    const float dt = std::clamp(deltaTime, 0.0f, 0.25f);
    for (Guardian& guardian : guardians_)
        updateGuardian(guardian, dt, controls, enemies);
    updateArrows(dt, enemies);
}

void GuardianManager::updateGuardian(Guardian& guardian, float dt,
                                      const GuardianControls& controls,
                                      EnemyTargets& enemies) {
    if (!guardian.alive) {
        guardian.animator.update(dt);
        return;
    }
    const GuardianStats stats = guardianStats(guardian.type);
    guardian.attackCooldown = std::max(0.0f, guardian.attackCooldown - dt);
    guardian.attackRemaining = std::max(0.0f, guardian.attackRemaining - dt);
    guardian.damageRemaining = std::max(0.0f, guardian.damageRemaining - dt);
    guardian.actionRemaining = std::max(0.0f, guardian.actionRemaining - dt);

    const bool selected = std::find(controls.selectedLanes.begin(),
                                    controls.selectedLanes.end(),
                                    guardian.laneId) != controls.selectedLanes.end();
    if (guardian.type != GuardianType::Sentinel) {
        if (selected && controls.jumpPressed) {
            guardian.action = GuardianAction::Jumping;
            guardian.actionRemaining = jumpDuration;
        } else if (guardian.actionRemaining <= 0.0f) {
            guardian.action = selected && controls.duckHeld
                    ? GuardianAction::Ducking : GuardianAction::Normal;
        }
    }

    guardian.blockedByEnemy = false;
    std::optional<EnemyTarget> contact;
    if (guardian.type == GuardianType::Khopesh) {
        const float nextX = std::min(Battlefield::playableRight - stats.halfWidth,
                                     guardian.position.x + guardian.speed * dt);
        contact = nearestContact(guardian, nextX, enemies.targets());
        if (contact) {
            guardian.blockedByEnemy = true;
            guardian.position.x = std::min(nextX,
                    contact->x - contact->halfWidth - stats.halfWidth);
        } else {
            guardian.position.x = nextX;
        }
    } else if (guardian.type == GuardianType::Archer) {
        const float nextX = std::min(Battlefield::playableRight - stats.halfWidth,
                                     guardian.position.x + guardian.speed * dt);
        contact = nearestContact(guardian, nextX, enemies.targets());
        if (contact) {
            guardian.blockedByEnemy = true;
            guardian.position.x = std::min(nextX,
                    contact->x - contact->halfWidth - stats.halfWidth);
        } else {
            guardian.position.x = nextX;
        }
    }

    if (guardian.attackCooldown <= 0.0f) {
        guardian.attackCooldown = stats.attackCooldown;
        guardian.attackRemaining = attackPoseDuration;
        if (guardian.type == GuardianType::Khopesh && contact)
            enemies.applyDamage(contact->id, 1);
        if (guardian.type == GuardianType::Archer)
            arrows_.push_back({guardian.id, guardian.laneId,
                               guardian.position.x + stats.halfWidth + 8.0f,
                               battlefield_.projectilePosition(
                                       guardian.laneId, guardian.position.x,
                                       ProjectileLevel::Middle).y - 9.0f});
    }

    AnimationState state = AnimationState::Moving;
    if (guardian.type == GuardianType::Sentinel) state = AnimationState::Idle;
    if (guardian.attackRemaining > 0.0f) state = AnimationState::Attacking;
    if (guardian.action == GuardianAction::Ducking) state = AnimationState::Ducking;
    if (guardian.action == GuardianAction::Jumping) state = AnimationState::Jumping;
    if (guardian.damageRemaining > 0.0f) state = AnimationState::TakingDamage;
    guardian.animator.setState(state);
    guardian.animator.update(dt);
}

void GuardianManager::updateArrows(float dt, EnemyTargets& enemies) {
    for (SunArrow& arrow : arrows_) {
        const float previousX = arrow.x;
        arrow.x += arrow.speed * dt;
        std::optional<EnemyTarget> firstHit;
        float firstX = Battlefield::playableRight + 100.0f;
        for (const EnemyTarget& target : enemies.targets()) {
            if (target.laneId != arrow.laneId ||
                target.x + target.halfWidth + arrowHitRadius < previousX ||
                target.x - target.halfWidth - arrowHitRadius > arrow.x ||
                target.x >= firstX) continue;
            firstHit = target;
            firstX = target.x;
        }
        if (firstHit) {
            enemies.applyDamage(firstHit->id, arrow.damage);
            arrow.x = Battlefield::playableRight + 1000.0f;
        }
    }
    std::erase_if(arrows_, [](const SunArrow& arrow) {
        return arrow.x > Battlefield::playableRight + 20.0f;
    });
}

void GuardianManager::clear() {
    guardians_.clear();
    arrows_.clear();
    nextId_ = 1;
}
