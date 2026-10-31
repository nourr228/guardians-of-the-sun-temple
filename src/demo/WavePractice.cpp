#include "demo/WavePractice.h"

#include <algorithm>
#include <sstream>

#include <GLFW/glfw3.h>

#include "core/InputManager.h"
#include "rendering/Camera2D.h"
#include "rendering/Renderer2D.h"
#include "world/Battlefield.h"

WavePractice::WavePractice(const Battlefield& battlefield,
                           const Renderer2D& renderer)
        : battlefield_(battlefield), renderer_(renderer),
          characters_(renderer), guardians_(battlefield),
          enemies_(battlefield), combat_(battlefield) {}

bool WavePractice::spend(int amount) {
    if (amount < 0 || amount > coins_) return false;
    coins_ -= amount;
    return true;
}

int WavePractice::inputLane() const {
    if (hoveredLane_ >= 0 && battlefield_.lane(hoveredLane_).selected)
        return hoveredLane_;
    const std::vector<int> selected = battlefield_.selectedLanes();
    return selected.size() == 1 ? selected.front() : -1;
}

void WavePractice::handleInput(const InputManager& input,
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
    if (waves_.phase() == WavePhase::Victory ||
        waves_.phase() == WavePhase::Defeat) return;
    if (input.keyPressed(GLFW_KEY_T) && waves_.phase() == WavePhase::Ready) {
        if (!guardians_.guardians().empty() || !enemies_.enemies().empty()) {
            message_ = "Press R, then T for the shield test";
        } else {
            guardians_.tryPlace(GuardianType::Khopesh, 0, *this);
            guardians_.tryPlace(GuardianType::Sentinel, 0, *this);
            guardians_.guardians()[0].speed = 0.0f;
            const auto id = enemies_.spawn(EnemyType::Scarab, 0);
            for (Enemy& enemy : enemies_.enemies())
                if (enemy.id == id) enemy.position.x = 310.0f;
            shieldScenario_ = true;
            message_ = "Shield test: Y move guardian ahead, H break shield";
        }
    }
    if (shieldScenario_ && input.keyPressed(GLFW_KEY_Y)) {
        guardianAhead_ = !guardianAhead_;
        for (Guardian& guardian : guardians_.guardians())
            if (guardian.type == GuardianType::Khopesh)
                guardian.position.x = guardianAhead_ ? 255.0f : 174.0f;
        message_ = guardianAhead_ ? "Guardian ahead of shield" :
                                  "Guardian behind shield";
    }
    if (shieldScenario_ && input.keyPressed(GLFW_KEY_H)) {
        for (Guardian& guardian : guardians_.guardians())
            if (guardian.type == GuardianType::Sentinel && guardian.alive)
                guardian.takeDamage(100);
        message_ = "Shield destroyed; guardian is exposed";
    }
    if (input.keyPressed(GLFW_KEY_P)) {
        const PlacementResult result = guardians_.tryPlace(
                selectedType_, inputLane(), *this);
        if (result == PlacementResult::Placed)
            message_ = std::string(guardianName(selectedType_)) + " placed";
        else if (result == PlacementResult::InvalidLane)
            message_ = "Select a lane; hover it if several are selected";
        else
            message_ = "Insufficient practice coins";
    }
    if (input.keyPressed(GLFW_KEY_ENTER) ||
        input.keyPressed(GLFW_KEY_KP_ENTER)) {
        if (waves_.phase() == WavePhase::Ready) {
            if (shieldScenario_) {
                message_ = "Press R to reset before starting waves";
            } else {
                waves_.start(enemies_);
                message_ = "Wave 1 started";
            }
        }
    }
    if (input.keyPressed(GLFW_KEY_K) &&
        waves_.phase() != WavePhase::Ready &&
        waves_.phase() != WavePhase::Victory &&
        waves_.phase() != WavePhase::Defeat) {
        for (const Enemy& enemy : enemies_.enemies())
            if (enemy.alive) enemies_.applyDamage(enemy.id, 100);
        message_ = "Active enemies defeated for wave testing";
    }
}

