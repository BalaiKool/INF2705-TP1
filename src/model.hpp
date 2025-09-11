#pragma once

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>

using namespace gl;

class Model
{
public:
    void load(const char* path);
    
    ~Model();
    
    void draw();

    glm::vec3 center_;

private:
    GLuint vao_, vbo_, ebo_;
    GLsizei count_;
};