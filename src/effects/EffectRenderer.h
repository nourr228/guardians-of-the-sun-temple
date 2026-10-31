#pragma once

class Camera2D;
class EffectManager;
class Renderer2D;

class EffectRenderer {
public:
    static void render(const Camera2D& camera, const Renderer2D& renderer,
                       const EffectManager& effects);
};