void WavePractice::update(float deltaTime, const InputManager& input) {
    const WavePhase phase = waves_.phase();
    if (phase == WavePhase::Victory || phase == WavePhase::Defeat) return;
    waves_.update(deltaTime, enemies_);
    GuardianControls controls;
    controls.selectedLanes = battlefield_.selectedLanes();
    controls.jumpPressed = input.keyPressed(GLFW_KEY_W) ||
                           input.keyPressed(GLFW_KEY_UP);
    controls.duckHeld = input.keyDown(GLFW_KEY_S) ||
                        input.keyDown(GLFW_KEY_DOWN);
    combat_.update(deltaTime, controls, guardians_, enemies_);
    defeated_ = combat_.defeatedCount();
    if (enemies_.breachedHome()) {
        waves_.markDefeat();
        message_ = "Sun Crystal captured; R to restart";
    } else if (waves_.phase() == WavePhase::Victory) {
        message_ = "All three waves cleared; R to restart";
    }
}

void WavePractice::drawHealthBar(const Camera2D& camera, float x,
                                 float feetY, float ratio,
                                 glm::vec4 fill) const {
    const float left = x - 22.0f;
    renderer_.drawRect(camera, {left, feetY - 77.0f}, {44.0f, 6.0f},
                       {0.9f, 0.69f, 0.24f, 1.0f});
    renderer_.drawRect(camera, {left + 1.0f, feetY - 76.0f},
                       {42.0f, 4.0f}, {0.14f, 0.08f, 0.1f, 1.0f});
    renderer_.drawRect(camera, {left + 1.0f, feetY - 76.0f},
                       {42.0f * std::clamp(ratio, 0.0f, 1.0f), 4.0f}, fill);
}

void WavePractice::render(const Camera2D& camera) const {
    for (const Guardian& guardian : guardians_.guardians()) {
        if (!guardian.alive && guardian.animator.complete()) continue;
        const float feet = battlefield_.lane(guardian.laneId).bottom - 9.0f;
        characters_.draw(camera, guardianStats(guardian.type).visual,
                         guardian.animator, {guardian.position.x, feet},
                         1.0f, 0.94f, guardian.healthRatio());
        drawHealthBar(camera, guardian.position.x, feet,
                      guardian.healthRatio(), {0.33f, 0.86f, 0.49f, 1.0f});
    }
    for (const Enemy& enemy : enemies_.enemies()) {
        if (!enemy.alive && enemy.animator.complete()) continue;
        const float feet = battlefield_.lane(enemy.laneId).bottom - 9.0f;
        characters_.draw(camera, enemyStats(enemy.type).visual,
                         enemy.animator, {enemy.position.x, feet},
                         -1.0f, 0.94f, enemy.healthRatio());
        drawHealthBar(camera, enemy.position.x, feet,
                      enemy.healthRatio(), {0.78f, 0.31f, 0.49f, 1.0f});
    }
    for (const Projectile& arrow : combat_.projectiles()) {
        if (arrow.kind != AttackKind::SunArrow) continue;
        renderer_.drawRect(camera, {arrow.x - 11.0f, arrow.y - 2.0f},
                           {20.0f, 4.0f}, {1.0f, 0.9f, 0.41f, 1.0f});
        renderer_.drawRect(camera, {arrow.x + 5.0f, arrow.y - 4.0f},
                           {7.0f, 8.0f}, {1.0f, 0.96f, 0.67f, 1.0f});
    }
    for (const Projectile& projectile : combat_.projectiles()) {
        if (projectile.kind == AttackKind::SunArrow) continue;
        const glm::vec4 glow = projectile.kind == AttackKind::ScarabCurse
                ? glm::vec4(0.63f, 0.29f, 0.82f, 0.7f)
                : glm::vec4(0.18f, 0.82f, 0.77f, 0.7f);
        renderer_.drawRect(camera,
                           {projectile.x - 9.0f, projectile.y - 5.0f},
                           {18.0f, 10.0f}, glow);
        renderer_.drawRect(camera,
                           {projectile.x - 5.0f, projectile.y - 3.0f},
                           {10.0f, 6.0f},
                           {0.92f, 0.8f, 1.0f, 0.9f});
    }
    for (const SunCoin& coin : combat_.coins()) {
        renderer_.drawRect(camera, {coin.x - 7.0f, coin.y - 7.0f},
                           {14.0f, 14.0f}, {0.98f, 0.78f, 0.27f, 1.0f}, 45.0f);
        renderer_.drawRect(camera, {coin.x - 3.0f, coin.y - 3.0f},
                           {6.0f, 6.0f}, {1.0f, 0.95f, 0.56f, 1.0f}, 45.0f);
    }
    for (const ImpactEffect& impact : combat_.impacts()) {
        const float progress = 1.0f - std::clamp(impact.remaining / 0.18f,
                                                  0.0f, 1.0f);
        const float side = 8.0f + progress * 10.0f;
        const glm::vec4 color = impact.kind == AttackKind::SunArrow ||
                impact.kind == AttackKind::KhopeshStrike
                ? glm::vec4(1.0f, 0.87f, 0.33f, 0.8f * (1.0f - progress))
                : glm::vec4(0.55f, 0.29f, 0.74f, 0.8f * (1.0f - progress));
        renderer_.drawRect(camera,
                           {impact.x - side * 0.5f, impact.y - side * 0.5f},
                           {side, side}, color, 45.0f);
    }
    // Three top markers indicate wave progress; final HUD comes later.
    for (int i = 0; i < 3; ++i) {
        const bool reached = i + 1 <= waves_.waveNumber() &&
                             waves_.phase() != WavePhase::Ready;
        renderer_.drawRect(camera, {445.0f + i * 38.0f, 16.0f},
                           {28.0f, 16.0f},
                           reached ? glm::vec4(1.0f, 0.76f, 0.3f, 1.0f)
                                   : glm::vec4(0.43f, 0.31f, 0.27f, 1.0f));
    }
    if (waves_.phase() == WavePhase::Victory ||
        waves_.phase() == WavePhase::Defeat) {
        const bool victory = waves_.phase() == WavePhase::Victory;
        renderer_.drawRect(camera, {0.0f, 0.0f},
                           {Battlefield::worldWidth, Battlefield::worldHeight},
                           victory ? glm::vec4(0.85f, 0.53f, 0.12f, 0.24f)
                                   : glm::vec4(0.15f, 0.02f, 0.19f, 0.5f));
        renderer_.drawRect(camera, {485.0f, 266.0f}, {30.0f, 68.0f},
                           victory ? glm::vec4(1.0f, 0.86f, 0.42f, 1.0f)
                                   : glm::vec4(0.52f, 0.19f, 0.67f, 1.0f));
    }
}

