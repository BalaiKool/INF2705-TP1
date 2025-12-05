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
    orientation.y += rotationSpeed * deltaTime;

    floatPhase += floatSpeed * deltaTime * glm::two_pi<float>();
    position.y = sin(floatPhase) * floatAmplitude;
}

void Crystal::draw(glm::mat4& projView)
{
    crystal_.draw();
}

void Crystal::setColorTexture(GLuint tex)
{ 
    crystal_.setColorTexture(tex); 
}

void Crystal::setNormalTexture(GLuint tex)
{ 
    crystal_.setNormalTexture(tex);
}

void Crystal::setRoughnessTexture(GLuint tex)
{ 
    crystal_.setRoughnessTexture(tex); 
}