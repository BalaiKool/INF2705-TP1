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

#include <imgui/imgui.h>

#include <inf2705/OpenGLApplication.hpp>

#include "model.hpp"
#include "crystal.hpp"
#include "rocky_floor.hpp"
#include "cloud.hpp"
#include "light.hpp"
#include "audiovisualizer.hpp"

#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;

struct Vertex
{
    vec3 position;
    vec3 color;
    vec2 uv;
};

const vec4 red = { 1.f, 0.f, 0.f, 1.0f };
const vec4 green = { 0.f, 1.f, 0.f, 1.0f };
const vec4 blue = { 0.f, 0.f, 1.f, 1.0f };

struct App : public OpenGLApplication
{
    App()
        : cameraPosition_(0.f, 0.f, 5.f)
        , cameraOrientation_(0.f, 0.f)
        , isMouseMotionEnabled_(false)
        , isQWERTY_(true)
    {
    }

    void init() override
    {
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

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        loadShaderPrograms();

        light_.initialize();

        loadModels();
        loadTextures();
        crystal_.mvpUniformLocation = mvpUniformLocation_;
        crystal_.colorModUniformLocation = colorModUniformLocation_;

        rockyFloor_.initialize();
        clouds_ = Clouds(50);
        clouds_.initialize();

        audioViz_.loadMusic("lofi-lofi-chill-lofi-girl-438671.mp3"); //Royalty-free music de https://pixabay.com/music/search/lofi/
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

    void drawFrame() override
    {
        static sf::Time lastTime = clock.getElapsedTime();
        sf::Time now = clock.getElapsedTime();
        deltaTime_ = (now - lastTime).asSeconds();
        lastTime = now;

        audioViz_.update(deltaTime_);

        float grayValue = audioViz_.getVolume();

        float baseDark = 0.1f;
        grayValue = baseDark + grayValue * (1.0f - baseDark);

        if (grayValue < 0.0f) grayValue = 0.0f;
        if (grayValue > 1.0f) grayValue = 1.0f;

        glClearColor(grayValue, grayValue, grayValue, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui::Begin("Scene Parameters");

        if (audioViz_.isMusicLoaded()) {
            if (ImGui::Button(audioViz_.isMusicPlaying() ? "Pause Music" : "Play Music")) {
                audioViz_.togglePlayback();
            }

            ImGui::SameLine();
            if (ImGui::Button("Stop")) {
                audioViz_.stopMusic();
            }


        }
        else {
            ImGui::Text("No audio file loaded");
        }

        ImGui::Text("Vitesse Rotation");
        float rotationDeg = glm::degrees(crystal_.rotationSpeed);
        ImGui::SliderFloat("##RotationSpeed", &rotationDeg, 0.0f, 360.0f);

        ImGui::Text("Vitesse Flottaison");
        ImGui::SliderFloat("##FloatSpeed", &crystal_.floatSpeed, 0.0f, 5.0f);

        ImGui::Text("Amplitude Flottaison");
        ImGui::SliderFloat("##FloatAmplitude", &crystal_.floatAmplitude, 0.0f, 1.0f);

        ImGui::Separator();
        ImGui::Text("Lumiere/ombres");

        auto& sunLight = light_.getSunLight();

        ImGui::Checkbox("Enable Lumiere", &sunLight.enabled);

        if (sunLight.enabled) {
            ImGui::Indent();

            ImGui::ColorEdit3("Couleur lumiere", &sunLight.color[0]);
            ImGui::SliderFloat("Intensite lumiere", &sunLight.intensity, 0.0f, 3.0f);

            ImGui::Text("Direction lumiere");
            ImGui::SliderFloat("Direction X", &sunLight.direction[0], -1.0f, 1.0f);
            ImGui::SliderFloat("Direction Y", &sunLight.direction[1], -1.0f, 1.0f);
            ImGui::SliderFloat("Direction Z", &sunLight.direction[2], -1.0f, 1.0f);

            if (ImGui::Button("Normaliser Direction")) {
                sunLight.direction = glm::normalize(sunLight.direction);
            }

            ImGui::Unindent();
        }

        ImGui::Separator();
        ImGui::Checkbox("Enable ombres", &sunLight.castShadows);

        ImGui::End();

        sceneMain();
    }

    void onClose() override
    {
        glDeleteBuffers(1, &vbo_);
        glDeleteBuffers(1, &ebo_);
        glDeleteVertexArrays(1, &vao_);

        if (crystalTexture_) glDeleteTextures(1, &crystalTexture_);
        if (crystalNormalTexture_) glDeleteTextures(1, &crystalNormalTexture_);
        if (crystalRoughnessTexture_) glDeleteTextures(1, &crystalRoughnessTexture_);
    }

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

    void loadModels()
    {
        crystal_.loadModels();
    }

    void loadTextures()
    {
        // Load color texture
        sf::Image imgColor;
        if (!imgColor.loadFromFile("../textures/crystal_uv_map_purple_full.png"))
        {
            std::cerr << "Failed to load color texture!" << std::endl;
            return;
        }
        imgColor.flipVertically();

        glGenTextures(1, &crystalTexture_);
        glBindTexture(GL_TEXTURE_2D, crystalTexture_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgColor.getSize().x, imgColor.getSize().y,
            0, GL_RGBA, GL_UNSIGNED_BYTE, imgColor.getPixelsPtr());
        glGenerateMipmap(GL_TEXTURE_2D);

        // Load normal map
        sf::Image imgNormal;
        if (!imgNormal.loadFromFile("../textures/crystal_normal.png"))
        {
            std::cerr << "Failed to load normal texture!" << std::endl;
            crystalNormalTexture_ = 0; // Mark as invalid
        }
        else
        {
            imgNormal.flipVertically();
            glGenTextures(1, &crystalNormalTexture_);
            glBindTexture(GL_TEXTURE_2D, crystalNormalTexture_);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgNormal.getSize().x, imgNormal.getSize().y,
                0, GL_RGBA, GL_UNSIGNED_BYTE, imgNormal.getPixelsPtr());
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        // Load roughness map
        sf::Image imgRoughness;
        if (!imgRoughness.loadFromFile("../textures/crystal_roughness.png"))
        {
            std::cerr << "Failed to load roughness texture!" << std::endl;
            crystalRoughnessTexture_ = 0; // Mark as invalid
        }
        else
        {
            imgRoughness.flipVertically();
            glGenTextures(1, &crystalRoughnessTexture_);
            glBindTexture(GL_TEXTURE_2D, crystalRoughnessTexture_);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgRoughness.getSize().x, imgRoughness.getSize().y,
                0, GL_RGBA, GL_UNSIGNED_BYTE, imgRoughness.getPixelsPtr());
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        crystal_.setColorTexture(crystalTexture_);
        crystal_.setNormalTexture(crystalNormalTexture_);
        crystal_.setRoughnessTexture(crystalRoughnessTexture_);
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
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
                positionOffset.y += SPEED;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
            positionOffset.y -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
            positionOffset.y += SPEED;

        positionOffset = glm::rotate(glm::mat4(1.0f), cameraOrientation_.y, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(positionOffset, 1);
        cameraPosition_ += positionOffset * glm::vec3(deltaTime_);
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

        const char* TRANSFORM_VERTEX_SRC_PATH = "./shaders/transform.vs.glsl";
        const char* TRANSFORM_FRAGMENT_SRC_PATH = "./shaders/transform.fs.glsl";

        GLuint vs2 = loadShaderObject(GL_VERTEX_SHADER, TRANSFORM_VERTEX_SRC_PATH);
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

        mvpUniformLocation_ = glGetUniformLocation(transformSP_, "uMVP");

        if (mvpUniformLocation_ == -1)
            std::cerr << "Warning: uMVP not found in transform shader (transformSP_)." << std::endl;

        const char* CRYSTAL_VERTEX_SRC_PATH = "./crystal.vs.glsl";
        const char* CRYSTAL_FRAGMENT_SRC_PATH = "./crystal.fs.glsl";

        GLuint vs3 = loadShaderObject(GL_VERTEX_SHADER, CRYSTAL_VERTEX_SRC_PATH);
        GLuint fs3 = loadShaderObject(GL_FRAGMENT_SHADER, CRYSTAL_FRAGMENT_SRC_PATH);

        crystalShaderProgram_ = glCreateProgram();
        glAttachShader(crystalShaderProgram_, vs3);
        glAttachShader(crystalShaderProgram_, fs3);
        glLinkProgram(crystalShaderProgram_);
        checkProgramLinkingError("crystalShader", crystalShaderProgram_);
        glDetachShader(crystalShaderProgram_, vs3);
        glDetachShader(crystalShaderProgram_, fs3);
        glDeleteShader(vs3);
        glDeleteShader(fs3);

        crystal_.mvpUniformLocation = glGetUniformLocation(crystalShaderProgram_, "uMVP");
        crystal_.modelUniformLocation = glGetUniformLocation(crystalShaderProgram_, "uModel");
        crystal_.textureUniformLocation = glGetUniformLocation(crystalShaderProgram_, "uTexture");
        crystal_.lightPosUniformLocation = glGetUniformLocation(crystalShaderProgram_, "uLightPos");
        crystal_.lightColorUniformLocation = glGetUniformLocation(crystalShaderProgram_, "uLightColor");
        crystal_.lightIntensityUniformLocation = glGetUniformLocation(crystalShaderProgram_, "uLightIntensity");
        crystal_.cameraPosUniformLocation = glGetUniformLocation(crystalShaderProgram_, "uCameraPos");
    }

    void drawCrystal(glm::mat4& projView) {
        glUseProgram(crystalShaderProgram_);

        crystal_.update(deltaTime_);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), crystal_.position);
        model = glm::rotate(model, crystal_.orientation.y, glm::vec3(0.f, 1.f, 0.f));
        model = glm::scale(model, glm::vec3(4.0f));

        glm::mat4 mvp = projView * model;

        glUniformMatrix4fv(crystal_.mvpUniformLocation, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(crystal_.modelUniformLocation, 1, GL_FALSE, glm::value_ptr(model));

        auto& sunLight = light_.getSunLight();

        GLint lightingEnabledLoc = glGetUniformLocation(crystalShaderProgram_, "uLightingEnabled");
        if (lightingEnabledLoc != -1) {
            glUniform1i(lightingEnabledLoc, sunLight.enabled ? 1 : 0);
        }

        if (sunLight.enabled) {
            glUniform3f(crystal_.lightPosUniformLocation,
                sunLight.direction.x, sunLight.direction.y, sunLight.direction.z);
            glUniform3f(crystal_.lightColorUniformLocation,
                sunLight.color.x, sunLight.color.y, sunLight.color.z);
            glUniform1f(crystal_.lightIntensityUniformLocation, sunLight.intensity);
        }

        glUniform3f(crystal_.cameraPosUniformLocation,
            cameraPosition_.x, cameraPosition_.y, cameraPosition_.z);


        GLint modelLoc = glGetUniformLocation(transformSP_, "uModel");
        if (modelLoc != -1)
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        GLint normalMatLoc = glGetUniformLocation(transformSP_, "uNormalMatrix");
        if (normalMatLoc != -1)
            glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, crystalTexture_);
        if (crystal_.textureUniformLocation != -1) {
            glUniform1i(crystal_.textureUniformLocation, 0);
        }

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, crystalNormalTexture_);
        glUniform1i(glGetUniformLocation(transformSP_, "uNormalMap"), 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, crystalRoughnessTexture_);
        glUniform1i(glGetUniformLocation(transformSP_, "uRoughnessMap"), 2);

        GLint lightPosLoc = glGetUniformLocation(transformSP_, "uLightPos");
        if (lightPosLoc != -1)
            glUniform3f(lightPosLoc, 5.0f, 5.0f, 5.0f);

        GLint viewPosLoc = glGetUniformLocation(transformSP_, "uViewPos");
        if (viewPosLoc != -1)
            glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPosition_));

        GLint lightColorLoc = glGetUniformLocation(transformSP_, "uLightColor");
        if (lightColorLoc != -1)
            glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);

        crystal_.draw();


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void sceneMain()
    {
        updateCameraInput();

        glm::mat4 proj = getPerspectiveProjectionMatrix();
        glm::mat4 view = getViewMatrix();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        std::vector<glm::vec3> cloudPositions;
        std::vector<float> cloudSizes;
        std::vector<float> cloudAlphas;

        for (int i = 0; i < clouds_.getCloudCount(); i++) {
            auto cloud = clouds_.getCloud(i);
            if (cloud.alpha > 0.01f) {
                cloudPositions.push_back(cloud.position);
                cloudSizes.push_back(cloud.scale.x);
                cloudAlphas.push_back(cloud.alpha);
            }
        }

        rockyFloor_.draw(proj, view, cameraPosition_,
            light_.getSunLight(),
            crystal_.position,
            crystal_.position.y,
            cloudPositions,
            cloudSizes,
            cloudAlphas);

        glm::mat4 projView = proj * view;

        drawCrystal(projView);

        clouds_.update(deltaTime_);
        clouds_.draw(proj, view, light_.getSunLight(), cameraPosition_);
    }

    glm::mat4 getPerspectiveProjectionMatrix()
    {
        float screenRatio = static_cast<float>(window_.getSize().x) / static_cast<float>(window_.getSize().y);
        return glm::perspective(glm::radians(70.0f), screenRatio, 0.1f, 100.0f);
    }

    glm::mat4 getViewMatrix()
    {
        glm::mat4 view(1.0f);

        view = glm::rotate(view, -cameraOrientation_.x, glm::vec3(1, 0, 0));
        view = glm::rotate(view, -cameraOrientation_.y, glm::vec3(0, 1, 0));
        view = glm::translate(view, -cameraPosition_);

        return view;
    }

private:
    GLuint basicSP_;
    GLuint transformSP_;
    GLuint crystalShaderProgram_;
    GLuint colorModUniformLocation_;
    GLuint mvpUniformLocation_;

    GLuint vbo_, ebo_, vao_;
    glm::vec3 cameraPosition_;
    glm::vec2 cameraOrientation_;

    Crystal crystal_;
    GLuint crystalTexture_;
    GLuint crystalNormalTexture_;
    GLuint crystalRoughnessTexture_;

    RockyFloor rockyFloor_;

    Clouds clouds_;
    float cloudSpeed_ = 1.0f;
    float cloudAlpha_ = 0.6f;

    sf::Clock clock;
    float deltaTime_;

    Light light_;

    AudioVisualizer audioViz_;
    bool audioEnabled_ = false;

    const char* const SCENE_NAMES[1] = {
        "Main Scene"
    };
    const int N_SCENE_NAMES = sizeof(SCENE_NAMES) / sizeof(SCENE_NAMES[0]);

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
    settings.context.majorVersion = 5;
    settings.context.minorVersion = 1;
    settings.context.attributeFlags = sf::ContextSettings::Attribute::Core;

    App app;
    app.run(argc, argv, "Tp4", settings);
}
