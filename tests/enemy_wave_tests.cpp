#include <cassert>
#include <stdexcept>

#include "combat/CombatSystem.h"
#include "systems/WaveManager.h"
#include "world/Battlefield.h"

namespace {
void eliminateAll(EnemyManager& enemies) {
    for (const Enemy& enemy : enemies.enemies())
        if (enemy.alive) enemies.applyDamage(enemy.id, 100);
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
    assert(enemyStats(EnemyType::Pharaoh).speed > enemyStats(EnemyType::Scarab).speed);

    EnemyManager enemies(field);
    bool rejectedLane = false;
    try { enemies.spawn(EnemyType::Scarab, 4); }
    catch (const std::out_of_range&) { rejectedLane = true; }
    assert(rejectedLane);
    const auto id = enemies.spawn(EnemyType::Scarab, 2);
    GuardianManager guardians(field);
    CombatSystem combat(field);
    combat.update(0.25f, {}, guardians, enemies);
    assert(enemies.enemies()[0].laneId == 2);
    assert(enemies.enemies()[0].position.x < Battlefield::playableRight);
    assert(!combat.projectiles().empty());
    assert(combat.projectiles()[0].kind == AttackKind::ScarabCurse);
    enemies.applyDamage(id, 5);
    assert(enemies.livingCount() == 0);
    assert(enemies.takeDefeats().size() == 1);
    enemies.applyDamage(id, 5);
    assert(enemies.takeDefeats().empty());

    EnemyManager waveEnemies(field);
    WaveManager waves(1234);
    assert(waves.phase() == WavePhase::Ready);
    waves.start(waveEnemies);
    assert(waves.spawnedCount() == 1);
    waves.update(5.0f, waveEnemies);
    assert(waves.phase() == WavePhase::Clearing);
    assert(waves.spawnedCount() == 3);
    assert(livingType(waveEnemies, EnemyType::Scarab) == 3);
    const auto first = waveEnemies.enemies()[0].id;
    const auto second = waveEnemies.enemies()[1].id;
    waveEnemies.applyDamage(first, 100);
    waveEnemies.applyDamage(second, 100);
    waves.update(10.0f, waveEnemies);
    assert(waves.waveNumber() == 1 && waves.phase() == WavePhase::Clearing);
    waveEnemies.applyDamage(waveEnemies.enemies()[2].id, 100);
    waves.update(0.0f, waveEnemies);
    assert(waves.waveNumber() == 2 && waves.spawnedCount() == 1);
    waves.update(9.0f, waveEnemies);
    assert(waves.spawnedCount() == 5);
    assert(livingType(waveEnemies, EnemyType::Scarab) == 3);
    assert(livingType(waveEnemies, EnemyType::Serpent) == 2);
    eliminateAll(waveEnemies);
    waves.update(0.0f, waveEnemies);
    assert(waves.waveNumber() == 3 && waves.spawnedCount() == 1);
    waves.update(20.0f, waveEnemies);
    assert(waves.spawnedCount() == 10);
    assert(livingType(waveEnemies, EnemyType::Scarab) == 4);
    assert(livingType(waveEnemies, EnemyType::Serpent) == 3);
    assert(livingType(waveEnemies, EnemyType::Pharaoh) == 3);
    for (const Enemy& enemy : waveEnemies.enemies())
        assert(enemy.laneId >= 0 && enemy.laneId < Battlefield::laneCount);
    eliminateAll(waveEnemies);
    waves.update(0.0f, waveEnemies);
    assert(waves.phase() == WavePhase::Victory);

    waveEnemies.clear();
    waves.reset(1234);
    waves.start(waveEnemies);
    EnemyManager copyEnemies(field);
    WaveManager copyWaves(1234);
    copyWaves.start(copyEnemies);
    waves.update(5.0f, waveEnemies);
    copyWaves.update(5.0f, copyEnemies);
    for (std::size_t i = 0; i < waveEnemies.enemies().size(); ++i) {
        assert(waveEnemies.enemies()[i].type == copyEnemies.enemies()[i].type);
        assert(waveEnemies.enemies()[i].laneId == copyEnemies.enemies()[i].laneId);
    }
    waves.markDefeat();
    assert(waves.phase() == WavePhase::Defeat);
    return 0;
}
