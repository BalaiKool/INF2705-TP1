#include "crystal.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace gl;
using namespace glm;

Crystal::Crystal()
    : position(0.0f, 0.0f, 0.0f), orientation(0.0f, 0.0f)
    , speed(0.f), wheelsRollAngle(0.f), steeringAngle(0.f)
    , isHeadlightOn(false), isBraking(false)
    , isLeftBlinkerActivated(false), isRightBlinkerActivated(false)
    , isBlinkerOn(false), blinkerTimer(0.f)
{
}

void Crystal::loadModels()
{
    frame_.load("../models/frame.ply");
    wheel_.load("../models/wheel.ply");
    blinker_.load("../models/blinker.ply");
    light_.load("../models/light.ply");
}

void Crystal::update(float deltaTime)
{
    if (isBraking)
    {
        const float LOW_SPEED_THRESHOLD = 0.1f;
        const float BRAKE_APPLIED_SPEED_THRESHOLD = 0.01f;
        const float BRAKING_FORCE = 4.f;

        if (fabs(speed) < LOW_SPEED_THRESHOLD) 
        {
            speed = 0.f;
            isBraking = false;
        }
        else if (speed > BRAKE_APPLIED_SPEED_THRESHOLD) speed -= BRAKING_FORCE * deltaTime;
        else if (speed < -BRAKE_APPLIED_SPEED_THRESHOLD) speed += BRAKING_FORCE * deltaTime;
    }

    const float WHEELBASE = 2.7f;
    float angularSpeed = speed * sin(-radians(steeringAngle)) / WHEELBASE;
    orientation.y += angularSpeed * deltaTime;

    vec3 posMod = vec3(rotate(mat4(1.0f), orientation.y, vec3(0.f, 1.f, 0.f)) * vec4(-speed, 0.f, 0.f, 1.f));
    position += posMod * deltaTime;

    const float WHEEL_RADIUS = 0.2f;
    wheelsRollAngle += speed / (2.f * M_PI * WHEEL_RADIUS) * deltaTime;

    if (wheelsRollAngle > M_PI) wheelsRollAngle -= 2.f * M_PI;
    else if (wheelsRollAngle < -M_PI) wheelsRollAngle += 2.f * M_PI;

    if (isRightBlinkerActivated || isLeftBlinkerActivated)
    {
        const float BLINKER_PERIOD = 0.5f;
        blinkerTimer += deltaTime;
        if (blinkerTimer > BLINKER_PERIOD)
        {
            blinkerTimer = 0.f;
            isBlinkerOn = !isBlinkerOn;
        }
    }
    else
    {
        isBlinkerOn = true;
        blinkerTimer = 0.f;
    }
}

void Crystal::draw(glm::mat4& projView)
{
    mat4 crystalTransform = translate(mat4(1.0f), position);
    crystalTransform = rotate(crystalTransform, orientation.y, vec3(0.f, 1.f, 0.f));
    mat4 crystalMVP = projView * crystalTransform;

    drawCrystal(crystalMVP);
}

void Crystal::drawCrystal(const mat4& crystalMVP)
{
    mat4 model = translate(mat4(1.0f), vec3(0.f, 0.25f, 0.f));
    mat4 mvp = crystalMVP * model;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, value_ptr(mvp));
    crystal_.draw();
}