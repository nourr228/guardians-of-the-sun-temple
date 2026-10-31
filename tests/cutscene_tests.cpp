#include <cassert>
#include <cmath>
#include <string_view>

#include "cutscene/CutsceneManager.h"
#include "ui/MenuManager.h"

namespace {
bool near(float a, float b, float tolerance = 0.01f) {
    return std::abs(a - b) < tolerance;
}
}

int main() {
    CutsceneManager scene;
    assert(near(scene.time(), 0.0f));
    assert(scene.frame().scene == 1);
    assert(scene.actor(CutsceneRole::Guardian).visible);
    assert(!scene.actor(CutsceneRole::Pharaoh).visible);
    assert(!scene.complete());

    scene.seek(5.0f);
    assert(scene.frame().scene == 2);
    assert(scene.frame().storm > 0.0f);
    assert(scene.actor(CutsceneRole::Pharaoh).visible);
    assert(scene.actor(CutsceneRole::Pharaoh).feet.x < 1000.0f);
    assert(!scene.actor(CutsceneRole::Archer).visible);

    scene.seek(11.3f);
    assert(scene.frame().scene == 3);
    assert(scene.frame().curseVisible);
    assert(scene.frame().clashVisible);
    assert(scene.actor(CutsceneRole::Guardian).animator.state() ==
           AnimationState::Attacking);
    assert(scene.frame().speaker == std::string_view("KHOPESH GUARDIAN"));

    scene.seek(14.0f);
    assert(scene.frame().scene == 4);
    assert(scene.frame().arrowVisible);
    assert(scene.actor(CutsceneRole::Archer).visible);
    assert(scene.actor(CutsceneRole::Sentinel).visible);
    assert(scene.actor(CutsceneRole::Sentinel).animator.state() ==
           AnimationState::Attacking);

    for (float step : {1.0f / 30.0f, 1.0f / 60.0f, 1.0f / 144.0f}) {
        scene.start();
        for (int i = 0; i < 5000 && !scene.complete(); ++i)
            scene.update(step);
        assert(scene.complete());
        assert(near(scene.time(), 16.0f));
    }
    scene.start();
    scene.update(2.0f);
    scene.skip();
    assert(scene.complete());
    scene.start();
    assert(!scene.complete());
    assert(near(scene.time(), 0.0f));

    MenuManager menu;
    menu.activate(MenuAction::Start);
    assert(menu.screen() == GameScreen::Cutscene);
    menu.finishCutscene();
    assert(menu.isPlaying());
    menu.escape();
    menu.activate(MenuAction::Restart);
    assert(menu.isPlaying()); // Restart goes straight to gameplay.
}
