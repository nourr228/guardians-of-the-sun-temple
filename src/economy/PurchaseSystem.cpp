#include "economy/PurchaseSystem.h"

#include "economy/EconomyManager.h"
#include "systems/GuardianManager.h"
#include "world/Battlefield.h"

PurchaseResult PurchaseSystem::tryPurchase(GuardianType type, int laneId,
        bool playing, GuardianManager& guardians,
        EconomyManager& economy) const {
    if (!playing) return PurchaseResult::NotPlaying;
    if (laneId < 0 || laneId >= Battlefield::laneCount)
        return PurchaseResult::InvalidLane;
    const CardState card = economy.cardState(type);
    if (!card.eligible) return PurchaseResult::Locked;
    if (!card.affordable) return PurchaseResult::InsufficientCoins;
    const PlacementResult result = guardians.tryPlace(type, laneId, economy);
    if (result == PlacementResult::Placed) return PurchaseResult::Purchased;
    if (result == PlacementResult::InvalidLane) return PurchaseResult::InvalidLane;
    return PurchaseResult::InsufficientCoins;
}
