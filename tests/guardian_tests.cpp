#include <cassert>
#include <cmath>

#include "combat/CombatSystem.h"
#include "world/Battlefield.h"

struct TestWallet final : GuardianWallet {
    int coins = 30;
    bool refuse = false;
    int balance() const override { return coins; }
    bool spend(int amount) override {
        if (refuse || amount > coins) return false;
        coins -= amount;
        return true;
    }
};

int main() {
    Battlefield field;
    GuardianManager guardians(field);
    EnemyManager enemies(field);
    CombatSystem combat(field);
    TestWallet wallet;
    assert(guardians.tryPlace(GuardianType::Khopesh, -1, wallet) ==
           PlacementResult::InvalidLane);
    wallet.refuse = true;
    assert(guardians.tryPlace(GuardianType::Khopesh, 0, wallet) ==
           PlacementResult::InsufficientCoins);
    assert(wallet.coins == 30 && guardians.guardians().empty());
    wallet.refuse = false;
    assert(guardians.tryPlace(GuardianType::Khopesh, 0, wallet) ==
           PlacementResult::Placed);
    assert(guardians.tryPlace(GuardianType::Khopesh, 0, wallet) ==
           PlacementResult::Placed);
    assert(wallet.coins == 10 && guardians.guardians().size() == 2);
    assert(guardians.guardians()[0].id != guardians.guardians()[1].id);
    assert(guardians.tryPlace(GuardianType::Archer, 0, wallet) ==
           PlacementResult::InsufficientCoins);
    assert(wallet.coins == 10);

    const float start = guardians.guardians()[0].position.x;
    combat.update(0.2f, {}, guardians, enemies);
    assert(std::abs(guardians.guardians()[0].position.x - (start + 9.0f)) < 0.01f);
    assert(guardians.guardians()[0].position.x == guardians.guardians()[1].position.x);
    combat.update(0.01f, {{0}, true, false}, guardians, enemies);
    assert(guardians.guardians()[0].action == GuardianAction::Jumping);
    combat.update(0.25f, {{0}, false, true}, guardians, enemies);
    assert(guardians.guardians()[0].action == GuardianAction::Jumping);
    for (int i = 0; i < 3; ++i)
        combat.update(0.25f, {{0}, false, true}, guardians, enemies);
    assert(guardians.guardians()[0].action == GuardianAction::Ducking);

    guardians.guardians()[0].takeDamage(2);
    assert(guardians.guardians()[0].health == 3);
    assert(std::abs(guardians.guardians()[0].healthRatio() - 0.6f) < 0.001f);
    guardians.clear();
    combat.reset();
    wallet.coins = 30;
    assert(guardians.tryPlace(GuardianType::Sentinel, 1, wallet) ==
           PlacementResult::Placed);
    const float sentinelX = guardians.guardians()[0].position.x;
    assert(guardians.guardians()[0].health == 7);
    combat.update(0.25f, {{1}, true, true}, guardians, enemies);
    assert(guardians.guardians()[0].position.x == sentinelX);
    assert(guardians.guardians()[0].action == GuardianAction::Normal);

    guardians.clear();
    enemies.clear();
    combat.reset();
    wallet.coins = 30;
    assert(guardians.tryPlace(GuardianType::Khopesh, 2, wallet) ==
           PlacementResult::Placed);
    const auto id = enemies.spawn(EnemyType::TrainingDummy, 2);
    enemies.enemies()[0].position.x = 230.0f;
    enemies.applyDamage(id, 4);
    float stoppedX = 0.0f;
    for (int i = 0; i < 15; ++i) {
        combat.update(0.2f, {}, guardians, enemies);
        if (guardians.guardians()[0].blockedByEnemy)
            stoppedX = guardians.guardians()[0].position.x;
        if (!enemies.enemies()[0].alive) break;
    }
    assert(stoppedX > 0.0f && !enemies.enemies()[0].alive);
    combat.update(0.2f, {}, guardians, enemies);
    assert(guardians.guardians()[0].position.x > stoppedX);

    GuardianManager coarse(field), fine(field);
    EnemyManager emptyA(field), emptyB(field);
    CombatSystem coarseCombat(field), fineCombat(field);
    TestWallet a, b;
    assert(coarse.tryPlace(GuardianType::Khopesh, 0, a) == PlacementResult::Placed);
    assert(fine.tryPlace(GuardianType::Khopesh, 0, b) == PlacementResult::Placed);
    for (int i = 0; i < 4; ++i) coarseCombat.update(0.25f, {}, coarse, emptyA);
    for (int i = 0; i < 100; ++i) fineCombat.update(0.01f, {}, fine, emptyB);
    assert(std::abs(coarse.guardians()[0].position.x -
                    fine.guardians()[0].position.x) < 0.01f);
    return 0;
}
