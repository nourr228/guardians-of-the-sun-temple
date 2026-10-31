#include <algorithm>
#include <cassert>
#include <cmath>

#include "combat/CombatSystem.h"
#include "world/Battlefield.h"

namespace {
struct Wallet final : GuardianWallet {
    int coins = 999;
    int balance() const override { return coins; }
    bool spend(int amount) override {
        if (amount > coins) return false;
        coins -= amount;
        return true;
    }
};

int rangedHit(EnemyType type, GuardianAction action, int guardianLane = 0,
              int enemyLane = 0) {
    Battlefield field;
    GuardianManager guardians(field);
    EnemyManager enemies(field);
    CombatSystem combat(field);
    Wallet wallet;
    assert(guardians.tryPlace(GuardianType::Khopesh, guardianLane, wallet) ==
           PlacementResult::Placed);
    guardians.guardians()[0].position.x = 920.0f;
    guardians.guardians()[0].action = action;
    enemies.spawn(type, enemyLane);
    GuardianControls controls;
    if (action == GuardianAction::Jumping) {
        controls.selectedLanes = {guardianLane};
        controls.jumpPressed = true;
    }
    if (action == GuardianAction::Ducking) {
        controls.selectedLanes = {guardianLane};
        controls.duckHeld = true;
    }
    combat.update(0.1f, controls, guardians, enemies);
    return guardians.guardians()[0].health;
}
}

