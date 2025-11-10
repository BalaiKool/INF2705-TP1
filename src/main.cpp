#include <cstddef>
#include <cstdint>

#include <array>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "happly.h"
#include <imgui/imgui.h>

#include <inf2705/OpenGLApplication.hpp>

#include "model.hpp"
#include "car.hpp"
#include "model_data.hpp"
#include "shaders.hpp"
#include "textures.hpp"
#include "uniform_buffer.hpp"

#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;

struct Vertex
{
    vec3 position;
    vec3 color;
};

// Définition des structures pour la communication avec le shader. NE PAS MODIFIER.

struct Material
{
    glm::vec4 emission;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec3 specular;
    GLfloat shininess;
};

struct DirectionalLight
{
    glm::vec4 ambient;   // vec3, but padded
    glm::vec4 diffuse;   // vec3, but padded
    glm::vec4 specular;  // vec3, but padded    
    glm::vec4 direction; // vec3, but padded
};

struct SpotLight
{
    glm::vec4 ambient;   // vec3, but padded
    glm::vec4 diffuse;   // vec3, but padded
    glm::vec4 specular;  // vec3, but padded
    
    glm::vec4 position;  // vec3, but padded
    glm::vec3 direction;
    GLfloat exponent;
    GLfloat openingAngle;
    
    GLfloat padding[3];
};

// Matériels

Material defaultMat = 
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.7f, 0.7f, 0.7f},
    10.0f
};

Material grassMat = 
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.8f, 0.8f, 0.8f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.05f, 0.05f, 0.05f},
    100.0f
};

Material streetMat = 
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.7f, 0.7f, 0.7f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.025f, 0.025f, 0.025f},
    300.0f
};

Material streetlightMat = 
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.8f, 0.8f, 0.8f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.7f, 0.7f, 0.7f},
    10.0f
};

Material streetlightLightMat = 
{
    {0.8f, 0.7f, 0.5f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.7f, 0.7f, 0.7f},
    10.0f
};

Material windowMat = 
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f},
    2.0f
};


// TODO: Petit ajout à votre main.cpp


Material bezierMat =
{
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},
    0.0f
};

struct BezierCurve
{
    glm::vec3 p0;
    glm::vec3 c0;
    glm::vec3 c1;
    glm::vec3 p1;
};

BezierCurve curves[5] =
{
    {
        glm::vec3(-28.7912, 1.4484, -1.7349),
        glm::vec3(-28.0654, 1.4484, 6.1932),
        glm::vec3(-10.3562, 8.8346, 6.5997),
        glm::vec3(-7.6701, 8.8346, 8.9952)
    },
    {
        glm::vec3(-7.6701, 8.8346, 8.9952),
        glm::vec3(-3.9578, 8.8346, 12.3057),
        glm::vec3(-2.5652, 2.4770, 13.6914),
        glm::vec3(2.5079, 1.4484, 11.6581)
    },
    {
        glm::vec3(2.5079, 1.4484, 11.6581),
        glm::vec3(7.5810, 0.4199, 9.6248),
        glm::vec3(16.9333, 3.3014, 5.7702),
        glm::vec3(28.4665, 6.6072, 3.9096)
    },
    {
        glm::vec3(28.4665, 6.6072, 3.9096),
        glm::vec3(39.9998, 9.9131, 2.0491),
        glm::vec3(30.8239, 5.7052, -15.2108),
        glm::vec3(21.3852, 5.7052, -9.0729)
    },
    {
        glm::vec3(21.3852, 5.7052, -9.0729),
        glm::vec3(11.9464, 5.7052, -2.9349),
        glm::vec3(-1.0452, 1.4484, -12.4989),
        glm::vec3(-12.2770, 1.4484, -13.2807)
    }
};

// Définition des couleurs
const vec4 red = { 1.f, 0.f, 0.f, 1.0f };
const vec4 green = { 0.f, 1.f, 0.f, 1.0f };
const vec4 blue = { 0.f, 0.f, 1.f, 1.0f };


