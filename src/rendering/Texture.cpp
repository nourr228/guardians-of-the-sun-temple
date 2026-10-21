#include "rendering/Texture.h"

#include <stdexcept>
#include <string>
#include <utility>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::Texture(const std::filesystem::path& path, Filter filter) {
    stbi_set_flip_vertically_on_load(true);
    int channels = 0;
    unsigned char* pixels = stbi_load(path.string().c_str(), &width_, &height_,
                                      &channels, STBI_rgb_alpha);
    if (pixels == nullptr) {
        throw std::runtime_error("Cannot load texture " + path.string() + ": " +
                                 stbi_failure_reason());
    }

    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_2D, id_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    if (filter == Filter::Linear) glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    filter == Filter::Nearest ? GL_NEAREST : GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    filter == Filter::Nearest ? GL_NEAREST : GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(pixels);
}

Texture::~Texture() {
    if (id_ != 0) glDeleteTextures(1, &id_);
}

Texture::Texture(Texture&& other) noexcept
        : id_(std::exchange(other.id_, 0)),
          width_(std::exchange(other.width_, 0)),
          height_(std::exchange(other.height_, 0)) {}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        if (id_ != 0) glDeleteTextures(1, &id_);
        id_ = std::exchange(other.id_, 0);
        width_ = std::exchange(other.width_, 0);
        height_ = std::exchange(other.height_, 0);
    }
    return *this;
}

void Texture::bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id_);
}
