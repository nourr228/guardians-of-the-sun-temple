#pragma once

#include <glm/vec2.hpp>

struct Lane {
    int id = 0;
    float left = 0.0f;
    float right = 0.0f;
    float top = 0.0f;
    float bottom = 0.0f;
    float centerY = 0.0f;
    bool hovered = false;
    bool selected = false;

    [[nodiscard]] bool contains(glm::vec2 point) const {
        return point.x >= left && point.x < right &&
               point.y >= top && point.y < bottom;
    }
};
