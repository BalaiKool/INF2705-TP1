#pragma once

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>
#include <vector>

using namespace gl;
using namespace glm;

class Model
{
public:
    struct Vertex
    {
        vec3 position;
        vec3 color;
        vec2 uv;
    };

    std::vector<Vertex> vertices;
    void load(const char* path);

    ~Model();

    void draw();

    glm::vec3 center_;

private:
    GLuint vao_, vbo_, ebo_;
    GLsizei count_;
};