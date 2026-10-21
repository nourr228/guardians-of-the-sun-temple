#pragma once

#include <memory>

#include <GLFW/glfw3.h>

class Camera2D;
class InputManager;
class Renderer2D;
class ResourceManager;
class SpriteRenderer;
class Battlefield;
class HomeBase;
class TempleEnvironment;

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
    bool paused_ = false;
};
