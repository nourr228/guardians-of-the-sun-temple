#pragma once

#include <optional>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "economy/EconomyManager.h"

class Camera2D;
class Renderer2D;

// Small temporary in-game counter and cards; final menu styling is Module 8.
class EconomyHud {
public:
    explicit EconomyHud(const Renderer2D& renderer) : renderer_(renderer) {}

    [[nodiscard]] std::optional<GuardianType> cardAt(glm::vec2 world) const;
    void render(const Camera2D& camera, const EconomySnapshot& economy,
                GuardianType selected,
                std::optional<GuardianType> hovered) const;

private:
    void drawDigit(const Camera2D& camera, int digit,
                   glm::vec2 position, float scale,
                   glm::vec4 color) const;
    void drawNumber(const Camera2D& camera, int value,
                    glm::vec2 position, float scale,
                    glm::vec4 color) const;

    const Renderer2D& renderer_;
};
