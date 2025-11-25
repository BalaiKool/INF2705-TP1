#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in float inZOrientation;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec2 inSize;

out VS_OUT {
    vec3 position;
    float zOrientation;
    vec4 color;
    vec2 size;
} vsOut;

uniform mat4 modelView;

void main()
{
    vsOut.position = (modelView * vec4(inPosition,1)).xyz;
    vsOut.zOrientation = inZOrientation;
    vsOut.color = inColor;
    vsOut.size = inSize;

    gl_Position = modelView * vec4(inPosition, 1.0);
}
