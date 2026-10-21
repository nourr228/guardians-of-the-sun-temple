#pragma once

#include <filesystem>

#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

class Shader {
public:
    Shader(const std::filesystem::path& vertexPath,
           const std::filesystem::path& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void use() const;
    void setMatrix(const char* name, const glm::mat4& value) const;
    void setColor(const char* name, const glm::vec4& value) const;
    void setInt(const char* name, int value) const;

private:
    GLuint program_ = 0;
};
