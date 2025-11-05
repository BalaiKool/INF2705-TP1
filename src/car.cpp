#include "car.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// TODO: À ajouter, et à compléter dans votre projet.

#include <map>

#include "shaders.hpp"

using namespace gl;
using namespace glm;

struct Material
{
    glm::vec4 emission; // vec3, but padded
    glm::vec4 ambient;  // vec3, but padded
    glm::vec4 diffuse;  // vec3, but padded
    glm::vec3 specular;
    GLfloat shininess;
};

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

    // ...
    // À ajouter, l'ordre est à considérer
    const char* WINDOW_MODEL_PATHES[] =
    {
        "../models/window.f.ply",
        "../models/window.r.ply",
        "../models/window.fl.ply",
        "../models/window.fr.ply",
        "../models/window.rl.ply",
        "../models/window.rr.ply"
    };
    for (unsigned int i = 0; i < 6; ++i)
    {
        windows[i].load(WINDOW_MODEL_PATHES[i]);
    }
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
    // À ajouter à la fin
    carModel = glm::mat4(1.0f);
    carModel = glm::translate(carModel, position);
    carModel = glm::rotate(carModel, orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
}


// TODO: Revoir vos méthodes de dessin. Elles seront à modifier pour la partie 2 et 3.
//       Partie 2: Ajouter le calcul de stencil pour le chassi et les roues pour avoir
//                 le contour de la voiture.

void Car::draw(glm::mat4& projView, glm::mat4& view)
{
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    celShadingShader->use();

    glm::mat4 carTransform = glm::translate(glm::mat4(1.0f), position);
    carTransform = glm::rotate(carTransform, orientation.y, glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 carMVP = projView * carTransform;

    drawFrame(projView, view, carTransform);
    drawWheels(carMVP);
    drawHeadlights(carMVP);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);

    edgeEffectShader->use();

    const float outlineScale = 1.03f;
    const float outlineHeightOffset = 0.22f;
    glm::mat4 scaledCarTransform = glm::translate(carTransform, glm::vec3(0.f, outlineHeightOffset, 0.f));
    scaledCarTransform = glm::scale(scaledCarTransform, glm::vec3(outlineScale));

    glm::mat4 scaledMVP = projView * scaledCarTransform;
    edgeEffectShader->setMatrices(scaledMVP, view, scaledCarTransform);
    frame_.draw();
    drawWheels(scaledMVP);

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
}


void Car::drawFrame(glm::mat4& projView, glm::mat4& view, const mat4& carTransform)
{
    mat4 model = translate(mat4(1.0f), vec3(0.f, 0.25f, 0.f));
    mat4 worldModel = carTransform * model;
    mat4 mvp = projView * worldModel;

    celShadingShader->setMatrices(mvp, view, worldModel);
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


void Car::drawWindows(glm::mat4& projView, glm::mat4& view)
{
    const glm::vec3 WINDOW_POSITION[] =
    {
        glm::vec3(-0.813, 0.755, 0.0),
        glm::vec3(1.092, 0.761, 0.0),
        glm::vec3(-0.3412, 0.757, 0.51),
        glm::vec3(-0.3412, 0.757, -0.51),
        glm::vec3(0.643, 0.756, 0.508),
        glm::vec3(0.643, 0.756, -0.508)
    };

    // TODO: À ajouter et compléter.
    //       Dessiner les vitres de la voiture. Celles-ci ont une texture transparente,
    //       il est donc nécessaire d'activer le mélange des couleurs (GL_BLEND).
    //       De plus, vous devez dessiner les fenêtres du plus loin vers le plus proche
    //       pour éviter les problèmes de mélange.
    //       Utiliser un map avec la distance en clef pour trier les fenêtres (les maps trient
    //       à l'insertion).
    //       Les fenêtres doivent être visibles des deux sens.
    //       Il est important de restaurer l'état du contexte qui a été modifié à la fin de la méthode.


    // Les fenêtres sont par rapport au chassi, à considérer dans votre matrice
    // model = glm::translate(model, glm::vec3(0.0f, 0.25f, 0.0f));

    std::map<float, unsigned int> sorted;
    for (unsigned int i = 0; i < 6; i++)
    {
        // TODO: Calcul de la distance par rapport à l'observateur (utiliser la matrice de vue!)
        //       et faite une insertion dans le map
    }

    // TODO: Itération à l'inverse (de la plus grande distance jusqu'à la plus petit)
    for (std::map<float, unsigned int>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
    {
        // TODO: Dessin des fenêtres
    }
}

void Car::drawBlinker(const mat4& carMVP, const vec3& pos, bool isLeft, bool isFront)
{
    const glm::vec3 ON_COLOR(1.0f, 0.7f, 0.3f);
    const glm::vec3 OFF_COLOR(0.5f, 0.35f, 0.15f);
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

    // TODO: À ajouter dans votre méthode. À compléter pour la partie 3.
    Material blinkerMat =
    {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {OFF_COLOR, 0.0f},
        {OFF_COLOR, 0.0f},
        {OFF_COLOR},
        10.0f
    };

    //if (isBlinkerOn && isBlinkerActivated)
    //    TODO: Modifier le matériel pour qu'il ait l'air d'émettre de la lumière.
    //    ... = glm::vec4(ON_COLOR, 0.0f);

    // TODO: Envoyer le matériel au shader. Partie 3.

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

    const glm::vec3 FRONT_ON_COLOR(1.0f, 1.0f, 1.0f);
    const glm::vec3 FRONT_OFF_COLOR(0.5f, 0.5f, 0.5f);
    const glm::vec3 REAR_ON_COLOR(1.0f, 0.1f, 0.1f);
    const glm::vec3 REAR_OFF_COLOR(0.5f, 0.1f, 0.1f);

    // TODO: À ajouter dans votre méthode. À compléter pour la partie 3.

    Material lightFrontMat =
    {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {FRONT_OFF_COLOR, 0.0f},
        {FRONT_OFF_COLOR, 0.0f},
        {FRONT_OFF_COLOR},
        10.0f
    };

    Material lightRearMat =
    {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {REAR_OFF_COLOR, 0.0f},
        {REAR_OFF_COLOR, 0.0f},
        {REAR_OFF_COLOR},
        10.0f
    };

    if (isFront)
    {
        // if (isHeadlightOn)
        //    TODO: Modifier le matériel pour qu'il ait l'air d'émettre de la lumière.
        //    ... = glm::vec4(FRONT_ON_COLOR, 0);

        // TODO: Envoyer le matériel au shader. Partie 3.
    }
    else
    {
        // if (isBraking)
        //    TODO: Modifier le matériel pour qu'il ait l'air d'émettre de la lumière.
        //    ... = glm::vec4(REAR_ON_COLOR, 0);

        // TODO: Envoyer le matériel au shader. Partie 3.
    }
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
