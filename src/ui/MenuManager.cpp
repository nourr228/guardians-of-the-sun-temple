#include "ui/MenuManager.h"

std::array<std::optional<MenuButton>, 4> MenuManager::buttons() const {
    constexpr glm::vec2 size{230, 54};
    constexpr float x = 385.0f;
    switch (screen_) {
    case GameScreen::MainMenu:
        if (soundSettingsOpen_)
            return {MenuButton{MenuAction::VolumeDown, "-", {385, 309}, {105, 54}},
                    MenuButton{MenuAction::VolumeUp, "+", {510, 309}, {105, 54}},
                    MenuButton{MenuAction::ToggleMute, "TOGGLE MUTE", {x, 375}, size},
                    MenuButton{MenuAction::Back, "BACK", {x, 441}, size}};
        return {MenuButton{MenuAction::Start, "START GAME", {x, 309}, size},
                MenuButton{MenuAction::SoundSettings, "SOUND SETTINGS", {x, 375}, size},
                MenuButton{MenuAction::Exit, "EXIT GAME", {x, 441}, size},
                std::nullopt};
    case GameScreen::Paused:
        return {MenuButton{MenuAction::Continue, "CONTINUE", {x, 265}, size},
                MenuButton{MenuAction::Restart, "RESTART", {x, 334}, size},
                MenuButton{MenuAction::MainMenu, "MAIN MENU", {x, 403}, size},
                std::nullopt};
    case GameScreen::Victory:
    case GameScreen::Defeat:
        return {MenuButton{MenuAction::Restart, "RESTART", {x, 350}, size},
                MenuButton{MenuAction::MainMenu, "MAIN MENU", {x, 419}, size},
                std::nullopt, std::nullopt};
    case GameScreen::Cutscene:
    case GameScreen::Playing:
        return {};
    }
    return {};
}

std::optional<MenuButton> MenuManager::buttonAt(glm::vec2 point) const {
    for (const auto& button : buttons()) {
        if (button && button->contains(point)) return button;
    }
    return std::nullopt;
}

void MenuManager::activate(MenuAction action) {
    switch (action) {
    case MenuAction::Start:
        if (screen_ == GameScreen::MainMenu && !soundSettingsOpen_) {
            soundSettingsOpen_ = false;
            screen_ = GameScreen::Cutscene;
        }
        break;
    case MenuAction::Continue:
        if (screen_ == GameScreen::Paused) screen_ = GameScreen::Playing;
        break;
    case MenuAction::Restart:
        if (screen_ == GameScreen::Paused || screen_ == GameScreen::Victory ||
            screen_ == GameScreen::Defeat) screen_ = GameScreen::Playing;
        break;
    case MenuAction::MainMenu:
        screen_ = GameScreen::MainMenu;
        soundSettingsOpen_ = false;
        break;
    case MenuAction::Exit:
        break;
    case MenuAction::SoundSettings:
        if (screen_ == GameScreen::MainMenu) soundSettingsOpen_ = true;
        break;
    case MenuAction::Back:
        if (screen_ == GameScreen::MainMenu) soundSettingsOpen_ = false;
        break;
    case MenuAction::VolumeDown:
    case MenuAction::VolumeUp:
    case MenuAction::ToggleMute:
        break;
    }
}

void MenuManager::escape() {
    if (screen_ == GameScreen::MainMenu && soundSettingsOpen_)
        soundSettingsOpen_ = false;
    else if (screen_ == GameScreen::Playing) screen_ = GameScreen::Paused;
    else if (screen_ == GameScreen::Paused) screen_ = GameScreen::Playing;
    else if (screen_ == GameScreen::Cutscene) screen_ = GameScreen::MainMenu;
}

void MenuManager::finishCutscene() {
    if (screen_ == GameScreen::Cutscene) screen_ = GameScreen::Playing;
}

void MenuManager::finish(bool victory) {
    if (screen_ == GameScreen::Playing)
        screen_ = victory ? GameScreen::Victory : GameScreen::Defeat;
}
