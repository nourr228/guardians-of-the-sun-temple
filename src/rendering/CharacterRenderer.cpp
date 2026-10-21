#include "rendering/CharacterRenderer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <span>
#include <vector>

#include <glm/common.hpp>

#include "rendering/Animator.h"
#include "rendering/Camera2D.h"
#include "rendering/Renderer2D.h"

namespace {
using Color = glm::vec4;
constexpr Color ink{0.14f, 0.09f, 0.16f, 1};
constexpr Color gold{0.99f, 0.76f, 0.29f, 1};
constexpr Color brightGold{1.0f, 0.91f, 0.53f, 1};
constexpr Color linen{0.88f, 0.76f, 0.56f, 1};
constexpr Color skin{0.72f, 0.42f, 0.25f, 1};
constexpr Color turquoise{0.24f, 0.83f, 0.77f, 1};
constexpr Color curse{0.47f, 0.21f, 0.63f, 1};
constexpr float pi = std::numbers::pi_v<float>;

glm::vec2 rotate(glm::vec2 point, glm::vec2 pivot, float degrees) {
    const float radians = degrees * pi / 180.0f;
    const glm::vec2 d = point - pivot;
    return pivot + glm::vec2(d.x * std::cos(radians) - d.y * std::sin(radians),
                              d.x * std::sin(radians) + d.y * std::cos(radians));
}

struct Motion {
    float bob = 0;
    float lift = 0;
    float stride = 0;
    float action = 0;
    float recoil = 0;
    float collapse = 0;
    float heightScale = 1;
    float wing = 0;
    float wave = 0;
};

Motion motionFor(const Animator& animator) {
    Motion m;
    const float p = animator.progress();
    const float cycle = 2.0f * pi * p;
    switch (animator.state()) {
        case AnimationState::Idle:
            m.bob = 1.2f * std::sin(cycle);
            m.wing = 0.3f * std::sin(cycle);
            m.wave = std::sin(cycle);
            break;
        case AnimationState::Moving:
            m.stride = 8.0f * std::sin(cycle);
            m.bob = -2.0f * std::abs(std::sin(cycle));
            m.wing = std::sin(cycle * 2.0f);
            m.wave = std::sin(cycle);
            break;
        case AnimationState::Attacking:
            m.action = std::sin(pi * p);
            m.recoil = p > 0.52f ? 1.5f * (p - 0.52f) : 0;
            m.wing = 0.7f * m.action;
            m.wave = m.action;
            break;
        case AnimationState::Jumping:
            m.lift = 31.0f * std::sin(pi * p);
            m.stride = 4.0f * std::sin(2.0f * pi * p);
            break;
        case AnimationState::Ducking:
            m.heightScale = 0.62f;
            m.bob = 0.8f * std::sin(cycle);
            break;
        case AnimationState::TakingDamage:
            m.recoil = 6.0f * std::sin(pi * p);
            m.wave = -m.recoil * 0.2f;
            break;
        case AnimationState::Dying:
            m.collapse = 72.0f * p * p;
            m.heightScale = 1.0f - 0.22f * p;
            m.bob = 4.0f * p;
            m.wing = -p;
            break;
    }
    return m;
}

class Painter {
public:
    Painter(const Camera2D& camera, const Renderer2D& renderer,
            glm::vec2 anchor, float facing, float scale,
            float health, const Motion& motion,
            AnimationState state)
            : camera_(camera), renderer_(renderer), anchor_(anchor),
              facing_(facing >= 0 ? 1.0f : -1.0f), scale_(scale),
              health_(health), motion_(motion), state_(state) {}

    [[nodiscard]] glm::vec2 world(glm::vec2 local) const {
        glm::vec2 p{local.x * facing_ * scale_,
                    local.y * motion_.heightScale * scale_};
        if (motion_.collapse != 0) {
            const float angle = motion_.collapse * facing_ * pi / 180.0f;
            p = {p.x * std::cos(angle) - p.y * std::sin(angle),
                 p.x * std::sin(angle) + p.y * std::cos(angle)};
        }
        return anchor_ + p +
               glm::vec2(-motion_.recoil * facing_ * scale_,
                         (motion_.bob - motion_.lift) * scale_);
    }

