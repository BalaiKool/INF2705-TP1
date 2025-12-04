#ifndef CLOUD_HPP
#define CLOUD_HPP

#include <vector>
#include <glm/glm.hpp>
#include <inf2705/OpenGLApplication.hpp>

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
    Clouds();
    Clouds(unsigned int cloudCount);
    ~Clouds();

    void initialize();
    void update(float deltaTime);
    void draw(const glm::mat4& proj, const glm::mat4& view);

private:
    void spawnCloud(unsigned int index);
    void initBuffers();
    void loadShaders();

    std::vector<Cloud> clouds_;
    unsigned int cloudCount_;

    unsigned int vao_ = 0;
    unsigned int vbo_ = 0;
    unsigned int ebo_ = 0;
    unsigned int shaderProgram_ = 0;

    int uProjLoc_ = -1;
    int uViewLoc_ = -1;
    int uModelLoc_ = -1;
    int uAlphaLoc_ = -1;

    size_t vertexCount_ = 0;
    size_t indexCount_ = 0;
};

#endif