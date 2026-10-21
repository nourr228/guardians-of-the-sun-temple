#include "rendering/ResourceManager.h"

#include <stdexcept>

Shader& ResourceManager::loadShader(const std::string& name,
                                    const std::filesystem::path& vertexPath,
                                    const std::filesystem::path& fragmentPath) {
    if (shaders_.contains(name)) {
        throw std::invalid_argument("Shader already loaded: " + name);
    }
    auto resource = std::make_unique<Shader>(vertexPath, fragmentPath);
    Shader& result = *resource;
    shaders_.emplace(name, std::move(resource));
    return result;
}

Texture& ResourceManager::loadTexture(const std::string& name,
                                      const std::filesystem::path& path) {
    if (textures_.contains(name)) {
        throw std::invalid_argument("Texture already loaded: " + name);
    }
    auto resource = std::make_unique<Texture>(path);
    Texture& result = *resource;
    textures_.emplace(name, std::move(resource));
    return result;
}

Shader& ResourceManager::shader(const std::string& name) const {
    const auto found = shaders_.find(name);
    if (found == shaders_.end()) throw std::out_of_range("Missing shader: " + name);
    return *found->second;
}

Texture& ResourceManager::texture(const std::string& name) const {
    const auto found = textures_.find(name);
    if (found == textures_.end()) throw std::out_of_range("Missing texture: " + name);
    return *found->second;
}

void ResourceManager::clear() {
    textures_.clear();
    shaders_.clear();
}