    void polygon(std::span<const glm::vec2> points, Color color) const {
        std::vector<glm::vec2> transformed;
        transformed.reserve(points.size());
        for (glm::vec2 point : points) transformed.push_back(world(point));
        renderer_.drawPolygon(camera_, transformed, tint(color));
    }

    void polygon(std::initializer_list<glm::vec2> points, Color color) const {
        polygon(std::span<const glm::vec2>(points.begin(), points.size()), color);
    }

    void rect(glm::vec2 position, glm::vec2 size, Color color,
              float rotation = 0) const {
        const glm::vec2 center = position + size * 0.5f;
        const std::array<glm::vec2, 4> corners{
                rotate(position, center, rotation),
                rotate({position.x + size.x, position.y}, center, rotation),
                rotate(position + size, center, rotation),
                rotate({position.x, position.y + size.y}, center, rotation)};
        polygon(corners, color);
    }

    void line(glm::vec2 start, glm::vec2 end, float width, Color color) const {
        const glm::vec2 d = end - start;
        const float length = std::sqrt(d.x * d.x + d.y * d.y);
        if (length < 0.001f) return;
        const glm::vec2 normal{-d.y / length * width * 0.5f,
                                d.x / length * width * 0.5f};
        polygon({start - normal, end - normal, end + normal, start + normal},
                color);
    }

    void ellipse(glm::vec2 center, glm::vec2 radius, Color color) const {
        std::array<glm::vec2, 16> points{};
        for (std::size_t i = 0; i < points.size(); ++i) {
            const float angle = 2.0f * pi * static_cast<float>(i) /
                                static_cast<float>(points.size());
            points[i] = center + glm::vec2(radius.x * std::cos(angle),
                                            radius.y * std::sin(angle));
        }
        polygon(points, color);
    }

    [[nodiscard]] float health() const { return health_; }
    [[nodiscard]] AnimationState state() const { return state_; }

private:
    [[nodiscard]] Color tint(Color color) const {
        Color tinted = CharacterRenderer::damageTint(color, health_);
        if (state_ == AnimationState::TakingDamage) {
            const float flash = 0.4f *
                    std::clamp(motion_.recoil / 6.0f, 0.0f, 1.0f);
            tinted = glm::mix(tinted, Color{1.0f, 0.32f, 0.29f, color.a}, flash);
        }
        return tinted;
    }

