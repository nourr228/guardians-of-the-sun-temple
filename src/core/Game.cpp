#include "core/Game.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <glad/glad.h>
#include <glm/vec2.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "core/InputManager.h"
#include "demo/CharacterShowcase.h"
#include "rendering/Camera2D.h"
#include "rendering/Renderer2D.h"
#include "rendering/ResourceManager.h"
#include "rendering/SpriteRenderer.h"
#include "world/Battlefield.h"
#include "world/HomeBase.h"
#include "world/TempleEnvironment.h"

namespace {
void onGlfwError(int code, const char* description) {
    std::cerr << "GLFW error " << code << ": " << description << '\n';
}
} // namespace

Game::Game() = default;

Game::~Game() {
    // Release GPU objects while the context is still current.
    showcase_.reset();
    environment_.reset();
    battlefield_.reset();
    homeBase_.reset();
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
    const char* capturePath = std::getenv("GOST_CAPTURE_PATH");
    double previousTime = glfwGetTime();
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(window_, &framebufferWidth, &framebufferHeight);
        if (framebufferWidth > 0 && framebufferHeight > 0) {
            camera_->setFramebufferSize(framebufferWidth, framebufferHeight);
        }
        input_->update();

        const double now = glfwGetTime();
        const float deltaTime = static_cast<float>(
                std::clamp(now - previousTime, 0.0, 0.25));
        previousTime = now;
        update(deltaTime);
        if (framebufferWidth > 0 && framebufferHeight > 0) {
            render();
            if (capturePath != nullptr && capturePath[0] != '\0') {
                std::vector<unsigned char> pixels(
                        static_cast<std::size_t>(framebufferWidth) *
                        framebufferHeight * 4);
                glReadPixels(0, 0, framebufferWidth, framebufferHeight,
                             GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
                stbi_flip_vertically_on_write(true);
                if (!stbi_write_png(capturePath, framebufferWidth,
                                    framebufferHeight, 4, pixels.data(),
                                    framebufferWidth * 4)) {
                    throw std::runtime_error("Could not save capture frame");
                }
                std::cout << "Captured frame: " << capturePath << std::endl;
                return 0;
            }
            glfwSwapBuffers(window_);
        }
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
    window_ = glfwCreateWindow(static_cast<int>(Battlefield::worldWidth),
                               static_cast<int>(Battlefield::worldHeight),
                               "Guardians of the Sun Temple - Battlefield",
                               nullptr, nullptr);
    if (window_ == nullptr) throw std::runtime_error("Window creation failed");
    glfwMakeContextCurrent(window_);
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        throw std::runtime_error("GLAD initialization failed");
    }
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << '\n';
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    input_ = std::make_unique<InputManager>(window_);
    camera_ = std::make_unique<Camera2D>(Battlefield::worldWidth,
                                         Battlefield::worldHeight);
    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(window_, &framebufferWidth, &framebufferHeight);
    camera_->setFramebufferSize(framebufferWidth, framebufferHeight);

    resources_ = std::make_unique<ResourceManager>();
    const std::filesystem::path assets = GOST_ASSET_DIR;
    auto& shader = resources_->loadShader("sprite", assets / "shaders/sprite.vert",
                                         assets / "shaders/sprite.frag");
    resources_->loadTexture("sun_crystal", assets / "textures/sun_crystal.png");
    renderer_ = std::make_unique<Renderer2D>(shader);
    sprites_ = std::make_unique<SpriteRenderer>(*renderer_);
    battlefield_ = std::make_unique<Battlefield>();
    homeBase_ = std::make_unique<HomeBase>();
    environment_ = std::make_unique<TempleEnvironment>();
    environment_->loadAssets(*resources_, assets);
    showcase_ = std::make_unique<CharacterShowcase>(*renderer_);
    if (const char* state = std::getenv("GOST_PREVIEW_STATE")) {
        const int stateIndex = std::atoi(state);
        if (stateIndex >= 0 && stateIndex <= 6) {
            int damageStep = 0;
            if (const char* damage = std::getenv("GOST_PREVIEW_DAMAGE")) {
                damageStep = std::atoi(damage);
            }
            showcase_->openPreview(static_cast<AnimationState>(stateIndex),
                                   damageStep);
            if (const char* time = std::getenv("GOST_PREVIEW_TIME")) {
                showcase_->update(std::max(0.0f, std::strtof(time, nullptr)));
            }
        }
    }
}

void Game::update(float deltaTime) {
    if (input_->keyPressed(GLFW_KEY_ESCAPE)) paused_ = !paused_;
    showcase_->handleInput(*input_);
    const std::string nextTitle = showcase_->statusText();
    if (nextTitle != windowTitle_) {
        windowTitle_ = nextTitle;
        glfwSetWindowTitle(window_, windowTitle_.c_str());
    }
    const glm::vec2 mouse = input_->mouseWorldPosition(*camera_);
    battlefield_->updateHover(mouse);
    if (!paused_ && input_->mousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
        battlefield_->toggleSelectionAt(mouse);
    }
    if (!paused_) {
        environment_->update(deltaTime);
        showcase_->update(deltaTime);
    }
}

void Game::render() const {
    const glm::ivec2 framebuffer = camera_->framebufferSize();
    glViewport(0, 0, framebuffer.x, framebuffer.y);
    glClearColor(0.07f, 0.035f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const ViewportRect view = camera_->viewportRect();
    glViewport(view.x, view.y, view.width, view.height);
    environment_->renderBackdrop(*camera_, *renderer_, *sprites_, *resources_);
    environment_->renderBattlefield(*camera_, *renderer_, *sprites_,
                                    *resources_, *battlefield_);
    homeBase_->render(*camera_, *renderer_, *sprites_, *resources_,
                      environment_->elapsedSeconds());
    environment_->renderForeground(*camera_, *renderer_, *sprites_, *resources_);
    showcase_->render(*camera_, *battlefield_);

    if (paused_) {
        renderer_->drawRect(*camera_, {0, 0},
                            {Battlefield::worldWidth, Battlefield::worldHeight},
                            {0.05f, 0.02f, 0.09f, 0.46f});
        renderer_->drawRect(*camera_, {467, 249}, {20, 96},
                            {1.0f, 0.81f, 0.33f, 1});
        renderer_->drawRect(*camera_, {513, 249}, {20, 96},
                            {1.0f, 0.81f, 0.33f, 1});
    }
}
