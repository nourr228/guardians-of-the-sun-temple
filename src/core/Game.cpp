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
#include "cutscene/CutsceneManager.h"
#include "cutscene/CutsceneRenderer.h"
#include "demo/CharacterShowcase.h"
#include "demo/GuardianPractice.h"
#include "demo/WavePractice.h"
#include "rendering/Camera2D.h"
#include "rendering/Renderer2D.h"
#include "rendering/ResourceManager.h"
#include "rendering/SpriteRenderer.h"
#include "ui/MenuRenderer.h"
#include "ui/TextRenderer.h"
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
    cutsceneRenderer_.reset();
    cutscene_.reset();
    menuRenderer_.reset();
    showcase_.reset();
    practice_.reset();
    wavePractice_.reset();
    text_.reset();
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
    text_ = std::make_unique<TextRenderer>(assets);
    menuRenderer_ = std::make_unique<MenuRenderer>(*renderer_, *text_);
    cutscene_ = std::make_unique<CutsceneManager>();
    cutsceneRenderer_ = std::make_unique<CutsceneRenderer>(*renderer_, *text_);
    battlefield_ = std::make_unique<Battlefield>();
    homeBase_ = std::make_unique<HomeBase>();
    environment_ = std::make_unique<TempleEnvironment>();
    environment_->loadAssets(*resources_, assets);
    showcase_ = std::make_unique<CharacterShowcase>(*renderer_);
    practice_ = std::make_unique<GuardianPractice>(*battlefield_, *renderer_);
    wavePractice_ = std::make_unique<WavePractice>(*battlefield_, *renderer_,
                                                   *text_);
    if (std::getenv("GOST_PRACTICE_PREVIEW") != nullptr) {
        menus_.activate(MenuAction::Start);
        menus_.finishCutscene();
        debugControls_ = true;
        practiceVisible_ = true;
        practice_->openPreview();
    }
    if (std::getenv("GOST_WAVE_PREVIEW") != nullptr) {
        menus_.activate(MenuAction::Start);
        menus_.finishCutscene();
        debugControls_ = true;
        wavePracticeVisible_ = true;
        practiceVisible_ = false;
        wavePractice_->openPreview();
    }
    if (std::getenv("GOST_ECONOMY_PREVIEW") != nullptr) {
        menus_.activate(MenuAction::Start);
        menus_.finishCutscene();
        debugControls_ = true;
        wavePracticeVisible_ = true;
        practiceVisible_ = false;
    }
    if (const char* state = std::getenv("GOST_PREVIEW_STATE")) {
        menus_.activate(MenuAction::Start);
        menus_.finishCutscene();
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
    if (const char* state = std::getenv("GOST_UI_PREVIEW")) {
        const std::string requested = state;
        if (requested == "play" || requested == "pause" ||
            requested == "win" || requested == "lose") {
            menus_.activate(MenuAction::Start);
            menus_.finishCutscene();
            wavePracticeVisible_ = true;
            if (requested == "pause") menus_.escape();
            if (requested == "win" || requested == "lose")
                menus_.finish(requested == "win");
        }
    }
    if (const char* time = std::getenv("GOST_CUTSCENE_PREVIEW_TIME")) {
        menus_.activate(MenuAction::Start);
        cutscene_->seek(std::strtof(time, nullptr));
    }
}

