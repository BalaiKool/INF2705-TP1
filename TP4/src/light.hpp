#pragma once

#include <glm/glm.hpp>
#include <inf2705/OpenGLApplication.hpp>
#include <iostream>

class Light {
public:
    struct LightSource {
        glm::vec3 position = glm::vec3(10.0f, 10.0f, 10.0f);
        glm::vec3 direction = glm::vec3(-1.0f, -1.0f, -1.0f);
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 0.95f);
        float intensity = 1.0f;

        bool enabled = true;
        bool castShadows = true;
        float shadowStrength = 0.7f;
        float shadowSoftness = 1.5f;
        bool isDirectional = true;
    };

    Light();
    ~Light();

    void initialize();

    glm::vec3 getLightDirection() const { return glm::normalize(sunLight_.direction); }

    LightSource& getSunLight() { return sunLight_; }
    const LightSource& getSunLight() const { return sunLight_; }

    void setSunLight(const LightSource& light) { sunLight_ = light; }

private:
    LightSource sunLight_;
};