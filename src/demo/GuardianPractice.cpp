#include "demo/GuardianPractice.h"

#include <algorithm>
#include <sstream>

#include <GLFW/glfw3.h>

#include "core/InputManager.h"
#include "rendering/Camera2D.h"
#include "rendering/Renderer2D.h"
#include "world/Battlefield.h"

GuardianPractice::GuardianPractice(const Battlefield& battlefield,
                                   const Renderer2D& renderer)
        : battlefield_(battlefield), renderer_(renderer),
          characters_(renderer), guardians_(battlefield),
          enemies_(battlefield), combat_(battlefield) {}

bool GuardianPractice::spend(int amount) {
    if (amount < 0 || amount > coins_) return false;
    coins_ -= amount;
    return true;
}

int GuardianPractice::inputLane() const {
    if (hoveredLane_ >= 0 && battlefield_.lane(hoveredLane_).selected)
        return hoveredLane_;
    const std::vector<int> selected = battlefield_.selectedLanes();
    return selected.size() == 1 ? selected.front() : -1;
}

void GuardianPractice::handleInput(const InputManager& input,
                                   const Camera2D& camera) {
    hoveredLane_ = battlefield_.laneAt(
            input.mouseWorldPosition(camera)).value_or(-1);
    if (input.keyPressed(GLFW_KEY_1)) selectedType_ = GuardianType::Khopesh;
    if (input.keyPressed(GLFW_KEY_2)) selectedType_ = GuardianType::Sentinel;
    if (input.keyPressed(GLFW_KEY_3)) selectedType_ = GuardianType::Archer;
    if (input.keyPressed(GLFW_KEY_R)) {
        reset();
        return;
    }
    if (input.keyPressed(GLFW_KEY_P)) {
        const PlacementResult result = guardians_.tryPlace(
                selectedType_, inputLane(), *this);
        if (result == PlacementResult::Placed)
            message_ = std::string(guardianName(selectedType_)) + " placed";
        else if (result == PlacementResult::InvalidLane)
            message_ = "Select a lane; hover it if several are selected";
        else
            message_ = "Insufficient Sun Coins";
    }
    if (input.keyPressed(GLFW_KEY_T)) {
        const int lane = inputLane();
        if (lane < 0) {
            message_ = "Select a lane for the target";
        } else {
            for (Enemy& enemy : enemies_.enemies())
                if (enemy.type == EnemyType::TrainingDummy &&
                    enemy.laneId == lane && enemy.alive)
                    enemies_.applyDamage(enemy.id, 100);
            const std::uint64_t id = enemies_.spawn(EnemyType::TrainingDummy, lane);
            for (Enemy& enemy : enemies_.enemies())
                if (enemy.id == id) enemy.position.x = 470.0f;
            message_ = "Training target placed in lane " +
                       std::to_string(lane + 1);
        }
    }
    if (input.keyPressed(GLFW_KEY_K)) {
        const int lane = inputLane();
        if (lane >= 0) {
            for (Enemy& enemy : enemies_.enemies())
                if (enemy.type == EnemyType::TrainingDummy &&
                    enemy.laneId == lane && enemy.alive)
                    enemies_.applyDamage(enemy.id, 100);
            message_ = "Training target removed";
        }
    }
    if (input.keyPressed(GLFW_KEY_H)) {
        const int lane = inputLane();
        if (lane >= 0) {
            for (Guardian& guardian : guardians_.guardians())
                if (guardian.laneId == lane && guardian.alive)
                    guardian.takeDamage(1);
            message_ = "Test damage applied to selected lane";
        }
    }
}

void GuardianPractice::update(float deltaTime, const InputManager& input) {
    GuardianControls controls;
    controls.selectedLanes = battlefield_.selectedLanes();
    controls.jumpPressed = input.keyPressed(GLFW_KEY_W) ||
                           input.keyPressed(GLFW_KEY_UP);
    controls.duckHeld = input.keyDown(GLFW_KEY_S) ||
                        input.keyDown(GLFW_KEY_DOWN);
    combat_.update(deltaTime, controls, guardians_, enemies_);
    static_cast<void>(combat_.takeEffectEvents());
}

