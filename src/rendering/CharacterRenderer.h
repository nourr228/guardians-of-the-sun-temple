#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "rendering/Animation.h"

class Animator;
class Camera2D;
class Renderer2D;

// Draws all six articulated models at a feet anchor in world coordinates.
// Health is supplied by guardian/enemy systems; this class never changes it.
class CharacterRenderer {
public:
    explicit CharacterRenderer(const Renderer2D& renderer)
            : renderer_(renderer) {}

    void draw(const Camera2D& camera, CharacterType type,
              const Animator& animator, glm::vec2 feetPosition,
              float facing, float scale = 1.0f,
              float healthRatio = 1.0f) const;

    [[nodiscard]] static glm::vec4 damageTint(glm::vec4 base,
                                                float healthRatio);

private:
    const Renderer2D& renderer_;
};
