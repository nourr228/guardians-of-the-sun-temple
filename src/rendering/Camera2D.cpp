#include "rendering/Camera2D.h"

#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

Camera2D::Camera2D(float width, float height) {
    worldSize_ = {std::max(width, 1.0f), std::max(height, 1.0f)};
    setFramebufferSize(static_cast<int>(worldSize_.x),
                       static_cast<int>(worldSize_.y));
}

void Camera2D::setFramebufferSize(int width, int height) {
    framebufferSize_ = {std::max(width, 1), std::max(height, 1)};
    const float scale = std::min(framebufferSize_.x / worldSize_.x,
                                 framebufferSize_.y / worldSize_.y);
    viewport_.width = std::max(1, static_cast<int>(std::round(worldSize_.x * scale)));
    viewport_.height = std::max(1, static_cast<int>(std::round(worldSize_.y * scale)));
    viewport_.x = (framebufferSize_.x - viewport_.width) / 2;
    viewport_.y = (framebufferSize_.y - viewport_.height) / 2;
}

glm::mat4 Camera2D::viewProjection() const {
    const glm::mat4 projection = glm::ortho(
            0.0f, worldSize_.x, worldSize_.y, 0.0f, -1.0f, 1.0f);
    const glm::mat4 view = glm::translate(glm::mat4(1.0f),
                                           glm::vec3(-position_, 0.0f));
    return projection * view;
}

glm::vec2 Camera2D::windowToWorld(glm::vec2 cursor,
                                   glm::vec2 windowSize) const {
    const float windowWidth = std::max(windowSize.x, 1.0f);
    const float windowHeight = std::max(windowSize.y, 1.0f);
    const float framebufferX = cursor.x * framebufferSize_.x / windowWidth;
    const float framebufferY = cursor.y * framebufferSize_.y / windowHeight;
    const float normalizedX = (framebufferX - viewport_.x) / viewport_.width;
    const float normalizedY = 1.0f -
            (framebufferSize_.y - framebufferY - viewport_.y) / viewport_.height;
    return position_ + glm::vec2(normalizedX * worldSize_.x,
                                  normalizedY * worldSize_.y);
}
