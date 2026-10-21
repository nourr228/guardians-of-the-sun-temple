#pragma once

#include <memory>

#include <GLFW/glfw3.h>

class Camera2D;
class InputManager;
class Renderer2D;
class ResourceManager;
class SpriteRenderer;

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
    [[nodiscard]] int laneAt(float worldY) const;

    GLFWwindow* window_ = nullptr;
    bool glfwInitialized_ = false;
    std::unique_ptr<InputManager> input_;
    std::unique_ptr<Camera2D> camera_;
    std::unique_ptr<ResourceManager> resources_;
    std::unique_ptr<Renderer2D> renderer_;
    std::unique_ptr<SpriteRenderer> sprites_;

    float demoTravel_ = 0.0f;
    float elapsed_ = 0.0f;
    int demoLane_ = 0;
    int hoveredLane_ = -1;
    int selectedLane_ = -1;
    bool paused_ = false;
};
