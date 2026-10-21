#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "entities/Enemy.h"
#include "systems/EnemyManager.h"
#include "systems/GuardianManager.h"
#include "systems/WaveManager.h"
#include "world/Battlefield.h"

namespace {
struct TestWallet final : GuardianWallet {
    int coins = 30;
    int balance() const override { return coins; }
    bool spend(int amount) override {
        if (amount > coins) return false;
        coins -= amount;
        return true;
    }
};

void eliminateAll(EnemyManager& enemies) {
    for (const EnemyTarget& target : enemies.targets())
        enemies.applyDamage(target.id, 100);
}

int livingType(const EnemyManager& enemies, EnemyType type) {
    int count = 0;
    for (const Enemy& enemy : enemies.enemies())
        if (enemy.alive && enemy.type == type) ++count;
    return count;
}
}

int main() {
    Battlefield field;
    assert(enemyStats(EnemyType::Scarab).maxHealth == 5);
    assert(enemyStats(EnemyType::Serpent).damage == 1);
    assert(enemyStats(EnemyType::Pharaoh).maxHealth == 7);
    assert(enemyStats(EnemyType::Pharaoh).damage == 2);
    assert(enemyStats(EnemyType::Pharaoh).speed >
           enemyStats(EnemyType::Scarab).speed);

    EnemyManager enemies(field);
    bool rejectedLane = false;
    try { enemies.spawn(EnemyType::Scarab, 4); }
    catch (const std::out_of_range&) { rejectedLane = true; }
    assert(rejectedLane);
    const auto scarabId = enemies.spawn(EnemyType::Scarab, 2);
    assert(enemies.enemies()[0].laneId == 2);
    assert(enemies.enemies()[0].position.x == Battlefield::playableRight);
    std::vector<Guardian> guardians;
    enemies.update(0.25f, guardians);
    assert(enemies.enemies()[0].position.x < Battlefield::playableRight);
    assert(enemies.enemies()[0].laneId == 2);
    assert(!enemies.projectiles().empty());
    assert(enemies.projectiles()[0].level == ProjectileLevel::High);
    enemies.applyDamage(scarabId, 4);
    assert(enemies.enemies()[0].health == 1);
    assert(enemies.enemies()[0].healthRatio() < 1.0f);
    enemies.applyDamage(scarabId, 1);
    assert(enemies.livingCount() == 0);
    assert(enemies.takeDefeats().size() == 1);
    enemies.applyDamage(scarabId, 1);
    assert(enemies.takeDefeats().empty());

    enemies.clear();
    guardians.emplace_back(1, GuardianType::Khopesh, 0,
                           glm::vec2{920.0f, field.lane(0).centerY});
    guardians[0].action = GuardianAction::Ducking;
    enemies.spawn(EnemyType::Scarab, 0);
    enemies.update(0.1f, guardians);
    assert(guardians[0].health == 5);
    assert(!enemies.projectiles().empty());
    enemies.clear();
    guardians[0].action = GuardianAction::Normal;
    enemies.spawn(EnemyType::Scarab, 0);
    enemies.update(0.1f, guardians);
    assert(guardians[0].health == 4);

    enemies.clear();
    guardians[0].health = 5;
    guardians[0].action = GuardianAction::Jumping;
    enemies.spawn(EnemyType::Serpent, 0);
    enemies.update(0.1f, guardians);
    assert(guardians[0].health == 5);
    assert(enemies.projectiles()[0].level == ProjectileLevel::Low);
    enemies.clear();
    guardians[0].action = GuardianAction::Normal;
    enemies.spawn(EnemyType::Serpent, 0);
    enemies.update(0.1f, guardians);
    assert(guardians[0].health == 4);

    enemies.clear();
    guardians[0].health = 5;
    guardians[0].position.x = 940.0f;
    enemies.spawn(EnemyType::Pharaoh, 0);
    enemies.update(0.1f, guardians);
    assert(guardians[0].health == 3);
    assert(enemies.enemies()[0].behavior == EnemyBehavior::Attacking);
    const float contactX = enemies.enemies()[0].position.x;
    enemies.update(0.1f, guardians);
    assert(enemies.enemies()[0].position.x == contactX);
    guardians[0].takeDamage(100);
    enemies.update(0.1f, guardians);
    assert(enemies.enemies()[0].position.x < contactX);

    enemies.clear();
    guardians[0].alive = true;
    guardians[0].health = 5;
    guardians[0].position.x = 963.0f;
    enemies.spawn(EnemyType::Scarab, 0);
    enemies.update(0.1f, guardians);
    assert(enemies.enemies()[0].position.x <= Battlefield::playableRight);

    enemies.clear();
    guardians.clear();
    GuardianManager archerManager(field);
    TestWallet wallet;
    assert(archerManager.tryPlace(GuardianType::Archer, 0, wallet) ==
           PlacementResult::Placed);
    const auto arrowTargetId = enemies.spawn(EnemyType::Scarab, 0);
    enemies.enemies()[0].position.x = 500.0f;
    for (int i = 0; i < 35; ++i) {
        archerManager.update(0.1f, {}, enemies);
        enemies.update(0.1f, archerManager.guardians());
    }
    assert(enemies.enemies()[0].id == arrowTargetId);
    assert(enemies.enemies()[0].health < 5);

    enemies.clear();
    guardians.clear();
    enemies.spawn(EnemyType::Scarab, 1);
    for (int i = 0; i < 120 && !enemies.breachedHome(); ++i)
        enemies.update(0.25f, guardians);
    assert(enemies.breachedHome());

    EnemyManager waveEnemies(field);
    WaveManager waves(1234);
    assert(waves.phase() == WavePhase::Ready);
    waves.start(waveEnemies);
    assert(waves.waveNumber() == 1);
    assert(waves.spawnedCount() == 1);
    assert(livingType(waveEnemies, EnemyType::Scarab) == 1);
    waves.update(5.0f, waveEnemies);
    assert(waves.phase() == WavePhase::Clearing);
    assert(waves.spawnedCount() == 3);
    assert(livingType(waveEnemies, EnemyType::Scarab) == 3);
    const auto firstTargets = waveEnemies.targets();
    waveEnemies.applyDamage(firstTargets[0].id, 100);
    waveEnemies.applyDamage(firstTargets[1].id, 100);
    waves.update(10.0f, waveEnemies);
    assert(waves.waveNumber() == 1 && waves.phase() == WavePhase::Clearing);
    waveEnemies.applyDamage(firstTargets[2].id, 100);
    waves.update(0.0f, waveEnemies);
    assert(waves.waveNumber() == 2 && waves.spawnedCount() == 1);
    waves.update(9.0f, waveEnemies);
    assert(waves.phase() == WavePhase::Clearing);
    assert(waves.spawnedCount() == 5);
    assert(livingType(waveEnemies, EnemyType::Scarab) == 3);
    assert(livingType(waveEnemies, EnemyType::Serpent) == 2);
    eliminateAll(waveEnemies);
    waves.update(0.0f, waveEnemies);
    assert(waves.waveNumber() == 3 && waves.spawnedCount() == 1);
    waves.update(20.0f, waveEnemies);
    assert(waves.phase() == WavePhase::Clearing);
    assert(waves.spawnedCount() == 10);
    assert(livingType(waveEnemies, EnemyType::Scarab) == 4);
    assert(livingType(waveEnemies, EnemyType::Serpent) == 3);
    assert(livingType(waveEnemies, EnemyType::Pharaoh) == 3);
    for (const EnemyTarget& target : waveEnemies.targets())
        assert(target.laneId >= 0 && target.laneId < Battlefield::laneCount);
    eliminateAll(waveEnemies);
    waves.update(0.0f, waveEnemies);
    assert(waves.phase() == WavePhase::Victory);
    waveEnemies.clear();
    waves.reset(1234);
    assert(waves.phase() == WavePhase::Ready);
    waves.start(waveEnemies);
    EnemyManager copyEnemies(field);
    WaveManager copyWaves(1234);
    copyWaves.start(copyEnemies);
    waves.update(5.0f, waveEnemies);
    copyWaves.update(5.0f, copyEnemies);
    assert(copyEnemies.enemies().size() == waveEnemies.enemies().size());
    for (std::size_t i = 0; i < copyEnemies.enemies().size(); ++i) {
        assert(copyEnemies.enemies()[i].laneId == waveEnemies.enemies()[i].laneId);
        assert(copyEnemies.enemies()[i].type == waveEnemies.enemies()[i].type);
    }
    waves.markDefeat();
    assert(waves.phase() == WavePhase::Defeat);
    return 0;
}
