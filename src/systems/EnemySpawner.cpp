#include "systems/EnemySpawner.h"

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <utility>

#include "systems/EnemyManager.h"
#include "world/Battlefield.h"

EnemySpawner::EnemySpawner(std::uint32_t seed) : random_(seed) {}

void EnemySpawner::reset(std::uint32_t seed) {
    random_.seed(seed);
    population_.clear();
    lanePlan_.clear();
    next_ = 0;
    untilNext_ = 0.0f;
}

void EnemySpawner::configure(std::vector<EnemyType> population,
                              float intervalSeconds,
                              std::vector<int> eligibleLanes) {
    if (population.empty() || intervalSeconds <= 0.0f)
        throw std::invalid_argument("Invalid enemy wave configuration");
    if (eligibleLanes.empty()) {
        for (int lane = 0; lane < Battlefield::laneCount; ++lane)
            eligibleLanes.push_back(lane);
    }
    std::sort(eligibleLanes.begin(), eligibleLanes.end());
    eligibleLanes.erase(std::unique(eligibleLanes.begin(), eligibleLanes.end()),
                        eligibleLanes.end());
    if (eligibleLanes.front() < 0 ||
        eligibleLanes.back() >= Battlefield::laneCount)
        throw std::invalid_argument("Invalid eligible enemy lane");
    population_ = std::move(population);
    std::shuffle(population_.begin(), population_.end(), random_);
    lanePlan_.clear();
    lanePlan_.reserve(population_.size());
    std::array<int, Battlefield::laneCount> laneCounts{};
    std::array<std::array<int, Battlefield::laneCount>, 3> typeCounts{};
    for (const EnemyType type : population_) {
        std::array<int, 3> best{
                std::numeric_limits<int>::max(),
                std::numeric_limits<int>::max(),
                std::numeric_limits<int>::max()};
        std::vector<int> choices;
        for (const int lane : eligibleLanes) {
            const int total = laneCounts[lane];
            const int sameType = typeCounts[static_cast<int>(type)][lane];
            const int repeat = !lanePlan_.empty() && lanePlan_.back() == lane;
            const std::array<int, 3> score = type == EnemyType::Pharaoh
                    ? std::array<int, 3>{sameType, total, repeat}
                    : std::array<int, 3>{total, repeat, sameType};
            if (score < best) {
                best = score;
                choices.clear();
            }
            if (score == best)
                choices.push_back(lane);
        }
        std::uniform_int_distribution<std::size_t> pick(0, choices.size() - 1);
        const int lane = choices[pick(random_)];
        lanePlan_.push_back(lane);
        ++laneCounts[lane];
        ++typeCounts[static_cast<int>(type)][lane];
    }
    next_ = 0;
    intervalSeconds_ = intervalSeconds;
    untilNext_ = 0.0f;
}

void EnemySpawner::spawnNext(EnemyManager& enemies) {
    if (complete()) return;
    enemies.spawn(population_[next_], lanePlan_[next_]);
    ++next_;
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
