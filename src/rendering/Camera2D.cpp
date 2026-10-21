#include "rendering/Camera2D.h"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

Camera2D::Camera2D(float width, float height) {
    setViewport(width, height);
}

void Camera2D::setViewport(float width, float height) {
    viewport_ = {std::max(width, 1.0f), std::max(height, 1.0f)};
}

glm::mat4 Camera2D::viewProjection() const {
    const glm::mat4 projection = glm::ortho(
            0.0f, viewport_.x, viewport_.y, 0.0f, -1.0f, 1.0f);
    const glm::mat4 view = glm::translate(glm::mat4(1.0f),
                                           glm::vec3(-position_, 0.0f));
    return projection * view;
}
