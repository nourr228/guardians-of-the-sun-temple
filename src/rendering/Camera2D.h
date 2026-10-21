#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

// World coordinates use a top-left origin: +X points right and +Y points down.
class Camera2D {
public:
    Camera2D(float width, float height);

    void setViewport(float width, float height);
    void setPosition(const glm::vec2& position) { position_ = position; }

    [[nodiscard]] glm::mat4 viewProjection() const;
    [[nodiscard]] glm::vec2 position() const { return position_; }
    [[nodiscard]] glm::vec2 viewport() const { return viewport_; }

private:
    glm::vec2 viewport_;
    glm::vec2 position_{0.0f};
};
