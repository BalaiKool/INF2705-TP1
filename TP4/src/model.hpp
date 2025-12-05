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

    void setColorTexture(GLuint tex) { texColor_ = tex; }
    void setNormalTexture(GLuint tex) { texNormal_ = tex; }
    void setRoughnessTexture(GLuint tex) { texRoughness_ = tex; }

    glm::vec3 center_;

private:
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
    GLsizei count_ = 0;
    GLuint texColor_ = 0;
    GLuint texNormal_ = 0;
    GLuint texRoughness_ = 0;
};