#include <cassert>

#include "combat/CombatSystem.h"
#include "economy/EconomyManager.h"
#include "economy/PurchaseSystem.h"
#include "world/Battlefield.h"

int main() {
    Battlefield field;
    GuardianManager guardians(field);
    EconomyManager economy;
    PurchaseSystem purchases;
    int notifications = 0;
    EconomySnapshot latest;
    economy.setOnChange([&](const EconomySnapshot& state) {
        ++notifications;
        latest = state;
    });
    assert(notifications == 1 && latest.balance == 30);
    assert(economy.balance() == 30 && economy.pickupCount() == 0);
    assert(economy.cardState(GuardianType::Khopesh).available);
    assert(!economy.cardState(GuardianType::Archer).eligible);
    assert(!economy.cardState(GuardianType::Sentinel).eligible);
    assert(economy.cardState(GuardianType::Archer).affordable);

    assert(purchases.tryPurchase(GuardianType::Khopesh, 0, false,
           guardians, economy) == PurchaseResult::NotPlaying);
    assert(purchases.tryPurchase(GuardianType::Khopesh, -1, true,
           guardians, economy) == PurchaseResult::InvalidLane);
    assert(purchases.tryPurchase(GuardianType::Archer, 0, true,
           guardians, economy) == PurchaseResult::Locked);
    assert(economy.balance() == 30 && guardians.guardians().empty());

    for (int lane = 0; lane < 3; ++lane)
        assert(purchases.tryPurchase(GuardianType::Khopesh, lane, true,
               guardians, economy) == PurchaseResult::Purchased);
    assert(economy.balance() == 0 && guardians.guardians().size() == 3);
    assert(!economy.cardState(GuardianType::Khopesh).available);
    assert(purchases.tryPurchase(GuardianType::Khopesh, 3, true,
           guardians, economy) == PurchaseResult::InsufficientCoins);
    assert(guardians.guardians().size() == 3);

    assert(economy.creditPickup({1, 1}));
    assert(economy.balance() == 10 && economy.pickupCount() == 1);
    const int beforeDuplicate = notifications;
    assert(!economy.creditPickup({1, 1}));
    assert(notifications == beforeDuplicate && economy.balance() == 10);
    assert(!economy.cardState(GuardianType::Archer).eligible);
    assert(economy.creditPickup({2, 2}));
    assert(economy.balance() == 20 && economy.collectedValue() == 20);
    assert(economy.cardState(GuardianType::Archer).available);
    assert(!economy.cardState(GuardianType::Sentinel).eligible);
    assert(purchases.tryPurchase(GuardianType::Archer, 3, true,
           guardians, economy) == PurchaseResult::Purchased);
    assert(economy.balance() == 0 && guardians.guardians().size() == 4);
    assert(economy.cardState(GuardianType::Archer).eligible);
    assert(!economy.cardState(GuardianType::Archer).available);

    assert(economy.creditPickup({3, 3}));
    assert(economy.cardState(GuardianType::Sentinel).eligible);
    assert(!economy.cardState(GuardianType::Sentinel).available);
    assert(economy.creditPickup({4, 4}));
    assert(economy.creditPickup({5, 5}));
    assert(economy.balance() == 30);
    assert(economy.cardState(GuardianType::Sentinel).available);
    const auto guardianCount = guardians.guardians().size();
    assert(purchases.tryPurchase(GuardianType::Sentinel, 4, true,
           guardians, economy) == PurchaseResult::InvalidLane);
    assert(purchases.tryPurchase(GuardianType::Sentinel, 0, false,
           guardians, economy) == PurchaseResult::NotPlaying);
    assert(economy.balance() == 30 &&
           guardians.guardians().size() == guardianCount);
    assert(purchases.tryPurchase(GuardianType::Sentinel, 0, true,
           guardians, economy) == PurchaseResult::Purchased);
    assert(economy.balance() == 0);
    assert(!economy.cardState(GuardianType::Sentinel).available);
    assert(latest.balance == 0);

    economy.reset();
    assert(economy.balance() == 30 && economy.pickupCount() == 0);
    assert(economy.cardState(GuardianType::Khopesh).available);
    assert(!economy.cardState(GuardianType::Archer).eligible);
    assert(economy.creditPickup({1, 1}));

    // Feed a real combat pickup through the economy once.
    GuardianManager collectors(field);
    EnemyManager enemies(field);
    CombatSystem combat(field);
    EconomyManager earned;
    assert(purchases.tryPurchase(GuardianType::Khopesh, 2, true,
           collectors, earned) == PurchaseResult::Purchased);
    collectors.guardians()[0].position.x = 185.0f;
    enemies.spawn(EnemyType::Scarab, 2);
    enemies.enemies()[0].position.x = 220.0f;
    enemies.applyDamage(enemies.enemies()[0].id, 4);
    for (int i = 0; i < 12; ++i) {
        combat.update(0.1f, {}, collectors, enemies);
        for (const CoinPickup& pickup : combat.takePickups())
            earned.creditPickup(pickup);
    }
    assert(combat.defeatedCount() == 1);
    assert(earned.balance() == 30 && earned.pickupCount() == 1);
    assert(earned.collectedValue() == 10);
    return 0;
}
