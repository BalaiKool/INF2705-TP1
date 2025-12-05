#ifndef CLOUD_HPP
#define CLOUD_HPP

#include <vector>
#include <glm/glm.hpp>
#include <inf2705/OpenGLApplication.hpp>
#include <light.hpp>

struct Cloud {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 scale;
    float rotationY;
    float rotationSpeedY;
    float floatOffset;
    float floatSpeed;
    float floatAmount;
    float speed;
    float lifetime;
    float maxLifetime;
    float fadePhase;
    float alpha;
};

class Clouds {
public:
    struct CloudData {
        glm::vec3 position;
        glm::vec3 direction;
        float speed;
        float lifetime;
        float maxLifetime;
        float fadePhase;
        float alpha;
        glm::vec3 scale;
        float rotationY;
        float rotationSpeedY;
        float floatOffset;
        float floatSpeed;
        float floatAmount;
    };

    Clouds();
    Clouds(unsigned int cloudCount);
    ~Clouds();

    void initialize();
    void update(float deltaTime);
    void draw(const glm::mat4& proj, const glm::mat4& view,
        const Light::LightSource& light, const glm::vec3& cameraPos);
    void drawShadow();
    void updateLightingUniforms(const Light::LightSource& light);

    unsigned int getCloudCount() const { return cloudCount_; }
    const CloudData& getCloud(unsigned int index) const {
        if (index < clouds_.size()) return clouds_[index];
        static CloudData empty;
        return empty;
    }
private:
    void spawnCloud(unsigned int index);
    void initBuffers();
    void loadShaders();

    std::vector<CloudData> clouds_;
    unsigned int cloudCount_;

    unsigned int vao_ = 0;
    unsigned int vbo_ = 0;
    unsigned int ebo_ = 0;
    unsigned int shaderProgram_ = 0;

    GLint uProjLoc_ = -1;
    GLint uViewLoc_ = -1;
    GLint uModelLoc_ = -1;
    GLint uAlphaLoc_ = -1;
    GLint uLightPosLoc_;
    GLint uLightColorLoc_;
    GLint uLightIntensityLoc_;
    GLint uCameraPosLoc_;
    GLint uShadowMapLoc_;
    GLint uLightSpaceMatrixLoc_;
    GLint uShadowStrengthLoc_;
    GLint uShadowSoftnessLoc_;

    size_t vertexCount_ = 0;
    size_t indexCount_ = 0;
};

#endif