#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aUV;

out vec3 vertexColor;
out vec2 texCoord;
out vec3 fragPos;
out vec3 fragNormal;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform mat3 uNormalMatrix;

void main()
{
    vertexColor = aColor;
    texCoord = aUV;
    fragPos = vec3(uModel * vec4(aPos, 1.0));
    fragNormal = uNormalMatrix * vec3(0.0, 0.0, 1.0);
    
    gl_Position = uMVP * vec4(aPos, 1.0);
}