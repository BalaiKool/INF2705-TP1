#include "cloud.hpp"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <map>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace gl;

static void generateCloudMesh(std::vector<float>& vertices, std::vector<unsigned int>& indices, int detail = 3) {
    vertices.clear();
    indices.clear();
    const float t = (1.0f + sqrt(5.0f)) / 2.0f;
    std::vector<glm::vec3> baseVertices = {
        glm::normalize(glm::vec3(-1, t, 0)),
        glm::normalize(glm::vec3(1, t, 0)),
        glm::normalize(glm::vec3(-1, -t, 0)),
        glm::normalize(glm::vec3(1, -t, 0)),
        glm::normalize(glm::vec3(0, -1, t)),
        glm::normalize(glm::vec3(0, 1, t)),
        glm::normalize(glm::vec3(0, -1, -t)),
        glm::normalize(glm::vec3(0, 1, -t)),
        glm::normalize(glm::vec3(t, 0, -1)),
        glm::normalize(glm::vec3(t, 0, 1)),
        glm::normalize(glm::vec3(-t, 0, -1)),
        glm::normalize(glm::vec3(-t, 0, 1))
    };
    std::vector<unsigned int> baseIndices = {   // forme de nuage hard-coded, généré par DeepSeek: "Générer indices pour la forme générale d'un nuage"
        0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
        1, 5, 9, 5, 11, 4, 11, 10, 2, 10, 7, 6, 7, 1, 8,
        3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
        4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9, 8, 1
    };
    for (int i = 0; i < detail; i++) {
        std::vector<glm::vec3> newVertices = baseVertices;
        std::vector<unsigned int> newIndices;
        struct Edge {
            int v1, v2;
            bool operator<(const Edge& other) const {
                return (v1 < other.v1) || (v1 == other.v1 && v2 < other.v2);
            }
        };
        std::map<Edge, int> midpointCache;
        auto getMidpoint = [&](int v1, int v2) -> int {
            Edge e = { std::min(v1, v2), std::max(v1, v2) };
            auto it = midpointCache.find(e);
            if (it != midpointCache.end()) return it->second;
            glm::vec3 mid = glm::normalize((baseVertices[v1] + baseVertices[v2]) * 0.5f);
            int index = newVertices.size();
            newVertices.push_back(mid);
            midpointCache[e] = index;
            return index;
            };
        for (size_t j = 0; j < baseIndices.size(); j += 3) {
            int v1 = baseIndices[j];
            int v2 = baseIndices[j + 1];
            int v3 = baseIndices[j + 2];
            int m12 = getMidpoint(v1, v2);
            int m23 = getMidpoint(v2, v3);
            int m31 = getMidpoint(v3, v1);
            newIndices.push_back(v1); newIndices.push_back(m12); newIndices.push_back(m31);
            newIndices.push_back(v2); newIndices.push_back(m23); newIndices.push_back(m12);
            newIndices.push_back(v3); newIndices.push_back(m31); newIndices.push_back(m23);
            newIndices.push_back(m12); newIndices.push_back(m23); newIndices.push_back(m31);
        }
        baseVertices = newVertices;
        baseIndices = newIndices;
    }
    for (size_t i = 0; i < baseVertices.size(); i++) {
        glm::vec3 pos = baseVertices[i];
        auto randFloat = []() -> float { return 0.8f + (rand() % 400) / 1000.0f; };
        float noise = randFloat();
        pos *= noise;
        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        vertices.push_back(pos.z);
    }
    indices = baseIndices;
}

Clouds::Clouds() : cloudCount_(20) {}
Clouds::Clouds(unsigned int cloudCount) : cloudCount_(cloudCount) {}

Clouds::~Clouds() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (ebo_) glDeleteBuffers(1, &ebo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (shaderProgram_) glDeleteProgram(shaderProgram_);
}

void Clouds::initialize() {
    if (clouds_.empty()) {
        clouds_.resize(cloudCount_);
        for (unsigned int i = 0; i < cloudCount_; ++i) spawnCloud(i);
    }
    if (vao_ == 0) {
        initBuffers();
        loadShaders();
    }
}

