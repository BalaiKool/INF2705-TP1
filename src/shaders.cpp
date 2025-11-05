#include "shaders.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>


void EdgeEffect::load()
{
    const char* VERTEX_SRC_PATH = "./shaders/edge.vs.glsl";
    const char* FRAGMENT_SRC_PATH = "./shaders/edge.fs.glsl";
    
    name_ = "EdgeEffect";
    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();
}

void EdgeEffect::getAllUniformLocations()
{
    mvpULoc = glGetUniformLocation(id_, "mvp");
    viewULoc = glGetUniformLocation(id_, "view");
    modelULoc = glGetUniformLocation(id_, "model");
}

void EdgeEffect::setMatrices(glm::mat4& mvp, glm::mat4& view, glm::mat4& model)
{
    glUniformMatrix4fv(mvpULoc, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(viewULoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(modelULoc, 1, GL_FALSE, glm::value_ptr(model));
}

void Sky::load()
{
    const char* VERTEX_SRC_PATH = "./shaders/sky.vs.glsl";
    const char* FRAGMENT_SRC_PATH = "./shaders/sky.fs.glsl";
    
    name_ = "Sky";
    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();
}

void Sky::getAllUniformLocations()
{
}


void CelShading::load()
{
    const char* VERTEX_SRC_PATH = "./shaders/phong.vs.glsl";
    const char* FRAGMENT_SRC_PATH = "./shaders/phong.fs.glsl";

    std::cout << "Loading CelShading shader..." << std::endl;
    name_ = "CelShading";
    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();
    std::cout << "CelShading shader loaded with ID: " << id_ << std::endl;
}

void CelShading::getAllUniformLocations()
{
    mvpULoc = glGetUniformLocation(id_, "mvp");
    viewULoc = glGetUniformLocation(id_, "view");
    modelViewULoc = glGetUniformLocation(id_, "modelView");
    normalULoc = glGetUniformLocation(id_, "normalMatrix");
    
    nSpotLightsULoc = glGetUniformLocation(id_, "nSpotLights");
    
    globalAmbientULoc = glGetUniformLocation(id_, "globalAmbient");
}

void CelShading::assignAllUniformBlockIndexes()
{
    setUniformBlockBinding("MaterialBlock", 0);
    setUniformBlockBinding("LightingBlock", 1);
}


void CelShading::setMatrices(glm::mat4& mvp, glm::mat4& view, glm::mat4& model)
{
    //use();
    glm::mat4 modelView = view * model;
    
    glUniformMatrix4fv(viewULoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(mvpULoc, 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(modelViewULoc, 1, GL_FALSE, &modelView[0][0]);
    glUniformMatrix3fv(normalULoc, 1, GL_TRUE, glm::value_ptr(glm::inverse(glm::mat3(modelView))));
}

