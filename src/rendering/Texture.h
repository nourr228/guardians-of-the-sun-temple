#pragma once

#include <filesystem>

#include <glad/glad.h>

class Texture {
public:
    explicit Texture(const std::filesystem::path& path);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void bind(unsigned int unit = 0) const;
    [[nodiscard]] int width() const { return width_; }
    [[nodiscard]] int height() const { return height_; }

private:
    GLuint id_ = 0;
    int width_ = 0;
    int height_ = 0;
};
