#pragma once

#include <cstdint>
#include <vector>

#include "effects/EffectEvent.h"
#include "systems/EnemyManager.h"
#include "systems/GuardianManager.h"

class Battlefield;

enum class AttackKind {
    KhopeshStrike, SunArrow, ScarabCurse, SandBlast, PharaohStrike
};
enum class AttackHeight { Middle, High, Low };
enum class TargetFaction { Guardians, Enemies };

struct AttackSpec {
    int damage;
    float range;
    float cooldown;
    AttackHeight height;
    float projectileSpeed;
    TargetFaction targetFaction;
};

[[nodiscard]] AttackSpec attackSpec(AttackKind kind);

struct Projectile {
    std::uint64_t sourceId;
    int laneId;
    TargetFaction targetFaction;
    AttackKind kind;
    float x;
    float y;
    float speed;
    int damage;
};

struct ImpactEffect {
    float x;
    float y;
    AttackKind kind;
    float remaining;
};

struct SunCoin {
    std::uint64_t id;
    int laneId;
    float x;
    float y;
};

struct CoinPickup {
    std::uint64_t coinId;
    std::uint64_t guardianId;
};

// The only live gameplay path for contact, attacks, damage and pickups.
class CombatSystem {
public:
    explicit CombatSystem(const Battlefield& battlefield);

    void update(float deltaTime, const GuardianControls& controls,
                GuardianManager& guardians, EnemyManager& enemies);
    void reset();

    [[nodiscard]] const std::vector<Projectile>& projectiles() const {
        return projectiles_;
    }
    [[nodiscard]] const std::vector<ImpactEffect>& impacts() const {
        return impacts_;
    }
    [[nodiscard]] const std::vector<SunCoin>& coins() const { return coins_; }
    [[nodiscard]] int defeatedCount() const { return defeatedCount_; }
    [[nodiscard]] int collectedCount() const { return collectedCount_; }
    [[nodiscard]] std::vector<CoinPickup> takePickups();
    [[nodiscard]] std::vector<EnemyDefeat> takeEnemyDefeats();
    [[nodiscard]] std::vector<EffectEvent> takeEffectEvents();

    // Attacks arrive from the right. Returns the rightmost living shield
    // between the source and an otherwise eligible guardian.
    [[nodiscard]] static Guardian* protectingSentinel(
            std::vector<Guardian>& guardians, int laneId,
            float attackerX, float targetX);

private:
    void advanceTimers(float dt, const GuardianControls& controls,
                       GuardianManager& guardians, EnemyManager& enemies);
    void moveAndEngage(float dt, GuardianManager& guardians,
                       EnemyManager& enemies);
    void launchAttacks(GuardianManager& guardians, EnemyManager& enemies);
    void moveProjectiles(float dt, GuardianManager& guardians,
                         EnemyManager& enemies);
    void processDefeatsAndCoins(GuardianManager& guardians,
                                EnemyManager& enemies);
    void animateAndPrune(float dt, GuardianManager& guardians,
                         EnemyManager& enemies);
    void addImpact(float x, float y, AttackKind kind);
    [[nodiscard]] Enemy* contactEnemy(const Guardian& guardian,
                                       EnemyManager& enemies) const;
    [[nodiscard]] Guardian* contactGuardian(const Enemy& enemy,
                                             GuardianManager& guardians) const;

    const Battlefield& battlefield_;
    std::vector<Projectile> projectiles_;
    std::vector<ImpactEffect> impacts_;
    std::vector<SunCoin> coins_;
    std::vector<CoinPickup> pickups_;
    std::vector<EnemyDefeat> defeats_;
    std::vector<EffectEvent> effectEvents_;
    std::uint64_t nextCoinId_ = 1;
    int defeatedCount_ = 0;
    int collectedCount_ = 0;
};
