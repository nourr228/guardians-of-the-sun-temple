#pragma once

#include <array>
#include <optional>
#include <string_view>

#include <glm/vec2.hpp>

enum class GameScreen { MainMenu, Cutscene, Playing, Paused, Victory, Defeat };
enum class MenuAction { Start, Continue, Restart, MainMenu, Exit };

struct MenuButton {
    MenuAction action;
    std::string_view label;
    glm::vec2 position;
    glm::vec2 size;

    [[nodiscard]] bool contains(glm::vec2 point) const {
        return point.x >= position.x && point.x <= position.x + size.x &&
               point.y >= position.y && point.y <= position.y + size.y;
    }
};

class MenuManager {
public:
    [[nodiscard]] GameScreen screen() const { return screen_; }
    [[nodiscard]] bool isPlaying() const { return screen_ == GameScreen::Playing; }
    [[nodiscard]] std::optional<MenuButton> buttonAt(glm::vec2 point) const;
    [[nodiscard]] std::array<std::optional<MenuButton>, 3> buttons() const;
    void activate(MenuAction action);
    void escape();
    void finishCutscene();
    void finish(bool victory);

private:
    GameScreen screen_ = GameScreen::MainMenu;
};
