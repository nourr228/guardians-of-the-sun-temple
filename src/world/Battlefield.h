#pragma once

#include <array>
#include <optional>
#include <vector>

#include <glm/vec2.hpp>

#include "world/Lane.h"

enum class ProjectileLevel { High, Middle, Low };

// Pure world geometry and lane interaction, independent of rendering and GLFW.
class Battlefield {
public:
    static constexpr int laneCount = 4;
    static constexpr float worldWidth = 1000.0f;
    static constexpr float worldHeight = 600.0f;
    static constexpr float homeBoundary = 154.0f;
    static constexpr float playableLeft = 158.0f;
    static constexpr float playableRight = 980.0f;
    static constexpr float laneTop = 156.0f;
    static constexpr float laneHeight = 100.0f;

    Battlefield();

    [[nodiscard]] const std::array<Lane, laneCount>& lanes() const { return lanes_; }
    [[nodiscard]] const Lane& lane(int id) const;
    [[nodiscard]] std::optional<int> laneAt(glm::vec2 worldPosition) const;
    void updateHover(glm::vec2 worldPosition);
    void toggleSelectionAt(glm::vec2 worldPosition);
    void clearSelections();
    [[nodiscard]] std::vector<int> selectedLanes() const;

    [[nodiscard]] glm::vec2 guardianEntry(int laneId) const;
    [[nodiscard]] glm::vec2 defenderAnchor(int laneId) const;
    [[nodiscard]] glm::vec2 enemyEntry(int laneId) const;
    [[nodiscard]] glm::vec2 projectilePosition(int laneId, float x,
                                                ProjectileLevel level) const;
    [[nodiscard]] glm::vec2 collectiblePosition(int laneId, float enemyX) const;

private:
    std::array<Lane, laneCount> lanes_{};
};
