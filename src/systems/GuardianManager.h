#pragma once

#include <cstdint>
#include <vector>

#include "entities/Guardian.h"

class Battlefield;

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

// Owns guardian instances and placement. CombatSystem updates their gameplay.
class GuardianManager {
public:
    explicit GuardianManager(const Battlefield& battlefield);

    PlacementResult tryPlace(GuardianType type, int laneId,
                             GuardianWallet& wallet);
    void clear();

    [[nodiscard]] const std::vector<Guardian>& guardians() const { return guardians_; }
    [[nodiscard]] std::vector<Guardian>& guardians() { return guardians_; }

private:
    const Battlefield& battlefield_;
    std::vector<Guardian> guardians_;
    std::uint64_t nextId_ = 1;
};
