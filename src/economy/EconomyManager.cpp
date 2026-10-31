#include "economy/EconomyManager.h"

#include <utility>

#include "combat/CombatSystem.h"

CardState EconomyManager::cardState(GuardianType type) const {
    int requiredCollection = 0;
    switch (type) {
    case GuardianType::Khopesh: requiredCollection = 0; break;
    case GuardianType::Archer: requiredCollection = 2 * rewardPerPickup; break;
    case GuardianType::Sentinel: requiredCollection = 3 * rewardPerPickup; break;
    }
    const bool eligible = collectedValue_ >= requiredCollection;
    const bool affordable = balance_ >= guardianStats(type).cost;
    return {eligible, affordable, eligible && affordable};
}

EconomySnapshot EconomyManager::snapshot() const {
    EconomySnapshot state;
    state.balance = balance_;
    state.collectedValue = collectedValue_;
    state.pickupCount = pickupCount_;
    state.cards[0] = cardState(GuardianType::Khopesh);
    state.cards[1] = cardState(GuardianType::Sentinel);
    state.cards[2] = cardState(GuardianType::Archer);
    return state;
}

bool EconomyManager::spend(int amount) {
    if (amount < 0 || amount > balance_) return false;
    balance_ -= amount;
    notifyChanged();
    return true;
}

bool EconomyManager::creditPickup(const CoinPickup& pickup) {
    if (!processedCoins_.insert(pickup.coinId).second) return false;
    balance_ += rewardPerPickup;
    collectedValue_ += rewardPerPickup;
    ++pickupCount_;
    notifyChanged();
    return true;
}

void EconomyManager::reset() {
    balance_ = startingCoins;
    collectedValue_ = 0;
    pickupCount_ = 0;
    processedCoins_.clear();
    notifyChanged();
}

void EconomyManager::setOnChange(
        std::function<void(const EconomySnapshot&)> listener) {
    onChange_ = std::move(listener);
    notifyChanged();
}

void EconomyManager::notifyChanged() const {
    if (onChange_) onChange_(snapshot());
}
