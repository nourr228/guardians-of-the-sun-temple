#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

struct ViewportRect {
    int x = 0;
    int y = 0;
    int width = 1;
    int height = 1;
};

// World coordinates use a top-left origin: +X points right and +Y points down.
class Camera2D {
public:
    Camera2D(float width, float height);

    void setFramebufferSize(int width, int height);
    void setPosition(const glm::vec2& position) { position_ = position; }

    [[nodiscard]] glm::mat4 viewProjection() const;
    [[nodiscard]] glm::vec2 windowToWorld(glm::vec2 cursor,
                                           glm::vec2 windowSize) const;
    [[nodiscard]] glm::vec2 position() const { return position_; }
    [[nodiscard]] glm::vec2 worldSize() const { return worldSize_; }
    [[nodiscard]] ViewportRect viewportRect() const { return viewport_; }
    [[nodiscard]] glm::ivec2 framebufferSize() const { return framebufferSize_; }

private:
    glm::vec2 worldSize_;
    glm::ivec2 framebufferSize_{1, 1};
    ViewportRect viewport_{};
    glm::vec2 position_{0.0f};
};