    const Camera2D& camera_;
    const Renderer2D& renderer_;
    glm::vec2 anchor_;
    float facing_;
    float scale_;
    float health_;
    const Motion& motion_;
    AnimationState state_;
};

void drawLegs(const Painter& p, Color legColor, float stride,
              bool armored) {
    p.line({-9, -25}, {-11 - stride * 0.45f, -10}, 8, ink);
    p.line({-11 - stride * 0.45f, -10}, {-13 - stride, -2}, 7, ink);
    p.line({7, -25}, {9 + stride * 0.45f, -10}, 8, ink);
    p.line({9 + stride * 0.45f, -10}, {10 + stride, -2}, 7, ink);
    p.line({-9, -24}, {-11 - stride * 0.45f, -10}, 5, legColor);
    p.line({7, -24}, {9 + stride * 0.45f, -10}, 5, legColor);
    p.rect({-18 - stride, -5}, {14, 5}, armored ? gold : linen);
    p.rect({4 + stride, -5}, {14, 5}, armored ? gold : linen);
}

void drawKhopesh(const Painter& p, const Motion& m) {
    drawLegs(p, linen, m.stride, true);
    p.polygon({{-17, -52}, {11, -52}, {18, -28}, {-19, -28}}, ink);
    p.polygon({{-14, -49}, {9, -49}, {13, -34}, {-15, -34}},
              {0.73f, 0.52f, 0.31f, 1});
    p.rect({-13, -47}, {21, 5}, gold);
    p.rect({-3, -43}, {5, 10}, brightGold);
    p.polygon({{-17, -31}, {16, -31}, {11, -21}, {-13, -21}}, linen);
    for (float x : {-8.0f, 0.0f, 8.0f}) p.line({x, -29}, {x - 1, -23}, 1.5f, gold);

    p.line({-14, -47}, {-25, -30}, 8, ink);
    p.line({-14, -47}, {-25, -30}, 5, skin);
    p.ellipse({-25, -28}, {4, 4}, skin);

    const glm::vec2 pivot{13, -43};
    const float swing = -35.0f + 102.0f * m.action;
    const auto arm = [&](glm::vec2 q) { return rotate(q, pivot, swing); };
    p.line(pivot, arm({24, -32}), 8, ink);
    p.line(pivot, arm({24, -32}), 5, skin);
    p.ellipse(arm({24, -32}), {4, 4}, skin);
    p.line(arm({23, -34}), arm({31, -44}), 4, {0.48f, 0.26f, 0.17f, 1});
    p.line(arm({26, -39}), arm({35, -49}), 2.5f, gold);
    p.polygon({arm({30, -45}), arm({35, -60}), arm({44, -65}),
               arm({49, -61}), arm({43, -58}), arm({38, -47})},
              {0.78f, 0.87f, 0.78f, 1});
    p.line(arm({36, -58}), arm({45, -62}), 1.8f, brightGold);
    if (m.action > 0.35f) p.ellipse(arm({46, -61}), {5, 5},
                                   {1, 0.86f, 0.35f, 0.46f});

    p.ellipse({-2, -63}, {11, 12}, ink);
    p.ellipse({-2, -63}, {8, 10}, skin);
    p.polygon({{-16, -70}, {-6, -80}, {7, -78}, {13, -68},
               {10, -64}, {-15, -64}}, {0.48f, 0.28f, 0.18f, 1});
    p.rect({-14, -69}, {27, 5}, gold);
    p.rect({-3, -72}, {6, 10}, brightGold);
    p.ellipse({6, -63}, {1.7f, 1.5f}, brightGold);
}

void drawSentinel(const Painter& p, const Motion& m) {
    const Color stone{0.70f, 0.58f, 0.43f, 1};
    const Color shade{0.43f, 0.35f, 0.33f, 1};
    p.rect({-23, -24}, {18, 24}, ink);
    p.rect({5, -24}, {18, 24}, ink);
    p.rect({-20, -21}, {12, 18}, stone);
    p.rect({8, -21}, {12, 18}, stone);
    p.polygon({{-26, -70}, {26, -70}, {30, -30}, {19, -22},
               {-20, -22}, {-30, -30}}, ink);
    p.polygon({{-22, -66}, {22, -66}, {24, -31}, {-24, -31}}, stone);
    p.rect({-22, -67}, {44, 6}, {0.89f, 0.71f, 0.45f, 1});
    p.rect({-10, -53}, {20, 17}, shade);
    p.rect({-5, -50}, {10, 11}, {0.82f, 0.67f, 0.46f, 1});
    p.line({-4, -48}, {4, -40}, 2, gold);
    p.line({4, -48}, {-4, -40}, 2, gold);
    p.rect({-33, -62}, {12, 35}, ink);
    p.rect({-30, -59}, {7, 29}, stone);
    p.rect({-17, -86}, {34, 19}, ink);
    p.rect({-13, -82}, {26, 15}, stone);
    p.polygon({{-18, -83}, {-10, -92}, {10, -92}, {18, -83}}, shade);
    p.rect({-9, -76}, {6, 3}, turquoise);
    p.rect({4, -76}, {6, 3}, turquoise);

    const float shieldShift = 8.0f * m.action;
    p.polygon({{10 + shieldShift, -64}, {33 + shieldShift, -68},
               {42 + shieldShift, -57}, {40 + shieldShift, -12},
               {25 + shieldShift, -4}, {10 + shieldShift, -13}}, ink);
    p.polygon({{14 + shieldShift, -60}, {32 + shieldShift, -63},
               {37 + shieldShift, -55}, {35 + shieldShift, -15},
               {25 + shieldShift, -9}, {14 + shieldShift, -16}}, stone);
    p.line({24 + shieldShift, -56}, {24 + shieldShift, -17}, 3, gold);
    p.line({16 + shieldShift, -37}, {34 + shieldShift, -37}, 3, gold);
    p.rect({20 + shieldShift, -43}, {8, 9}, turquoise);
    if (m.action > 0.2f) {
        p.ellipse({27 + shieldShift, -37}, {23, 31},
                  {0.42f, 0.95f, 0.85f, 0.16f + 0.16f * m.action});
    }
    if (p.health() < 0.75f) {
        p.line({-19, -60}, {-11, -46}, 1.5f, shade);
        p.line({-11, -46}, {-15, -38}, 1.5f, shade);
    }
    if (p.health() < 0.4f) {
        p.line({10, -58}, {2, -45}, 1.5f, shade);
        p.line({2, -45}, {9, -31}, 1.5f, shade);
        p.line({32 + shieldShift, -47}, {24 + shieldShift, -33},
               1.5f, shade);
    }
}

void drawArcher(const Painter& p, const Motion& m, float phase) {
    drawLegs(p, {0.46f, 0.35f, 0.25f, 1}, m.stride, false);
    p.polygon({{-20, -58}, {-9, -63}, {-7, -27}, {-21, -32}}, ink);
    p.polygon({{-18, -56}, {-11, -58}, {-10, -33}, {-18, -35}},
              {0.58f, 0.33f, 0.21f, 1});
    for (int i = 0; i < 3; ++i) {
        p.line({-15.0f + i * 2.0f, -62},
               {-15.0f + i * 2.0f, -49}, 1.5f, brightGold);
    }
    p.polygon({{-12, -52}, {12, -52}, {15, -29}, {-15, -29}}, ink);
    p.polygon({{-9, -49}, {9, -49}, {11, -31}, {-11, -31}},
              {0.91f, 0.79f, 0.55f, 1});
    p.line({-8, -47}, {9, -34}, 4, gold);
    p.polygon({{-13, -31}, {13, -31}, {8, -22}, {-9, -22}},
              {0.43f, 0.53f, 0.42f, 1});
    p.line({-11, -47}, {-2, -34}, 6, skin);
    p.line({10, -47}, {20, -33}, 6, skin);
    p.ellipse({-1, -63}, {10, 11}, ink);
    p.ellipse({-1, -63}, {8, 9}, skin);
    p.polygon({{-13, -72}, {-6, -79}, {8, -76}, {14, -69},
               {10, -65}, {-12, -65}}, {0.67f, 0.51f, 0.26f, 1});
    p.rect({-11, -69}, {24, 4}, gold);
    p.ellipse({7, -62}, {1.6f, 1.4f}, brightGold);

    const float pull = 11.0f * m.action;
    p.line({26, -60}, {34, -52}, 4, gold);
    p.line({34, -52}, {37, -35}, 4, gold);
    p.line({37, -35}, {34, -18}, 4, gold);
    p.line({34, -18}, {26, -10}, 4, gold);
    p.line({26, -60}, {26 - pull, -35}, 1.7f, brightGold);
    p.line({26 - pull, -35}, {26, -10}, 1.7f, brightGold);
    p.ellipse({26 - pull, -35}, {3, 3}, skin);
    if (m.action > 0.08f) {
        const float release = phase > 0.58f ? (phase - 0.58f) * 42 : 0;
        p.line({17 + release, -35}, {43 + release, -35}, 2.5f,
               brightGold);
        p.polygon({{45 + release, -35}, {39 + release, -39},
                   {39 + release, -31}}, gold);
        p.line({18 + release, -35}, {14 + release, -40}, 1.3f, linen);
        p.line({18 + release, -35}, {14 + release, -30}, 1.3f, linen);
    }
}

void drawScarab(const Painter& p, const Motion& m) {
    const Color shell{0.15f, 0.37f, 0.39f, 1};
    const Color chitin{0.23f, 0.60f, 0.59f, 1};
    const float flap = m.wing * 9.0f;
    for (int i = 0; i < 3; ++i) {
        const float y = -29.0f + i * 8.0f;
        p.line({-17, y}, {-34, y + 5.0f + i * 3.0f}, 3.0f, ink);
        p.line({16, y}, {34, y + 5.0f + i * 3.0f}, 3.0f, ink);
        p.line({-34, y + 5.0f + i * 3.0f},
               {-39, y + 11.0f + i * 3.0f}, 2.0f, chitin);
        p.line({34, y + 5.0f + i * 3.0f},
               {39, y + 11.0f + i * 3.0f}, 2.0f, chitin);
    }
    p.polygon({{-9, -45}, {-34, -62 - flap}, {-29, -31 - flap},
               {-9, -24}}, ink);
    p.polygon({{-8, -43}, {-30, -57 - flap}, {-27, -34 - flap},
               {-8, -27}}, {0.39f, 0.77f, 0.72f, 0.8f});
    p.polygon({{9, -45}, {34, -62 + flap}, {29, -31 + flap},
               {9, -24}}, ink);
    p.polygon({{8, -43}, {30, -57 + flap}, {27, -34 + flap},
               {8, -27}}, {0.39f, 0.77f, 0.72f, 0.8f});
    p.ellipse({0, -36}, {22, 19}, ink);
    p.ellipse({0, -37}, {18, 16}, shell);
    p.ellipse({0, -40}, {11, 10}, chitin);
    p.line({0, -53}, {0, -24}, 2, ink);
    for (float y : {-44.0f, -38.0f, -32.0f}) {
        p.line({-14, y}, {-4, y + 2}, 1.5f, turquoise);
        p.line({4, y + 2}, {14, y}, 1.5f, turquoise);
    }
    p.ellipse({22, -35}, {10, 11}, ink);
    p.ellipse({24, -35}, {7, 8}, {0.27f, 0.49f, 0.44f, 1});
    p.ellipse({27, -39}, {2.5f, 2.5f}, {0.84f, 0.37f, 0.91f, 1});
    p.line({25, -44}, {32, -55}, 2, ink);
    p.line({32, -55}, {39, -57}, 1.5f, turquoise);
    p.line({19, -45}, {18, -57}, 2, ink);
    p.ellipse({35, -30}, {3.2f, 3.2f},
              {0.65f, 0.27f, 0.79f, 0.8f});
    if (m.action > 0.3f) {
        p.line({30, -47}, {38, -54}, 2.0f, curse);
        p.ellipse({40, -55}, {7 * m.action, 7 * m.action},
                  {0.84f, 0.36f, 0.87f, 0.35f});
    }
}

void drawSerpent(const Painter& p, const Motion& m) {
    const Color scales{0.53f, 0.39f, 0.25f, 1};
    const Color lightScales{0.77f, 0.61f, 0.32f, 1};
    const Color belly{0.85f, 0.72f, 0.48f, 1};
    p.polygon({{-47, -14}, {-34, -29}, {-28, -13}, {-38, -5}}, ink);
    p.polygon({{-43, -13}, {-35, -25}, {-31, -12}, {-38, -7}}, scales);
    for (int i = 0; i < 5; ++i) {
        const float x = -31.0f + i * 12.0f;
        const float wave = std::sin(i * 1.5f + m.wave * 1.8f) * 4.0f;
        p.ellipse({x, -14 + wave}, {12, 10}, ink);
        p.ellipse({x, -16 + wave}, {10, 8}, scales);
        p.rect({x - 5, -10 + wave}, {10, 3}, belly);
        p.line({x - 4, -20 + wave}, {x + 1, -18 + wave},
               1.5f, lightScales);
    }
    const float neckLift = 3.0f * m.action;
    p.line({21, -20}, {24, -37 - neckLift}, 11, ink);
    p.line({21, -20}, {24, -37 - neckLift}, 8, scales);
    p.polygon({{14, -43 - neckLift}, {30, -47 - neckLift},
               {43 + m.action * 5, -40 - neckLift},
               {38 + m.action * 5, -29 - neckLift},
               {17, -31 - neckLift}}, ink);
    p.polygon({{17, -40 - neckLift}, {30, -44 - neckLift},
               {40 + m.action * 5, -39 - neckLift},
               {36 + m.action * 5, -33 - neckLift},
               {19, -33 - neckLift}}, lightScales);
    p.ellipse({31, -40 - neckLift}, {3, 2}, curse);
    p.polygon({{36, -32 - neckLift}, {41, -31 - neckLift},
               {39, -23 - neckLift}}, brightGold);
    p.polygon({{29, -32 - neckLift}, {34, -31 - neckLift},
               {31, -25 - neckLift}}, brightGold);
    if (m.action > 0.15f) {
        p.ellipse({47 + m.action * 6, -14},
                  {5 + 7 * m.action, 3 + 4 * m.action},
                  {0.76f, 0.58f, 0.36f, 0.7f});
        p.line({38, -30 - neckLift}, {48 + m.action * 4, -15},
               2, {0.80f, 0.66f, 0.43f, 0.75f});
    }
}

void drawPharaoh(const Painter& p, const Motion& m) {
    const Color wraps{0.72f, 0.66f, 0.52f, 1};
    const Color armor{0.38f, 0.24f, 0.42f, 1};
    drawLegs(p, wraps, m.stride * 1.25f, false);
    for (float x : {-12.0f, 7.0f}) {
        p.line({x, -20}, {x + 5, -17}, 1.4f, ink);
        p.line({x, -13}, {x + 5, -10}, 1.4f, ink);
    }
    p.polygon({{-19, -57}, {17, -57}, {23, -29}, {-23, -29}}, ink);
    p.polygon({{-15, -53}, {13, -53}, {18, -31}, {-18, -31}}, armor);
    p.polygon({{-17, -51}, {-6, -48}, {-3, -34}, {-14, -37}}, gold);
    p.polygon({{15, -51}, {4, -48}, {1, -34}, {12, -37}}, gold);
    p.rect({-3, -48}, {7, 15}, curse);
    p.rect({-21, -29}, {43, 7}, {0.62f, 0.48f, 0.33f, 1});
    p.line({-18, -50}, {-26, -31}, 9, ink);
    p.line({-18, -50}, {-26, -31}, 6, wraps);
    p.rect({-29, -35}, {10, 4}, gold);

    const glm::vec2 pivot{17, -48};
    const float swing = -40.0f + 105.0f * m.action;
    const auto arm = [&](glm::vec2 q) { return rotate(q, pivot, swing); };
    p.line(pivot, arm({29, -32}), 9, ink);
    p.line(pivot, arm({29, -32}), 6, wraps);
    p.rect(arm({26, -36}), {10, 4}, gold, swing);
    p.line(arm({31, -33}), arm({35, -82}), 5, ink);
    p.line(arm({31, -33}), arm({35, -82}), 3, {0.59f, 0.38f, 0.29f, 1});
    p.polygon({arm({35, -89}), arm({43, -78}), arm({35, -70}),
               arm({27, -78})}, curse);
    p.ellipse(arm({35, -79}), {4, 4}, turquoise);
    if (m.action > 0.35f) {
        p.ellipse(arm({40, -81}), {10 * m.action, 10 * m.action},
                  {0.57f, 0.23f, 0.72f, 0.25f});
    }

    p.ellipse({-1, -67}, {11, 12}, ink);
    p.ellipse({-1, -67}, {8, 9}, wraps);
    p.polygon({{-21, -76}, {-12, -89}, {11, -89}, {21, -76},
               {15, -56}, {7, -62}, {-7, -62}, {-15, -56}}, ink);
    p.polygon({{-17, -75}, {-9, -86}, {9, -86}, {17, -75},
               {12, -61}, {7, -65}, {-7, -65}, {-12, -61}},
              {0.27f, 0.21f, 0.39f, 1});
    p.rect({-10, -84}, {20, 5}, gold);
    p.rect({-7, -77}, {4, 10}, gold);
    p.rect({3, -77}, {4, 10}, gold);
    p.ellipse({6, -66}, {2, 2}, turquoise);
    p.line({-13, -60}, {-18, -54}, 3, gold);
    p.line({12, -60}, {17, -54}, 3, gold);
}
} // namespace

glm::vec4 CharacterRenderer::damageTint(glm::vec4 base, float healthRatio) {
    const float damage = 1.0f - std::clamp(healthRatio, 0.0f, 1.0f);
    const glm::vec4 target{0.93f, 0.13f, 0.17f, base.a};
    return glm::mix(base, target, damage * 0.72f);
}

void CharacterRenderer::draw(const Camera2D& camera, CharacterType type,
                             const Animator& animator, glm::vec2 feetPosition,
                             float facing, float scale,
                             float healthRatio) const {
    if (animator.type() != type || scale <= 0) return;
    const Motion m = motionFor(animator);
    const Painter p(camera, renderer_, feetPosition, facing, scale,
                    healthRatio, m, animator.state());
    switch (type) {
        case CharacterType::KhopeshGuardian: drawKhopesh(p, m); break;
        case CharacterType::StoneSentinel: drawSentinel(p, m); break;
        case CharacterType::SunArcher:
            drawArcher(p, m, animator.progress()); break;
        case CharacterType::CursedScarab: drawScarab(p, m); break;
        case CharacterType::SandSerpent: drawSerpent(p, m); break;
        case CharacterType::CursedPharaoh: drawPharaoh(p, m); break;
    }
}
