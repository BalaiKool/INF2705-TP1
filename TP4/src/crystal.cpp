#include "crystal.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace gl;
using namespace glm;

Crystal::Crystal()
    : position(0.0f, 0.0f, 0.0f), orientation(0.0f, 0.0f)
{
}

void Crystal::loadModels()
{
    crystal_.load("../models/crystal.ply");
}

void Crystal::update(float deltaTime)
{
}

void Crystal::draw(glm::mat4& projView)
{
    mat4 crystalTransform = translate(mat4(1.0f), position);
    crystalTransform = rotate(crystalTransform, orientation.y, vec3(0.f, 1.f, 0.f));
    mat4 crystalMVP = projView * crystalTransform;

    drawCrystal(crystalMVP);
}

void Crystal::drawCrystal(const mat4& crystalMVP)
{
    mat4 model = translate(mat4(1.0f), vec3(0.f, 0.25f, 0.f));
    mat4 mvp = crystalMVP * model;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, value_ptr(mvp));
    crystal_.draw();
}