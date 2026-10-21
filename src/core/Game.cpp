#include "core/Game.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdexcept>

#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "core/InputManager.h"
#include "rendering/Camera2D.h"
#include "rendering/Renderer2D.h"
#include "rendering/ResourceManager.h"
#include "rendering/SpriteRenderer.h"

namespace {
constexpr int initialWidth = 1000;
constexpr int initialHeight = 600;
constexpr int laneCount = 4;
constexpr float laneTopFraction = 0.27f;
constexpr float laneBottomFraction = 0.95f;

void onGlfwError(int code, const char* description) {
    std::cerr << "GLFW error " << code << ": " << description << '\n';
}

void onFramebufferSize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}
} // namespace

Game::Game() = default;

Game::~Game() {
    // GL objects must be destroyed before their context and GLFW are released.
    sprites_.reset();
    renderer_.reset();
    resources_.reset();
    input_.reset();
    camera_.reset();
    if (window_ != nullptr) glfwDestroyWindow(window_);
    if (glfwInitialized_) glfwTerminate();
}

int Game::run() {
    initialize();

    double previousTime = glfwGetTime();
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        int width = 0;
        int height = 0;
        glfwGetWindowSize(window_, &width, &height);
        camera_->setViewport(static_cast<float>(width), static_cast<float>(height));
        input_->update();

        const double now = glfwGetTime();
        const float deltaTime = static_cast<float>(
                std::clamp(now - previousTime, 0.0, 0.25));
        previousTime = now;

        update(deltaTime);
        render();
        glfwSwapBuffers(window_);
    }
    return 0;
}

void Game::initialize() {
    glfwSetErrorCallback(onGlfwError);
    if (glfwInit() != GLFW_TRUE) throw std::runtime_error("GLFW initialization failed");
    glfwInitialized_ = true;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window_ = glfwCreateWindow(initialWidth, initialHeight,
                               "Guardians of the Sun Temple - Engine Demo",
                               nullptr, nullptr);
    if (window_ == nullptr) throw std::runtime_error("Window creation failed");
    glfwMakeContextCurrent(window_);
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        throw std::runtime_error("GLAD initialization failed");
    }
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << '\n';

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(window_, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glfwSetFramebufferSizeCallback(window_, onFramebufferSize);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    input_ = std::make_unique<InputManager>(window_);
    camera_ = std::make_unique<Camera2D>(initialWidth, initialHeight);
    resources_ = std::make_unique<ResourceManager>();
    const std::filesystem::path assets = GOST_ASSET_DIR;
    auto& shader = resources_->loadShader("sprite", assets / "shaders/sprite.vert",
                                         assets / "shaders/sprite.frag");
    resources_->loadTexture("sun_crystal", assets / "textures/sun_crystal.png");
    renderer_ = std::make_unique<Renderer2D>(shader);
    sprites_ = std::make_unique<SpriteRenderer>(*renderer_);
}

int Game::laneAt(float worldY) const {
    const float height = camera_->viewport().y;
    const float top = height * laneTopFraction;
    const float bottom = height * laneBottomFraction;
    if (worldY < top || worldY >= bottom) return -1;
    return std::clamp(static_cast<int>((worldY - top) /
                                       ((bottom - top) / laneCount)),
                      0, laneCount - 1);
}

void Game::update(float deltaTime) {
    if (input_->keyPressed(GLFW_KEY_ESCAPE)) paused_ = !paused_;
    const glm::vec2 mouse = input_->mouseWorldPosition(*camera_);
    hoveredLane_ = laneAt(mouse.y);
    if (input_->mousePressed(GLFW_MOUSE_BUTTON_LEFT) && hoveredLane_ >= 0) {
        selectedLane_ = selectedLane_ == hoveredLane_ ? -1 : hoveredLane_;
    }

    if (paused_) return;
    if (input_->keyPressed(GLFW_KEY_W) || input_->keyPressed(GLFW_KEY_UP)) {
        demoLane_ = std::max(0, demoLane_ - 1);
    }
    if (input_->keyPressed(GLFW_KEY_S) || input_->keyPressed(GLFW_KEY_DOWN)) {
        demoLane_ = std::min(laneCount - 1, demoLane_ + 1);
    }

    demoTravel_ += 115.0f * deltaTime;
    elapsed_ += deltaTime;
}

