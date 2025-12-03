#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texCoords;

out vec3 fragColor;
out vec2 fragTexCoords;
out vec3 fragNormal;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(position, 1.0);
    fragColor = color;
    fragTexCoords = texCoords;
    fragNormal = normal;
}