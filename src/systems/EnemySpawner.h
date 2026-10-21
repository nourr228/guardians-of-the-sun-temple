#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "entities/Enemy.h"

class EnemyManager;

class EnemySpawner {
public:
    explicit EnemySpawner(std::uint32_t seed = 0x5A17u);

    void reset(std::uint32_t seed);
    void configure(std::vector<EnemyType> population, float intervalSeconds);
    void spawnFirst(EnemyManager& enemies);
    void update(float deltaTime, EnemyManager& enemies);

    [[nodiscard]] int spawnedCount() const { return next_; }
    [[nodiscard]] int totalCount() const {
        return static_cast<int>(population_.size());
    }
    [[nodiscard]] bool complete() const { return next_ == totalCount(); }

private:
    void spawnNext(EnemyManager& enemies);

    std::mt19937 random_;
    std::vector<EnemyType> population_;
    int next_ = 0;
    float intervalSeconds_ = 2.2f;
    float untilNext_ = 0.0f;
};
