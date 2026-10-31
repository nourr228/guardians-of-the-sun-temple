#pragma once

#include <string>

#include "combat/CombatSystem.h"
#include "rendering/CharacterRenderer.h"
#include "systems/EnemyManager.h"
#include "systems/GuardianManager.h"
#include "systems/WaveManager.h"

class Camera2D;
class InputManager;
class Renderer2D;
class Battlefield;

// Temporary playable view for checking Module 5 before final menus/economy.
class WavePractice final : public GuardianWallet {
public:
    WavePractice(const Battlefield& battlefield, const Renderer2D& renderer);

    void handleInput(const InputManager& input, const Camera2D& camera);
    void update(float deltaTime, const InputManager& input);
    void render(const Camera2D& camera) const;
    void reset();
    void openPreview();

    [[nodiscard]] int balance() const override { return coins_; }
    bool spend(int amount) override;
    [[nodiscard]] std::string statusText() const;

private:
    [[nodiscard]] int inputLane() const;
    void drawHealthBar(const Camera2D& camera, float x, float feetY,
                       float ratio, glm::vec4 fill) const;

    const Battlefield& battlefield_;
    const Renderer2D& renderer_;
    CharacterRenderer characters_;
    GuardianManager guardians_;
    EnemyManager enemies_;
    CombatSystem combat_;
    WaveManager waves_;
    GuardianType selectedType_ = GuardianType::Khopesh;
    int hoveredLane_ = -1;
    int coins_ = 999;
    int defeated_ = 0;
    bool shieldScenario_ = false;
    bool guardianAhead_ = false;
    std::string message_ = "Place guardians, then Enter to start";
};
