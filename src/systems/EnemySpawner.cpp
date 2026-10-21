#include "systems/EnemySpawner.h"

#include <algorithm>
#include <stdexcept>

#include "systems/EnemyManager.h"
#include "world/Battlefield.h"

EnemySpawner::EnemySpawner(std::uint32_t seed) : random_(seed) {}

void EnemySpawner::reset(std::uint32_t seed) {
    random_.seed(seed);
    population_.clear();
    next_ = 0;
    untilNext_ = 0.0f;
}

void EnemySpawner::configure(std::vector<EnemyType> population,
                              float intervalSeconds) {
    if (population.empty() || intervalSeconds <= 0.0f)
        throw std::invalid_argument("Invalid enemy wave configuration");
    population_ = std::move(population);
    std::shuffle(population_.begin(), population_.end(), random_);
    next_ = 0;
    intervalSeconds_ = intervalSeconds;
    untilNext_ = 0.0f;
}

void EnemySpawner::spawnNext(EnemyManager& enemies) {
    if (complete()) return;
    std::uniform_int_distribution<int> lanes(0, Battlefield::laneCount - 1);
    enemies.spawn(population_[next_++], lanes(random_));
}

void EnemySpawner::spawnFirst(EnemyManager& enemies) {
    if (next_ != 0) return;
    spawnNext(enemies);
    untilNext_ = intervalSeconds_;
}

void EnemySpawner::update(float deltaTime, EnemyManager& enemies) {
    if (complete() || next_ == 0) return;
    untilNext_ -= std::max(0.0f, deltaTime);
    while (!complete() && untilNext_ <= 0.0f) {
        spawnNext(enemies);
        untilNext_ += intervalSeconds_;
    }
}
