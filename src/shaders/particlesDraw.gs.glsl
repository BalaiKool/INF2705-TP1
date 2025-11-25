#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in VS_OUT {
    vec3 position;
    float zOrientation;
    vec4 color;
    vec2 size;
} gsIn[];

out GS_OUT {
    vec2 texCoord;
    vec4 color;
} gsOut;

uniform mat4 projection;
uniform vec3 cameraRight;
uniform vec3 cameraUp;

void main()
{
    vec3 pos = gsIn[0].position;
    float angle = gsIn[0].zOrientation;
    vec2 halfSize = gsIn[0].size * 0.5;

    vec3 offsets[4];
    offsets[0] = (-halfSize.x * cameraRight - halfSize.y * cameraUp);
    offsets[1] = ( halfSize.x * cameraRight - halfSize.y * cameraUp);
    offsets[2] = (-halfSize.x * cameraRight + halfSize.y * cameraUp);
    offsets[3] = ( halfSize.x * cameraRight + halfSize.y * cameraUp);

    for(int i=0;i<4;i++){
        float x = offsets[i].x;
        float y = offsets[i].y;
        offsets[i].x = x*cos(angle) - y*sin(angle);
        offsets[i].y = x*sin(angle) + y*cos(angle);
    }

    vec2 texCoords[4] = vec2[4](
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0)
    );

    for(int i=0;i<4;i++){
        gsOut.texCoord = texCoords[i];
        gsOut.color = gsIn[0].color;
        gl_Position = projection * vec4(pos + offsets[i], 1.0);
        EmitVertex();
    }
    EndPrimitive();
}
