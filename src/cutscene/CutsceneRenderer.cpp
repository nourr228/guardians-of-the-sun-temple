#include "cutscene/CutsceneRenderer.h"

#include <algorithm>
#include <cmath>

#include <glm/vec4.hpp>

#include "cutscene/CutsceneManager.h"
#include "rendering/Camera2D.h"
#include "rendering/Renderer2D.h"
#include "ui/TextRenderer.h"

namespace {
constexpr glm::vec4 gold{1.0f, 0.79f, 0.28f, 1.0f};
constexpr glm::vec4 pale{1.0f, 0.94f, 0.78f, 1.0f};
}

void CutsceneRenderer::render(const Camera2D& camera,
                              const CutsceneManager& cutscene) const {
    const CutsceneFrame frame = cutscene.frame();
    const float t = frame.time;

    // The familiar temple remains visible while a foreground stage focuses the action.
    renderer_.drawRect(camera, {0, 0}, {1000, 600},
                       {0.10f, 0.04f, 0.16f, frame.darkness});
    renderer_.drawRect(camera, {157, 252}, {843, 273},
                       {0.12f, 0.08f, 0.15f, 0.43f});
    renderer_.drawRect(camera, {157, 470 + frame.shake}, {843, 52},
                       {0.53f, 0.35f, 0.24f, 0.96f});
    renderer_.drawRect(camera, {157, 470 + frame.shake}, {843, 5},
                       {0.96f, 0.69f, 0.36f, 0.95f});
    for (int i = 0; i < 9; ++i) {
        const float x = 180.0f + i * 95.0f;
        renderer_.drawRect(camera, {x, 487 + frame.shake}, {34, 2},
                           {0.27f, 0.15f, 0.16f, 0.45f});
        if (frame.scene >= 2 && i % 3 == 0)
            renderer_.drawRect(camera, {x + 22, 473 + frame.shake}, {3, 17},
                               {0.18f, 0.10f, 0.15f, 0.8f}, -25.0f);
    }

    const float pulse = 1.0f + 0.11f * std::sin(t * 4.2f);
    renderer_.drawRect(camera,
                       {77.0f - 51.0f * pulse, 342.0f - 51.0f * pulse},
                       {102.0f * pulse, 102.0f * pulse},
                       {1.0f, 0.75f, 0.24f, frame.crystalGlow * 0.35f}, 45.0f);
    renderer_.drawRect(camera, {61, 326}, {32, 32},
                       {1.0f, 0.90f, 0.43f, frame.crystalGlow * 0.42f}, 45.0f);

    if (frame.storm > 0.01f) {
        renderer_.drawRect(camera, {660, 235}, {340, 287},
                           {0.47f, 0.25f, 0.26f, 0.18f * frame.storm});
        for (int i = 0; i < 45; ++i) {
            const float x = 650.0f + std::fmod(i * 83.0f + t * (33 + i % 5 * 9),
                                                350.0f);
            const float y = 284.0f + i % 8 * 24.0f +
                            14.0f * std::sin(t * 2.8f + i * 1.93f);
            const float side = 3.0f + i % 4 * 2.0f;
            renderer_.drawRect(camera, {x, y}, {side, side},
                               {0.94f, 0.70f, 0.42f, 0.35f * frame.storm},
                               45.0f);
        }
    }
    if (frame.scene >= 2)
        renderer_.drawRect(camera, {632, 361}, {95, 95},
                           {0.53f, 0.16f, 0.68f, 0.13f * frame.storm}, 45.0f);

    constexpr CutsceneRole drawOrder[]{CutsceneRole::Sentinel,
                                       CutsceneRole::Archer,
                                       CutsceneRole::Guardian,
                                       CutsceneRole::Pharaoh,
                                       CutsceneRole::ScarabA,
                                       CutsceneRole::ScarabB};
    for (CutsceneRole role : drawOrder) {
        const CutsceneActor& actor = cutscene.actor(role);
        if (actor.visible)
            characters_.draw(camera, actor.type, actor.animator,
                             actor.feet + glm::vec2{0.0f, frame.shake},
                             actor.facing, actor.scale);
    }

    if (frame.curseVisible) {
        renderer_.drawRect(camera,
                           frame.cursePosition - glm::vec2{30, 7}, {60, 14},
                           {0.55f, 0.19f, 0.81f, 0.34f});
        renderer_.drawRect(camera,
                           frame.cursePosition - glm::vec2{17, 3}, {34, 6},
                           {0.91f, 0.56f, 1.0f, 0.95f});
        renderer_.drawRect(camera,
                           frame.cursePosition - glm::vec2{8, 8}, {16, 16},
                           {0.61f, 0.32f, 0.87f, 0.9f}, 45.0f);
    }
    if (frame.clashVisible) {
        const float size = 27.0f + 20.0f *
                std::sin((t - 11.24f) / 0.61f * 3.14159f);
        renderer_.drawRect(camera, {438 - size * 0.5f, 404 - size * 0.5f},
                           {size, size},
                           {1.0f, 0.87f, 0.30f, 0.65f}, 45.0f);
        renderer_.drawRect(camera, {427, 393}, {22, 22}, pale, 45.0f);
    }
    if (frame.arrowVisible) {
        renderer_.drawRect(camera,
                           frame.arrowPosition - glm::vec2{33, 8}, {68, 16},
                           {1.0f, 0.77f, 0.24f, 0.24f});
        renderer_.drawRect(camera,
                           frame.arrowPosition - glm::vec2{23, 2}, {44, 4},
                           pale);
        renderer_.drawRect(camera,
                           frame.arrowPosition + glm::vec2{15, -5}, {12, 10},
                           gold, 45.0f);
    }
    if (frame.scene == 4) {
        const float shieldGlow = std::clamp((t - 12.7f) / 0.8f, 0.0f, 1.0f);
        renderer_.drawRect(camera, {197, 383}, {103, 103},
                           {1.0f, 0.81f, 0.35f, 0.16f * shieldGlow}, 45.0f);
    }

    renderer_.drawRect(camera, {0, 0}, {1000, 62},
                       {0.09f, 0.05f, 0.12f, 0.93f});
    renderer_.drawRect(camera, {0, 522}, {1000, 78},
                       {0.09f, 0.05f, 0.12f, 0.94f});
    renderer_.drawRect(camera, {0, 61}, {1000, 3}, gold);
    renderer_.drawRect(camera, {0, 521}, {1000, 3}, gold);
    text_.draw(camera, "THE AWAKENING OF THE CURSE", {35, 17}, 27,
               gold, UIFont::Heading);
    text_.draw(camera, "SPACE  SKIP", {852, 25}, 15, pale);
    if (!frame.dialogue.empty()) {
        text_.draw(camera, frame.speaker, {62, 530}, 14,
                   {1.0f, 0.78f, 0.37f, 1}, UIFont::Heading);
        text_.draw(camera, frame.dialogue, {62, 552}, 20, pale);
    }
    if (frame.transition > 0.0f)
        renderer_.drawRect(camera, {0, 0}, {1000, 600},
                           {1.0f, 0.84f, 0.36f, frame.transition});
}
