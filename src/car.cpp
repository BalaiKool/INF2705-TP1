#include "car.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace gl;
using namespace glm;

Car::Car()
    : position(0.0f, 0.0f, 0.0f), orientation(0.0f, 0.0f)
    , speed(0.f), wheelsRollAngle(0.f), steeringAngle(0.f)
    , isHeadlightOn(false), isBraking(false)
    , isLeftBlinkerActivated(false), isRightBlinkerActivated(false)
    , isBlinkerOn(false), blinkerTimer(0.f)
{
}

void Car::loadModels()
{
    frame_.load("../models/frame.ply");
    wheel_.load("../models/wheel.ply");
    blinker_.load("../models/blinker.ply");
    light_.load("../models/light.ply");
}

void Car::update(float deltaTime)
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

void Car::draw(glm::mat4& projView)
{
    mat4 carTransform = translate(mat4(1.0f), position);
    carTransform = rotate(carTransform, orientation.y, vec3(0.f, 1.f, 0.f));
    mat4 carMVP = projView * carTransform;

    drawFrame(carMVP);
    drawWheels(carMVP);
    drawHeadlights(carMVP);
}

void Car::drawFrame(const mat4& carMVP)
{
    mat4 model = translate(mat4(1.0f), vec3(0.f, 0.25f, 0.f));
    mat4 mvp = carMVP * model;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, value_ptr(mvp));
    frame_.draw();
}

void Car::drawWheel(const glm::mat4& carMVP, const glm::vec3& pos, bool isFront)
{
    mat4 model = translate(mat4(1.0f), pos);
    model = translate(model, -wheel_.center_);
    if (isFront)
        model = rotate(model, -radians(steeringAngle), vec3(0.f, 1.f, 0.f));
    model = rotate(model, wheelsRollAngle, vec3(0.f, 0.f, 1.f));
    model = translate(model, wheel_.center_);

    mat4 mvp = carMVP * model;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, value_ptr(mvp));
    wheel_.draw();
}

void Car::drawWheels(const mat4& carMVP)
{
    const vec3 positions[4] = {
        vec3(-1.29f, 0.245f, -0.62f),
        vec3(-1.29f, 0.245f,  0.44f),
        vec3(1.40f, 0.245f, -0.62f),
        vec3(1.40f, 0.245f,  0.44f)
    };


    for (int i = 0; i < 4; ++i) {
        bool isFront = (i == 0 || i == 1); // roues avant
        drawWheel(carMVP, positions[i], isFront);
    }
}

void Car::drawBlinker(const mat4& carMVP, const vec3& pos, bool isLeft, bool isFront)
{
    mat4 model = translate(mat4(1.0f), pos);

    const float BLINKER_OFFSET = 0.001f;
    if (isFront)
        model = translate(model, vec3(-BLINKER_OFFSET, 0.f, 0.f));
    else
        model = translate(model, vec3(BLINKER_OFFSET, 0.f, 0.f));

    mat4 mvp = carMVP * model;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, value_ptr(mvp));

    vec3 color = (isBlinkerOn && ((isLeft && isLeftBlinkerActivated) || (!isLeft && isRightBlinkerActivated)))
        ? vec3(1.f, 0.7f, 0.3f)
        : vec3(0.5f, 0.35f, 0.15f);
    glUniform3fv(colorModUniformLocation, 1, value_ptr(color));
    blinker_.draw();
}


void Car::drawLight(const mat4& carMVP, const vec3& pos, bool isFront)
{
    mat4 model = translate(mat4(1.0f), pos);
    mat4 mvp = carMVP * model;
    glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, value_ptr(mvp));

    vec3 color = isFront ? (isHeadlightOn ? vec3(1.f) : vec3(0.5f))
        : (isBraking ? vec3(1.f, 0.1f, 0.1f) : vec3(0.5f, 0.1f, 0.1f));
    glUniform3fv(colorModUniformLocation, 1, value_ptr(color));
    light_.draw();
}

void Car::drawHeadlight(const mat4& carMVP, const vec3& pos, bool isLeft, bool isFront)
{
    drawLight(carMVP, pos, isFront);
    drawBlinker(carMVP, pos, isLeft, isFront);
}

void Car::drawHeadlights(const mat4& carMVP)
{
    const vec3 positions[4] = {
        vec3(-2.0019f, 0.64f, -0.45f), // AVG
        vec3(-2.0019f, 0.64f,  0.45f), // AVD
        vec3(2.0019f, 0.64f, -0.45f), // ARG
        vec3(2.0019f, 0.64f,  0.45f)  // ARD
    };

    for (int i = 0; i < 4; ++i)
    {
        bool isFront = i < 2;
        bool isLeft = i % 2 == 1;
        drawHeadlight(carMVP, positions[i], isLeft, isFront);
    }
}
