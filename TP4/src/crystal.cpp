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
    // Load PLY and compute UVs directly in Model
    crystal_.load("../models/crystal.ply");
}

void Crystal::update(float deltaTime)
{
    // Optional animation
}

void Crystal::draw(glm::mat4& projView)
{
    mat4 crystalTransform = translate(mat4(1.0f), position);
    crystalTransform = rotate(crystalTransform, orientation.y, vec3(0.f, 1.f, 0.f));
    crystalTransform = scale(crystalTransform, vec3(4.0f)); // adjust scale if needed

    mat4 crystalMVP = projView * crystalTransform;

    crystal_.draw();
}

