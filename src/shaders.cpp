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

void GrassShader::load()
{
    const char* VERTEX_SRC_PATH = "./shaders/grass.vs.glsl";
    const char* TCS_SRC_PATH = "./shaders/grass.tcs.glsl";
    const char* TES_SRC_PATH = "./shaders/grass.tes.glsl";
    const char* GEOM_SRC_PATH = "./shaders/grass.gs.glsl";
    const char* FRAGMENT_SRC_PATH = "./shaders/grass.fs.glsl";

    name_ = "GrassShader";

    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_TESS_CONTROL_SHADER, TCS_SRC_PATH);
    loadShaderSource(GL_TESS_EVALUATION_SHADER, TES_SRC_PATH);
    loadShaderSource(GL_GEOMETRY_SHADER, GEOM_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);

    link();
}

void GrassShader::getAllUniformLocations()
{
    mvpULoc = glGetUniformLocation(id_, "mvp");
    modelViewULoc = glGetUniformLocation(id_, "modelView");

}

void GrassShader::setMatrices(glm::mat4& mvp, glm::mat4& model)
{
    glUniformMatrix4fv(mvpULoc, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(modelULoc, 1, GL_FALSE, glm::value_ptr(model));
}

void GrassShader::setModelView(const glm::mat4& mv)
{
    glUniformMatrix4fv(modelViewULoc, 1, GL_FALSE, glm::value_ptr(mv));
}

void ParticlesShader::load()
{
    const char* VERTEX_SRC_PATH = "./shaders/particlesDraw.vs.glsl";
    const char* GEOMETRY_SRC_PATH = "./shaders/particlesDraw.gs.glsl";
    const char* FRAGMENT_SRC_PATH = "./shaders/particlesDraw.fs.glsl";

    name_ = "Particles";

    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_GEOMETRY_SHADER, GEOMETRY_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();
}

void ParticlesShader::getAllUniformLocations()
{
    modelViewULoc = glGetUniformLocation(id_, "modelView");
    projectionULoc = glGetUniformLocation(id_, "projection");
    texSamplerULoc = glGetUniformLocation(id_, "textureSampler");
    cameraRightULoc = glGetUniformLocation(id_, "cameraRight");
    cameraUpULoc = glGetUniformLocation(id_, "cameraUp");
}

void ParticlesShader::setMatrices(const glm::mat4& modelView,
    const glm::mat4& projection,
    const glm::vec3& cameraRight,
    const glm::vec3& cameraUp)
{
    glUniformMatrix4fv(modelViewULoc, 1, GL_FALSE, glm::value_ptr(modelView));
    glUniformMatrix4fv(projectionULoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(cameraRightULoc, 1, glm::value_ptr(cameraRight));
    glUniform3fv(cameraUpULoc, 1, glm::value_ptr(cameraUp));
}

void ParticlesUpdateShader::load()
{
    const char* COMPUTE_SRC_PATH = "./shaders/particlesUpdate.cs.glsl";

    name_ = "ParticlesUpdate";
    std::cout << "Loading compute shader: " << COMPUTE_SRC_PATH << std::endl;

    loadShaderSource(GL_COMPUTE_SHADER, COMPUTE_SRC_PATH);
    link();

    std::cout << "Compute shader loaded with ID: " << id_ << std::endl;
}

void ParticlesUpdateShader::getAllUniformLocations()
{
    timeULoc = glGetUniformLocation(id_, "time");
    deltaTimeULoc = glGetUniformLocation(id_, "deltaTime");
    emitterPosULoc = glGetUniformLocation(id_, "emitterPosition");
    emitterDirULoc = glGetUniformLocation(id_, "emitterDirection");
}