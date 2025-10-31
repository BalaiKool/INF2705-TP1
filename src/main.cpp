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

#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;

struct Vertex
{
    vec3 position;
    vec3 color;
};


// Définition des couleurs
const vec4 red = { 1.f, 0.f, 0.f, 1.0f };
const vec4 green = { 0.f, 1.f, 0.f, 1.0f };
const vec4 blue = { 0.f, 0.f, 1.f, 1.0f };

struct App : public OpenGLApplication
{
    App()
        : cameraPosition_(17.f, 9.f, 4.5f)
        , cameraOrientation_(-.3, 1.25f)
        , isMouseMotionEnabled_(false)
    {
    }

    void init() override
    {
        // Le message expliquant les touches de clavier.
        setKeybindMessage(
            "ESC : quitter l'application." "\n"
            "W : déplacer la caméra vers l'avant." "\n"
            "S : déplacer la caméra vers l'arrière." "\n"
            "A : déplacer la caméra vers la gauche." "\n"
            "D : déplacer la caméra vers la droite." "\n"
            "Q : déplacer la caméra vers le bas." "\n"
            "E : déplacer la caméra vers le haut." "\n"
            "Flèches : tourner la caméra." "\n"
            "Souris : tourner la caméra" "\n"
            "Espace : activer/désactiver la souris." "\n"
        );

        // Config de base.
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        loadShaderPrograms();

        // TP 1
        loadModels();
        car_.mvpUniformLocation = mvpUniformLocation_;
        car_.colorModUniformLocation = colorModUniformLocation_;
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sceneModels();
    }

    // Appelée lorsque la fenêtre se ferme.
    void onClose() override
    {
        glDeleteBuffers(1, &vbo_);
        glDeleteBuffers(1, &ebo_);
        glDeleteVertexArrays(1, &vao_);
        glDeleteProgram(basicSP_);
        glDeleteProgram(transformSP_);
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

        positionOffset = glm::rotate(glm::mat4(1.0f), cameraOrientation_.y, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(positionOffset, 1);
        cameraPosition_ += positionOffset * glm::vec3(deltaTime_);
    }

    void loadModels()
    {
        car_.loadModels();
        tree_.load("../models/tree.ply");
        streetlight_.load("../models/streetlight.ply");
        grass_.load("../models/grass.ply");
        street_.load("../models/street.ply");
        initModelProperties();
    }

    void initModelProperties()
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
    GLuint loadShaderObject(GLenum type, const char* path)
    {
        std::ifstream file(path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();
        const char* src = source.c_str();

        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        checkShaderCompilingError(path, shader);
        return shader;
    }

    void loadShaderPrograms()
{
    GLuint vs = loadShaderObject(GL_VERTEX_SHADER, "./shaders/basic.vs.glsl");
    GLuint fs = loadShaderObject(GL_FRAGMENT_SHADER, "./shaders/basic.fs.glsl");
    basicSP_ = glCreateProgram();
    glAttachShader(basicSP_, vs);
    glAttachShader(basicSP_, fs);
    glLinkProgram(basicSP_);
    checkProgramLinkingError("basicSP", basicSP_);
    glDetachShader(basicSP_, vs);
    glDetachShader(basicSP_, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    const char* TRANSFORM_VERTEX_SRC_PATH   = "./shaders/transform.vs.glsl";
    const char* TRANSFORM_FRAGMENT_SRC_PATH = "./shaders/transform.fs.glsl";

    GLuint vs2 = loadShaderObject(GL_VERTEX_SHADER,   TRANSFORM_VERTEX_SRC_PATH);
    GLuint fs2 = loadShaderObject(GL_FRAGMENT_SHADER, TRANSFORM_FRAGMENT_SRC_PATH);
    transformSP_ = glCreateProgram();
    glAttachShader(transformSP_, vs2);
    glAttachShader(transformSP_, fs2);
    glLinkProgram(transformSP_);
    checkProgramLinkingError("transformSP", transformSP_);
    glDetachShader(transformSP_, vs2);
    glDetachShader(transformSP_, fs2);
    glDeleteShader(vs2);
    glDeleteShader(fs2);

    mvpUniformLocation_      = glGetUniformLocation(transformSP_, "uMVP");
    colorModUniformLocation_ = glGetUniformLocation(transformSP_, "uColorMod");

    if (mvpUniformLocation_ == -1)
        std::cerr << "Warning: uMVP not found in transform shader (transformSP_)." << std::endl;
    if (colorModUniformLocation_ == -1)
        std::cerr << "Warning: uColorMod not found in transform shader (transformSP_)." << std::endl;
}


    void drawStreetlights(glm::mat4& projView)
    {
        glUseProgram(transformSP_);

        for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
        {
            float x = lightsPosition[i];
            float z = (i % 2 == 0 ? 3.f : -3.f);

            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(x, -0.15f, z));

            if (i % 2 == 0) {
                model = model * glm::rotate(glm::mat4(1.0f), glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f));
            } else {
                model = model * glm::rotate(glm::mat4(1.0f), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
            }

            glUniformMatrix4fv(mvpUniformLocation_, 1, GL_FALSE, glm::value_ptr(projView * model));
            glUniform3fv(colorModUniformLocation_, 1, glm::value_ptr(glm::vec3(1.f, 1.f, 1.f)));
            streetlight_.draw();
        }
    }



    void drawTrees(glm::mat4& projView)
    {
        glUseProgram(transformSP_);

        for (unsigned int i = 0; i < N_TREES; i++)
        {
            glm::mat4 model(1.0f);
            float x = treesPosition[i];
            float z = (i % 2 == 0) ? 3.f : -3.f;
            model = glm::translate(model, glm::vec3(x, -0.15f, z));

            float angleRad = treesOrientation[i];
            model = glm::rotate(model, angleRad, glm::vec3(0.f, 1.f, 0.f));

            float scale = treesScale[i];
            model = glm::scale(model, glm::vec3(scale));

            glUniformMatrix4fv(mvpUniformLocation_, 1, GL_FALSE, glm::value_ptr(projView * model));
            glUniform3fv(colorModUniformLocation_, 1, glm::value_ptr(glm::vec3(0.5f, 0.3f, 0.1f))); // brown tree
            tree_.draw();
        }
    }



    void drawGround(glm::mat4& projView)
    {
        glUseProgram(transformSP_);
        
        glm::mat4 model(1.0f);
        model = glm::scale(model, glm::vec3(100.f, 1.f, 5.f));
        glUniformMatrix4fv(mvpUniformLocation_, 1, GL_FALSE, glm::value_ptr(projView * model));
        glUniform3fv(colorModUniformLocation_, 1, glm::value_ptr(glm::vec3(1.f, 1.f, 1.f)));
        street_.draw();
        
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -0.1f, 0.f));
        model = glm::scale(model, glm::vec3(100.f, 1.f, 50.f));
        glUniformMatrix4fv(mvpUniformLocation_, 1, GL_FALSE, glm::value_ptr(projView * model));
        glUniform3fv(colorModUniformLocation_, 1, glm::value_ptr(glm::vec3(0.4f, 0.8f, 0.4f)));
        grass_.draw();
    }

