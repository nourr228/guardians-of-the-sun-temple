#include "systems/WaveManager.h"

#include <algorithm>
#include <array>
#include <vector>

#include "systems/EnemyManager.h"

namespace {
std::vector<EnemyType> populationFor(int waveIndex) {
    std::vector<EnemyType> population;
    const std::array<std::array<int, 3>, 3> counts{{
            {{3, 0, 0}}, {{3, 2, 0}}, {{4, 3, 3}}}};
    for (int type = 0; type < 3; ++type)
        population.insert(population.end(), counts[waveIndex][type],
                          static_cast<EnemyType>(type));
    return population;
}
}

WaveManager::WaveManager(std::uint32_t seed) : spawner_(seed) {}

void WaveManager::beginWave(EnemyManager& enemies) {
    spawner_.configure(populationFor(waveIndex_),
                       waveIndex_ == 0 ? 2.4f : 2.0f);
    spawner_.spawnFirst(enemies);
    phase_ = spawner_.complete() ? WavePhase::Clearing : WavePhase::Spawning;
}

void WaveManager::start(EnemyManager& enemies) {
    if (phase_ == WavePhase::Ready) beginWave(enemies);
}

void WaveManager::update(float deltaTime, EnemyManager& enemies) {
    if (phase_ == WavePhase::Ready || phase_ == WavePhase::Victory ||
        phase_ == WavePhase::Defeat) return;
    if (enemies.breachedHome()) {
        phase_ = WavePhase::Defeat;
        return;
    }
    if (phase_ == WavePhase::Spawning) {
        spawner_.update(deltaTime, enemies);
        if (spawner_.complete()) phase_ = WavePhase::Clearing;
    }
    if (phase_ == WavePhase::Clearing && enemies.livingCount() == 0) {
        if (waveIndex_ == 2) {
            phase_ = WavePhase::Victory;
        } else {
            ++waveIndex_;
            beginWave(enemies);
        }
    }
}

void WaveManager::markDefeat() {
    if (phase_ != WavePhase::Victory) phase_ = WavePhase::Defeat;
}

void WaveManager::reset(std::uint32_t seed) {
    spawner_.reset(seed);
    phase_ = WavePhase::Ready;
    waveIndex_ = 0;
}
