#include "world/Battlefield.h"

#include <algorithm>
#include <stdexcept>

Battlefield::Battlefield() {
    for (int id = 0; id < laneCount; ++id) {
        const float top = laneTop + id * laneHeight;
        lanes_[id] = Lane{id, playableLeft, playableRight, top,
                          top + laneHeight, top + laneHeight * 0.5f};
    }
}

const Lane& Battlefield::lane(int id) const {
    if (id < 0 || id >= laneCount) throw std::out_of_range("Invalid lane ID");
    return lanes_[id];
}

std::optional<int> Battlefield::laneAt(glm::vec2 worldPosition) const {
    for (const Lane& candidate : lanes_) {
        if (candidate.contains(worldPosition)) return candidate.id;
    }
    return std::nullopt;
}

void Battlefield::updateHover(glm::vec2 worldPosition) {
    const auto hovered = laneAt(worldPosition);
    for (Lane& candidate : lanes_) candidate.hovered = hovered == candidate.id;
}

void Battlefield::toggleSelectionAt(glm::vec2 worldPosition) {
    const auto target = laneAt(worldPosition);
    if (target) lanes_[*target].selected = !lanes_[*target].selected;
}

void Battlefield::clearSelections() {
    for (Lane& candidate : lanes_) candidate.selected = false;
}

std::vector<int> Battlefield::selectedLanes() const {
    std::vector<int> result;
    for (const Lane& candidate : lanes_) {
        if (candidate.selected) result.push_back(candidate.id);
    }
    return result;
}

glm::vec2 Battlefield::guardianEntry(int laneId) const {
    return {playableLeft + 16.0f, lane(laneId).centerY};
}

glm::vec2 Battlefield::defenderAnchor(int laneId) const {
    // Leaves a short staging area behind the stationary defender.
    return {playableLeft + 65.0f, lane(laneId).centerY};
}

glm::vec2 Battlefield::enemyEntry(int laneId) const {
    return {playableRight, lane(laneId).centerY};
}

glm::vec2 Battlefield::projectilePosition(int laneId, float x,
                                          ProjectileLevel level) const {
    const Lane& chosen = lane(laneId);
    float y = chosen.centerY;
    if (level == ProjectileLevel::High) y -= 26.0f;
    if (level == ProjectileLevel::Low) y += 26.0f;
    return {x, y};
}

glm::vec2 Battlefield::collectiblePosition(int laneId, float enemyX) const {
    const Lane& chosen = lane(laneId);
    return {std::clamp(enemyX, chosen.left, chosen.right),
            chosen.bottom - 16.0f};
}
