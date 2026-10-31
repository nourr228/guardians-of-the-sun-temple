#include "demo/WavePractice.h"

#include <algorithm>
#include <sstream>

#include <GLFW/glfw3.h>

#include "core/InputManager.h"
#include "rendering/Camera2D.h"
#include "rendering/Renderer2D.h"
#include "ui/TextRenderer.h"
#include "world/Battlefield.h"

WavePractice::WavePractice(const Battlefield& battlefield,
                           const Renderer2D& renderer,
                           const TextRenderer& text)
        : battlefield_(battlefield), renderer_(renderer), text_(text),
          characters_(renderer), guardians_(battlefield),
          enemies_(battlefield), combat_(battlefield), hud_(renderer, text) {
    economy_.setOnChange([this](const EconomySnapshot& state) {
        economyView_ = state;
    });
}

bool WavePractice::uiContains(glm::vec2 world) const {
    return hud_.contains(world);
}

int WavePractice::inputLane() const {
    if (hoveredLane_ >= 0 && battlefield_.lane(hoveredLane_).selected)
        return hoveredLane_;
    const std::vector<int> selected = battlefield_.selectedLanes();
    return selected.size() == 1 ? selected.front() : -1;
}

void WavePractice::handleInput(const InputManager& input,
                               const Camera2D& camera,
                               bool debugControls) {
    const glm::vec2 mouse = input.mouseWorldPosition(camera);
    hoveredLane_ = battlefield_.laneAt(mouse).value_or(-1);
    hoveredCard_ = hud_.cardAt(mouse);
    if (debugControls && input.keyPressed(GLFW_KEY_R)) {
        reset();
        return;
    }
    if (waves_.phase() == WavePhase::Victory ||
        waves_.phase() == WavePhase::Defeat) return;
    auto selectCard = [this](GuardianType type) {
        if (economy_.cardState(type).available) {
            selectedType_ = type;
            message_ = std::string(guardianName(type)) + " selected";
        } else {
            message_ = std::string(guardianName(type)) + " card unavailable";
        }
    };
    if (input.mousePressed(GLFW_MOUSE_BUTTON_LEFT) && hoveredCard_ &&
        economy_.cardState(*hoveredCard_).available)
        selectCard(*hoveredCard_);
    if (input.keyPressed(GLFW_KEY_1)) selectCard(GuardianType::Khopesh);
    if (input.keyPressed(GLFW_KEY_2)) selectCard(GuardianType::Sentinel);
    if (input.keyPressed(GLFW_KEY_3)) selectCard(GuardianType::Archer);
    if (debugControls && input.keyPressed(GLFW_KEY_T) &&
        waves_.phase() == WavePhase::Ready) {
        if (!guardians_.guardians().empty() || !enemies_.enemies().empty()) {
            message_ = "Press R, then T for the shield test";
        } else {
            guardians_.guardians().emplace_back(900001,
                    GuardianType::Khopesh, 0, battlefield_.guardianEntry(0));
            guardians_.guardians().emplace_back(900002,
                    GuardianType::Sentinel, 0, battlefield_.defenderAnchor(0));
            guardians_.guardians()[0].speed = 0.0f;
            const auto id = enemies_.spawn(EnemyType::Scarab, 0);
            for (Enemy& enemy : enemies_.enemies())
                if (enemy.id == id) enemy.position.x = 310.0f;
            shieldScenario_ = true;
            message_ = "Shield test: Y move guardian ahead, H break shield";
        }
    }
    if (debugControls && shieldScenario_ && input.keyPressed(GLFW_KEY_Y)) {
        guardianAhead_ = !guardianAhead_;
        for (Guardian& guardian : guardians_.guardians())
            if (guardian.type == GuardianType::Khopesh)
                guardian.position.x = guardianAhead_ ? 255.0f : 174.0f;
        message_ = guardianAhead_ ? "Guardian ahead of shield" :
                                  "Guardian behind shield";
    }
    if (debugControls && shieldScenario_ && input.keyPressed(GLFW_KEY_H)) {
        for (Guardian& guardian : guardians_.guardians())
            if (guardian.type == GuardianType::Sentinel && guardian.alive)
                guardian.takeDamage(100);
        message_ = "Shield destroyed; guardian is exposed";
    }
    if (input.keyPressed(GLFW_KEY_P)) {
        const PurchaseResult result = purchases_.tryPurchase(
                selectedType_, inputLane(), !shieldScenario_,
                guardians_, economy_);
        switch (result) {
        case PurchaseResult::Purchased:
            message_ = std::string(guardianName(selectedType_)) + " purchased";
            break;
        case PurchaseResult::InvalidLane:
            message_ = "Select a lane; hover it if several are selected";
            break;
        case PurchaseResult::Locked:
            message_ = "Collect more Sun Coins to activate this card";
            break;
        case PurchaseResult::InsufficientCoins:
            message_ = "Not enough spendable Sun Coins";
            break;
        case PurchaseResult::NotPlaying:
            message_ = "Reset the shield test before purchasing";
            break;
        }
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
    if (debugControls && input.keyPressed(GLFW_KEY_K) &&
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
    int credited = 0;
    for (const CoinPickup& pickup : combat_.takePickups())
        if (economy_.creditPickup(pickup)) ++credited;
    if (credited > 0)
        message_ = std::to_string(credited * EconomyManager::rewardPerPickup) +
                   " Sun Coins collected";
    if (!economy_.cardState(selectedType_).available &&
        economy_.cardState(GuardianType::Khopesh).available)
        selectedType_ = GuardianType::Khopesh;
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
    hud_.render(camera, economyView_, selectedType_, hoveredCard_);
    renderer_.drawRect(camera, {348, 10}, {226, 126},
                       {0.92f, 0.67f, 0.31f, 0.9f});
    renderer_.drawRect(camera, {351, 13}, {220, 120},
                       {0.20f, 0.13f, 0.17f, 0.97f});
    const char* phase = "PREPARE";
    switch (waves_.phase()) {
    case WavePhase::Ready: phase = "PREPARE"; break;
    case WavePhase::Spawning: phase = "SPAWNING"; break;
    case WavePhase::Clearing: phase = "CLEARING"; break;
    case WavePhase::Victory: phase = "COMPLETE"; break;
    case WavePhase::Defeat: phase = "BREACHED"; break;
    }
    text_.draw(camera, "WAVE " + std::to_string(waves_.waveNumber()) + " / 3",
               {365, 22}, 21, {1.0f, 0.85f, 0.43f, 1}, UIFont::Heading);
    text_.draw(camera, phase, {365, 53}, 15,
               {1.0f, 0.93f, 0.76f, 1});
    const int total = waves_.phase() == WavePhase::Ready
            ? 3 : waves_.waveTotal();
    text_.draw(camera, std::to_string(waves_.spawnedCount()) + " / " +
               std::to_string(total) + " SPAWNED",
               {365, 77}, 17, {1.0f, 0.93f, 0.76f, 1});
    text_.draw(camera, std::to_string(enemies_.livingCount()) + " ENEMIES LEFT",
               {365, 100}, 16, {1.0f, 0.93f, 0.76f, 1});
    renderer_.drawRect(camera, {158, 562}, {842, 38},
                       {0.12f, 0.07f, 0.12f, 0.88f});
    const std::string footer = waves_.phase() == WavePhase::Ready
            ? "Click a lane, choose a card, P to place, Enter to begin"
            : message_;
    text_.draw(camera, footer, {173, 570}, 17,
               {1.0f, 0.88f, 0.55f, 1});
}

void WavePractice::reset() {
    guardians_.clear();
    enemies_.clear();
    combat_.reset();
    waves_.reset();
    economy_.reset();
    selectedType_ = GuardianType::Khopesh;
    hoveredCard_.reset();
    defeated_ = 0;
    shieldScenario_ = false;
    guardianAhead_ = false;
    message_ = "Place guardians, then Enter to start";
}

void WavePractice::openPreview() {
    reset();
    economy_.creditPickup({900001, 0});
    economy_.creditPickup({900002, 0});
    economy_.creditPickup({900003, 0});
    guardians_.guardians().emplace_back(900001,
            GuardianType::Khopesh, 0, battlefield_.guardianEntry(0));
    guardians_.guardians().emplace_back(900002,
            GuardianType::Sentinel, 1, battlefield_.defenderAnchor(1));
    guardians_.guardians().emplace_back(900003,
            GuardianType::Archer, 2, battlefield_.guardianEntry(2));
    enemies_.spawn(EnemyType::Scarab, 0);
    enemies_.spawn(EnemyType::Serpent, 1);
    enemies_.spawn(EnemyType::Pharaoh, 2);
    for (Enemy& enemy : enemies_.enemies())
        enemy.position.x = 820.0f;
    combat_.update(0.1f, {}, guardians_, enemies_);
    message_ = "Module 5 visual preview";
}

std::string WavePractice::statusText(bool debugControls) const {
    const char* phaseName = "Ready";
    switch (waves_.phase()) {
    case WavePhase::Ready: phaseName = "Ready"; break;
    case WavePhase::Spawning: phaseName = "Spawning"; break;
    case WavePhase::Clearing: phaseName = "Clearing"; break;
    case WavePhase::Victory: phaseName = "Victory"; break;
    case WavePhase::Defeat: phaseName = "Defeat"; break;
    }
    std::ostringstream out;
    out << "Guardians of the Sun Temple | Wave " << waves_.waveNumber() << "/3 "
        << phaseName << " " << waves_.spawnedCount() << "/"
        << waves_.waveTotal() << " spawned, " << enemies_.livingCount()
        << " alive | Sun Coins " << economy_.balance()
        << " | Pickups " << economy_.pickupCount() << " | " << message_;
    if (debugControls)
        out << " | Debug: K clear, T shield test, R reset";
    return out.str();
}
