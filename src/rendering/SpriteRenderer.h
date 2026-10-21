#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

class Camera2D;
class Renderer2D;
class Texture;

// A sprite-specific interface over Renderer2D's shared quad geometry.
class SpriteRenderer {
public:
    explicit SpriteRenderer(Renderer2D& renderer) : renderer_(renderer) {}

    void draw(const Camera2D& camera, const Texture& texture,
              glm::vec2 position, glm::vec2 size,
              glm::vec4 tint = glm::vec4(1.0f),
              float rotationDegrees = 0.0f) const;

private:
    Renderer2D& renderer_;
};
