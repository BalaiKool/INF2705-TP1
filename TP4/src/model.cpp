#include "model.hpp"
#include "happly.h"
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

using namespace gl;

struct PVertex {
    glm::vec3 pos;
    glm::vec3 col;
    glm::vec2 uv;
};

void Model::load(const char* path)
{
    happly::PLYData plyIn(path);

    // --- Read vertex properties ---
    auto& vertex = plyIn.getElement("vertex");
    std::vector<float> px = vertex.getProperty<float>("x");
    std::vector<float> py = vertex.getProperty<float>("y");
    std::vector<float> pz = vertex.getProperty<float>("z");

    // Blender exported UVs as s/t
    std::vector<float> us = vertex.getProperty<float>("s");
    std::vector<float> vs = vertex.getProperty<float>("t");

    const size_t vcount = px.size();
    std::cout << "[Model::load] Blender UVs detected ("
        << us.size() << " coords)." << std::endl;

    // --- Build vertex array ---
    std::vector<PVertex> vtx;
    vtx.resize(vcount);

    for (size_t i = 0; i < vcount; ++i)
    {
        vtx[i].pos = glm::vec3(px[i], py[i], pz[i]);
        vtx[i].col = glm::vec3(1.0f);

        // IMPORTANT:
        // Your texture loader flips the PNG vertically (flipVertically()).
        // So we must flip V as well.
        vtx[i].uv = glm::vec2(us[i], 1.0f - vs[i]);
    }

    // --- Read Blender faces and triangulate ---
    auto faces = plyIn.getFaceIndices<unsigned int>();

    std::vector<GLuint> idx;
    idx.reserve(faces.size() * 3);

    for (auto& f : faces)
    {
        if (f.size() == 3)
        {
            idx.push_back(f[0]);
            idx.push_back(f[1]);
            idx.push_back(f[2]);
        }
        else if (f.size() == 4)
        {
            // Blender quads → triangle 1
            idx.push_back(f[0]);
            idx.push_back(f[1]);
            idx.push_back(f[2]);

            // Blender quads → triangle 2
            idx.push_back(f[0]);
            idx.push_back(f[2]);
            idx.push_back(f[3]);
        }
        else
        {
            // (Blender does not export ngons in PLY; we can ignore)
        }
    }
    count_ = static_cast<GLsizei>(idx.size());

    // --- Compute bounding box center ---
    glm::vec3 minV(FLT_MAX), maxV(-FLT_MAX);
    for (auto& v : vtx) {
        minV = glm::min(minV, v.pos);
        maxV = glm::max(maxV, v.pos);
    }
    center_ = 0.5f * (minV + maxV);

    // --- Upload to GPU ---
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

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(PVertex), (void*)offsetof(PVertex, uv));

    glBindVertexArray(0);

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
