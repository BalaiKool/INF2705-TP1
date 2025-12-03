#include "model.hpp"
#include "happly.h"
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

using namespace gl;

struct PVertex {
    glm::vec3 pos;
    glm::vec3 col;
};

void Model::load(const char* path)
{
    happly::PLYData plyIn(path);

    happly::Element& vertex = plyIn.getElement("vertex");
    std::vector<float> positionX = vertex.getProperty<float>("x");
    std::vector<float> positionY = vertex.getProperty<float>("y");
    std::vector<float> positionZ = vertex.getProperty<float>("z");

    std::vector<std::vector<unsigned int>> facesIndices = plyIn.getFaceIndices<unsigned int>();

    std::vector<PVertex> vtx;
    vtx.reserve(positionX.size());
    for (size_t i = 0; i < positionX.size(); ++i)
    {
        PVertex pv;
        pv.pos = glm::vec3(positionX[i], positionY[i], positionZ[i]);
        
        pv.col = glm::vec3(0.8f, 0.8f, 0.8f);
        
        vtx.push_back(pv);
    }

    std::vector<GLuint> idx;
    idx.reserve(facesIndices.size() * 3);
    for (auto& face : facesIndices)
    {
        if (face.size() == 3) {
            idx.push_back(static_cast<GLuint>(face[0]));
            idx.push_back(static_cast<GLuint>(face[1]));
            idx.push_back(static_cast<GLuint>(face[2]));
        }
    }
    count_ = static_cast<GLsizei>(idx.size());

    glm::vec3 minV(FLT_MAX), maxV(-FLT_MAX);
    for (const auto& pv : vtx) {
        minV = glm::min(minV, pv.pos);
        maxV = glm::max(maxV, pv.pos);
    }
    center_ = 0.5f * (minV + maxV);

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vtx.size() * sizeof(PVertex), vtx.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PVertex), (void*)offsetof(PVertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PVertex), (void*)offsetof(PVertex, col));

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Model::~Model()
{
    if (ebo_) glDeleteBuffers(1, &ebo_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
}

void Model::draw()
{
    if (vao_ == 0 || count_ == 0) return;
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, count_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