void GuardianPractice::render(const Camera2D& camera) const {
    // The training target is a distinct marker, not a final enemy model.
    for (const Enemy& dummy : enemies_.enemies()) {
        if (!dummy.alive) continue;
        const float feet = battlefield_.lane(dummy.laneId).bottom - 9.0f;
        renderer_.drawRect(camera, {dummy.position.x - 19.0f, feet - 63.0f},
                           {38.0f, 63.0f}, {0.38f, 0.19f, 0.13f, 1.0f});
        renderer_.drawRect(camera, {dummy.position.x - 13.0f, feet - 57.0f},
                           {26.0f, 39.0f}, {0.77f, 0.52f, 0.29f, 1.0f});
        renderer_.drawRect(camera, {dummy.position.x - 17.0f, feet - 69.0f},
                           {34.0f, 5.0f}, {0.95f, 0.72f, 0.26f, 1.0f});
        renderer_.drawRect(camera, {dummy.position.x - 16.0f, feet - 68.0f},
                           {32.0f * dummy.healthRatio(), 3.0f},
                           {0.23f, 0.86f, 0.57f, 1.0f});
    }
    for (const Guardian& guardian : guardians_.guardians()) {
        if (!guardian.alive && guardian.animator.complete()) continue;
        const float feet = battlefield_.lane(guardian.laneId).bottom - 9.0f;
        characters_.draw(camera, guardianStats(guardian.type).visual,
                         guardian.animator, {guardian.position.x, feet},
                         1.0f, 0.94f, guardian.healthRatio());
        const float barX = guardian.position.x - 22.0f;
        renderer_.drawRect(camera, {barX, feet - 77.0f}, {44.0f, 6.0f},
                           {0.9f, 0.69f, 0.24f, 1.0f});
        renderer_.drawRect(camera, {barX + 1.0f, feet - 76.0f},
                           {42.0f, 4.0f}, {0.14f, 0.08f, 0.1f, 1.0f});
        renderer_.drawRect(camera, {barX + 1.0f, feet - 76.0f},
                           {42.0f * guardian.healthRatio(), 4.0f},
                           {0.33f, 0.86f, 0.49f, 1.0f});
    }
    for (const Projectile& arrow : combat_.projectiles()) {
        if (arrow.kind != AttackKind::SunArrow) continue;
        renderer_.drawRect(camera, {arrow.x - 11.0f, arrow.y - 2.0f},
                           {20.0f, 4.0f}, {1.0f, 0.9f, 0.41f, 1.0f});
        renderer_.drawRect(camera, {arrow.x + 5.0f, arrow.y - 4.0f},
                           {7.0f, 8.0f}, {1.0f, 0.96f, 0.67f, 1.0f});
    }
    for (const ImpactEffect& impact : combat_.impacts()) {
        const float progress = 1.0f - std::clamp(impact.remaining / 0.18f,
                                                  0.0f, 1.0f);
        const float side = 8.0f + progress * 10.0f;
        renderer_.drawRect(camera,
                {impact.x - side * 0.5f, impact.y - side * 0.5f},
                {side, side},
                {1.0f, 0.87f, 0.33f, 0.8f * (1.0f - progress)}, 45.0f);
    }
    // Simple card markers; the final card UI belongs to Module 7.
    constexpr glm::vec4 gold{0.91f, 0.67f, 0.23f, 1.0f};
    for (int i = 0; i < 3; ++i) {
        const GuardianType type = static_cast<GuardianType>(i);
        const float x = 385.0f + i * 78.0f;
        renderer_.drawRect(camera, {x, 13.0f}, {64.0f, 37.0f}, gold);
        renderer_.drawRect(camera, {x + 2.0f, 15.0f}, {60.0f, 33.0f},
                           selectedType_ == type
                                   ? glm::vec4(0.51f, 0.33f, 0.16f, 1.0f)
                                   : glm::vec4(0.18f, 0.13f, 0.14f, 1.0f));
        const int cost = guardianStats(type).cost;
        for (int coin = 0; coin < cost / 10; ++coin)
            renderer_.drawRect(camera,
                    {x + 11.0f + coin * 15.0f, 26.0f}, {10.0f, 10.0f},
                    coins_ >= cost ? gold : glm::vec4(0.35f, 0.34f, 0.32f, 1.0f));
    }
}

void GuardianPractice::reset() {
    guardians_.clear();
    enemies_.clear();
    combat_.reset();
    coins_ = 30;
    message_ = "Practice reset";
}

void GuardianPractice::openPreview() {
    reset();
    coins_ = 90;
    guardians_.tryPlace(GuardianType::Khopesh, 0, *this);
    guardians_.tryPlace(GuardianType::Sentinel, 1, *this);
    guardians_.tryPlace(GuardianType::Archer, 2, *this);
    for (int lane : {0, 2}) {
        const auto id = enemies_.spawn(EnemyType::TrainingDummy, lane);
        for (Enemy& enemy : enemies_.enemies())
            if (enemy.id == id) enemy.position.x = 470.0f;
    }
    message_ = "Module 4 preview";
}

std::string GuardianPractice::statusText() const {
    std::ostringstream out;
    out << "F4 Guardian Practice | Coins " << coins_ << " | "
        << guardianName(selectedType_) << " (" << guardianStats(selectedType_).cost
        << ") | " << message_
        << " | 1-3 type, P place, T target, K clear, W/Up jump, S/Down duck, H hit, R reset";
    return out.str();
}
