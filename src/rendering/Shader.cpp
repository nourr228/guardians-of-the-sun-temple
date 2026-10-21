#include "rendering/Shader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <glm/gtc/type_ptr.hpp>

namespace {
std::string readFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open shader: " + path.string());
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

GLuint compile(GLenum type, const std::string& source,
               const std::filesystem::path& path) {
    const GLuint shader = glCreateShader(type);
    const char* data = source.c_str();
    glShaderSource(shader, 1, &data, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::string log(static_cast<std::size_t>(length), '\0');
        glGetShaderInfoLog(shader, length, nullptr, log.data());
        glDeleteShader(shader);
        throw std::runtime_error("Shader compile failed (" + path.string() + "): " + log);
    }
    return shader;
}
} // namespace

Shader::Shader(const std::filesystem::path& vertexPath,
               const std::filesystem::path& fragmentPath) {
    const std::string vertexSource = readFile(vertexPath);
    const std::string fragmentSource = readFile(fragmentPath);
    const GLuint vertex = compile(GL_VERTEX_SHADER, vertexSource, vertexPath);
    GLuint fragment = 0;
    try {
        fragment = compile(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);
        program_ = glCreateProgram();
        glAttachShader(program_, vertex);
        glAttachShader(program_, fragment);
        glLinkProgram(program_);

        GLint success = GL_FALSE;
        glGetProgramiv(program_, GL_LINK_STATUS, &success);
        if (success == GL_FALSE) {
            GLint length = 0;
            glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &length);
            std::string log(static_cast<std::size_t>(length), '\0');
            glGetProgramInfoLog(program_, length, nullptr, log.data());
            throw std::runtime_error("Shader link failed: " + log);
        }
    } catch (...) {
        glDeleteShader(vertex);
        if (fragment != 0) glDeleteShader(fragment);
        if (program_ != 0) glDeleteProgram(program_);
        program_ = 0;
        throw;
    }
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() {
    if (program_ != 0) glDeleteProgram(program_);
}

Shader::Shader(Shader&& other) noexcept : program_(std::exchange(other.program_, 0)) {}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (program_ != 0) glDeleteProgram(program_);
        program_ = std::exchange(other.program_, 0);
    }
    return *this;
}

void Shader::use() const { glUseProgram(program_); }

void Shader::setMatrix(const char* name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(program_, name), 1, GL_FALSE,
                       glm::value_ptr(value));
}

void Shader::setColor(const char* name, const glm::vec4& value) const {
    glUniform4fv(glGetUniformLocation(program_, name), 1, glm::value_ptr(value));
}

void Shader::setInt(const char* name, int value) const {
    glUniform1i(glGetUniformLocation(program_, name), value);
}
