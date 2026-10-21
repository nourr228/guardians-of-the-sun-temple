#include "core/InputManager.h"

#include "rendering/Camera2D.h"

InputManager::InputManager(GLFWwindow* window) : window_(window) {
    glfwSetWindowUserPointer(window_, this);
    glfwSetKeyCallback(window_, onKey);
    glfwSetMouseButtonCallback(window_, onMouseButton);
}

InputManager::~InputManager() {
    glfwSetKeyCallback(window_, nullptr);
    glfwSetMouseButtonCallback(window_, nullptr);
    glfwSetWindowUserPointer(window_, nullptr);
}

void InputManager::update() {
    frameKeyPresses_ = pendingKeyPresses_;
    pendingKeyPresses_.fill(false);
    frameMousePresses_ = pendingMousePresses_;
    pendingMousePresses_.fill(false);

    double x = 0.0;
    double y = 0.0;
    glfwGetCursorPos(window_, &x, &y);
    mousePosition_ = {static_cast<float>(x), static_cast<float>(y)};
}

bool InputManager::keyDown(int key) const {
    return key >= 0 && key <= GLFW_KEY_LAST && keys_[key];
}

bool InputManager::keyPressed(int key) const {
    return key >= 0 && key <= GLFW_KEY_LAST && frameKeyPresses_[key];
}

bool InputManager::mouseDown(int button) const {
    return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST && buttons_[button];
}

bool InputManager::mousePressed(int button) const {
    return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST &&
           frameMousePresses_[button];
}

void InputManager::onKey(GLFWwindow* window, int key, int, int action, int) {
    if (key < 0 || key > GLFW_KEY_LAST) return;
    auto* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (input == nullptr) return;
    if (action == GLFW_PRESS) {
        if (!input->keys_[key]) input->pendingKeyPresses_[key] = true;
        input->keys_[key] = true;
    } else if (action == GLFW_RELEASE) {
        input->keys_[key] = false;
    }
}

void InputManager::onMouseButton(GLFWwindow* window, int button,
                                 int action, int) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return;
    auto* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (input == nullptr) return;
    if (action == GLFW_PRESS) {
        if (!input->buttons_[button]) input->pendingMousePresses_[button] = true;
        input->buttons_[button] = true;
    } else if (action == GLFW_RELEASE) {
        input->buttons_[button] = false;
    }
}

glm::vec2 InputManager::mouseWorldPosition(const Camera2D& camera) const {
    int width = 1;
    int height = 1;
    glfwGetWindowSize(window_, &width, &height);
    return camera.windowToWorld(mousePosition_,
                                {static_cast<float>(width),
                                 static_cast<float>(height)});
}
