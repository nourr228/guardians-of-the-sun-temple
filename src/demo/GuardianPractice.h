#pragma once

#include <string>
#include <vector>

#include "rendering/CharacterRenderer.h"
#include "systems/GuardianManager.h"

class Battlefield;
class Camera2D;
class InputManager;
class Renderer2D;

// Temporary Module 4 test harness. Real economy and enemies replace these ports.
class GuardianPractice final : public GuardianWallet, public EnemyTargets {
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
    [[nodiscard]] std::vector<EnemyTarget> targets() const override;
    void applyDamage(std::uint64_t id, int damage) override;
    [[nodiscard]] std::string statusText() const;

private:
    struct Dummy {
        std::uint64_t id;
        int laneId;
        float x;
        int health;
    };

    [[nodiscard]] int inputLane() const;

    const Battlefield& battlefield_;
    const Renderer2D& renderer_;
    CharacterRenderer characters_;
    GuardianManager guardians_;
    std::vector<Dummy> dummies_;
    GuardianType selectedType_ = GuardianType::Khopesh;
    int hoveredLane_ = -1;
    int coins_ = 30;
    std::uint64_t nextDummyId_ = 1;
    std::string message_ = "Click lane, 1/2/3 type, P place, T target";
};
