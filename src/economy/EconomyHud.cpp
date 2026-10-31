#include "economy/EconomyHud.h"

#include <array>
#include <string>

#include "rendering/Camera2D.h"
#include "rendering/Renderer2D.h"
#include "ui/TextRenderer.h"

namespace {
constexpr std::array<GuardianType, 3> order{
        GuardianType::Khopesh, GuardianType::Sentinel, GuardianType::Archer};
constexpr float firstCardX = 589.0f;
constexpr float cardY = 8.0f;
constexpr float cardWidth = 122.0f;
constexpr float cardHeight = 134.0f;
constexpr float cardStep = 132.0f;
constexpr glm::vec4 gold{0.97f, 0.75f, 0.30f, 1.0f};
constexpr glm::vec4 pale{1.0f, 0.93f, 0.75f, 1.0f};
constexpr glm::vec4 gray{0.70f, 0.68f, 0.66f, 1.0f};
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

bool EconomyHud::contains(glm::vec2 world) const {
    return world.x >= 158.0f && world.x <= 1000.0f &&
           world.y >= 0.0f && world.y <= 146.0f;
}

void EconomyHud::render(const Camera2D& camera,
                        const EconomySnapshot& economy, GuardianType selected,
                        std::optional<GuardianType> hovered) const {
    renderer_.drawRect(camera, {157, 0}, {843, 147},
                       {0.10f, 0.07f, 0.13f, 0.80f});
    renderer_.drawRect(camera, {163, 10}, {177, 126},
                       {0.92f, 0.67f, 0.31f, 0.9f});
    renderer_.drawRect(camera, {166, 13}, {171, 120},
                       {0.20f, 0.13f, 0.17f, 0.97f});
    text_.draw(camera, "SUN COINS", {178, 23}, 18, gold, UIFont::Heading);
    renderer_.drawRect(camera, {179, 61}, {25, 25}, gold, 45.0f);
    renderer_.drawRect(camera, {185, 67}, {13, 13}, pale, 45.0f);
    text_.draw(camera, std::to_string(economy.balance), {220, 55}, 35, pale);
    text_.draw(camera, "PICKUPS  " + std::to_string(economy.pickupCount),
               {178, 102}, 15, gold);

    for (int i = 0; i < 3; ++i) {
        const GuardianType type = order[i];
        const CardState state = economy.cards[static_cast<int>(type)];
        const float x = firstCardX + i * cardStep;
        const bool isSelected = type == selected && state.available;
        const bool isHovered = hovered == type && state.available;
        const glm::vec4 border = state.available
                ? (isSelected || isHovered ? pale : gold) : gray;
        renderer_.drawRect(camera, {x, cardY}, {cardWidth, cardHeight}, border);
        renderer_.drawRect(camera, {x + 3, cardY + 3},
                           {cardWidth - 6, cardHeight - 6},
                           state.available
                                   ? glm::vec4{0.36f, 0.22f, 0.18f, 1.0f}
                                   : glm::vec4{0.23f, 0.22f, 0.23f, 1.0f});
        const glm::vec4 ink = state.available ? gold : gray;
        renderer_.drawRect(camera, {x + 8, 18}, {19, 19}, ink, 45.0f);
        text_.draw(camera, std::to_string(i + 1), {x + 13, 19}, 13,
                   state.available ? glm::vec4{0.22f, 0.13f, 0.13f, 1} :
                                     glm::vec4{0.12f, 0.12f, 0.12f, 1});
        if (type == GuardianType::Khopesh) {
            renderer_.drawRect(camera, {x + 58, 21}, {7, 31}, ink, 24.0f);
            renderer_.drawRect(camera, {x + 45, 18}, {22, 5}, ink, -18.0f);
            renderer_.drawRect(camera, {x + 44, 19}, {5, 12}, ink, 14.0f);
        } else if (type == GuardianType::Sentinel) {
            renderer_.drawRect(camera, {x + 46, 19}, {29, 34}, ink);
            renderer_.drawRect(camera, {x + 51, 24}, {19, 23},
                               {0.27f, 0.22f, 0.22f, 1});
            renderer_.drawRect(camera, {x + 58, 21}, {5, 28}, ink);
        } else {
            renderer_.drawRect(camera, {x + 49, 22}, {5, 31}, ink, -20.0f);
            renderer_.drawRect(camera, {x + 70, 22}, {5, 31}, ink, 20.0f);
            renderer_.drawRect(camera, {x + 48, 37}, {27, 3}, ink);
        }
        const char* first = type == GuardianType::Khopesh ? "KHOPESH" :
                            type == GuardianType::Sentinel ? "STONE" : "SUN";
        const char* second = type == GuardianType::Khopesh ? "GUARDIAN" :
                             type == GuardianType::Sentinel ? "SENTINEL" : "ARCHER";
        text_.draw(camera, first, {x + 9, 59}, 16, ink, UIFont::Heading);
        text_.draw(camera, second, {x + 9, 76}, 15, ink, UIFont::Heading);
        text_.draw(camera, std::to_string(guardianStats(type).cost) + " COINS",
                   {x + 9, 96}, 15, ink);
        const char* status = !state.eligible ? "LOCKED" :
                             !state.affordable ? "NEED COINS" :
                             isSelected ? "SELECTED" : "AVAILABLE";
        text_.draw(camera, status, {x + 9, 115}, 13,
                   state.available ? pale : gray);
    }
}