void Game::render() const {
    const float width = camera_->viewport().x;
    const float height = camera_->viewport().y;
    const float laneTop = height * laneTopFraction;
    const float laneBottom = height * laneBottomFraction;
    const float laneHeight = (laneBottom - laneTop) / laneCount;
    auto rect = [this](glm::vec2 position, glm::vec2 size, glm::vec4 color,
                       float rotation = 0.0f) {
        renderer_->drawRect(*camera_, position, size, color, rotation);
    };

    glClearColor(0.07f, 0.035f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Layered desert backdrop and a simple shrine made from reusable quads.
    rect({0, 0}, {width, height}, {0.24f, 0.11f, 0.24f, 1});
    rect({0, height * 0.14f}, {width, height * 0.14f},
         {0.47f, 0.23f, 0.29f, 1});
    rect({0, height * 0.23f}, {width, height * 0.77f},
         {0.39f, 0.25f, 0.19f, 1});
    rect({width * 0.72f, height * 0.055f},
         {height * 0.10f, height * 0.10f}, {1.0f, 0.67f, 0.25f, 1},
         45.0f);

    for (int lane = 0; lane < laneCount; ++lane) {
        const float y = laneTop + lane * laneHeight;
        const glm::vec4 sand = lane % 2 == 0
                ? glm::vec4(0.65f, 0.45f, 0.27f, 1)
                : glm::vec4(0.58f, 0.39f, 0.23f, 1);
        rect({width * 0.15f, y + 2}, {width * 0.82f, laneHeight - 4}, sand);
        rect({width * 0.15f, y + laneHeight - 5},
             {width * 0.82f, 5}, {0.31f, 0.20f, 0.15f, 1});
        if (lane == hoveredLane_) {
            rect({width * 0.15f, y + 2}, {width * 0.82f, laneHeight - 7},
                 {1.0f, 0.85f, 0.34f, 0.12f});
        }
        if (lane == selectedLane_) {
            rect({width * 0.15f, y + 2}, {width * 0.82f, laneHeight - 7},
                 {1.0f, 0.84f, 0.22f, 0.25f});
        }
    }

    // Shrine placeholder: later art modules can replace these pieces.
    rect({width * 0.025f, laneTop - 15},
         {width * 0.12f, laneBottom - laneTop + 30},
         {0.27f, 0.18f, 0.16f, 1});
    rect({width * 0.035f, laneTop},
         {width * 0.10f, laneBottom - laneTop},
         {0.82f, 0.61f, 0.34f, 1});
    rect({width * 0.046f, laneTop + 12},
         {width * 0.078f, laneBottom - laneTop - 24},
         {0.30f, 0.20f, 0.25f, 1});
    rect({width * 0.017f, laneTop - 24}, {width * 0.135f, 18},
         {0.91f, 0.68f, 0.30f, 1});
    const float crystalSize = std::min(width, height) * 0.115f;
    const float pulse = 1.0f + 0.05f * std::sin(elapsed_ * 3.0f);
    sprites_->draw(*camera_, resources_->texture("sun_crystal"),
                   {width * 0.085f - crystalSize * pulse * 0.5f,
                    height * 0.57f - crystalSize * pulse * 0.5f},
                   {crystalSize * pulse, crystalSize * pulse});

    const float travelWidth = width * 0.70f;
    const float markerX = width * 0.19f + std::fmod(demoTravel_,
                                                    std::max(travelWidth, 1.0f));
    const float markerY = laneTop + (demoLane_ + 0.5f) * laneHeight;
    rect({markerX - 17, markerY - 17}, {34, 34},
         {0.98f, 0.74f, 0.28f, 1}, 45.0f);
    rect({markerX - 7, markerY - 7}, {14, 14},
         {1.0f, 0.96f, 0.63f, 1}, 45.0f);

    const glm::vec2 mouse = input_->mouseWorldPosition(*camera_);
    rect({mouse.x - 9, mouse.y - 1}, {18, 2}, {1.0f, 0.94f, 0.63f, 1});
    rect({mouse.x - 1, mouse.y - 9}, {2, 18}, {1.0f, 0.94f, 0.63f, 1});

    if (paused_) {
        rect({0, 0}, {width, height}, {0.05f, 0.02f, 0.09f, 0.5f});
        rect({width * 0.47f, height * 0.42f},
             {width * 0.02f, height * 0.16f}, {1.0f, 0.8f, 0.32f, 1});
        rect({width * 0.51f, height * 0.42f},
             {width * 0.02f, height * 0.16f}, {1.0f, 0.8f, 0.32f, 1});
    }
}
