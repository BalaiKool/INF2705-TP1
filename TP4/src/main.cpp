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

#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;


struct App : public OpenGLApplication
{
    App()
    : cameraPosition_(0.f, 0.f, 0.f)
    , cameraOrientation_(0.f, 0.f)
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
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        loadShaderPrograms();

	}

	// Appelée à chaque trame. Le buffer swap est fait juste après.
	void drawFrame() override
	{
        
        ImGui::Begin("Scene Parameters");
        ImGui::End();
        
        sceneMain();
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
    
    void sceneMain()
    {
        updateCameraInput();
    }
    
private:    
    glm::vec3 cameraPosition_;
    glm::vec2 cameraOrientation_;
    

    GLuint vbo_, ebo_, vao_;

    // Imgui var
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
	settings.context.majorVersion = 3;
	settings.context.minorVersion = 3;
	settings.context.attributeFlags = sf::ContextSettings::Attribute::Core;

	App app;
	app.run(argc, argv, "Tp4", settings);
}
