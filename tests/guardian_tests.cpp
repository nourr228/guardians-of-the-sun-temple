#include <cassert>
#include <cmath>
#include <cstdint>
#include <vector>

#include "systems/GuardianManager.h"
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

struct TestEnemies final : EnemyTargets {
    struct Target { std::uint64_t id; int lane; float x; int health; };
    std::vector<Target> active;
    std::vector<EnemyTarget> targets() const override {
        std::vector<EnemyTarget> result;
        for (const auto& target : active)
            if (target.health > 0)
                result.push_back({target.id, target.lane, target.x, 18.0f});
        return result;
    }
    void applyDamage(std::uint64_t id, int damage) override {
        for (auto& target : active)
            if (target.id == id) target.health -= damage;
    }
};

int main() {
    Battlefield field;
    TestWallet wallet;
    TestEnemies enemies;
    GuardianManager manager(field);
    assert(manager.tryPlace(GuardianType::Khopesh, -1, wallet) ==
           PlacementResult::InvalidLane);
    assert(wallet.coins == 30 && manager.guardians().empty());
    wallet.refuse = true;
    assert(manager.tryPlace(GuardianType::Khopesh, 0, wallet) ==
           PlacementResult::InsufficientCoins);
    assert(wallet.coins == 30 && manager.guardians().empty());
    wallet.refuse = false;
    assert(manager.tryPlace(GuardianType::Khopesh, 0, wallet) ==
           PlacementResult::Placed);
    assert(manager.tryPlace(GuardianType::Khopesh, 0, wallet) ==
           PlacementResult::Placed);
    assert(manager.guardians().size() == 2 && wallet.coins == 10);
    assert(manager.guardians()[0].id != manager.guardians()[1].id);
    assert(manager.guardians()[0].health == 5);
    assert(manager.tryPlace(GuardianType::Archer, 1, wallet) ==
           PlacementResult::InsufficientCoins);
    assert(wallet.coins == 10 && manager.guardians().size() == 2);

    const float startX = manager.guardians()[0].position.x;
    manager.update(0.1f, {}, enemies);
    manager.update(0.1f, {}, enemies);
    assert(std::abs(manager.guardians()[0].position.x -
                    (startX + 9.0f)) < 0.01f);
    assert(manager.guardians()[0].position.x == manager.guardians()[1].position.x);
    GuardianManager coarse(field);
    GuardianManager fine(field);
    TestWallet coarseWallet;
    TestWallet fineWallet;
    assert(coarse.tryPlace(GuardianType::Khopesh, 0, coarseWallet) ==
           PlacementResult::Placed);
    assert(fine.tryPlace(GuardianType::Khopesh, 0, fineWallet) ==
           PlacementResult::Placed);
    for (int i = 0; i < 4; ++i) coarse.update(0.25f, {}, enemies);
    for (int i = 0; i < 100; ++i) fine.update(0.01f, {}, enemies);
    assert(std::abs(coarse.guardians()[0].position.x -
                    fine.guardians()[0].position.x) < 0.01f);
    GuardianControls selected{{0}, true, false};
    manager.update(0.016f, selected, enemies);
    assert(manager.guardians()[0].action == GuardianAction::Jumping);
    selected.jumpPressed = false;
    selected.duckHeld = true;
    manager.update(0.25f, selected, enemies);
    assert(manager.guardians()[0].action == GuardianAction::Jumping);
    for (int i = 0; i < 3; ++i) manager.update(0.25f, selected, enemies);
    assert(manager.guardians()[0].action == GuardianAction::Ducking);
    manager.guardians()[0].takeDamage(2);
    assert(manager.guardians()[0].health == 3);
    assert(std::abs(manager.guardians()[0].healthRatio() - 0.6f) < 0.001f);
    manager.guardians()[0].takeDamage(3);
    assert(!manager.guardians()[0].alive && !manager.guardians()[0].active);

    manager.clear();
    wallet.coins = 30;
    assert(manager.tryPlace(GuardianType::Sentinel, 1, wallet) ==
           PlacementResult::Placed);
    auto& sentinel = manager.guardians()[0];
    assert(sentinel.health == 7);
    const float sentinelX = sentinel.position.x;
    manager.update(0.25f, {{1}, true, true}, enemies);
    assert(sentinel.position.x == sentinelX);
    assert(sentinel.action == GuardianAction::Normal);

    manager.clear();
    wallet.coins = 30;
    assert(manager.tryPlace(GuardianType::Khopesh, 2, wallet) ==
           PlacementResult::Placed);
    enemies.active.push_back({1, 2, 230.0f, 1});
    float stoppedX = 0;
    for (int i = 0; i < 8; ++i) {
        manager.update(0.2f, {}, enemies);
        if (manager.guardians()[0].blockedByEnemy)
            stoppedX = manager.guardians()[0].position.x;
        if (enemies.active[0].health <= 0) break;
    }
    assert(stoppedX > 0 && enemies.active[0].health <= 0);
    manager.update(0.2f, {}, enemies);
    assert(manager.guardians()[0].position.x > stoppedX);

    manager.clear();
    wallet.coins = 30;
    enemies.active.clear();
    assert(manager.tryPlace(GuardianType::Archer, 3, wallet) ==
           PlacementResult::Placed);
    assert(manager.tryPlace(GuardianType::Khopesh, 3, wallet) ==
           PlacementResult::Placed);
    const int friendlyHealth = manager.guardians()[1].health;
    enemies.active.push_back({2, 3, 420.0f, 5});
    for (int i = 0; i < 32; ++i) manager.update(0.2f, {}, enemies);
    assert(enemies.active[0].health < 5);
    assert(manager.guardians()[1].health == friendlyHealth);

    return 0;
}
