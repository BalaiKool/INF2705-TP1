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
        : nSide_(5)
        , oldNSide_(0)
        , cameraPosition_(0.f, 0.f, 0.f)
        , cameraOrientation_(0.f, 0.f)
        , currentScene_(0)
        , isMouseMotionEnabled_(false)
    {
    }

    void init() override
    {
        // Le message expliquant les touches de clavier.
        setKeybindMessage(
            "ESC : quitter l'application." "\n"
            "T : changer de scène." "\n"
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

        // Partie 1
        initShapeData();

        // Partie 2
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

        ImGui::Begin("Scene Parameters");
        ImGui::Combo("Scene", &currentScene_, SCENE_NAMES, N_SCENE_NAMES);
        ImGui::End();

        switch (currentScene_)
        {
        case 0: sceneShape();  break;
        case 1: sceneModels(); break;
        }
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
        case T:
            currentScene_ = ++currentScene_ < N_SCENE_NAMES ? currentScene_ : 0;
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
    }

    void initStreetlights()
    {
        lightsPosition.clear();
        lightsPosition.reserve(N_STREETLIGHTS);
        float z = -60.f; // Terrain va de -60 à 40 dans les z


        for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
        {
            z += 100.f / N_STREETLIGHTS+(rand() % N_STREETLIGHTS);
            z = std::min(z, 40.f);
            lightsPosition.push_back(z);
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
    // Partie 1 : basic (déjà présent)
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

    // Partie 2 : transform (pour modèles 3D)
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

    // Récupérer les locations d'uniformes (uMVP et uColorMod)
    mvpUniformLocation_      = glGetUniformLocation(transformSP_, "uMVP");
    colorModUniformLocation_ = glGetUniformLocation(transformSP_, "uColorMod");

    if (mvpUniformLocation_ == -1)
        std::cerr << "Warning: uMVP not found in transform shader (transformSP_)." << std::endl;
    if (colorModUniformLocation_ == -1)
        std::cerr << "Warning: uColorMod not found in transform shader (transformSP_)." << std::endl;
}

    void generateNgon(unsigned int side)
    {
        const float RADIUS = 0.7f;
        vertices_[0].position = glm::vec3(0.f, 0.f, 0.f);
        vertices_[0].color = glm::vec3(1.f, 1.f, 1.f);

        for (unsigned int i = 0; i < side; ++i)
        {
            float angle = (2.f * M_PI * i) / side + M_PI/2;
            vertices_[i + 1].position = glm::vec3(
                RADIUS * cos(angle),
                RADIUS * sin(angle),
                0.f
            );

            switch (i % 3) {
            case 0: vertices_[i + 1].color = red; break; // rouge
            case 1: vertices_[i + 1].color = green; break; // vert
            case 2: vertices_[i + 1].color = blue; break; // bleu
            }
        }

        for (unsigned int i = 0; i < side; ++i)
        {
            elements_[i * 3 + 0] = 0;
            elements_[i * 3 + 1] = i + 1;
            elements_[i * 3 + 2] = (i + 1) % side + 1;
        }
    }




    void initShapeData()
    {
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);

        glBindVertexArray(vao_);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_), nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements_), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void sceneShape()
    {
        ImGui::Begin("Scene Parameters");
        ImGui::SliderInt("Sides", &nSide_, MIN_N_SIDES, MAX_N_SIDES);
        ImGui::End();

        bool hasNumberOfSidesChanged = nSide_ != oldNSide_;
        if (hasNumberOfSidesChanged)
        {
            oldNSide_ = nSide_;
            generateNgon(nSide_);

            glBindBuffer(GL_ARRAY_BUFFER, vbo_);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * (nSide_ + 1), vertices_.data());

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * nSide_ * 3, elements_.data());
        }

        glUseProgram(basicSP_);
        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, nSide_ * 3, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }



    void drawStreetlights(glm::mat4& projView)
    {
        glUseProgram(transformSP_);

        for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
        {
            float z = lightsPosition[i];
            float x = (i % 2 == 0 ? 3.f : -3.f);

            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(x, -0.15f, z));

            if (i % 2 == 1) {
                model = model * glm::rotate(glm::mat4(1.0f), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
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
            float z = i * 8.f;
            float x = (i % 2 == 0) ? 3.f : -3.f;
            model = glm::translate(model, glm::vec3(x, -0.15f, z));

            float angleDeg = static_cast<float>(rand() % 360);
            float angleRad = glm::radians(angleDeg);
            model = glm::rotate(model, angleRad, glm::vec3(0.f, 1.f, 0.f));

            float scale = 0.6f + (rand() % 60) / 100.f;
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
        model = glm::scale(model, glm::vec3(5.f, 1.f, 100.f));
        glUniformMatrix4fv(mvpUniformLocation_, 1, GL_FALSE, glm::value_ptr(projView * model));
        glUniform3fv(colorModUniformLocation_, 1, glm::value_ptr(glm::vec3(1.f, 1.f, 1.f)));
        street_.draw();

        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -0.1f, 0.f));
        model = glm::scale(model, glm::vec3(50.f, 1.f, 100.f));
        glUniformMatrix4fv(mvpUniformLocation_, 1, GL_FALSE, glm::value_ptr(projView * model));
        glUniform3fv(colorModUniformLocation_, 1, glm::value_ptr(glm::vec3(0.4f, 0.8f, 0.4f)));
        grass_.draw();
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
        car_.update(deltaTime_);

        glm::mat4 proj = getPerspectiveProjectionMatrix();
        glm::mat4 view = getViewMatrix();
        glm::mat4 projView = proj * view;

        drawGround(projView);
        drawTrees(projView);
        drawStreetlights(projView);

        glUseProgram(transformSP_);
        car_.draw(projView);
    }


private:
    // Shaders
    GLuint basicSP_;
    GLuint transformSP_;
    GLuint colorModUniformLocation_;
    GLuint mvpUniformLocation_;

    // Partie 1
    GLuint vbo_, ebo_, vao_;

    static constexpr unsigned int MIN_N_SIDES = 5;
    static constexpr unsigned int MAX_N_SIDES = 12;

    // TODO: Modifiez les types de vertices_ et elements_ pour votre besoin.
    std::array<Vertex, MAX_N_SIDES + 1> vertices_;
    std::array<GLuint, MAX_N_SIDES * 3> elements_;

    int nSide_, oldNSide_;

    // Partie 2
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

    //Objects properties
    std::vector<float> lightsPosition;

    // Imgui var
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
