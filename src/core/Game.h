#pragma once

#include <memory>
#include <string>

#include <GLFW/glfw3.h>

#include "ui/MenuManager.h"

class Camera2D;
class InputManager;
class Renderer2D;
class ResourceManager;
class SpriteRenderer;
class Battlefield;
class HomeBase;
class TempleEnvironment;
class CharacterShowcase;
class GuardianPractice;
class WavePractice;
class TextRenderer;
class MenuRenderer;
class CutsceneManager;
class CutsceneRenderer;

class Game {
public:
    Game();
    ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    int run();

private:
    void initialize();
    void update(float deltaTime);
    void render() const;

    GLFWwindow* window_ = nullptr;
    bool glfwInitialized_ = false;
    std::unique_ptr<InputManager> input_;
    std::unique_ptr<Camera2D> camera_;
    std::unique_ptr<ResourceManager> resources_;
    std::unique_ptr<Renderer2D> renderer_;
    std::unique_ptr<SpriteRenderer> sprites_;
    std::unique_ptr<Battlefield> battlefield_;
    std::unique_ptr<HomeBase> homeBase_;
    std::unique_ptr<TempleEnvironment> environment_;
    std::unique_ptr<CharacterShowcase> showcase_;
    std::unique_ptr<GuardianPractice> practice_;
    std::unique_ptr<WavePractice> wavePractice_;
    std::unique_ptr<TextRenderer> text_;
    std::unique_ptr<MenuRenderer> menuRenderer_;
    std::unique_ptr<CutsceneManager> cutscene_;
    std::unique_ptr<CutsceneRenderer> cutsceneRenderer_;
    MenuManager menus_;
    std::string windowTitle_;
    bool practiceVisible_ = false;
    bool wavePracticeVisible_ = false;
    bool debugControls_ = false;
    float revealRemaining_ = 0.0f;
};
