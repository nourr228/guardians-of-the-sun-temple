#include "world/TempleEnvironment.h"

#include <algorithm>
#include <cmath>

#include <glm/common.hpp>
#include <glm/vec4.hpp>

#include "rendering/Camera2D.h"
#include "rendering/Renderer2D.h"
#include "rendering/ResourceManager.h"
#include "rendering/SpriteRenderer.h"
#include "world/Battlefield.h"

namespace {
constexpr glm::vec4 stoneLight{0.72f, 0.51f, 0.31f, 1.0f};
constexpr glm::vec4 stoneDark{0.33f, 0.20f, 0.17f, 1.0f};
constexpr glm::vec4 gold{0.98f, 0.72f, 0.29f, 1.0f};

void drawObelisk(const Camera2D& camera, const Renderer2D& renderer,
                  float x, float groundY, float scale) {
    const auto r = [&](glm::vec2 p, glm::vec2 s, glm::vec4 color,
                       float rotation = 0.0f) {
        renderer.drawRect(camera, p, s, color, rotation);
    };
    const float w = 27.0f * scale;
    const float h = 92.0f * scale;
    r({x - w * 0.67f, groundY - 8 * scale},
      {w * 1.34f, 8 * scale}, stoneDark);
    r({x - w * 0.54f, groundY - h}, {w * 1.08f, h - 8 * scale},
      {0.77f, 0.54f, 0.31f, 1});
    r({x - w * 0.54f, groundY - h}, {w * 0.22f, h - 8 * scale},
      {0.96f, 0.72f, 0.41f, 1});
    r({x + w * 0.34f, groundY - h}, {w * 0.2f, h - 8 * scale},
      {0.46f, 0.27f, 0.18f, 1});
    r({x - w * 0.33f, groundY - h - w * 0.24f},
      {w * 0.66f, w * 0.66f}, {0.96f, 0.69f, 0.35f, 1}, 45.0f);
    for (int i = 0; i < 5; ++i) {
        const float y = groundY - h + (23 + i * 12) * scale;
        r({x - w * 0.18f, y}, {w * 0.36f, 2 * scale},
          {0.37f, 0.24f, 0.20f, 1});
        r({x - w * 0.1f, y - 3 * scale}, {2 * scale, 6 * scale},
          {0.37f, 0.24f, 0.20f, 1});
    }
}

void drawTorch(const Camera2D& camera, const Renderer2D& renderer,
               float x, float y, float time, float phase) {
    const auto r = [&](glm::vec2 p, glm::vec2 s, glm::vec4 color,
                       float rotation = 0.0f) {
        renderer.drawRect(camera, p, s, color, rotation);
    };
    const float flicker = std::sin(time * 12.0f + phase);
    r({x - 9, y}, {18, 5}, stoneDark);
    r({x - 6, y - 10}, {12, 12}, {0.55f, 0.29f, 0.18f, 1});
    r({x - 3, y + 4}, {6, 27}, {0.56f, 0.36f, 0.22f, 1});
    r({x - 13, y - 43}, {26, 36},
      {1.0f, 0.45f, 0.09f, 0.15f + 0.06f * flicker}, 45.0f);
    r({x - 6 + flicker * 2, y - 34}, {12, 25 + flicker * 3},
      {1.0f, 0.47f, 0.07f, 0.9f}, 45.0f);
    r({x - 3, y - 26}, {6, 14}, {1.0f, 0.88f, 0.35f, 1}, 45.0f);
}
} // namespace

TempleEnvironment::TempleEnvironment() { reset(); }

void TempleEnvironment::reset() {
    elapsed_ = 0.0f;
    for (std::size_t i = 0; i < sand_.size(); ++i) {
        sand_[i].position = {
                static_cast<float>((i * 173 + 43) % 1000),
                static_cast<float>(145 + (i * 97) % 400)};
        sand_[i].speed = 9.0f + static_cast<float>((i * 7) % 16);
    }
}

void TempleEnvironment::loadAssets(ResourceManager& resources,
                                    const std::filesystem::path& assetRoot) const {
    const auto root = assetRoot / "environment/kimssuki";
    constexpr auto crisp = Texture::Filter::Nearest;
    resources.loadTexture("env.mountains", root / "sand_mountains.png", crisp);
    resources.loadTexture("env.pyramid", root / "pyramid.png", crisp);
    resources.loadTexture("env.sphinx", root / "sphinx.png", crisp);
    resources.loadTexture("env.pillar", root / "pillar.png", crisp);
    resources.loadTexture("env.palmLarge", root / "palm_large.png", crisp);
    resources.loadTexture("env.palmSmall", root / "palm_small.png", crisp);
    resources.loadTexture("env.debris", root / "debris.png", crisp);
}

void TempleEnvironment::update(float deltaTime) {
    elapsed_ += deltaTime;
    for (auto& particle : sand_) {
        particle.position.x -= particle.speed * deltaTime;
        if (particle.position.x < Battlefield::playableLeft) {
            particle.position.x += Battlefield::playableRight -
                                   Battlefield::playableLeft;
        }
    }
}

