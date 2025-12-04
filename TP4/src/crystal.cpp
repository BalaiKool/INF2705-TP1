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
    orientation.y += rotationSpeed * deltaTime;

    floatPhase += floatSpeed * deltaTime * glm::two_pi<float>();
    position.y = sin(floatPhase) * floatAmplitude;
}

void Crystal::draw()
{
    crystal_.draw();
}

void Crystal::drawShadow() {
    if (vao == 0) return;

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()),
        GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

