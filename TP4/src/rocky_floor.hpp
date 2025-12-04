#ifndef ROCKY_FLOOR_HPP
#define ROCKY_FLOOR_HPP

#include <vector>
#include <glm/glm.hpp>
#include <inf2705/OpenGLApplication.hpp>
#include <light.hpp>

class RockyFloor {
public:
    RockyFloor();
    ~RockyFloor();

    void initialize();
    void draw(const glm::mat4& proj, const glm::mat4& view,
        const glm::vec3& cameraPos,
        const Light::LightSource& light,
        const glm::vec3& crystalPos,
        float crystalHeight,
        const std::vector<glm::vec3>& cloudPositions,
        const std::vector<float>& cloudSizes,
        const std::vector<float>& cloudAlphas);

private:
    void createGeometry();
    void loadShaders();

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
    GLuint shaderProgram_ = 0;

    int vertexCount_ = 0;
    int patchCount_ = 0;
    float time_ = 0.0f;

    GLint uProjLoc_;
    GLint uViewLoc_;
    GLint uModelLoc_;
    GLint uCameraPosLoc_;
    GLint uTimeLoc_;

    GLint uLightPosLoc_;
    GLint uLightColorLoc_;
    GLint uLightIntensityLoc_;

    GLint uCrystalPosLoc_;
    GLint uCrystalHeightLoc_;
    GLint uCloudPositionsLoc_;
    GLint uCloudSizesLoc_;
    GLint uCloudAlphasLoc_;
    GLint uCloudCountLoc_;
};

#endif