int main() {
    assert(attackSpec(AttackKind::KhopeshStrike).damage == 1);
    assert(attackSpec(AttackKind::SunArrow).damage == 1);
    assert(attackSpec(AttackKind::SunArrow).targetFaction == TargetFaction::Enemies);
    assert(attackSpec(AttackKind::ScarabCurse).height == AttackHeight::High);
    assert(attackSpec(AttackKind::SandBlast).height == AttackHeight::Low);
    assert(attackSpec(AttackKind::SandBlast).targetFaction == TargetFaction::Guardians);
    assert(attackSpec(AttackKind::PharaohStrike).damage == 2);
    assert(rangedHit(EnemyType::Scarab, GuardianAction::Normal) == 4);
    assert(rangedHit(EnemyType::Scarab, GuardianAction::Jumping) == 4);
    assert(rangedHit(EnemyType::Scarab, GuardianAction::Ducking) == 5);
    assert(rangedHit(EnemyType::Serpent, GuardianAction::Normal) == 4);
    assert(rangedHit(EnemyType::Serpent, GuardianAction::Jumping) == 5);
    assert(rangedHit(EnemyType::Serpent, GuardianAction::Ducking) == 4);
    assert(rangedHit(EnemyType::Scarab, GuardianAction::Normal, 0, 1) == 5);

    {
        Battlefield sweepField;
        Wallet sweepWallet;
        GuardianManager sweepGuardians(sweepField);
        EnemyManager sweepEnemies(sweepField);
        CombatSystem sweepCombat(sweepField);
        assert(sweepGuardians.tryPlace(GuardianType::Khopesh, 0,
                                       sweepWallet) == PlacementResult::Placed);
        sweepGuardians.guardians()[0].position.x = 920.0f;
        sweepGuardians.guardians()[0].speed = 0.0f;
        sweepEnemies.spawn(EnemyType::Scarab, 0);
        sweepCombat.update(0.25f, {}, sweepGuardians, sweepEnemies);
        assert(sweepGuardians.guardians()[0].health == 4);
        assert(sweepCombat.projectiles().empty());
    }

    Battlefield field;
    Wallet wallet;
    GuardianManager guardians(field);
    EnemyManager enemies(field);
    CombatSystem combat(field);
    assert(guardians.tryPlace(GuardianType::Khopesh, 0, wallet) ==
           PlacementResult::Placed);
    assert(guardians.tryPlace(GuardianType::Sentinel, 0, wallet) ==
           PlacementResult::Placed);
    const float sentinelX = guardians.guardians()[1].position.x;
    assert(CombatSystem::protectingSentinel(guardians.guardians(), 0,
            300.0f, guardians.guardians()[0].position.x) ==
            &guardians.guardians()[1]);
    assert(CombatSystem::protectingSentinel(guardians.guardians(), 1,
            300.0f, guardians.guardians()[0].position.x) == nullptr);
    enemies.spawn(EnemyType::Scarab, 0);
    enemies.enemies()[0].position.x = 300.0f;
    combat.update(0.1f, {}, guardians, enemies);
    assert(guardians.guardians()[0].health == 5);
    assert(guardians.guardians()[1].health == 6);
    assert(guardians.guardians()[1].position.x == sentinelX);
    assert(combat.projectiles().empty());
    assert(!combat.impacts().empty());
    combat.update(0.25f, {}, guardians, enemies);
    assert(combat.impacts().empty());

    guardians.guardians()[0].position.x = 250.0f;
    guardians.guardians()[0].speed = 0.0f;
    enemies.clear();
    combat.reset();
    enemies.spawn(EnemyType::Scarab, 0);
    enemies.enemies()[0].position.x = 300.0f;
    combat.update(0.25f, {}, guardians, enemies);
    assert(guardians.guardians()[0].health == 4);
    assert(guardians.guardians()[1].health == 6);
    assert(CombatSystem::protectingSentinel(guardians.guardians(), 0,
            300.0f, guardians.guardians()[0].position.x) == nullptr);

    guardians.guardians()[0].position.x = 174.0f;
    guardians.guardians()[0].speed = 0.0f;
    guardians.guardians()[1].takeDamage(6);
    assert(!guardians.guardians()[1].alive);
    assert(CombatSystem::protectingSentinel(guardians.guardians(), 0,
            300.0f, 174.0f) == nullptr);
    enemies.clear();
    combat.reset();
    enemies.spawn(EnemyType::Scarab, 0);
    enemies.enemies()[0].position.x = 300.0f;
    for (int i = 0; i < 8; ++i)
        combat.update(0.25f, {}, guardians, enemies);
    assert(guardians.guardians()[0].health < 4);

    guardians.clear();
    enemies.clear();
    combat.reset();
    assert(guardians.tryPlace(GuardianType::Khopesh, 0, wallet) ==
           PlacementResult::Placed);
    assert(guardians.tryPlace(GuardianType::Khopesh, 0, wallet) ==
           PlacementResult::Placed);
    guardians.guardians()[0].position.x = 920.0f;
    guardians.guardians()[1].position.x = 890.0f;
    guardians.guardians()[0].speed = 0.0f;
    guardians.guardians()[1].speed = 0.0f;
    enemies.spawn(EnemyType::Scarab, 0);
    combat.update(0.1f, {}, guardians, enemies);
    assert(guardians.guardians()[0].health == 4);
    assert(guardians.guardians()[1].health == 5);
    assert(combat.projectiles().empty());

    guardians.clear();
    enemies.clear();
    combat.reset();
    assert(guardians.tryPlace(GuardianType::Archer, 1, wallet) ==
           PlacementResult::Placed);
    assert(guardians.tryPlace(GuardianType::Khopesh, 1, wallet) ==
           PlacementResult::Placed);
    guardians.guardians()[0].position.x = 300.0f;
    guardians.guardians()[1].position.x = 380.0f;
    guardians.guardians()[0].speed = 0.0f;
    guardians.guardians()[1].speed = 0.0f;
    enemies.spawn(EnemyType::TrainingDummy, 1);
    enemies.enemies()[0].position.x = 500.0f;
    for (int i = 0; i < 12; ++i)
        combat.update(0.1f, {}, guardians, enemies);
    assert(enemies.enemies()[0].health < 5);
    assert(guardians.guardians()[1].health == 5);

    guardians.clear();
    enemies.clear();
    combat.reset();
    assert(guardians.tryPlace(GuardianType::Khopesh, 2, wallet) ==
           PlacementResult::Placed);
    guardians.guardians()[0].position.x = 185.0f;
    enemies.spawn(EnemyType::Scarab, 2);
    enemies.enemies()[0].position.x = 220.0f;
    enemies.applyDamage(enemies.enemies()[0].id, 4);
    for (int i = 0; i < 12; ++i)
        combat.update(0.1f, {}, guardians, enemies);
    assert(combat.defeatedCount() == 1);
    assert(combat.collectedCount() == 1);
    assert(combat.coins().empty());
    assert(combat.takePickups().size() == 1);
    assert(combat.takePickups().empty());
    assert(combat.takeEnemyDefeats().size() == 1);
    assert(combat.takeEnemyDefeats().empty());
    const auto effects = combat.takeEffectEvents();
    assert(std::count_if(effects.begin(), effects.end(), [](const EffectEvent& e) {
        return e.kind == EffectEventKind::CoinPickup;
    }) == 1);
    assert(std::count_if(effects.begin(), effects.end(), [](const EffectEvent& e) {
        return e.kind == EffectEventKind::EnemyDefeat;
    }) == 1);
    assert(combat.takeEffectEvents().empty());

    guardians.clear();
    enemies.clear();
    combat.reset();
    enemies.spawn(EnemyType::Scarab, 3);
    enemies.enemies()[0].position.x = Battlefield::homeBoundary +
                                      enemies.enemies()[0].halfWidth;
    combat.update(0.0f, {}, guardians, enemies);
    assert(enemies.breachedHome());

    guardians.clear();
    enemies.clear();
    combat.reset();
    assert(guardians.tryPlace(GuardianType::Khopesh, 0, wallet) ==
           PlacementResult::Placed);
    guardians.guardians()[0].position.x = 940.0f;
    enemies.spawn(EnemyType::Pharaoh, 0);
    combat.update(0.1f, {}, guardians, enemies);
    assert(guardians.guardians()[0].health == 3);

    for (EnemyType type : {EnemyType::Scarab, EnemyType::Serpent,
                           EnemyType::Pharaoh}) {
        guardians.clear();
        enemies.clear();
        combat.reset();
        assert(guardians.tryPlace(GuardianType::Khopesh, 0, wallet) ==
               PlacementResult::Placed);
        guardians.guardians()[0].speed = 0.0f;
        const float x = guardians.guardians()[0].position.x;
        enemies.spawn(type, 0);
        enemies.enemies()[0].position.x = x +
                guardianStats(GuardianType::Khopesh).halfWidth +
                enemies.enemies()[0].halfWidth;
        enemies.enemies()[0].speed = 0.0f;
        combat.update(0.01f, {}, guardians, enemies);
        assert(enemies.enemies()[0].health == enemyStats(type).maxHealth - 1);
    }

    guardians.clear();
    enemies.clear();
    combat.reset();
    assert(guardians.tryPlace(GuardianType::Sentinel, 0, wallet) ==
           PlacementResult::Placed);
    enemies.spawn(EnemyType::Pharaoh, 0);
    enemies.enemies()[0].position.x = guardians.guardians()[0].position.x +
            guardianStats(GuardianType::Sentinel).halfWidth +
            enemies.enemies()[0].halfWidth;
    enemies.enemies()[0].speed = 0.0f;
    combat.update(0.01f, {}, guardians, enemies);
    assert(guardians.guardians()[0].health == 5);
    return 0;
}
