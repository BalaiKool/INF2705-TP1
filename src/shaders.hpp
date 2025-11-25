#include "shader_program.hpp"

#include <glm/glm.hpp>

// Implémentation de vos shaders ici.
// Ils doivent hérité de ShaderProgram et implémenter les méthodes virtuelles pures
// load() et getAllUniformLocations().
// Mémoriser les uniforms locations dans des attributs public. Vous pouvez ajouter ce que
// vous voulez dans les classes et mieux séparer le code.
// Voir exemple avec le shader du tp1, considérant les variables uniformes dans le shader:


class EdgeEffect : public ShaderProgram
{
public:
    GLuint mvpULoc = 0;
    GLuint viewULoc = 0;
    GLuint modelULoc = 0;

    void setMatrices(glm::mat4& mvp, glm::mat4& view, glm::mat4& model);

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};


class Sky : public ShaderProgram
{
public:

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};


class CelShading : public ShaderProgram
{
public:
    GLuint mvpULoc;
    GLuint viewULoc;
    GLuint modelViewULoc;
    GLuint normalULoc;
    
    GLuint nSpotLightsULoc;
    
    GLuint globalAmbientULoc;

    inline void use() { glUseProgram(id_); }

public:
    void setMatrices(glm::mat4& mvp, glm::mat4& view, glm::mat4& model);

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
    virtual void assignAllUniformBlockIndexes() override;
};


class BasicShader : public ShaderProgram
{
public:
    GLint mvpULoc = -1;

protected:
    void load() override {
        loadShaderSource(GL_VERTEX_SHADER, "./shaders/basic.vs.glsl");
        loadShaderSource(GL_FRAGMENT_SHADER, "./shaders/basic.fs.glsl");
        link();
    }

    void getAllUniformLocations() override {
        mvpULoc = glGetUniformLocation(id_, "mvp");
    }
};

class GrassShader : public ShaderProgram
{
public:
    GLuint mvpULoc = 0;
    GLuint modelULoc = 0;
    GLuint timeULoc = 0;
    GLint modelViewULoc;

    inline void use() { glUseProgram(id_); }

    void setMatrices(glm::mat4& mvp, glm::mat4& model);
    void setModelView(const glm::mat4& mv);

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};

class ParticlesShader : public ShaderProgram
{
public:
    GLuint modelViewULoc = 0;
    GLuint projectionULoc = 0;
    GLuint texSamplerULoc = 0;
    GLuint cameraRightULoc = 0;
    GLuint cameraUpULoc = 0;

    void setMatrices(const glm::mat4& modelView, const glm::mat4& projection, const glm::vec3& cameraRight, const glm::vec3& cameraUp);

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};

class ParticlesUpdateShader : public ShaderProgram
{
public:
    GLuint timeULoc;
    GLuint deltaTimeULoc;
    GLuint emitterPosULoc;
    GLuint emitterDirULoc;

protected:
    // Load shader and link
    virtual void load() override;

    // Get uniform locations after linking
    virtual void getAllUniformLocations() override;
};