#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <unordered_set>

#include "entities/Guardian.h"
#include "systems/GuardianManager.h"

struct CoinPickup;

struct CardState {
    bool eligible = false;
    bool affordable = false;
    bool available = false;
};

struct EconomySnapshot {
    int balance = 30;
    int collectedValue = 0;
    int pickupCount = 0;
    std::array<CardState, 3> cards{};
};

// Spendable currency and progression eligibility are separate by design.
class EconomyManager final : public GuardianWallet {
public:
    static constexpr int startingCoins = 30;
    static constexpr int rewardPerPickup = 10;

    [[nodiscard]] int balance() const override { return balance_; }
    [[nodiscard]] int collectedValue() const { return collectedValue_; }
    [[nodiscard]] int pickupCount() const { return pickupCount_; }
    [[nodiscard]] CardState cardState(GuardianType type) const;
    [[nodiscard]] EconomySnapshot snapshot() const;

    bool spend(int amount) override;
    bool creditPickup(const CoinPickup& pickup);
    void reset();
    void setOnChange(std::function<void(const EconomySnapshot&)> listener);

private:
    void notifyChanged() const;

    int balance_ = startingCoins;
    int collectedValue_ = 0;
    int pickupCount_ = 0;
    std::unordered_set<std::uint64_t> processedCoins_;
    std::function<void(const EconomySnapshot&)> onChange_;
};
