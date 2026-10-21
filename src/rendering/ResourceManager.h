#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include "rendering/Shader.h"
#include "rendering/Texture.h"

// Owns shared graphics resources. Clear it while the OpenGL context is current.
class ResourceManager {
public:
    Shader& loadShader(const std::string& name,
                       const std::filesystem::path& vertexPath,
                       const std::filesystem::path& fragmentPath);
    Texture& loadTexture(const std::string& name,
                         const std::filesystem::path& path);
    Shader& shader(const std::string& name) const;
    Texture& texture(const std::string& name) const;
    void clear();

private:
    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders_;
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;
};
