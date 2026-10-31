#pragma once

#include <string>

#include "combat/CombatSystem.h"
#include "economy/EconomyHud.h"
#include "economy/PurchaseSystem.h"
#include "effects/EffectManager.h"
#include "rendering/CharacterRenderer.h"
#include "systems/EnemyManager.h"
#include "systems/GuardianManager.h"
#include "systems/WaveManager.h"

class Camera2D;
class InputManager;
class Renderer2D;
class Battlefield;
class TextRenderer;
class AudioManager;

class WavePractice final {
public:
    WavePractice(const Battlefield& battlefield, const Renderer2D& renderer,
                 const TextRenderer& text, AudioManager& audio);

    void handleInput(const InputManager& input, const Camera2D& camera,
                     bool debugControls);
    void update(float deltaTime, const InputManager& input);
    void updatePresentation(float deltaTime);
    void render(const Camera2D& camera) const;
    void renderEffects(const Camera2D& camera) const;
    void reset();
    void openPreview();
    void openEffectPreview();
    void openOutcomePreview(bool victory);
    [[nodiscard]] bool uiContains(glm::vec2 world) const;
    [[nodiscard]] WavePhase phase() const { return waves_.phase(); }

    [[nodiscard]] std::string statusText(bool debugControls = false) const;

private:
    [[nodiscard]] int inputLane() const;
    void drawHealthBar(const Camera2D& camera, float x, float feetY,
                       float ratio, glm::vec4 fill) const;

    const Battlefield& battlefield_;
    const Renderer2D& renderer_;
    const TextRenderer& text_;
    AudioManager& audio_;
    CharacterRenderer characters_;
    GuardianManager guardians_;
    EnemyManager enemies_;
    CombatSystem combat_;
    EffectManager effects_;
    WaveManager waves_;
    EconomyManager economy_;
    PurchaseSystem purchases_;
    EconomyHud hud_;
    EconomySnapshot economyView_;
    GuardianType selectedType_ = GuardianType::Khopesh;
    std::optional<GuardianType> hoveredCard_;
    int hoveredLane_ = -1;
    int defeated_ = 0;
    bool shieldScenario_ = false;
    bool guardianAhead_ = false;
    std::string message_ = "Place guardians, then Enter to start";
};
