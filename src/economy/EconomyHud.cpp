#include "economy/EconomyHud.h"

#include <algorithm>
#include <array>
#include <string>

#include <glm/vec4.hpp>

#include "rendering/Camera2D.h"
#include "rendering/Renderer2D.h"

namespace {
constexpr std::array<GuardianType, 3> order{
        GuardianType::Khopesh, GuardianType::Sentinel, GuardianType::Archer};
constexpr float firstCardX = 713.0f;
constexpr float cardY = 13.0f;
constexpr float cardWidth = 78.0f;
constexpr float cardHeight = 76.0f;
constexpr float cardStep = 88.0f;
constexpr glm::vec4 gold{0.96f, 0.73f, 0.26f, 1.0f};
constexpr glm::vec4 gray{0.46f, 0.43f, 0.42f, 1.0f};
}

std::optional<GuardianType> EconomyHud::cardAt(glm::vec2 world) const {
    if (world.y < cardY || world.y > cardY + cardHeight)
        return std::nullopt;
    for (int i = 0; i < 3; ++i) {
        const float x = firstCardX + i * cardStep;
        if (world.x >= x && world.x <= x + cardWidth) return order[i];
    }
    return std::nullopt;
}

void EconomyHud::drawDigit(const Camera2D& camera, int digit,
                           glm::vec2 position, float scale,
                           glm::vec4 color) const {
    constexpr std::array<int, 10> segments{
            0b1111110, 0b0110000, 0b1101101, 0b1111001, 0b0110011,
            0b1011011, 0b1011111, 0b1110000, 0b1111111, 0b1111011};
    if (digit < 0 || digit > 9) return;
    const int mask = segments[digit];
    auto segment = [&](int bit, glm::vec2 offset, glm::vec2 size) {
        if (mask & (1 << bit))
            renderer_.drawRect(camera, position + offset * scale,
                               size * scale, color);
    };
    segment(6, {2, 0}, {10, 2});
    segment(5, {12, 2}, {2, 9});
    segment(4, {12, 13}, {2, 9});
    segment(3, {2, 22}, {10, 2});
    segment(2, {0, 13}, {2, 9});
    segment(1, {0, 2}, {2, 9});
    segment(0, {2, 11}, {10, 2});
}

void EconomyHud::drawNumber(const Camera2D& camera, int value,
                            glm::vec2 position, float scale,
                            glm::vec4 color) const {
    const std::string digits = std::to_string(std::max(0, value));
    for (std::size_t i = 0; i < digits.size(); ++i)
        drawDigit(camera, digits[i] - '0',
                  position + glm::vec2(static_cast<float>(i) * 17.0f * scale,
                                       0.0f), scale, color);
}

void EconomyHud::render(const Camera2D& camera,
                        const EconomySnapshot& economy, GuardianType selected,
                        std::optional<GuardianType> hovered) const {
    renderer_.drawRect(camera, {160, 10}, {182, 58},
                       {0.11f, 0.08f, 0.14f, 0.91f});
    renderer_.drawRect(camera, {164, 14}, {174, 50},
                       {0.43f, 0.29f, 0.15f, 0.78f});
    renderer_.drawRect(camera, {175, 29}, {24, 24}, gold, 45.0f);
    renderer_.drawRect(camera, {180, 34}, {14, 14},
                       {1.0f, 0.93f, 0.55f, 1.0f}, 45.0f);
    drawNumber(camera, economy.balance, {215, 24}, 1.25f, gold);

    renderer_.drawRect(camera, {704, 7}, {276, 89},
                       {0.1f, 0.07f, 0.13f, 0.92f});
    for (int i = 0; i < 3; ++i) {
        const GuardianType type = order[i];
        const CardState state = economy.cards[static_cast<int>(type)];
        const float x = firstCardX + i * cardStep;
        const bool isSelected = type == selected && state.available;
        const bool isHovered = hovered == type && state.available;
        const glm::vec4 border = state.available
                ? (isSelected || isHovered
                        ? glm::vec4(1.0f, 0.94f, 0.54f, 1.0f) : gold)
                : gray;
        renderer_.drawRect(camera, {x, cardY}, {cardWidth, cardHeight}, border);
        renderer_.drawRect(camera, {x + 3, cardY + 3},
                           {cardWidth - 6, cardHeight - 6},
                           state.available
                                   ? glm::vec4(0.33f, 0.21f, 0.16f, 1.0f)
                                   : glm::vec4(0.22f, 0.21f, 0.22f, 1.0f));
        const glm::vec4 emblem = state.available ? gold : gray;
        if (type == GuardianType::Khopesh) {
            renderer_.drawRect(camera, {x + 34, 27}, {7, 28}, emblem, 24.0f);
            renderer_.drawRect(camera, {x + 21, 22}, {22, 5}, emblem, -18.0f);
            renderer_.drawRect(camera, {x + 19, 22}, {5, 12}, emblem, 14.0f);
        } else if (type == GuardianType::Sentinel) {
            renderer_.drawRect(camera, {x + 25, 25}, {28, 33}, emblem);
            renderer_.drawRect(camera, {x + 30, 30}, {18, 22},
                               state.available
                                       ? glm::vec4(0.33f, 0.21f, 0.16f, 1.0f)
                                       : glm::vec4(0.22f, 0.21f, 0.22f, 1.0f));
            renderer_.drawRect(camera, {x + 36, 27}, {6, 28}, emblem);
        } else {
            renderer_.drawRect(camera, {x + 27, 29}, {5, 29}, emblem, -20.0f);
            renderer_.drawRect(camera, {x + 45, 29}, {5, 29}, emblem, 20.0f);
            renderer_.drawRect(camera, {x + 27, 42}, {27, 3}, emblem);
        }
        drawNumber(camera, guardianStats(type).cost,
                   {x + 26, 66}, 0.62f, emblem);
        drawNumber(camera, i + 1, {x + 7, 18}, 0.5f, emblem);
    }
}
