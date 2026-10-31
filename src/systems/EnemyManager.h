#pragma once

#include <cstdint>
#include <vector>

#include "entities/Enemy.h"

class Battlefield;

struct EnemyDefeat {
    std::uint64_t id;
    EnemyType type;
    int laneId;
    float x;
};

// Owns enemy instances and one-shot defeat events. CombatSystem drives them.
class EnemyManager {
public:
    explicit EnemyManager(const Battlefield& battlefield);

    std::uint64_t spawn(EnemyType type, int laneId);
    void applyDamage(std::uint64_t id, int damage);
    [[nodiscard]] std::vector<EnemyDefeat> takeDefeats();
    [[nodiscard]] int livingCount() const;
    [[nodiscard]] bool breachedHome() const { return breachedHome_; }
    void markHomeBreach() { breachedHome_ = true; }
    void clear();

    [[nodiscard]] const std::vector<Enemy>& enemies() const { return enemies_; }
    [[nodiscard]] std::vector<Enemy>& enemies() { return enemies_; }

private:
    const Battlefield& battlefield_;
    std::vector<Enemy> enemies_;
    std::vector<EnemyDefeat> defeats_;
    std::uint64_t nextId_ = 1;
    bool breachedHome_ = false;
};
