#pragma once

#include <array>
#include <filesystem>

#include <glm/vec2.hpp>

class Battlefield;
class Camera2D;
class Renderer2D;
class ResourceManager;
class SpriteRenderer;

class TempleEnvironment {
public:
    TempleEnvironment();

    void loadAssets(ResourceManager& resources,
                    const std::filesystem::path& assetRoot) const;
    void update(float deltaTime);
    [[nodiscard]] float elapsedSeconds() const { return elapsed_; }
    void renderBackdrop(const Camera2D& camera, const Renderer2D& renderer,
                        const SpriteRenderer& sprites,
                        const ResourceManager& resources) const;
    void renderBattlefield(const Camera2D& camera, const Renderer2D& renderer,
                           const SpriteRenderer& sprites,
                           const ResourceManager& resources,
                           const Battlefield& battlefield) const;
    void renderForeground(const Camera2D& camera, const Renderer2D& renderer,
                          const SpriteRenderer& sprites,
                          const ResourceManager& resources) const;

private:
    struct SandParticle {
        glm::vec2 position;
        float speed;
    };
    std::array<SandParticle, 28> sand_{};
    float elapsed_ = 0.0f;
};
