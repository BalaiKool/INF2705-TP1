#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aUV;

out vec3 vertexColor;
out vec2 texCoord;
out vec3 vWorldPos;
out vec3 vNormal;

uniform mat4 uMVP;
uniform mat4 uModel;

void main()
{
    vertexColor = aColor;
    texCoord = aUV;
    
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;
    
    vNormal = normalize(mat3(transpose(inverse(uModel))) * aPos);
    
    gl_Position = uMVP * vec4(aPos, 1.0);
}