// TODO: Ajouter ces attributs
unsigned int bezierNPoints = 3;
unsigned int oldBezierNPoints = 0;

int cameraMode = 0;
float cameraAnimation = 0.f;
bool isAnimatingCamera = false;

struct App : public OpenGLApplication
{
    App()
        : isDay_(true)
        , cameraPosition_(17.f, 9.f, 4.5f)
        , cameraOrientation_(-.3, 1.25f)
        , isMouseMotionEnabled_(false)
        , isQWERTY_(true)
    {
    }

    void init() override
    {
        // Le message expliquant les touches de clavier.
        setKeybindMessage(
            "ESC : quitter l'application." "\n"
			"T : changer de clavier (QWERTY | AZERTY )" "\n"
            "W | Z: déplacer la caméra vers l'avant." "\n"
            "S | S: déplacer la caméra vers l'arrière." "\n"
            "A | Q: déplacer la caméra vers la gauche." "\n"
            "D | D: déplacer la caméra vers la droite." "\n"
            "Q | A: déplacer la caméra vers le bas." "\n"
            "E : déplacer la caméra vers le haut." "\n"
            "Flèches : tourner la caméra." "\n"
            "Souris : tourner la caméra" "\n"
            "Espace : activer/désactiver la souris." "\n"
        );


        // Config de base.
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_STENCIL_TEST);

        // Partie 1
        celShadingShader_.create();
        edgeEffectShader_.create();
        skyShader_.create(); 

        car_.celShadingShader = &celShadingShader_;
        car_.edgeEffectShader = &edgeEffectShader_;
        car_.material = &material_;
        car_.mvpUniformLocation = celShadingShader_.mvpULoc;

        const char* pathes[] = {
            "../textures/skybox/Daylight Box_Right.bmp",
            "../textures/skybox/Daylight Box_Left.bmp",
            "../textures/skybox/Daylight Box_Top.bmp",
            "../textures/skybox/Daylight Box_Bottom.bmp",
            "../textures/skybox/Daylight Box_Front.bmp",
            "../textures/skybox/Daylight Box_Back.bmp",
        };

        const char* nightPathes[] = {
            "../textures/skyboxNight/right.png",
            "../textures/skyboxNight/left.png",
            "../textures/skyboxNight/top.png",
            "../textures/skyboxNight/bottom.png",
            "../textures/skyboxNight/front.png",
            "../textures/skyboxNight/back.png",
        };

        std::cout << "Loading models" << std::endl;
        loadModels();
        loadTextures();
        initStaticModelMatrices();

        // Partie 3

        material_.allocate(&defaultMat, sizeof(Material));
        material_.setBindingIndex(0);

        lightsData_.dirLight =
        {
            {0.2f, 0.2f, 0.2f, 0.0f},
            {1.0f, 1.0f, 1.0f, 0.0f},
            {0.5f, 0.5f, 0.5f, 0.0f},
            {0.5f, -1.0f, 0.5f, 0.0f}
        };

        for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
        {
            lightsData_.spotLights[i].position = glm::vec4(streetlightLightPositions[i], 0.0f);
            lightsData_.spotLights[i].direction = glm::vec3(0, -1, 0);
            lightsData_.spotLights[i].exponent = 6.0f;
            lightsData_.spotLights[i].openingAngle = 60.f;
        }

        // Initialisation des paramètres de lumière des phares

        lightsData_.spotLights[N_STREETLIGHTS].position = glm::vec4(-1.6, 0.64, -0.45, 0.0f);
        lightsData_.spotLights[N_STREETLIGHTS].direction = glm::vec3(-10, -1, 0);
        lightsData_.spotLights[N_STREETLIGHTS].exponent = 4.0f;
        lightsData_.spotLights[N_STREETLIGHTS].openingAngle = 30.f;

        lightsData_.spotLights[N_STREETLIGHTS + 1].position = glm::vec4(-1.6, 0.64, 0.45, 0.0f);
        lightsData_.spotLights[N_STREETLIGHTS + 1].direction = glm::vec3(-10, -1, 0);
        lightsData_.spotLights[N_STREETLIGHTS + 1].exponent = 4.0f;
        lightsData_.spotLights[N_STREETLIGHTS + 1].openingAngle = 30.f;