void TempleEnvironment::renderBackdrop(const Camera2D& camera,
                                        const Renderer2D& renderer,
                                        const SpriteRenderer& sprites,
                                        const ResourceManager& resources) const {
    constexpr glm::vec4 topColor{0.19f, 0.10f, 0.26f, 1};
    constexpr glm::vec4 horizonColor{0.94f, 0.48f, 0.24f, 1};
    for (int band = 0; band < 54; ++band) {
        const float t = static_cast<float>(band) / 53.0f;
        renderer.drawRect(camera, {0, band * 5.0f}, {1000, 5.1f},
                          glm::mix(topColor, horizonColor, t));
    }

    const float glow = 0.10f + 0.025f * std::sin(elapsed_ * 1.8f);
    renderer.drawRect(camera, {742, 72}, {132, 132},
                      {1.0f, 0.77f, 0.28f, glow}, 45.0f);
    renderer.drawRect(camera, {764, 94}, {88, 88},
                      {1.0f, 0.78f, 0.36f, 0.37f}, 45.0f);
    renderer.drawRect(camera, {789, 119}, {38, 38},
                      {1.0f, 0.91f, 0.55f, 0.75f}, 45.0f);

    sprites.draw(camera, resources.texture("env.mountains"),
                 {0, -55}, {1000, 260});
    sprites.draw(camera, resources.texture("env.pyramid"),
                 {460, 69}, {174, 124}, {0.75f, 0.49f, 0.42f, 0.9f});
    sprites.draw(camera, resources.texture("env.pyramid"),
                 {640, 92}, {133, 95}, {0.65f, 0.45f, 0.45f, 0.8f});
    sprites.draw(camera, resources.texture("env.palmSmall"),
                 {290, 43}, {73, 110}, {0.54f, 0.36f, 0.35f, 0.85f});
    sprites.draw(camera, resources.texture("env.palmLarge"),
                 {766, 13}, {102, 142}, {0.58f, 0.42f, 0.36f, 0.9f});
}

void TempleEnvironment::renderBattlefield(const Camera2D& camera,
                                           const Renderer2D& renderer,
                                           const SpriteRenderer& sprites,
                                           const ResourceManager& resources,
                                           const Battlefield& battlefield) const {
    renderer.drawRect(camera, {0, 150}, {1000, 450},
                      {0.37f, 0.23f, 0.18f, 1});
    renderer.drawRect(camera, {150, 150}, {835, 414},
                      {0.27f, 0.17f, 0.16f, 1});

    for (const Lane& lane : battlefield.lanes()) {
        const glm::vec4 base = lane.id % 2 == 0
                ? glm::vec4(0.68f, 0.47f, 0.29f, 1)
                : glm::vec4(0.62f, 0.42f, 0.26f, 1);
        renderer.drawRect(camera, {lane.left, lane.top + 2},
                          {lane.right - lane.left, lane.bottom - lane.top - 4}, base);
        renderer.drawRect(camera, {lane.left, lane.top + 2},
                          {lane.right - lane.left, 4},
                          {0.89f, 0.66f, 0.41f, 1});
        renderer.drawRect(camera, {lane.left, lane.bottom - 7},
                          {lane.right - lane.left, 5}, stoneDark);

        for (int column = 0; column < 10; ++column) {
            const float x = lane.left + 74.0f + column * 82.0f;
            renderer.drawRect(camera, {x, lane.top + 8},
                              {2, 30}, {0.48f, 0.31f, 0.21f, 0.3f});
            renderer.drawRect(camera, {x + 35, lane.top + 56},
                              {2, 27}, {0.38f, 0.24f, 0.19f, 0.34f});
        }

        if (lane.hovered) {
            renderer.drawRect(camera, {lane.left, lane.top + 5},
                              {lane.right - lane.left, Battlefield::laneHeight - 12},
                              {0.48f, 0.93f, 0.93f, 0.16f});
            renderer.drawRect(camera, {lane.left, lane.top + 5},
                              {lane.right - lane.left, 3},
                              {0.47f, 0.94f, 0.92f, 0.75f});
        }
        if (lane.selected) {
            renderer.drawRect(camera, {lane.left, lane.top + 5},
                              {lane.right - lane.left, Battlefield::laneHeight - 12},
                              {1.0f, 0.75f, 0.20f, 0.22f});
            renderer.drawRect(camera, {lane.left, lane.bottom - 11},
                              {lane.right - lane.left, 4},
                              {1.0f, 0.83f, 0.34f, 0.95f});
        }
    }

    for (int i = 0; i < 4; ++i) {
        sprites.draw(camera, resources.texture("env.debris"),
                     {260.0f + i * 185.0f, 542.0f}, {53, 20},
                     {0.78f, 0.58f, 0.41f, 0.8f});
    }
}

void TempleEnvironment::renderForeground(const Camera2D& camera,
                                          const Renderer2D& renderer,
                                          const SpriteRenderer& sprites,
                                          const ResourceManager& resources) const {
    // Rear-strip props end before the first lane, leaving unit paths clear.
    // Pillars, obelisks, sphinx, torches, and palms are distinct objects.
    sprites.draw(camera, resources.texture("env.pillar"),
                 {185, 25}, {53, 130});
    sprites.draw(camera, resources.texture("env.sphinx"),
                 {369, 76}, {100, 78});

    drawObelisk(camera, renderer, 649, 154, 0.80f);
    drawTorch(camera, renderer, 270, 121, elapsed_, 0.0f);
    drawTorch(camera, renderer, 922, 121, elapsed_, 1.7f);

    for (const auto& particle : sand_) {
        renderer.drawRect(camera, particle.position, {2, 2},
                          {1.0f, 0.84f, 0.52f, 0.32f});
    }
}
