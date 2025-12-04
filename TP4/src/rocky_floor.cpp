#include "rocky_floor.hpp"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace gl;

RockyFloor::RockyFloor() : time_(0.0f) {
}

RockyFloor::~RockyFloor() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (shaderProgram_) glDeleteProgram(shaderProgram_);
}

void RockyFloor::initialize() {
    createGeometry();
    loadShaders();
}

void RockyFloor::createGeometry() {
    const int gridSize = 8;
    const float patchSize = 60.0f / gridSize;

    std::vector<float> vertices;

    for (int z = 0; z <= gridSize; ++z) {
        for (int x = 0; x <= gridSize; ++x) {
            float worldX = -30.0f + x * patchSize;
            float worldZ = -30.0f + z * patchSize;

            vertices.push_back(worldX);
            vertices.push_back(0.0f);
            vertices.push_back(worldZ);
        }
    }

    std::vector<unsigned int> indices;
    for (int z = 0; z < gridSize; ++z) {
        for (int x = 0; x < gridSize; ++x) {
            int topLeft = z * (gridSize + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (gridSize + 1) + x;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    vertexCount_ = vertices.size();
    patchCount_ = indices.size() / 4;

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

void RockyFloor::loadShaders() {
    const char* vsSource = R"GLSL(
#version 410 core
layout(location = 0) in vec3 aPos;
out vec3 vPosition;
uniform mat4 uModel;
void main() {
    vPosition = (uModel * vec4(aPos, 1.0)).xyz;
    gl_Position = vec4(aPos, 1.0);
}
)GLSL";

    const char* tcsSource = R"GLSL(
#version 410 core
layout(vertices = 4) out;
uniform vec3 uCameraPos;

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    
    if (gl_InvocationID == 0) {
        vec3 pos0 = gl_in[0].gl_Position.xyz;
        vec3 pos1 = gl_in[1].gl_Position.xyz;
        vec3 pos2 = gl_in[2].gl_Position.xyz;
        vec3 pos3 = gl_in[3].gl_Position.xyz;
        
        float dist0 = distance(uCameraPos, pos0);
        float dist1 = distance(uCameraPos, pos1);
        float dist2 = distance(uCameraPos, pos2);
        float dist3 = distance(uCameraPos, pos3);
        
        float minDist = min(min(dist0, dist1), min(dist2, dist3));
        
        float tessLevel;
        if (minDist < 5.0) {
            tessLevel = 32.0;
        } else if (minDist < 20.0) {
            tessLevel = mix(32.0, 16.0, (minDist - 5.0) / 15.0);
        } else {
            tessLevel = mix(16.0, 8.0, min(1.0, (minDist - 20.0) / 30.0));
        }
        
        tessLevel = max(2.0, min(64.0, tessLevel));
        
        gl_TessLevelInner[0] = tessLevel;
        gl_TessLevelInner[1] = tessLevel;
        gl_TessLevelOuter[0] = tessLevel;
        gl_TessLevelOuter[1] = tessLevel;
        gl_TessLevelOuter[2] = tessLevel;
        gl_TessLevelOuter[3] = tessLevel;
    }
}
)GLSL";

    const char* tesSource = R"GLSL(
#version 410 core
layout(quads, equal_spacing, ccw) in;
in vec3 vPosition[];
uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;
uniform float uTime;
out vec3 tePosition;
out vec3 teNormal;

float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float noise(vec2 pos, float freq) {
    vec2 i = floor(pos * freq);
    vec2 f = fract(pos * freq);
    
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    vec2 u = f * f * (3.0 - 2.0 * f);
    
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float terrainHeight(vec2 pos) {
    float height = 0.0;
    float dist = length(pos);
    
    height += smoothstep(5.0, 20.0, dist) * 8.0;
    height *= exp(-dist * 0.1);
    
    height += noise(pos, 0.2) * 2.0 * smoothstep(3.0, 15.0, dist);
    
    height += noise(pos, 1.0) * 0.5;
    height += noise(pos, 4.0) * 0.2;
    
    if (dist < 8.0) {
        float crater = 1.0 - smoothstep(0.0, 8.0, dist);
        height -= 4.0 * crater * crater;
    }
    
    return height;
}

void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    
    vec3 position = 
        (1.0 - u) * (1.0 - v) * vPosition[0] +
        u * (1.0 - v) * vPosition[1] +
        (1.0 - u) * v * vPosition[2] +
        u * v * vPosition[3];
    
    float height = terrainHeight(position.xz);
    position.y = height;
    
    float eps = 0.1;
    float hL = terrainHeight(position.xz - vec2(eps, 0.0));
    float hR = terrainHeight(position.xz + vec2(eps, 0.0));
    float hD = terrainHeight(position.xz - vec2(0.0, eps));
    float hU = terrainHeight(position.xz + vec2(0.0, eps));
    
    teNormal = normalize(vec3(hL - hR, 2.0 * eps, hD - hU));
    tePosition = position;
    
    gl_Position = uProj * uView * uModel * vec4(position, 1.0);
}
)GLSL";

    const char* fsSource = R"GLSL(
#version 410 core
in vec3 tePosition;
in vec3 teNormal;

out vec4 FragColor;

uniform vec3 uCameraPos;
uniform float uTime;
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform float uLightIntensity;
uniform bool uLightingEnabled;
uniform bool uShadowsEnabled;

uniform vec3 uCrystalPos;
uniform float uCrystalHeight;
uniform vec3 uCloudPositions[20];
uniform float uCloudSizes[20];
uniform float uCloudAlphas[20];
uniform int uCloudCount;

float calculateSimpleShadows(vec3 groundPos) {
    if (!uShadowsEnabled) {
        return 0.0;
    }
    float totalShadow = 0.0;
    
    vec2 crystalGroundPos = uCrystalPos.xz;
    float crystalDist = distance(groundPos.xz, crystalGroundPos);
    
    float crystalShadowRadius = 2.5 + uCrystalHeight * 0.3;
    
    float crystalShadow = 1.0 - smoothstep(0.0, crystalShadowRadius, crystalDist);
    
    crystalShadow = pow(crystalShadow, 1.3) * 0.7;
    
    totalShadow = max(totalShadow, crystalShadow);
    
    for(int i = 0; i < min(uCloudCount, 20); i++) {
        if (uCloudAlphas[i] < 0.01) continue;
        
        vec2 cloudGroundPos = uCloudPositions[i].xz;
        float cloudDist = distance(groundPos.xz, cloudGroundPos);
        
        float cloudShadowRadius = uCloudSizes[i] * 1.5;
        float cloudShadowStrength = uCloudAlphas[i] * 0.6;
        
        float cloudShadow = 1.0 - smoothstep(0.0, cloudShadowRadius, cloudDist);
        cloudShadow = pow(cloudShadow, 1.1) * cloudShadowStrength;
        
        totalShadow = max(totalShadow, cloudShadow);
    }
    
    return min(totalShadow, 0.85);
}

void main() {
    float dist = distance(tePosition, uCameraPos);
    
    vec3 rockDark = vec3(0.35, 0.28, 0.22);
    vec3 rockMid = vec3(0.55, 0.48, 0.42);
    vec3 rockLight = vec3(0.65, 0.58, 0.52);
    
    vec3 baseColor = rockMid;
    
    float texVar = sin(tePosition.x * 2.5) * cos(tePosition.z * 3.5) * 0.05;
    baseColor += texVar;
    
    vec3 lightDir = normalize(-uLightPos);
    
    vec3 normal = normalize(teNormal);
    
    float NdotL = max(dot(normal, lightDir), 0.0);
    float ambient = 0.3;
    
    float objectShadow = calculateSimpleShadows(tePosition);
    
    float shadowMultiplier = 1.0 - objectShadow * 0.8;
    
    float slopeAO = 1.0 - (1.0 - normal.y) * 0.3;
    
    float selfShadow = 1.0;
    if (normal.y < 0.3) {
        selfShadow = 0.7 + normal.y * 1.0;
    }
    
    float diffuse = NdotL * selfShadow * slopeAO * uLightIntensity;
    float finalLight = ambient * shadowMultiplier + diffuse * shadowMultiplier;
    
    vec3 litColor = baseColor * finalLight * uLightColor;
    
    if (NdotL > 0.0 && objectShadow < 0.3) {
        vec3 viewDir = normalize(uCameraPos - tePosition);
        vec3 halfDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfDir), 0.0), 32.0);
        litColor += vec3(0.1, 0.1, 0.15) * spec * 0.3 * (1.0 - objectShadow);
    }
    
    float fog = smoothstep(40.0, 80.0, dist);
    vec3 fogColor = vec3(0.5, 0.5, 0.55) * uLightColor * 0.5;
    vec3 color = mix(litColor, fogColor, fog);
    
    float craterDist = length(tePosition.xz);
    if (craterDist < 12.0) {
        float craterFactor = 1.0 - smoothstep(8.0, 12.0, craterDist);
        color = mix(color, color * 0.85, craterFactor);
    }
    
    color = pow(color, vec3(1.0/1.1));
    
    FragColor = vec4(color, 1.0);
}
)GLSL";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSource, nullptr);
    glCompileShader(vs);

    GLuint tcs = glCreateShader(GL_TESS_CONTROL_SHADER);
    glShaderSource(tcs, 1, &tcsSource, nullptr);
    glCompileShader(tcs);

    GLuint tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
    glShaderSource(tes, 1, &tesSource, nullptr);
    glCompileShader(tes);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSource, nullptr);
    glCompileShader(fs);

    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, vs);
    glAttachShader(shaderProgram_, tcs);
    glAttachShader(shaderProgram_, tes);
    glAttachShader(shaderProgram_, fs);
    glLinkProgram(shaderProgram_);

    GLint success;
    glGetProgramiv(shaderProgram_, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram_, 512, nullptr, infoLog);
        std::cerr << "Shader linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(tcs);
    glDeleteShader(tes);
    glDeleteShader(fs);

    uProjLoc_ = glGetUniformLocation(shaderProgram_, "uProj");
    uViewLoc_ = glGetUniformLocation(shaderProgram_, "uView");
    uModelLoc_ = glGetUniformLocation(shaderProgram_, "uModel");
    uCameraPosLoc_ = glGetUniformLocation(shaderProgram_, "uCameraPos");
    uTimeLoc_ = glGetUniformLocation(shaderProgram_, "uTime");

    uLightPosLoc_ = glGetUniformLocation(shaderProgram_, "uLightPos");
    uLightColorLoc_ = glGetUniformLocation(shaderProgram_, "uLightColor");
    uLightIntensityLoc_ = glGetUniformLocation(shaderProgram_, "uLightIntensity");

    uCrystalPosLoc_ = glGetUniformLocation(shaderProgram_, "uCrystalPos");
    uCrystalHeightLoc_ = glGetUniformLocation(shaderProgram_, "uCrystalHeight");
    uCloudPositionsLoc_ = glGetUniformLocation(shaderProgram_, "uCloudPositions");
    uCloudSizesLoc_ = glGetUniformLocation(shaderProgram_, "uCloudSizes");
    uCloudAlphasLoc_ = glGetUniformLocation(shaderProgram_, "uCloudAlphas");
    uCloudCountLoc_ = glGetUniformLocation(shaderProgram_, "uCloudCount");

    if (uCameraPosLoc_ == -1) {
        std::cerr << "ERROR: uCameraPos uniform not found!" << std::endl;
    }
}