        lightsData_.spotLights[N_STREETLIGHTS + 2].position = glm::vec4(1.6, 0.64, -0.45, 0.0f);
        lightsData_.spotLights[N_STREETLIGHTS + 2].direction = glm::vec3(10, -1, 0);
        lightsData_.spotLights[N_STREETLIGHTS + 2].exponent = 4.0f;
        lightsData_.spotLights[N_STREETLIGHTS + 2].openingAngle = 60.f;

        lightsData_.spotLights[N_STREETLIGHTS + 3].position = glm::vec4(1.6, 0.64, 0.45, 0.0f);
        lightsData_.spotLights[N_STREETLIGHTS + 3].direction = glm::vec3(10, -1, 0);
        lightsData_.spotLights[N_STREETLIGHTS + 3].exponent = 4.0f;
        lightsData_.spotLights[N_STREETLIGHTS + 3].openingAngle = 60.f;


        toggleStreetlight();
        updateCarLight();

        setLightingUniform();

        lights_.allocate(&lightsData_, sizeof(lightsData_));
        lights_.setBindingIndex(1);

        // TODO: Création des nouveaux shaders

        // TODO: Initialisation des meshes (béziers, patches)


        glEnable(GL_PROGRAM_POINT_SIZE); // pour être en mesure de modifier gl_PointSize dans les shaders

