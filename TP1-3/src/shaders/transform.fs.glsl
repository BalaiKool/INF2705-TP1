#version 330 core

in vec3 vertexColor;
uniform vec3 uColorMod;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor * uColorMod, 1.0);
}