void RockyFloor::draw(const glm::mat4& proj, const glm::mat4& view,
    const glm::vec3& cameraPos,
    const Light::LightSource& light,
    const glm::vec3& crystalPos,
    float crystalHeight,
    const std::vector<glm::vec3>& cloudPositions,
    const std::vector<float>& cloudSizes,
    const std::vector<float>& cloudAlphas) {

    if (!shaderProgram_ || !vao_) return;

    time_ += 0.01f;

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glUseProgram(shaderProgram_);
    glBindVertexArray(vao_);

    glm::mat4 model = glm::mat4(1.0f);

    glUniformMatrix4fv(uProjLoc_, 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(uViewLoc_, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(uModelLoc_, 1, GL_FALSE, glm::value_ptr(model));

    glUniform3f(uCameraPosLoc_, cameraPos.x, cameraPos.y, cameraPos.z);
    glUniform1f(uTimeLoc_, time_);

    glUniform3f(uLightPosLoc_, light.direction.x, light.direction.y, light.direction.z);
    glUniform3f(uLightColorLoc_, light.color.x, light.color.y, light.color.z);
    glUniform1f(uLightIntensityLoc_, light.intensity);

    glUniform3f(uCrystalPosLoc_, crystalPos.x, crystalPos.y, crystalPos.z);
    glUniform1f(uCrystalHeightLoc_, crystalHeight);

    int cloudCount = std::min(static_cast<int>(cloudPositions.size()), 20);
    glUniform1i(uCloudCountLoc_, cloudCount);

    GLint lightingEnabledLoc = glGetUniformLocation(shaderProgram_, "uLightingEnabled");
    GLint shadowsEnabledLoc = glGetUniformLocation(shaderProgram_, "uShadowsEnabled");

    if (lightingEnabledLoc != -1) {
        glUniform1i(lightingEnabledLoc, light.enabled ? 1 : 0);
    }
    if (shadowsEnabledLoc != -1) {
        glUniform1i(shadowsEnabledLoc, (light.enabled && light.castShadows) ? 1 : 0);
    }

    for (int i = 0; i < cloudCount; i++) {
        std::string posName = "uCloudPositions[" + std::to_string(i) + "]";
        GLint loc = glGetUniformLocation(shaderProgram_, posName.c_str());
        if (loc != -1) {
            glUniform3f(loc, cloudPositions[i].x, cloudPositions[i].y, cloudPositions[i].z);
        }
    }

    for (int i = 0; i < cloudCount; i++) {
        std::string sizeName = "uCloudSizes[" + std::to_string(i) + "]";
        GLint loc = glGetUniformLocation(shaderProgram_, sizeName.c_str());
        if (loc != -1 && i < cloudSizes.size()) {
            glUniform1f(loc, cloudSizes[i]);
        }
    }

    for (int i = 0; i < cloudCount; i++) {
        std::string alphaName = "uCloudAlphas[" + std::to_string(i) + "]";
        GLint loc = glGetUniformLocation(shaderProgram_, alphaName.c_str());
        if (loc != -1 && i < cloudAlphas.size()) {
            glUniform1f(loc, cloudAlphas[i]);
        }
    }

    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glDrawElements(GL_PATCHES, (GLsizei)(patchCount_ * 4), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}