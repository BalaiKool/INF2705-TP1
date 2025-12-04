#ifndef ROCKY_FLOOR_HPP
#define ROCKY_FLOOR_HPP

#include <vector>
#include <glm/glm.hpp>
#include <inf2705/OpenGLApplication.hpp>

class RockyFloor {
public:
    RockyFloor();
    ~RockyFloor();

    void initialize();
    void draw(const glm::mat4& proj, const glm::mat4& view, const glm::vec3& cameraPos);

private:
    void createGeometry();
    void loadShaders();

    unsigned int vao_ = 0;
    unsigned int vbo_ = 0;
    unsigned int ebo_ = 0;
    unsigned int shaderProgram_ = 0;

    int uProjLoc_ = -1;
    int uViewLoc_ = -1;
    int uModelLoc_ = -1;
    int uCameraPosLoc_ = -1;
    int uTimeLoc_ = -1;

    size_t vertexCount_ = 0;
    float time_ = 0.0f;
    size_t patchCount_ = 0;
};

#endif