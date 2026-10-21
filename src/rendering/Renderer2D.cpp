#include "rendering/Renderer2D.h"

#include <glm/gtc/matrix_transform.hpp>

#include "rendering/Camera2D.h"
#include "rendering/Shader.h"
#include "rendering/Texture.h"

Renderer2D::Renderer2D(Shader& shader) : shader_(shader) {
    // x, y, u, v. The image's top-left appears at the quad's top-left.
    constexpr float vertices[] = {
            0.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
    };
    constexpr unsigned int indices[] = {0, 1, 2, 2, 3, 0};

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          reinterpret_cast<const void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

Renderer2D::~Renderer2D() {
    if (ebo_ != 0) glDeleteBuffers(1, &ebo_);
    if (vbo_ != 0) glDeleteBuffers(1, &vbo_);
    if (vao_ != 0) glDeleteVertexArrays(1, &vao_);
}

void Renderer2D::drawRect(const Camera2D& camera, glm::vec2 position,
                          glm::vec2 size, glm::vec4 color,
                          float rotationDegrees) const {
    draw(camera, position, size, color, rotationDegrees, nullptr);
}

void Renderer2D::drawTexture(const Camera2D& camera, const Texture& texture,
                             glm::vec2 position, glm::vec2 size,
                             glm::vec4 tint, float rotationDegrees) const {
    draw(camera, position, size, tint, rotationDegrees, &texture);
}

void Renderer2D::draw(const Camera2D& camera, glm::vec2 position, glm::vec2 size,
                      glm::vec4 color, float rotationDegrees,
                      const Texture* texture) const {
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::translate(model, glm::vec3(size * 0.5f, 0.0f));
    model = glm::rotate(model, glm::radians(rotationDegrees), glm::vec3(0, 0, 1));
    model = glm::translate(model, glm::vec3(-size * 0.5f, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    shader_.use();
    shader_.setMatrix("uModel", model);
    shader_.setMatrix("uViewProjection", camera.viewProjection());
    shader_.setColor("uColor", color);
    shader_.setInt("uUseTexture", texture != nullptr ? 1 : 0);
    shader_.setInt("uTexture", 0);
    if (texture != nullptr) texture->bind();

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
