#include "effects/EffectRenderer.h"

#include <algorithm>

#include "effects/EffectManager.h"
#include "rendering/Camera2D.h"
#include "rendering/Renderer2D.h"

void EffectRenderer::render(const Camera2D& camera,
                            const Renderer2D& renderer,
                            const EffectManager& effects) {
    for (const EffectParticle& particle : effects.particles()) {
        const float fade = std::clamp(particle.life / particle.maxLife,
                                      0.0f, 1.0f);
        glm::vec4 color = particle.color;
        color.a *= fade;
        renderer.drawRect(camera,
                          particle.position - glm::vec2{particle.size * 0.5f},
                          {particle.size, particle.size}, color, 45.0f);
    }
}
