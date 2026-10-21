#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "entities/Guardian.h"

class Battlefield;

struct EnemyTarget {
    std::uint64_t id;
    int laneId;
    float x;
    float halfWidth;
};

class EnemyTargets {
public:
    virtual ~EnemyTargets() = default;
    [[nodiscard]] virtual std::vector<EnemyTarget> targets() const = 0;
    virtual void applyDamage(std::uint64_t id, int damage) = 0;
};

class GuardianWallet {
public:
    virtual ~GuardianWallet() = default;
    [[nodiscard]] virtual int balance() const = 0;
    virtual bool spend(int amount) = 0;
};

enum class PlacementResult { Placed, InvalidLane, InsufficientCoins };

struct GuardianControls {
    std::vector<int> selectedLanes;
    bool jumpPressed = false;
    bool duckHeld = false;
};

struct SunArrow {
    std::uint64_t ownerId;
    int laneId;
    float x;
    float y;
    float speed = 290.0f;
    int damage = 1;
};

// Gameplay state only. The enemy and wallet ports are supplied by later modules.
class GuardianManager {
public:
    explicit GuardianManager(const Battlefield& battlefield);

    PlacementResult tryPlace(GuardianType type, int laneId,
                             GuardianWallet& wallet);
    void update(float deltaTime, const GuardianControls& controls,
                EnemyTargets& enemies);
    void clear();

    [[nodiscard]] const std::vector<Guardian>& guardians() const { return guardians_; }
    [[nodiscard]] std::vector<Guardian>& guardians() { return guardians_; }
    [[nodiscard]] const std::vector<SunArrow>& arrows() const { return arrows_; }

private:
    [[nodiscard]] std::optional<EnemyTarget> nearestContact(
            const Guardian& guardian, float nextX,
            const std::vector<EnemyTarget>& targets) const;
    void updateGuardian(Guardian& guardian, float deltaTime,
                        const GuardianControls& controls,
                        EnemyTargets& enemies);
    void updateArrows(float deltaTime, EnemyTargets& enemies);

    const Battlefield& battlefield_;
    std::vector<Guardian> guardians_;
    std::vector<SunArrow> arrows_;
    std::uint64_t nextId_ = 1;
};
