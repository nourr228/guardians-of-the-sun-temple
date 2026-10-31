#pragma once

#include <optional>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "economy/EconomyManager.h"

class Camera2D;
class Renderer2D;
class TextRenderer;

class EconomyHud {
public:
    EconomyHud(const Renderer2D& renderer, const TextRenderer& text)
            : renderer_(renderer), text_(text) {}

    [[nodiscard]] std::optional<GuardianType> cardAt(glm::vec2 world) const;
    [[nodiscard]] bool contains(glm::vec2 world) const;
    void render(const Camera2D& camera, const EconomySnapshot& economy,
                GuardianType selected,
                std::optional<GuardianType> hovered) const;

private:
    const Renderer2D& renderer_;
    const TextRenderer& text_;
};
