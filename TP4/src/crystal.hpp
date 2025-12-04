#pragma once

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>

#include "model.hpp"

class Crystal
{
public:
    Crystal();

    void loadModels();

    void update(float deltaTime);

    void draw();
    void drawShadow();

private:
    void drawCrystal(const glm::mat4& crystalMVP);

private:
    Model crystal_;
    unsigned int vao_ = 0;
    size_t vertexCount_ = 0;

public:
    glm::vec3 position;
    glm::vec2 orientation;

    float rotationSpeed = glm::radians(20.f);
    float floatSpeed = 0.2f;
    float floatAmplitude = 0.25f;
    float floatPhase = 0.0f;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;

    GLint mvpUniformLocation = -1;
    GLint modelUniformLocation = -1;
    GLint textureUniformLocation = -1;
    GLint lightPosUniformLocation = -1;
    GLint lightColorUniformLocation = -1;
    GLint lightIntensityUniformLocation = -1;
    GLint cameraPosUniformLocation = -1;
    GLint colorModUniformLocation = -1;
};
