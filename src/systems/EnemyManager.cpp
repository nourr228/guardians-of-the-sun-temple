#include "systems/EnemyManager.h"

#include <algorithm>
#include <stdexcept>

#include "world/Battlefield.h"

EnemyManager::EnemyManager(const Battlefield& battlefield)
        : battlefield_(battlefield) {}

std::uint64_t EnemyManager::spawn(EnemyType type, int laneId) {
    if (laneId < 0 || laneId >= Battlefield::laneCount)
        throw std::out_of_range("Invalid enemy lane");
    const std::uint64_t id = nextId_++;
    enemies_.emplace_back(id, type, laneId, battlefield_.enemyEntry(laneId));
    return id;
}

void EnemyManager::applyDamage(std::uint64_t id, int damage) {
    for (Enemy& enemy : enemies_) {
        if (enemy.id != id) continue;
        if (enemy.takeDamage(damage))
            defeats_.push_back({enemy.id, enemy.type, enemy.laneId,
                                enemy.position.x});
        return;
    }
}

std::vector<EnemyDefeat> EnemyManager::takeDefeats() {
    std::vector<EnemyDefeat> result;
    result.swap(defeats_);
    return result;
}

int EnemyManager::livingCount() const {
    return static_cast<int>(std::count_if(enemies_.begin(), enemies_.end(),
            [](const Enemy& enemy) { return enemy.alive &&
                    enemy.type != EnemyType::TrainingDummy; }));
}

void EnemyManager::clear() {
    enemies_.clear();
    defeats_.clear();
    nextId_ = 1;
    breachedHome_ = false;
}
