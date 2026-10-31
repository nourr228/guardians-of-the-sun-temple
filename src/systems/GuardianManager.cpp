#include "systems/GuardianManager.h"

#include "world/Battlefield.h"

GuardianManager::GuardianManager(const Battlefield& battlefield)
        : battlefield_(battlefield) {}

PlacementResult GuardianManager::tryPlace(GuardianType type, int laneId,
                                          GuardianWallet& wallet) {
    if (laneId < 0 || laneId >= Battlefield::laneCount)
        return PlacementResult::InvalidLane;
    const GuardianStats stats = guardianStats(type);
    if (wallet.balance() < stats.cost)
        return PlacementResult::InsufficientCoins;
    const glm::vec2 entry = type == GuardianType::Sentinel
            ? battlefield_.defenderAnchor(laneId)
            : battlefield_.guardianEntry(laneId);
    guardians_.emplace_back(nextId_, type, laneId, entry);
    if (!wallet.spend(stats.cost)) {
        guardians_.pop_back();
        return PlacementResult::InsufficientCoins;
    }
    ++nextId_;
    return PlacementResult::Placed;
}

void GuardianManager::clear() {
    guardians_.clear();
    nextId_ = 1;
}