        CHECK_GL_ERROR;
    }


    void checkShaderCompilingError(const char* name, GLuint id)
    {
        GLint success;
        GLchar infoLog[1024];

        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(id, 1024, NULL, infoLog);
            glDeleteShader(id);
            std::cout << "Shader \"" << name << "\" compile error: " << infoLog << std::endl;
        }
    }


    void checkProgramLinkingError(const char* name, GLuint id)
    {
        GLint success;
        GLchar infoLog[1024];

        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(id, 1024, NULL, infoLog);
            glDeleteProgram(id);
            std::cout << "Program \"" << name << "\" linking error: " << infoLog << std::endl;
        }
    }


    // Appelée à chaque trame. Le buffer swap est fait juste après.
    void drawFrame() override
    {
        CHECK_GL_ERROR;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        ImGui::Begin("Scene Parameters");
        if (ImGui::Button("Reload Shaders"))
        {
            CHECK_GL_ERROR;
            edgeEffectShader_.reload();
            celShadingShader_.reload();
            skyShader_.reload();

            setLightingUniform();
            CHECK_GL_ERROR;
        }
        ImGui::End();

        sceneMain();
        CHECK_GL_ERROR;
    }


    // Appelée lorsque la fenêtre se ferme.
    void onClose() override
    {
        glDeleteBuffers(1, &vbo_);
        glDeleteBuffers(1, &ebo_);
        glDeleteVertexArrays(1, &vao_);
    }

    // Appelée lors d'une touche de clavier.
    void onKeyPress(const sf::Event::KeyPressed& key) override
    {
        using enum sf::Keyboard::Key;
        switch (key.code)
        {
        case Escape:
            window_.close();
            break;
        case Space:
            isMouseMotionEnabled_ = !isMouseMotionEnabled_;
            if (isMouseMotionEnabled_)
            {
                window_.setMouseCursorGrabbed(true);
                window_.setMouseCursorVisible(false);
            }
            else
            {
                window_.setMouseCursorGrabbed(false);
                window_.setMouseCursorVisible(true);
            }
            break;
        case T:
            isQWERTY_ = !isQWERTY_; break;
        default: break;
        }
    }

    void onResize(const sf::Event::Resized& event) override
    {
    }

    void onMouseMove(const sf::Event::MouseMoved& mouseDelta) override
    {
        if (!isMouseMotionEnabled_)
            return;

        const float MOUSE_SENSITIVITY = 0.1;
        float cameraMouvementX = mouseDelta.position.y * MOUSE_SENSITIVITY;
        float cameraMouvementY = mouseDelta.position.x * MOUSE_SENSITIVITY;
        cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;
    }

    void updateCameraInput()
    {
        if (!window_.hasFocus())
            return;

        if (isMouseMotionEnabled_)
        {
            sf::Vector2u windowSize = window_.getSize();
            sf::Vector2i windowHalfSize(windowSize.x / 2.0f, windowSize.y / 2.0f);
            sf::Mouse::setPosition(windowHalfSize, window_);
        }

        float cameraMouvementX = 0;
        float cameraMouvementY = 0;

        const float KEYBOARD_MOUSE_SENSITIVITY = 1.5f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            cameraMouvementX -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            cameraMouvementX += KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            cameraMouvementY -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            cameraMouvementY += KEYBOARD_MOUSE_SENSITIVITY;

        cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;

        // Keyboard input
        glm::vec3 positionOffset = glm::vec3(0.0);
        const float SPEED = 10.f;
        if (isQWERTY_)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
                positionOffset.z -= SPEED;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
                positionOffset.z += SPEED;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
                positionOffset.x -= SPEED;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
                positionOffset.x += SPEED;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
                positionOffset.y -= SPEED;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
                positionOffset.y += SPEED;
        }
        else {

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z))
                positionOffset.z -= SPEED;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
                positionOffset.z += SPEED;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
                positionOffset.x -= SPEED;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
                positionOffset.x += SPEED;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
                positionOffset.y -= SPEED;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
                positionOffset.y -= SPEED;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
                positionOffset.y += SPEED;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
                positionOffset.y += SPEED;
        }
        positionOffset = glm::rotate(glm::mat4(1.0f), cameraOrientation_.y, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(positionOffset, 1);
        cameraPosition_ += positionOffset * glm::vec3(deltaTime_);
    }

    void loadModels()
    {
        car_.loadModels();
        tree_.load("../models/tree.ply");
        streetlight_.load("../models/streetlight.ply");
        streetlightLight_.load("../models/streetlight_light.ply");
        skybox_.load("../models/skybox.ply");

        grass_.load(ground, sizeof(ground), planeElements, sizeof(planeElements)); 
        street_.load(street, sizeof(street), planeElements, sizeof(planeElements));
    }

    void loadTextures()
    {
        carTexture_.load("../textures/car.png");
        treeTexture_.load("../textures/tree.jpg");
        streetlightTexture_.load("../textures/streetlight.jpg");
        streetlightLightTexture_.load("../textures/streetlight_light.png");

        grassTexture_.load("../textures/grass.jpg");
        streetTexture_.load("../textures/street.jpg");
    }


    void initStaticModelMatrices()
    {
        initStreetlights();
        initTrees();
    }

    void initStreetlights()
    {
        lightsPosition.clear();
        lightsPosition.reserve(N_STREETLIGHTS);
        float position = -0.f;


        for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
        {
            position = 110.f *i / N_STREETLIGHTS+(rand() % N_STREETLIGHTS);
            position = std::fmod(position, 100.f);
            lightsPosition.push_back(position - 50.f);

            streetlightLightPositions[i] = glm::vec3(streetlightModelMatrices_[i] * glm::vec4(-2.77, 5.2, 0.0, 1.0));
        }
    }

    void initTrees()
    {
        treesPosition.clear();
        treesOrientation.clear();
        treesScale.clear();

        treesPosition.reserve(N_TREES);
        treesOrientation.reserve(N_TREES);
        treesScale.reserve(N_TREES);

        float position = 0.f;

        for (unsigned int i = 0; i < N_TREES; i++)
        {
            position = 110.f*i / N_TREES + (rand() % N_TREES);
            position = std::fmod(position,100.f);
            treesPosition.push_back(position -50.f);
        
            float angleDeg = static_cast<float>(rand() % 360);
            float angleRad = glm::radians(angleDeg);
            treesOrientation.push_back(angleRad);

            float scale = 0.6f + (rand() % 60) / 100.f;
            treesScale.push_back(scale);
        }
    }


    void drawStreetlights(glm::mat4& projView)
    {
        glm::mat4 view = getViewMatrix();
        streetlightTexture_.use();
        streetlightTexture_.setWrap(GL_CLAMP_TO_EDGE);
        streetlightTexture_.enableMipmap();
        streetlightTexture_.setFiltering(GL_NEAREST_MIPMAP_NEAREST);
        //streetlightTexture_.setFiltering(GL_NEAREST);

        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        glClear(GL_STENCIL_BUFFER_BIT);

        celShadingShader_.use();

        // Draw streetlights normally
        for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
        {
            float x = lightsPosition[i];
            float z = (i % 2 == 0 ? 3.f : -3.f);

            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(x, -0.15f, z));
            if (i % 2 == 0)
                model = glm::rotate(model, glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f));
            else
                model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));

            glm::mat4 mvp = projView * model;

            if (!isDay_)
                setMaterial(streetlightLightMat);
            else
                setMaterial(streetlightMat);

            celShadingShader_.setMatrices(mvp, view, model);
            streetlight_.draw();
            
        }

        // Draw outlines using stencil
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glEnable(GL_DEPTH_TEST);

        edgeEffectShader_.use();

        for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
        {
            float x = lightsPosition[i];
            float z = (i % 2 == 0 ? 3.f : -3.f);

            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(x, -0.15f, z));
            if (i % 2 == 0)
                model = glm::rotate(model, glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f));
            else
                model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));

            glm::vec3 center = streetlight_.center_;
            glm::mat4 scaledModel = glm::translate(model, center);
            scaledModel = glm::scale(scaledModel, glm::vec3(1.03f));
            scaledModel = glm::translate(scaledModel, -center);

            glm::mat4 mvpScaled = projView * scaledModel;
            edgeEffectShader_.setMatrices(mvpScaled, view, scaledModel);
            streetlight_.draw();
        }

        glStencilMask(0xFF);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
    }




    void drawTrees(glm::mat4& projView)
    {
        glm::mat4 view = getViewMatrix();

        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        glClear(GL_STENCIL_BUFFER_BIT);

        celShadingShader_.use();
        setMaterial(grassMat);
        treeTexture_.use();
        treeTexture_.setWrap(GL_REPEAT);
        treeTexture_.enableMipmap();
        treeTexture_.setFiltering(GL_NEAREST_MIPMAP_NEAREST);


        for (unsigned int i = 0; i < N_TREES; i++)
        {
            glm::mat4 model(1.0f);
            float x = treesPosition[i];
            float z = (i % 2 == 0) ? 3.f : -3.f;
            model = glm::translate(model, glm::vec3(x, -0.15f, z));
            model = glm::rotate(model, treesOrientation[i], glm::vec3(0.f, 1.f, 0.f));
            model = glm::scale(model, glm::vec3(treesScale[i]));

            glm::mat4 mvp = projView * model;
            celShadingShader_.setMatrices(mvp, view, model);
            tree_.draw();
        }

        glEnable(GL_DEPTH_TEST);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);

        edgeEffectShader_.use();

        for (unsigned int i = 0; i < N_TREES; i++)
        {
            glm::mat4 model(1.0f);
            float x = treesPosition[i];
            float z = (i % 2 == 0 ? 3.f : -3.f);
            model = glm::translate(model, glm::vec3(x, -0.15f, z));
            model = glm::rotate(model, treesOrientation[i], glm::vec3(0.f, 1.f, 0.f));
            model = glm::scale(model, glm::vec3(treesScale[i]));

            glm::mat4 scaleAroundCenter = glm::translate(glm::mat4(1.0f), tree_.center_);
            scaleAroundCenter = glm::scale(scaleAroundCenter, glm::vec3(1.03f));
            scaleAroundCenter = glm::translate(scaleAroundCenter, -tree_.center_);

            glm::mat4 finalModel = model * scaleAroundCenter;
            glm::mat4 mvpScaled = projView * finalModel;

            edgeEffectShader_.setMatrices(mvpScaled, view, finalModel);
            tree_.draw();
        }

        glStencilMask(0xFF);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
    }

    void drawGround(const glm::mat4& projView)
    {

        celShadingShader_.use();
        glm::mat4 view = getViewMatrix();

        setMaterial(streetMat);
        streetTexture_.use();
        streetTexture_.setWrap(GL_REPEAT);
        streetTexture_.enableMipmap();
        streetTexture_.setFiltering(GL_NEAREST_MIPMAP_NEAREST);
        glm::mat4 model(1.0f);
        model = glm::scale(model, glm::vec3(100.f, 1.f, 5.f));

        mat4 mvp = projView * model;
        celShadingShader_.setMatrices(mvp, view, model);
        street_.draw();
        
        
        setMaterial(grassMat);
        grassTexture_.use();
        grassTexture_.setWrap(GL_REPEAT);
        grassTexture_.enableMipmap();
        grassTexture_.setFiltering(GL_NEAREST_MIPMAP_NEAREST);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -0.1f, 0.f));
        model = glm::scale(model, glm::vec3(100.f, 1.f, 50.f));
        mvp = projView * model;
        celShadingShader_.setMatrices(mvp, view, model);
        grass_.draw();
    }

    void drawCar(glm::mat4& projView, glm::mat4& view)
    { 
        setMaterial(defaultMat);
        carTexture_.use();
        carTexture_.setWrap(GL_REPEAT);
        carTexture_.enableMipmap();
        carTexture_.setFiltering(GL_NEAREST_MIPMAP_NEAREST);

        car_.update(deltaTime_);
        car_.draw(projView, view);
        }


    glm::mat4 getViewMatrix()
    {
        glm::mat4 view(1.0f);

        view = glm::rotate(view, -cameraOrientation_.x, glm::vec3(1, 0, 0));
        view = glm::rotate(view, -cameraOrientation_.y, glm::vec3(0, 1, 0));
        view = glm::translate(view, -cameraPosition_);

        return view;
    }


    glm::mat4 getPerspectiveProjectionMatrix()
    {
        // TODO: Pertinent de modifier la distance ici.
        const float far = 300.f;

        float screenRatio = static_cast<float>(window_.getSize().x) / static_cast<float>(window_.getSize().y);
        return glm::perspective(glm::radians(70.0f), screenRatio, 0.1f, far);

    }

    void setLightingUniform()
    {
        celShadingShader_.use();
        glUniform1i(celShadingShader_.nSpotLightsULoc, N_STREETLIGHTS + 4);

        float ambientIntensity = 0.05;
        glUniform3f(celShadingShader_.globalAmbientULoc, ambientIntensity, ambientIntensity, ambientIntensity);
    }

    void toggleSun()
    {
        if (isDay_)
        {
            lightsData_.dirLight.ambient = glm::vec4(0.2f, 0.2f, 0.2f, 0.0f);
            lightsData_.dirLight.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
            lightsData_.dirLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);
        }
        else
        {
            lightsData_.dirLight.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            lightsData_.dirLight.diffuse = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            lightsData_.dirLight.specular = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }

    void toggleStreetlight()
    {
        if (isDay_)
        {
            for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
            {
                lightsData_.spotLights[i].ambient = glm::vec4(glm::vec3(0.0f), 0.0f);
                lightsData_.spotLights[i].diffuse = glm::vec4(glm::vec3(0.0f), 0.0f);
                lightsData_.spotLights[i].specular = glm::vec4(glm::vec3(0.0f), 0.0f);
            }
        }
        else
        {
            for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
            {
                lightsData_.spotLights[i].ambient = glm::vec4(glm::vec3(0.02f), 0.0f);
                lightsData_.spotLights[i].diffuse = glm::vec4(glm::vec3(0.8f), 0.0f);
                lightsData_.spotLights[i].specular = glm::vec4(glm::vec3(0.4f), 0.0f);
            }
        }
    }

    void updateCarLight()
    {
        if (car_.isHeadlightOn)
        {
            lightsData_.spotLights[N_STREETLIGHTS].ambient = glm::vec4(glm::vec3(0.01), 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS].diffuse = glm::vec4(glm::vec3(1.0), 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS].specular = glm::vec4(glm::vec3(0.4), 0.0f);

            lightsData_.spotLights[N_STREETLIGHTS + 1].ambient = glm::vec4(glm::vec3(0.01), 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 1].diffuse = glm::vec4(glm::vec3(1.0), 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 1].specular = glm::vec4(glm::vec3(0.4), 0.0f);

            lightsData_.spotLights[N_STREETLIGHTS].position = glm::vec4(-1.6, 0.64, -0.45, 1.0f);
            lightsData_.spotLights[N_STREETLIGHTS].direction = glm::vec3(-10, -1, 0);

            lightsData_.spotLights[N_STREETLIGHTS + 1].position = glm::vec4(-1.6, 0.64, 0.45, 1.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 1].direction = glm::vec3(-10, -1, 0);
        }
        else
        {
            lightsData_.spotLights[N_STREETLIGHTS].ambient = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS].diffuse = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS].specular = glm::vec4(0.0f);

            lightsData_.spotLights[N_STREETLIGHTS + 1].ambient = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 1].diffuse = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 1].specular = glm::vec4(0.0f);
        }

        if (car_.isBraking)
        {
            lightsData_.spotLights[N_STREETLIGHTS + 2].ambient = glm::vec4(0.01, 0.0, 0.0, 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 2].diffuse = glm::vec4(0.9, 0.1, 0.1, 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 2].specular = glm::vec4(0.35, 0.05, 0.05, 0.0f);

            lightsData_.spotLights[N_STREETLIGHTS + 3].ambient = glm::vec4(0.01, 0.0, 0.0, 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 3].diffuse = glm::vec4(0.9, 0.1, 0.1, 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 3].specular = glm::vec4(0.35, 0.05, 0.05, 0.0f);

            lightsData_.spotLights[N_STREETLIGHTS + 2].position = glm::vec4(1.6, 0.64, -0.45, 1.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 2].direction = glm::vec3(10, -1, 0);

            lightsData_.spotLights[N_STREETLIGHTS + 3].position = glm::vec4(1.6, 0.64, 0.45, 1.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 3].direction = glm::vec3(10, -1, 0);
        }
        else
        {
            lightsData_.spotLights[N_STREETLIGHTS + 2].ambient = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 2].diffuse = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 2].specular = glm::vec4(0.0f);

            lightsData_.spotLights[N_STREETLIGHTS + 3].ambient = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 3].diffuse = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 3].specular = glm::vec4(0.0f);
        }
    }

    void setMaterial(Material& mat)
    {
        material_.updateData(&mat, 0, sizeof(Material));
    }

    void sceneMain()
    {
        ImGui::Begin("Scene Parameters");
        // TODO: À ajouter
        ImGui::SliderInt("Bezier Number Of Points", (int*)&bezierNPoints, 0, 16);
        if (ImGui::Button("Animate Camera"))
        {
            isAnimatingCamera = true;
            cameraMode = 1;
        }
        //
        if (ImGui::Button("Toggle Day/Night"))
        {
            isDay_ = !isDay_;
            toggleSun();
            toggleStreetlight();
            lights_.updateData(&lightsData_, 0, sizeof(DirectionalLight) + N_STREETLIGHTS * sizeof(SpotLight));
        }
        ImGui::SliderFloat("Car Speed", &car_.speed, -10.0f, 10.0f, "%.2f m/s");
        ImGui::SliderFloat("Steering Angle", &car_.steeringAngle, -30.0f, 30.0f, "%.2f°");
        if (ImGui::Button("Reset Steering"))
            car_.steeringAngle = 0.f;
        ImGui::Checkbox("Headlight", &car_.isHeadlightOn);
        ImGui::Checkbox("Left Blinker", &car_.isLeftBlinkerActivated);
        ImGui::Checkbox("Right Blinker", &car_.isRightBlinkerActivated);
        ImGui::Checkbox("Brake", &car_.isBraking);
        ImGui::End();


        std::cout << "camera value: " << isAnimatingCamera << std::endl;
        if (isAnimatingCamera)
        {
            
            if (cameraAnimation < 5)
            {
                // TODO: Animation de la caméra
                // cameraPosition_ = ...

                cameraAnimation += deltaTime_ / 3.0;
            }
            else
            {
                // Remise à 0 de l'orientation
                glm::vec3 diff = car_.position - cameraPosition_;
                cameraOrientation_.y = M_PI + atan2(diff.z, diff.x);

                cameraAnimation = 0.f;
                isAnimatingCamera = false;
                cameraMode = 0;
            }
        }
        updateCameraInput();
        car_.update(deltaTime_);

        updateCarLight();
        lights_.updateData(&lightsData_.spotLights[N_STREETLIGHTS], sizeof(DirectionalLight) + N_STREETLIGHTS * sizeof(SpotLight), 4 * sizeof(SpotLight));

        glm::mat4 view = getViewMatrix();
        glm::mat4 proj = getPerspectiveProjectionMatrix();
        glm::mat4 projView = proj * view;

        setMaterial(windowMat);

        setMaterial(grassMat);
        drawTrees(projView);

        drawGround(projView);

        setMaterial(streetlightMat);
        drawStreetlights(projView);

        setMaterial(defaultMat);
        drawCar(projView, view);

        bool hasNumberOfSidesChanged = bezierNPoints != oldBezierNPoints;
        if (hasNumberOfSidesChanged)
        {
            oldBezierNPoints = bezierNPoints;

            // TODO: Calcul et mise à jour de la courbe
        }

        // TODO: Dessin de la courbe
        // glDraw...


        // TODO: Dessin du gazon
        // glDraw...
    }



    // TODO: Ajouter les attributs de vbo, ebo, vao nécessaire

