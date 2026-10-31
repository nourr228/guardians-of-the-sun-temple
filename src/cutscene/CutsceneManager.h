#pragma once

#include <array>
#include <string_view>

#include <glm/vec2.hpp>

#include "rendering/Animator.h"

enum class CutsceneRole { Guardian, Pharaoh, ScarabA, ScarabB, Archer, Sentinel };

struct CutsceneActor {
    explicit CutsceneActor(CharacterType kind) : type(kind), animator(kind) {}

    CharacterType type;
    Animator animator;
    glm::vec2 feet{0.0f};
    float facing = 1.0f;
    float scale = 1.3f;
    bool visible = false;
};

struct CutsceneFrame {
    float time = 0.0f;
    int scene = 1;
    float crystalGlow = 0.0f;
    float storm = 0.0f;
    float darkness = 0.0f;
    float shake = 0.0f;
    float transition = 0.0f;
    bool curseVisible = false;
    glm::vec2 cursePosition{0.0f};
    bool clashVisible = false;
    bool arrowVisible = false;
    glm::vec2 arrowPosition{0.0f};
    std::string_view speaker;
    std::string_view dialogue;
};

class CutsceneManager {
public:
    static constexpr float duration = 16.0f;

    CutsceneManager();
    void start();
    void update(float deltaTime);
    void skip();
    void seek(float time); // Deterministic visual preview and tests.

    [[nodiscard]] bool complete() const { return time_ >= duration; }
    [[nodiscard]] float time() const { return time_; }
    [[nodiscard]] CutsceneFrame frame() const;
    [[nodiscard]] const CutsceneActor& actor(CutsceneRole role) const {
        return actors_[static_cast<std::size_t>(role)];
    }
    [[nodiscard]] const std::array<CutsceneActor, 6>& actors() const {
        return actors_;
    }

private:
    void evaluate(float animationDelta);

    float time_ = 0.0f;
    std::array<CutsceneActor, 6> actors_;
};
