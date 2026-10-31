#include "ui/MenuRenderer.h"

#include <glm/vec4.hpp>

#include "rendering/Camera2D.h"
#include "rendering/Renderer2D.h"
#include "ui/TextRenderer.h"
#include "world/Battlefield.h"

namespace {
constexpr glm::vec4 gold{1.0f, 0.82f, 0.38f, 1.0f};
constexpr glm::vec4 pale{1.0f, 0.94f, 0.77f, 1.0f};
}

void MenuRenderer::render(const Camera2D& camera, const MenuManager& menu,
                          glm::vec2 mouse) const {
    const GameScreen screen = menu.screen();
    if (screen == GameScreen::Playing || screen == GameScreen::Cutscene) return;
    const bool main = screen == GameScreen::MainMenu;
    const bool victory = screen == GameScreen::Victory;
    const bool defeat = screen == GameScreen::Defeat;
    renderer_.drawRect(camera, {0, 0},
                       {Battlefield::worldWidth, Battlefield::worldHeight},
                       defeat ? glm::vec4{0.12f, 0.02f, 0.17f, 0.75f}
                              : glm::vec4{0.08f, 0.04f, 0.12f,
                                          main ? 0.68f : 0.64f});
    if (victory || defeat) {
        const glm::vec4 glow = victory
                ? glm::vec4{1.0f, 0.76f, 0.19f, 0.43f}
                : glm::vec4{0.39f, 0.12f, 0.58f, 0.55f};
        renderer_.drawRect(camera, {20, 285}, {115, 115}, glow, 45.0f);
        renderer_.drawRect(camera, {48, 313}, {59, 59},
                           victory ? glm::vec4{1.0f, 0.94f, 0.48f, 0.94f}
                                   : glm::vec4{0.32f, 0.14f, 0.43f, 0.96f},
                           45.0f);
        if (defeat) {
            renderer_.drawRect(camera, {71, 313}, {4, 56},
                               {0.06f, 0.03f, 0.1f, 1}, 21.0f);
            renderer_.drawRect(camera, {89, 336}, {4, 27},
                               {0.06f, 0.03f, 0.1f, 1}, -33.0f);
        }
    }
    const glm::vec2 panel = main ? glm::vec2{190, 79} : glm::vec2{260, 121};
    const glm::vec2 size = main ? glm::vec2{620, 441} : glm::vec2{480, 398};
    renderer_.drawRect(camera, panel, size, {0.92f, 0.67f, 0.30f, 0.94f});
    renderer_.drawRect(camera, panel + glm::vec2{4, 4},
                       size - glm::vec2{8, 8},
                       {0.14f, 0.09f, 0.15f, 0.97f});
    renderer_.drawRect(camera, panel + glm::vec2{13, 13},
                       size - glm::vec2{26, 26},
                       {0.28f, 0.18f, 0.20f, 0.72f});
    auto centered = [&](std::string_view label, float y, float height,
                        glm::vec4 color, UIFont font) {
        text_.draw(camera, label,
                   {(1000.0f - text_.width(label, height, font)) * 0.5f, y},
                   height, color, font);
    };
    if (main) {
        centered("GUARDIANS OF THE", 139, 34, gold, UIFont::Heading);
        centered("SUN TEMPLE", 184, 51, pale, UIFont::Heading);
        centered("Defend the Heart of the Sun", 256, 23, pale, UIFont::Body);
    } else if (screen == GameScreen::Paused) {
        centered("PAUSED", 170, 46, gold, UIFont::Heading);
        centered("The temple awaits your command", 230, 18, pale,
                 UIFont::Body);
    } else if (victory) {
        centered("VICTORY", 162, 48, gold, UIFont::Heading);
        centered("The Sun Crystal shines again", 237, 20, pale,
                 UIFont::Body);
    } else {
        centered("DEFEAT", 162, 48,
                 {0.82f, 0.53f, 0.93f, 1}, UIFont::Heading);
        centered("The crystal has been captured", 237, 20, pale,
                 UIFont::Body);
    }
    for (const auto& button : menu.buttons()) {
        if (!button) continue;
        const bool hovered = button->contains(mouse);
        renderer_.drawRect(camera, button->position, button->size,
                           hovered ? pale : gold);
        renderer_.drawRect(camera, button->position + glm::vec2{3, 3},
                           button->size - glm::vec2{6, 6},
                           hovered ? glm::vec4{0.48f, 0.30f, 0.19f, 1}
                                   : glm::vec4{0.28f, 0.17f, 0.18f, 1});
        text_.draw(camera, button->label,
                   {button->position.x + (button->size.x -
                    text_.width(button->label, 22, UIFont::Heading)) * 0.5f,
                    button->position.y + 14},
                   22, hovered ? pale : gold, UIFont::Heading);
    }
    if (screen == GameScreen::Paused)
        centered("ESC  CONTINUE", 482, 14, pale, UIFont::Body);
}
