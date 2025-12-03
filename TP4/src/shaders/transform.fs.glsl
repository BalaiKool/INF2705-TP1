#version 330 core
in vec3 vertexColor;
in vec2 texCoord;

uniform sampler2D uTexture;

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(uTexture, texCoord);
    FragColor = texColor * vec4(vertexColor, 1.0);
}   