void WavePractice::reset() {
    guardians_.clear();
    enemies_.clear();
    combat_.reset();
    waves_.reset();
    selectedType_ = GuardianType::Khopesh;
    coins_ = 999;
    defeated_ = 0;
    shieldScenario_ = false;
    guardianAhead_ = false;
    message_ = "Place guardians, then Enter to start";
}

void WavePractice::openPreview() {
    reset();
    guardians_.tryPlace(GuardianType::Khopesh, 0, *this);
    guardians_.tryPlace(GuardianType::Sentinel, 1, *this);
    guardians_.tryPlace(GuardianType::Archer, 2, *this);
    enemies_.spawn(EnemyType::Scarab, 0);
    enemies_.spawn(EnemyType::Serpent, 1);
    enemies_.spawn(EnemyType::Pharaoh, 2);
    for (Enemy& enemy : enemies_.enemies())
        enemy.position.x = 820.0f;
    combat_.update(0.1f, {}, guardians_, enemies_);
    message_ = "Module 5 visual preview";
}

std::string WavePractice::statusText() const {
    const char* phaseName = "Ready";
    switch (waves_.phase()) {
    case WavePhase::Ready: phaseName = "Ready"; break;
    case WavePhase::Spawning: phaseName = "Spawning"; break;
    case WavePhase::Clearing: phaseName = "Clearing"; break;
    case WavePhase::Victory: phaseName = "Victory"; break;
    case WavePhase::Defeat: phaseName = "Defeat"; break;
    }
    std::ostringstream out;
    out << "F5 Wave Practice | Wave " << waves_.waveNumber() << "/3 "
        << phaseName << " " << waves_.spawnedCount() << "/"
        << waves_.waveTotal() << " spawned, " << enemies_.livingCount()
        << " alive | Defeated " << defeated_
        << " | Picked up " << combat_.collectedCount() << " | " << message_
        << " | Enter start, 1-3 type, P place, K clear, T shield test, R reset";
    return out.str();
}
