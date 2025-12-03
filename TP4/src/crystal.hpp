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

private:
    void drawCrystal(const glm::mat4& crystalMVP);

private:
    Model crystal_;

public:
    glm::vec3 position;
    glm::vec2 orientation;

    GLuint colorModUniformLocation;
    GLuint mvpUniformLocation;
};
