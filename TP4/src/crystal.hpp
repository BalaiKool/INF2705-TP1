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

    void draw(glm::mat4& projView);

    void setColorTexture(GLuint tex);
    void setNormalTexture(GLuint tex);
    void setRoughnessTexture(GLuint tex);

private:
    void drawCrystal(const glm::mat4& crystalMVP);

private:
    Model crystal_;

public:
    glm::vec3 position;
    glm::vec2 orientation;

    float rotationSpeed = glm::radians(20.f);
    float floatSpeed = 0.2f;
    float floatAmplitude = 0.25f;
    float floatPhase = 0.0f;

    GLuint colorModUniformLocation;
    GLuint mvpUniformLocation;
};
