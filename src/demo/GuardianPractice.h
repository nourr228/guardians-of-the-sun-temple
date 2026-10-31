#pragma once

#include <string>

#include "combat/CombatSystem.h"
#include "rendering/CharacterRenderer.h"

class Battlefield;
class Camera2D;
class InputManager;
class Renderer2D;

// Temporary Module 4 test harness. Real economy and enemies replace these ports.
class GuardianPractice final : public GuardianWallet {
public:
    GuardianPractice(const Battlefield& battlefield,
                     const Renderer2D& renderer);

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

    const Battlefield& battlefield_;
    const Renderer2D& renderer_;
    CharacterRenderer characters_;
    GuardianManager guardians_;
    EnemyManager enemies_;
    CombatSystem combat_;
    GuardianType selectedType_ = GuardianType::Khopesh;
    int hoveredLane_ = -1;
    int coins_ = 30;
    std::string message_ = "Click lane, 1/2/3 type, P place, T target";
};
