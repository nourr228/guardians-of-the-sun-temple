#pragma once

#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <span>

class Camera2D;
class Shader;
class Texture;

// One indexed unit quad serves solid shapes and textured sprites.
class Renderer2D {
public:
    explicit Renderer2D(Shader& shader);
    ~Renderer2D();

    Renderer2D(const Renderer2D&) = delete;
    Renderer2D& operator=(const Renderer2D&) = delete;

    void drawRect(const Camera2D& camera, glm::vec2 position, glm::vec2 size,
                  glm::vec4 color, float rotationDegrees = 0.0f) const;
    void drawTexture(const Camera2D& camera, const Texture& texture,
                     glm::vec2 position, glm::vec2 size,
                     glm::vec4 tint = glm::vec4(1.0f),
                     float rotationDegrees = 0.0f) const;
    // Convex, filled polygon in world coordinates.
    void drawPolygon(const Camera2D& camera,
                     std::span<const glm::vec2> vertices,
                     glm::vec4 color) const;

private:
    void draw(const Camera2D& camera, glm::vec2 position, glm::vec2 size,
              glm::vec4 color, float rotationDegrees,
              const Texture* texture) const;

    Shader& shader_;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
    GLuint polygonVao_ = 0;
    GLuint polygonVbo_ = 0;
};
