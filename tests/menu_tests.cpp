#include <cassert>

#include "ui/MenuManager.h"

int main() {
    MenuManager menu;
    assert(menu.screen() == GameScreen::MainMenu);
    assert(!menu.buttonAt({100, 250}));
    assert(menu.buttonAt({500, 350})->action == MenuAction::Start);
    menu.activate(MenuAction::SoundSettings);
    assert(menu.soundSettingsOpen());
    assert(menu.buttonAt({430, 335})->action == MenuAction::VolumeDown);
    assert(menu.buttonAt({550, 335})->action == MenuAction::VolumeUp);
    assert(menu.buttonAt({500, 400})->action == MenuAction::ToggleMute);
    menu.escape();
    assert(!menu.soundSettingsOpen());
    menu.escape();
    assert(menu.screen() == GameScreen::MainMenu);
    menu.activate(MenuAction::Start);
    assert(menu.screen() == GameScreen::Cutscene);
    assert(!menu.buttonAt({500, 350}));
    menu.escape();
    assert(menu.screen() == GameScreen::MainMenu);
    menu.activate(MenuAction::Start);
    menu.finishCutscene();
    assert(menu.isPlaying());
    assert(!menu.buttonAt({500, 350}));
    menu.escape();
    assert(menu.screen() == GameScreen::Paused);
    assert(menu.buttonAt({500, 360})->action == MenuAction::Restart);
    menu.activate(MenuAction::Continue);
    assert(menu.isPlaying());
    menu.finish(false);
    assert(menu.screen() == GameScreen::Defeat);
    assert(menu.buttonAt({500, 375})->action == MenuAction::Restart);
    menu.escape();
    assert(menu.screen() == GameScreen::Defeat);
    menu.activate(MenuAction::Restart);
    assert(menu.isPlaying());
    menu.finish(true);
    assert(menu.screen() == GameScreen::Victory);
    menu.activate(MenuAction::MainMenu);
    assert(menu.screen() == GameScreen::MainMenu);
    menu.activate(MenuAction::Start);
    assert(menu.screen() == GameScreen::Cutscene);
}
