#version 330 core

in GS_OUT {
    vec2 texCoord;
    vec4 color;
} fsIn;

out vec4 FragColor;

uniform sampler2D textureSampler;

void main()
{
    vec4 texColor = texture(textureSampler, fsIn.texCoord);
    if(texColor.a < 0.02)
        discard;

    FragColor = texColor * fsIn.color;
}
