#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "rendering/Camera2D.h"
#include "world/Battlefield.h"
#include "world/HomeBase.h"

namespace {
void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

bool near(float a, float b) { return std::abs(a - b) < 0.01f; }
} // namespace

int main() {
    Battlefield field;
    check(field.lanes().size() == 4, "four lanes");
    for (int id = 0; id < Battlefield::laneCount; ++id) {
        const Lane& lane = field.lane(id);
        check(lane.id == id && near(lane.centerY, lane.top + 50),
              "lane identity and center");
        check(field.laneAt({400, lane.centerY}) == id, "lane hit test");
        check(near(field.guardianEntry(id).y, lane.centerY) &&
              near(field.enemyEntry(id).y, lane.centerY),
              "entries remain in original lane");
    }
    check(!field.laneAt({Battlefield::playableLeft - 1, 200}),
          "shrine cannot be selected");
    check(!field.laneAt({Battlefield::playableRight, 200}),
          "right boundary is outside lanes");

    field.updateHover({400, field.lane(0).centerY});
    check(field.lane(0).hovered && !field.lane(1).hovered,
          "hover only one lane");
    field.toggleSelectionAt({400, field.lane(0).centerY});
    field.toggleSelectionAt({400, field.lane(2).centerY});
    check(field.selectedLanes() == std::vector<int>({0, 2}),
          "multiple selected lanes persist");
    field.toggleSelectionAt({400, field.lane(0).centerY});
    check(field.selectedLanes() == std::vector<int>({2}),
          "click deselects only target lane");

    check(field.projectilePosition(1, 500, ProjectileLevel::High).y <
          field.projectilePosition(1, 500, ProjectileLevel::Middle).y,
          "high projectile is above middle");
    check(field.projectilePosition(1, 500, ProjectileLevel::Low).y >
          field.projectilePosition(1, 500, ProjectileLevel::Middle).y,
          "low projectile is below middle");
    check(field.collectiblePosition(3, 1200).x == Battlefield::playableRight,
          "collectible stays within playable area");
    HomeBase base;
    check(base.reachedBy(Battlefield::homeBoundary) &&
          !base.reachedBy(Battlefield::homeBoundary + 1),
          "home breach threshold");

    Camera2D camera(1000, 600);
    camera.setFramebufferSize(1600, 900);
    const ViewportRect view = camera.viewportRect();
    check(view.x == 50 && view.y == 0 &&
          view.width == 1500 && view.height == 900,
          "letterbox viewport");
    const glm::vec2 center = camera.windowToWorld({400, 225}, {800, 450});
    check(near(center.x, 500) && near(center.y, 300),
          "DPI-scaled cursor maps to center");
    const glm::vec2 leftBar = camera.windowToWorld({10, 225}, {800, 450});
    check(leftBar.x < 0 && !field.laneAt(leftBar),
          "letterbox bar cannot select a lane");

    std::cout << "World geometry tests passed\n";
}
