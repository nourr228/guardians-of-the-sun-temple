#pragma once

#include <optional>

#include <glm/vec2.hpp>

#include "ui/MenuManager.h"

class Camera2D;
class Renderer2D;
class TextRenderer;

class MenuRenderer {
public:
    MenuRenderer(const Renderer2D& renderer, const TextRenderer& text)
            : renderer_(renderer), text_(text) {}

    void render(const Camera2D& camera, const MenuManager& menu,
                glm::vec2 mouse, float volume, bool muted) const;

private:
    const Renderer2D& renderer_;
    const TextRenderer& text_;
};
