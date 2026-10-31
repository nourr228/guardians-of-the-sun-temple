#pragma once

#include <array>
#include <filesystem>
#include <string_view>

#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "rendering/Shader.h"

class Camera2D;

enum class UIFont { Body, Heading };

class TextRenderer {
public:
    explicit TextRenderer(const std::filesystem::path& assets);
    ~TextRenderer();

    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    void draw(const Camera2D& camera, std::string_view text,
              glm::vec2 topLeft, float height, glm::vec4 color,
              UIFont font = UIFont::Body) const;
    [[nodiscard]] float width(std::string_view text, float height,
                              UIFont font = UIFont::Body) const;

private:
    struct Glyph {
        float x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        float u0 = 0, v0 = 0, u1 = 0, v1 = 0;
        float advance = 0;
    };
    struct Atlas {
        GLuint texture = 0;
        std::array<Glyph, 95> glyphs{};
        float baseHeight = 40.0f;
    };

    static Atlas loadAtlas(const std::filesystem::path& path, float height);
    [[nodiscard]] const Atlas& atlas(UIFont font) const;

    Shader shader_;
    Atlas body_;
    Atlas heading_;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
};