private:
    // Shaders
    EdgeEffect edgeEffectShader_;
    CelShading celShadingShader_;
    Sky skyShader_;

    // Textures
    Texture2D grassTexture_;
    Texture2D streetTexture_;
    Texture2D carTexture_;
    Texture2D carWindowTexture_;
    Texture2D treeTexture_;
    Texture2D streetlightTexture_;
    Texture2D streetlightLightTexture_;
    TextureCubeMap skyboxTexture_;
    TextureCubeMap skyboxNightTexture_;

    // Uniform buffers
    UniformBuffer material_;
    UniformBuffer lights_;

    struct {
        DirectionalLight dirLight;
        SpotLight spotLights[16];
        //PointLight pointLights[4];
    } lightsData_;

    bool isDay_;

    Model tree_;
    Model streetlight_;
    Model streetlightLight_;
    Model grass_;
    Model street_;
    Model skybox_;
  

    GLuint vbo_, ebo_, vao_;

    // TP1
    Car car_;

    glm::vec3 cameraPosition_;
    glm::vec2 cameraOrientation_;

    static constexpr unsigned int N_TREES = 12;
    static constexpr unsigned int N_STREETLIGHTS = 5;
    glm::mat4 treeModelMatrices_[N_TREES];
    glm::mat4 streetlightModelMatrices_[N_STREETLIGHTS];
    glm::vec3 streetlightLightPositions[N_STREETLIGHTS];

    std::vector<float> lightsPosition;
    std::vector<float> treesPosition;
    std::vector<float> treesOrientation;
    std::vector<float> treesScale;

    // Imgui var
    const char* const SCENE_NAMES[1] = {
        "Main Scene",
    };
    const int N_SCENE_NAMES = sizeof(SCENE_NAMES) / sizeof(SCENE_NAMES[0]);
    int currentScene_;

    bool isMouseMotionEnabled_;
    bool isQWERTY_;

};


int main(int argc, char* argv[])
{
    WindowSettings settings = {};
    settings.fps = 60;
    settings.context.depthBits = 24;
    settings.context.stencilBits = 8;
    settings.context.antiAliasingLevel = 4;
    settings.context.majorVersion = 3;
    settings.context.minorVersion = 3;
    settings.context.attributeFlags = sf::ContextSettings::Attribute::Core;
    App app;
    app.run(argc, argv, "Tp2", settings);
}