    void drawCar(glm::mat4& projView)
    { 
        car_.update(deltaTime_);
        car_.draw(projView);
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
        float screenRatio = static_cast<float>(window_.getSize().x) / static_cast<float>(window_.getSize().y);
        return glm::perspective(glm::radians(70.0f), screenRatio, 0.1f, 100.0f);
    }


    void sceneModels()
    {
        ImGui::Begin("Scene Parameters");
        ImGui::SliderFloat("Car speed", &car_.speed, -10.0f, 10.0f, "%.2f m/s");
        ImGui::SliderFloat("Steering Angle", &car_.steeringAngle, -30.0f, 30.0f, "%.2f°");
        if (ImGui::Button("Reset steering"))
            car_.steeringAngle = 0.f;
        ImGui::Checkbox("Headlight", &car_.isHeadlightOn);
        ImGui::Checkbox("Left Blinker", &car_.isLeftBlinkerActivated);
        ImGui::Checkbox("Right Blinker", &car_.isRightBlinkerActivated);
        ImGui::Checkbox("Brake", &car_.isBraking);
        ImGui::End();

        updateCameraInput();

        glm::mat4 proj = getPerspectiveProjectionMatrix();
        glm::mat4 view = getViewMatrix();
        glm::mat4 projView = proj * view;

        drawGround(projView);
        drawTrees(projView);
        drawStreetlights(projView);
        drawCar(projView);
    }


private:
    // Shaders
    GLuint basicSP_;
    GLuint transformSP_;
    GLuint colorModUniformLocation_;
    GLuint mvpUniformLocation_;

    GLuint vbo_, ebo_, vao_;

    // TP1
    Model tree_;
    Model streetlight_;
    Model grass_;
    Model street_;

    Car car_;

    glm::vec3 cameraPosition_;
    glm::vec2 cameraOrientation_;

    static constexpr unsigned int N_TREES = 12;
    static constexpr unsigned int N_STREETLIGHTS = 5;
    glm::mat4 treeModelMatrices_[N_TREES];
    glm::mat4 streetlightModelMatrices_[N_STREETLIGHTS];

    std::vector<float> lightsPosition;
    std::vector<float> treesPosition;
    std::vector<float> treesOrientation;
    std::vector<float> treesScale;

    const char* const SCENE_NAMES[2] = {
        "Introduction",
        "3D Model & transformation",
    };
    const int N_SCENE_NAMES = sizeof(SCENE_NAMES) / sizeof(SCENE_NAMES[0]);
    int currentScene_;

    bool isMouseMotionEnabled_;
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
    app.run(argc, argv, "Tp1", settings);
}
