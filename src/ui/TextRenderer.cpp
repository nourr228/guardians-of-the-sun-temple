#include "ui/TextRenderer.h"

#include <fstream>
#include <stdexcept>
#include <vector>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include "rendering/Camera2D.h"

namespace {
constexpr int atlasSide = 1024;
struct Vertex { float x, y, u, v; };
}

TextRenderer::TextRenderer(const std::filesystem::path& assets)
        : shader_(assets / "shaders/text.vert", assets / "shaders/text.frag") {
    body_ = loadAtlas(assets / "fonts/NotoSans.ttf", 38.0f);
    heading_ = loadAtlas(assets / "fonts/Cinzel.ttf", 42.0f);
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(sizeof(float) * 2));
    glBindVertexArray(0);
}

TextRenderer::~TextRenderer() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (body_.texture) glDeleteTextures(1, &body_.texture);
    if (heading_.texture) glDeleteTextures(1, &heading_.texture);
}

TextRenderer::Atlas TextRenderer::loadAtlas(const std::filesystem::path& path,
                                           float height) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) throw std::runtime_error("Cannot open font: " + path.string());
    const auto length = file.tellg();
    if (length <= 0) throw std::runtime_error("Font is empty: " + path.string());
    std::vector<unsigned char> bytes(static_cast<std::size_t>(length));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(bytes.data()), length);
    std::vector<unsigned char> pixels(atlasSide * atlasSide);
    std::array<stbtt_bakedchar, 95> baked{};
    if (stbtt_BakeFontBitmap(bytes.data(), 0, height, pixels.data(),
                            atlasSide, atlasSide, 32, 95, baked.data()) <= 0)
        throw std::runtime_error("Font atlas is too small: " + path.string());
    Atlas result;
    result.baseHeight = height;
    for (int i = 0; i < 95; ++i) {
        const auto& b = baked[i];
        result.glyphs[i] = {
                b.xoff, b.yoff, b.xoff + b.x1 - b.x0,
                b.yoff + b.y1 - b.y0,
                static_cast<float>(b.x0) / atlasSide,
                static_cast<float>(b.y0) / atlasSide,
                static_cast<float>(b.x1) / atlasSide,
                static_cast<float>(b.y1) / atlasSide,
                b.xadvance};
    }
    glGenTextures(1, &result.texture);
    glBindTexture(GL_TEXTURE_2D, result.texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, atlasSide, atlasSide, 0,
                 GL_RED, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return result;
}

const TextRenderer::Atlas& TextRenderer::atlas(UIFont font) const {
    return font == UIFont::Heading ? heading_ : body_;
}

float TextRenderer::width(std::string_view text, float height, UIFont font) const {
    const Atlas& selected = atlas(font);
    float advance = 0;
    for (unsigned char ch : text) {
        if (ch >= 32 && ch <= 126)
            advance += selected.glyphs[ch - 32].advance;
    }
    return advance * height / selected.baseHeight;
}

void TextRenderer::draw(const Camera2D& camera, std::string_view text,
                        glm::vec2 topLeft, float height, glm::vec4 color,
                        UIFont font) const {
    const Atlas& selected = atlas(font);
    const float scale = height / selected.baseHeight;
    const float baseline = topLeft.y + height * 0.82f;
    std::vector<Vertex> vertices;
    vertices.reserve(text.size() * 6);
    float penX = topLeft.x;
    for (unsigned char ch : text) {
        if (ch < 32 || ch > 126) continue;
        const Glyph& g = selected.glyphs[ch - 32];
        const float x0 = penX + g.x0 * scale;
        const float y0 = baseline + g.y0 * scale;
        const float x1 = penX + g.x1 * scale;
        const float y1 = baseline + g.y1 * scale;
        if (x1 > x0 && y1 > y0) {
            vertices.insert(vertices.end(), {
                    {x0,y0,g.u0,g.v0}, {x1,y0,g.u1,g.v0},
                    {x1,y1,g.u1,g.v1}, {x0,y0,g.u0,g.v0},
                    {x1,y1,g.u1,g.v1}, {x0,y1,g.u0,g.v1}});
        }
        penX += g.advance * scale;
    }
    if (vertices.empty()) return;
    shader_.use();
    shader_.setMatrix("uViewProjection", camera.viewProjection());
    shader_.setColor("uColor", color);
    shader_.setInt("uTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, selected.texture);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
    glBindVertexArray(0);
}
