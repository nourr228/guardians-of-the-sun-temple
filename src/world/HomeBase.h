#pragma once

#include "world/Battlefield.h"

class Camera2D;
class Renderer2D;
class ResourceManager;
class SpriteRenderer;

class HomeBase {
public:
    static constexpr float boundaryX = Battlefield::homeBoundary;

    [[nodiscard]] bool reachedBy(float enemyLeftEdge) const {
        return enemyLeftEdge <= boundaryX;
    }

    void render(const Camera2D& camera, const Renderer2D& renderer,
                const SpriteRenderer& sprites,
                const ResourceManager& resources,
                float elapsedSeconds) const;
};
