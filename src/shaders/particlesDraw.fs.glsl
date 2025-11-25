#version 430 core

in vec2 texCoord;
in vec4 color;

out vec4 FragColor;

uniform sampler2D particlesTex;

void main()
{
    vec4 texColor = texture(particlesTex, texCoord);
    FragColor = texColor * color;

    // Discard almost transparent tex
    if (FragColor.a < 0.01)
        discard;
}
