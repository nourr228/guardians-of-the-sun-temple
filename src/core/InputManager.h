#pragma once

#include <array>

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

class Camera2D;

class InputManager {
public:
    explicit InputManager(GLFWwindow* window);
    ~InputManager();

    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    // Call once after glfwPollEvents and before each update.
    void update();

    [[nodiscard]] bool keyDown(int key) const;
    [[nodiscard]] bool keyPressed(int key) const;
    [[nodiscard]] bool mouseDown(int button) const;
    [[nodiscard]] bool mousePressed(int button) const;
    [[nodiscard]] glm::vec2 mouseWindowPosition() const { return mousePosition_; }
    [[nodiscard]] glm::vec2 mouseWorldPosition(const Camera2D& camera) const;

private:
    static void onKey(GLFWwindow* window, int key, int scancode,
                      int action, int mods);
    static void onMouseButton(GLFWwindow* window, int button,
                              int action, int mods);

    GLFWwindow* window_;
    std::array<bool, GLFW_KEY_LAST + 1> keys_{};
    std::array<bool, GLFW_KEY_LAST + 1> pendingKeyPresses_{};
    std::array<bool, GLFW_KEY_LAST + 1> frameKeyPresses_{};
    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> buttons_{};
    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> pendingMousePresses_{};
    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> frameMousePresses_{};
    glm::vec2 mousePosition_{0.0f};
};
