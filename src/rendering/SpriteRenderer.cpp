#include "rendering/SpriteRenderer.h"

#include "rendering/Renderer2D.h"

void SpriteRenderer::draw(const Camera2D& camera, const Texture& texture,
                          glm::vec2 position, glm::vec2 size,
                          glm::vec4 tint, float rotationDegrees) const {
    renderer_.drawTexture(camera, texture, position, size, tint, rotationDegrees);
}
