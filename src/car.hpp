#pragma once

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>

#include "model.hpp"

class Car
{
public:
    Car();

    void loadModels();

    void update(float deltaTime);

    void draw(glm::mat4& projView);

private:
    void drawFrame(const glm::mat4& carMVP);
    void drawWheel(const glm::mat4& carMVP, const glm::vec3& pos, bool isFront);
    void drawWheels(const glm::mat4& carMVP);

    void drawBlinker(const glm::mat4& carMVP, const glm::vec3& pos, bool isLeft, bool isFront);
    void drawLight(const glm::mat4& carMVP, const glm::vec3& pos, bool isFront);
    void drawHeadlight(const glm::mat4& carMVP, const glm::vec3& pos, bool isLeft, bool isFront);
    void drawHeadlights(const glm::mat4& carMVP);

private:
    Model frame_;
    Model wheel_;
    Model blinker_;
    Model light_;

public:
    glm::vec3 position;
    glm::vec2 orientation;

    float speed;
    float wheelsRollAngle;
    float steeringAngle;
    bool isHeadlightOn;
    bool isBraking;
    bool isLeftBlinkerActivated;
    bool isRightBlinkerActivated;

    bool isBlinkerOn;
    float blinkerTimer;

    GLuint colorModUniformLocation;
    GLuint mvpUniformLocation;
};