void Clouds::spawnCloud(unsigned int index) {
    auto randomFloat = [](float min = 0.0f, float max = 1.0f) -> float {
        return min + (rand() % 10000) * (max - min) / 10000.0f;
        };
    clouds_[index].position = glm::vec3(
        (rand() % 4000 - 2000) / 100.0f,
        (rand() % 350) / 100.0f + 1.5f,
        (rand() % 4000 - 2000) / 100.0f
    );
    clouds_[index].direction = glm::normalize(glm::vec3(
        randomFloat(-1.0f, 1.0f),
        0.0f,
        randomFloat(-1.0f, 1.0f)
    ));
    clouds_[index].speed = randomFloat(0.15f, 0.4f);
    clouds_[index].lifetime = 0.0f;
    clouds_[index].maxLifetime = randomFloat(25.0f, 35.0f);
    clouds_[index].fadePhase = 0.0f;
    clouds_[index].alpha = 0.0f;
    clouds_[index].scale = glm::vec3(
        randomFloat(1.8f, 2.8f),
        randomFloat(0.7f, 1.1f),
        randomFloat(1.5f, 2.5f)
    );
    clouds_[index].rotationY = randomFloat(0.0f, M_PI * 2.0f);
    clouds_[index].rotationSpeedY = randomFloat(-0.03f, 0.03f);
    clouds_[index].floatOffset = 0.0f;
    clouds_[index].floatSpeed = randomFloat(0.5f, 1.5f);
    clouds_[index].floatAmount = randomFloat(0.05f, 0.15f);
}

void Clouds::initBuffers() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    generateCloudMesh(vertices, indices, 2);
    vertexCount_ = vertices.size() / 3;
    indexCount_ = indices.size();
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void Clouds::loadShaders() {
    const char* vsSource = R"GLSL(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;

out vec3 vWorldPos;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;
    gl_Position = uProj * uView * worldPos;
}
)GLSL";

    const char* fsSource = R"GLSL(
#version 330 core
in vec3 vWorldPos;
out vec4 FragColor;

uniform float uAlpha;
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform float uLightIntensity;
uniform vec3 uCameraPos;

void main() {
    vec3 cloudColor = vec3(0.85, 0.87, 0.92); // Lighter, whiter clouds
    
    vec3 normal = normalize(vWorldPos);
    vec3 lightDir = normalize(-uLightPos);
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor * uLightIntensity;
    vec3 ambient = vec3(0.3, 0.35, 0.4);
    
    vec3 viewDir = normalize(uCameraPos - vWorldPos);
    float rim = 1.0 - max(dot(normal, viewDir), 0.0);
    rim = smoothstep(0.5, 1.0, rim);
    vec3 rimLight = rim * vec3(0.95, 0.95, 1.0) * 0.3;
    
    vec3 litColor = cloudColor * (ambient + diffuse) + rimLight;
    float colorVar = sin(vWorldPos.x * 0.5 + vWorldPos.z * 0.3) * 0.05;
    litColor += vec3(colorVar, colorVar * 0.5, 0.0);
    
    float depth = distance(vWorldPos, uCameraPos);
    float depthFade = smoothstep(40.0, 60.0, depth);
    float finalAlpha = uAlpha * 0.85 * (1.0 - depthFade * 0.5);
    
    FragColor = vec4(litColor, finalAlpha);
}
)GLSL";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSource, nullptr);
    glCompileShader(vs);

    // Check compilation
    GLint success;
    char infoLog[512];
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, nullptr, infoLog);
        std::cerr << "Cloud vertex shader compilation failed: " << infoLog << std::endl;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSource, nullptr);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fs, 512, nullptr, infoLog);
        std::cerr << "Cloud fragment shader compilation failed: " << infoLog << std::endl;
    }

    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, vs);
    glAttachShader(shaderProgram_, fs);
    glLinkProgram(shaderProgram_);

    glGetProgramiv(shaderProgram_, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram_, 512, nullptr, infoLog);
        std::cerr << "Cloud shader program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    uProjLoc_ = glGetUniformLocation(shaderProgram_, "uProj");
    uViewLoc_ = glGetUniformLocation(shaderProgram_, "uView");
    uModelLoc_ = glGetUniformLocation(shaderProgram_, "uModel");
    uAlphaLoc_ = glGetUniformLocation(shaderProgram_, "uAlpha");

    uLightPosLoc_ = glGetUniformLocation(shaderProgram_, "uLightPos");
    uLightColorLoc_ = glGetUniformLocation(shaderProgram_, "uLightColor");
    uLightIntensityLoc_ = glGetUniformLocation(shaderProgram_, "uLightIntensity");
    uCameraPosLoc_ = glGetUniformLocation(shaderProgram_, "uCameraPos");
}

