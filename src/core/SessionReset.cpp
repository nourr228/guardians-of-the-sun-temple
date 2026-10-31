#include "core/SessionReset.h"

#include "combat/CombatSystem.h"
#include "economy/EconomyManager.h"
#include "effects/EffectManager.h"
#include "systems/EnemyManager.h"
#include "systems/GuardianManager.h"
#include "systems/WaveManager.h"

void SessionReset::run(GuardianManager& guardians, EnemyManager& enemies,
                       CombatSystem& combat, WaveManager& waves,
                       EconomyManager& economy, EffectManager& effects) {
    guardians.clear();
    enemies.clear();
    combat.reset();
    waves.reset();
    economy.reset();
    effects.reset();
}
