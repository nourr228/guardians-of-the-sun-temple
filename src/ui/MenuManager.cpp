#include "ui/MenuManager.h"

std::array<std::optional<MenuButton>, 3> MenuManager::buttons() const {
    constexpr glm::vec2 size{230, 54};
    constexpr float x = 385.0f;
    switch (screen_) {
    case GameScreen::MainMenu:
        return {MenuButton{MenuAction::Start, "START GAME", {x, 326}, size},
                MenuButton{MenuAction::Exit, "EXIT GAME", {x, 395}, size},
                std::nullopt};
    case GameScreen::Paused:
        return {MenuButton{MenuAction::Continue, "CONTINUE", {x, 265}, size},
                MenuButton{MenuAction::Restart, "RESTART", {x, 334}, size},
                MenuButton{MenuAction::MainMenu, "MAIN MENU", {x, 403}, size}};
    case GameScreen::Victory:
    case GameScreen::Defeat:
        return {MenuButton{MenuAction::Restart, "RESTART", {x, 350}, size},
                MenuButton{MenuAction::MainMenu, "MAIN MENU", {x, 419}, size},
                std::nullopt};
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
        if (screen_ == GameScreen::MainMenu) screen_ = GameScreen::Playing;
        break;
    case MenuAction::Continue:
        if (screen_ == GameScreen::Paused) screen_ = GameScreen::Playing;
        break;
    case MenuAction::Restart:
        if (screen_ == GameScreen::Paused || screen_ == GameScreen::Victory ||
            screen_ == GameScreen::Defeat) screen_ = GameScreen::Playing;
        break;
    case MenuAction::MainMenu:
        if (screen_ != GameScreen::MainMenu) screen_ = GameScreen::MainMenu;
        break;
    case MenuAction::Exit:
        break;
    }
}

void MenuManager::escape() {
    if (screen_ == GameScreen::Playing) screen_ = GameScreen::Paused;
    else if (screen_ == GameScreen::Paused) screen_ = GameScreen::Playing;
}

void MenuManager::finish(bool victory) {
    if (screen_ == GameScreen::Playing)
        screen_ = victory ? GameScreen::Victory : GameScreen::Defeat;
}
