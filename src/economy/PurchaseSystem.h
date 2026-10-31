#pragma once

#include "entities/Guardian.h"

class EconomyManager;
class GuardianManager;

enum class PurchaseResult {
    Purchased,
    NotPlaying,
    InvalidLane,
    Locked,
    InsufficientCoins
};

// Transaction boundary: validation occurs before GuardianManager spends.
class PurchaseSystem {
public:
    [[nodiscard]] PurchaseResult tryPurchase(GuardianType type, int laneId,
            bool playing,
            GuardianManager& guardians, EconomyManager& economy) const;
};