void Clouds::update(float deltaTime) {
    for (auto& cloud : clouds_) {
        cloud.lifetime += deltaTime;
        if (cloud.lifetime < 4.0f) {
            cloud.fadePhase = 0;
            cloud.alpha = cloud.lifetime / 4.0f;
        }
        else if (cloud.lifetime < cloud.maxLifetime - 4.0f) {
            cloud.fadePhase = 1;
            cloud.alpha = 1.0f;
        }
        else if (cloud.lifetime < cloud.maxLifetime) {
            cloud.fadePhase = 2;
            cloud.alpha = (cloud.maxLifetime - cloud.lifetime) / 4.0f;
        }
        else {
            spawnCloud(&cloud - &clouds_[0]);
            continue;
        }
        glm::vec3 horizontalMovement = glm::vec3(cloud.direction.x, 0.0f, cloud.direction.z);
        cloud.position += horizontalMovement * cloud.speed * deltaTime;
        cloud.rotationY += cloud.rotationSpeedY * deltaTime;
        cloud.floatOffset += cloud.floatSpeed * deltaTime;
        float floatY = sin(cloud.floatOffset) * cloud.floatAmount;
        cloud.position.y += floatY * deltaTime;
        float maxDistance = 30.0f;
        if (glm::length(cloud.position) > maxDistance) spawnCloud(&cloud - &clouds_[0]);
    }
}

void Clouds::draw(const glm::mat4& proj, const glm::mat4& view,
    const Light::LightSource& light, const glm::vec3& cameraPos) {
    if (!shaderProgram_ || !vao_ || clouds_.empty()) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glUseProgram(shaderProgram_);

    glUniformMatrix4fv(uProjLoc_, 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(uViewLoc_, 1, GL_FALSE, glm::value_ptr(view));

    if (uCameraPosLoc_ != -1) {
        glUniform3f(uCameraPosLoc_, cameraPos.x, cameraPos.y, cameraPos.z);
    }
    updateLightingUniforms(light);

    glBindVertexArray(vao_);

    for (auto& cloud : clouds_) {
        if (cloud.alpha <= 0.01f) continue;

        glUniform1f(uAlphaLoc_, cloud.alpha * 0.85f);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), cloud.position);
        model = glm::rotate(model, cloud.rotationY, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, cloud.scale);
        glUniformMatrix4fv(uModelLoc_, 1, GL_FALSE, glm::value_ptr(model));

        glDrawElements(GL_TRIANGLES, (GLsizei)indexCount_, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

void Clouds::updateLightingUniforms(const Light::LightSource& light) {
    if (uLightPosLoc_ != -1) {
        glUniform3f(uLightPosLoc_, light.direction.x, light.direction.y, light.direction.z);
    }
    if (uLightColorLoc_ != -1) {
        glUniform3f(uLightColorLoc_, light.color.x, light.color.y, light.color.z);
    }
    if (uLightIntensityLoc_ != -1) {
        glUniform1f(uLightIntensityLoc_, light.intensity);
    }
}

void Clouds::drawShadow() {
    if (!vao_) return;

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, (GLsizei)indexCount_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}