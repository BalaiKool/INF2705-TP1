#version 430 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in VS_OUT {
    vec3 position;
    float zOrientation;
    vec4 color;
    vec2 size;
} gsIn[];

out vec2 texCoord;
out vec4 color;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraRight;
uniform vec3 cameraUp;

void main()
{
    vec3 center = gsIn[0].position;
    vec2 halfSize = gsIn[0].size * 0.5;

    vec3 right = cameraRight * halfSize.x;
    vec3 up = cameraUp * halfSize.y;

    color = gsIn[0].color;

    texCoord = vec2(0.0, 0.0);
    gl_Position = projection * view * vec4(center - right - up, 1.0);
    EmitVertex();

    texCoord = vec2(1.0, 0.0);
    gl_Position = projection * view * vec4(center + right - up, 1.0);
    EmitVertex();

    texCoord = vec2(0.0, 1.0);
    gl_Position = projection * view * vec4(center - right + up, 1.0);
    EmitVertex();

    texCoord = vec2(1.0, 1.0);
    gl_Position = projection * view * vec4(center + right + up, 1.0);
    EmitVertex();

    EndPrimitive();
}
