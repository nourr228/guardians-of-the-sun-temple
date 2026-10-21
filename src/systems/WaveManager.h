#pragma once

#include <cstdint>

#include "systems/EnemySpawner.h"

class EnemyManager;

enum class WavePhase { Ready, Spawning, Clearing, Victory, Defeat };

class WaveManager {
public:
    explicit WaveManager(std::uint32_t seed = 0x5A17u);

    void start(EnemyManager& enemies);
    void update(float deltaTime, EnemyManager& enemies);
    void markDefeat();
    void reset(std::uint32_t seed = 0x5A17u);

    [[nodiscard]] WavePhase phase() const { return phase_; }
    [[nodiscard]] int waveNumber() const { return waveIndex_ + 1; }
    [[nodiscard]] int spawnedCount() const { return spawner_.spawnedCount(); }
    [[nodiscard]] int waveTotal() const { return spawner_.totalCount(); }

private:
    void beginWave(EnemyManager& enemies);

    EnemySpawner spawner_;
    WavePhase phase_ = WavePhase::Ready;
    int waveIndex_ = 0;
};
