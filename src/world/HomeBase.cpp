#include "world/HomeBase.h"

#include <cmath>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "rendering/Camera2D.h"
#include "rendering/Renderer2D.h"
#include "rendering/ResourceManager.h"
#include "rendering/SpriteRenderer.h"

void HomeBase::render(const Camera2D& camera, const Renderer2D& renderer,
                      const SpriteRenderer& sprites,
                      const ResourceManager& resources,
                      float elapsedSeconds) const {
    const auto rect = [&](glm::vec2 position, glm::vec2 size, glm::vec4 color,
                          float rotation = 0.0f) {
        renderer.drawRect(camera, position, size, color, rotation);
    };
    constexpr glm::vec4 dark{0.21f, 0.12f, 0.18f, 1.0f};
    constexpr glm::vec4 sandstone{0.77f, 0.52f, 0.31f, 1.0f};
    constexpr glm::vec4 gold{0.98f, 0.76f, 0.32f, 1.0f};

    // This entire structure lies to the left of the precise defeat boundary.
    rect({0, 146}, {boundaryX, 430}, dark);
    rect({7, 163}, {133, 389}, sandstone);
    rect({15, 173}, {117, 369}, {0.39f, 0.24f, 0.22f, 1});
    rect({23, 181}, {101, 352}, {0.16f, 0.10f, 0.18f, 1});
    rect({0, 146}, {boundaryX, 15}, gold);
    rect({9, 165}, {129, 9}, {0.95f, 0.67f, 0.32f, 1});
    rect({0, 550}, {boundaryX, 22}, {0.51f, 0.31f, 0.23f, 1});
    rect({0, 572}, {boundaryX, 10}, gold);

    // Layered pillars and a stepped stone crown frame the shrine.
    for (float x : {13.0f, 119.0f}) {
        rect({x, 170}, {21, 375}, {0.48f, 0.28f, 0.21f, 1});
        rect({x + 3, 170}, {14, 375}, {0.88f, 0.63f, 0.36f, 1});
        for (int y = 202; y < 528; y += 46) {
            rect({x + 5, static_cast<float>(y)}, {10, 3},
                 {0.42f, 0.27f, 0.20f, 1});
            rect({x + 9, static_cast<float>(y - 6)}, {3, 10},
                 {0.42f, 0.27f, 0.20f, 1});
        }
    }
    rect({4, 174}, {142, 15}, {0.91f, 0.65f, 0.34f, 1});
    rect({4, 142}, {146, 16}, {0.55f, 0.32f, 0.23f, 1});
    rect({10, 145}, {134, 7}, gold);
    rect({18, 130}, {118, 12}, {0.78f, 0.51f, 0.29f, 1});
    rect({26, 119}, {102, 11}, {0.94f, 0.68f, 0.35f, 1});
    rect({37, 108}, {80, 11}, {0.72f, 0.45f, 0.28f, 1});
    rect({60, 99}, {34, 9}, gold);
    rect({49, 170}, {56, 8}, {0.35f, 0.20f, 0.18f, 1});

    // Four bands align with the combat paths and lead the eye to the relic.
    for (int i = 0; i < Battlefield::laneCount; ++i) {
        const float y = Battlefield::laneTop + i * Battlefield::laneHeight;
        rect({38, y + 50}, {74, 2}, {0.78f, 0.56f, 0.29f, 0.35f});
    }
    rect({40, 273}, {73, 112}, {0.57f, 0.35f, 0.27f, 1});
    rect({49, 281}, {55, 96}, {0.11f, 0.08f, 0.16f, 1});
    rect({51, 384}, {51, 7}, gold);
    rect({62, 391}, {29, 15}, {0.49f, 0.28f, 0.20f, 1});

    const float pulse = 1.0f + 0.055f * std::sin(elapsedSeconds * 2.5f);
    rect({47 - 7 * pulse, 304 - 7 * pulse},
         {60 + 14 * pulse, 60 + 14 * pulse},
         {1.0f, 0.72f, 0.22f, 0.10f}, 45.0f);
    rect({55 - 4 * pulse, 312 - 4 * pulse},
         {44 + 8 * pulse, 44 + 8 * pulse},
         {1.0f, 0.88f, 0.38f, 0.20f}, 45.0f);
    const float crystalSize = 57.0f * pulse;
    sprites.draw(camera, resources.texture("sun_crystal"),
                 {77.0f - crystalSize * 0.5f,
                  342.0f - crystalSize * 0.5f},
                 {crystalSize, crystalSize});

    rect({boundaryX - 4, 156}, {4, 400}, gold);
}
