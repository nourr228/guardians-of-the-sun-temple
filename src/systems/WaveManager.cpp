#include "systems/WaveManager.h"

#include <algorithm>
#include <array>
#include <utility>
#include <vector>

#include "systems/EnemyManager.h"
#include "systems/GuardianManager.h"

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

void WaveManager::beginWave(EnemyManager& enemies,
                            std::vector<int> eligibleLanes) {
    spawner_.configure(populationFor(waveIndex_),
                       waveIndex_ == 0 ? 2.4f : 2.0f,
                       std::move(eligibleLanes));
    spawner_.spawnFirst(enemies);
    phase_ = spawner_.complete() ? WavePhase::Clearing : WavePhase::Spawning;
}

void WaveManager::start(EnemyManager& enemies,
                        const GuardianManager* guardians) {
    if (phase_ != WavePhase::Ready) return;
    std::vector<int> defendedLanes;
    if (guardians != nullptr) {
        for (const Guardian& guardian : guardians->guardians()) {
            if (guardian.alive &&
                std::find(defendedLanes.begin(), defendedLanes.end(),
                          guardian.laneId) == defendedLanes.end())
                defendedLanes.push_back(guardian.laneId);
        }
    }
    // The opening wave teaches combat in prepared lanes; later waves use all four.
    beginWave(enemies, defendedLanes);
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
