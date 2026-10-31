#include <cassert>

#include "combat/CombatSystem.h"
#include "core/SessionReset.h"
#include "economy/EconomyManager.h"
#include "effects/EffectManager.h"
#include "systems/EnemyManager.h"
#include "systems/GuardianManager.h"
#include "systems/WaveManager.h"
#include "world/Battlefield.h"

int main() {
    Battlefield battlefield;
    GuardianManager guardians(battlefield);
    EnemyManager enemies(battlefield);
    CombatSystem combat(battlefield);
    WaveManager waves;
    EconomyManager economy;
    EffectManager effects;

    battlefield.toggleSelectionAt({200.0f, 200.0f});
    assert(guardians.tryPlace(GuardianType::Khopesh, 0, economy) ==
           PlacementResult::Placed);
    assert(economy.balance() == 20);
    assert(economy.creditPickup({42, 1}));
    waves.start(enemies);
    effects.emit({EffectEventKind::CurseImpact, 300.0f, 250.0f});
    assert(!enemies.enemies().empty());
    assert(!effects.particles().empty());

    SessionReset::run(guardians, enemies, combat, waves, economy, effects);
    battlefield.clearSelections();
    assert(guardians.guardians().empty());
    assert(enemies.enemies().empty());
    assert(!enemies.breachedHome());
    assert(combat.projectiles().empty());
    assert(combat.coins().empty());
    assert(combat.takeEffectEvents().empty());
    assert(waves.phase() == WavePhase::Ready);
    assert(waves.waveNumber() == 1);
    assert(economy.balance() == 30);
    assert(economy.pickupCount() == 0);
    assert(economy.cardState(GuardianType::Khopesh).available);
    assert(!economy.cardState(GuardianType::Archer).available);
    assert(!economy.cardState(GuardianType::Sentinel).available);
    assert(effects.particles().empty());
    assert(battlefield.selectedLanes().empty());
}
