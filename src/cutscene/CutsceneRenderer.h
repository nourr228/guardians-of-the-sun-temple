#pragma once

#include "rendering/CharacterRenderer.h"

class Camera2D;
class CutsceneManager;
class Renderer2D;
class TextRenderer;

class CutsceneRenderer {
public:
    CutsceneRenderer(const Renderer2D& renderer, const TextRenderer& text)
            : renderer_(renderer), text_(text), characters_(renderer) {}

    void render(const Camera2D& camera, const CutsceneManager& cutscene) const;

private:
    const Renderer2D& renderer_;
    const TextRenderer& text_;
    CharacterRenderer characters_;
};
