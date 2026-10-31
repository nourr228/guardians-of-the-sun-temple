#pragma once

class GuardianManager;
class EnemyManager;
class CombatSystem;
class WaveManager;
class EconomyManager;
class EffectManager;

// The single reset path for a playable session's simulation state.
class SessionReset {
public:
    static void run(GuardianManager& guardians, EnemyManager& enemies,
                    CombatSystem& combat, WaveManager& waves,
                    EconomyManager& economy, EffectManager& effects);
};
