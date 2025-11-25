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

class GrassShader : public ShaderProgram
{
public:
    GLuint mvpULoc = 0;
    GLuint modelULoc = 0;
    GLuint timeULoc = 0;

    inline void use() { glUseProgram(id_); }

    void setMatrices(glm::mat4& mvp, glm::mat4& model);

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};