void Game::update(float deltaTime) {
    if (input_->keyPressed(GLFW_KEY_ESCAPE)) menus_.escape();
    if (menus_.screen() == GameScreen::Cutscene) {
        if (input_->keyPressed(GLFW_KEY_SPACE)) cutscene_->skip();
        else cutscene_->update(deltaTime);
        environment_->update(deltaTime);
        if (cutscene_->complete()) {
            menus_.finishCutscene();
            revealRemaining_ = 0.65f;
        }
        const std::string title = "Guardians of the Sun Temple - The Awakening of the Curse";
        if (title != windowTitle_) {
            windowTitle_ = title;
            glfwSetWindowTitle(window_, title.c_str());
        }
        return;
    }
    const glm::vec2 mouse = input_->mouseWorldPosition(*camera_);
    if (!menus_.isPlaying()) {
        battlefield_->updateHover({-1, -1});
        if (input_->mousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
            if (const auto button = menus_.buttonAt(mouse)) {
                const MenuAction action = button->action;
                menus_.activate(action);
                if (action == MenuAction::Start || action == MenuAction::Restart) {
                    wavePractice_->reset();
                    battlefield_->clearSelections();
                    practiceVisible_ = false;
                    wavePracticeVisible_ = true;
                    debugControls_ = false;
                    revealRemaining_ = 0.0f;
                    if (action == MenuAction::Start) cutscene_->start();
                } else if (action == MenuAction::MainMenu) {
                    wavePractice_->reset();
                    battlefield_->clearSelections();
                    practiceVisible_ = false;
                    wavePracticeVisible_ = false;
                    debugControls_ = false;
                    revealRemaining_ = 0.0f;
                } else if (action == MenuAction::Exit) {
                    glfwSetWindowShouldClose(window_, GLFW_TRUE);
                }
            }
        }
        if (menus_.screen() == GameScreen::MainMenu)
            environment_->update(deltaTime);
        const std::string title = menus_.screen() == GameScreen::MainMenu
                ? "Guardians of the Sun Temple - Main Menu"
                : menus_.screen() == GameScreen::Paused
                ? "Guardians of the Sun Temple - Paused"
                : menus_.screen() == GameScreen::Victory
                ? "Guardians of the Sun Temple - Victory"
                : menus_.screen() == GameScreen::Defeat
                ? "Guardians of the Sun Temple - Defeat"
                : menus_.screen() == GameScreen::Cutscene
                ? "Guardians of the Sun Temple - The Awakening of the Curse"
                : "Guardians of the Sun Temple";
        if (title != windowTitle_) {
            windowTitle_ = title;
            glfwSetWindowTitle(window_, title.c_str());
        }
        return;
    }
    if (input_->keyPressed(GLFW_KEY_F4)) {
        practiceVisible_ = !practiceVisible_;
        if (practiceVisible_) {
            wavePracticeVisible_ = false;
            debugControls_ = true;
        }
    }
    if (input_->keyPressed(GLFW_KEY_F5)) {
        wavePracticeVisible_ = !wavePracticeVisible_;
        if (wavePracticeVisible_) {
            practiceVisible_ = false;
            debugControls_ = true;
        }
    }
    if (!practiceVisible_ && !wavePracticeVisible_)
        showcase_->handleInput(*input_);
    const bool overHud = wavePracticeVisible_ && wavePractice_->uiContains(mouse);
    battlefield_->updateHover(overHud ? glm::vec2{-1, -1} : mouse);
    if (!overHud && input_->mousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
        battlefield_->toggleSelectionAt(mouse);
    }
    if (practiceVisible_) practice_->handleInput(*input_, *camera_);
    if (wavePracticeVisible_)
        wavePractice_->handleInput(*input_, *camera_, debugControls_);
    environment_->update(deltaTime);
    revealRemaining_ = std::max(0.0f, revealRemaining_ - deltaTime);
    if (practiceVisible_) practice_->update(deltaTime, *input_);
    else if (wavePracticeVisible_) {
        wavePractice_->update(deltaTime, *input_);
        if (wavePractice_->phase() == WavePhase::Victory) menus_.finish(true);
        else if (wavePractice_->phase() == WavePhase::Defeat) menus_.finish(false);
    } else showcase_->update(deltaTime);
    const std::string nextTitle = practiceVisible_ ? practice_->statusText()
            : wavePracticeVisible_ ? wavePractice_->statusText(debugControls_)
            : showcase_->statusText();
    if (nextTitle != windowTitle_) {
        windowTitle_ = nextTitle;
        glfwSetWindowTitle(window_, windowTitle_.c_str());
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
    if (menus_.screen() == GameScreen::Cutscene) {
        cutsceneRenderer_->render(*camera_, *cutscene_);
    } else if (menus_.screen() != GameScreen::MainMenu) {
        if (practiceVisible_) practice_->render(*camera_);
        else if (wavePracticeVisible_) wavePractice_->render(*camera_);
        else showcase_->render(*camera_, *battlefield_);
    }
    menuRenderer_->render(*camera_, menus_, input_->mouseWorldPosition(*camera_));
    if (menus_.isPlaying() && revealRemaining_ > 0.0f)
        renderer_->drawRect(*camera_, {0, 0}, {1000, 600},
                            {1.0f, 0.84f, 0.36f,
                             revealRemaining_ / 0.65f});
}
