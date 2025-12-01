#version 400 core

layout(triangles) in;

out ATTRIBS_TES_OUT
{
    vec3 worldPos;
} attribsOut;

uniform mat4 mvp;

void main()
{
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;

    vec3 pos = p0 * gl_TessCoord.x + p1 * gl_TessCoord.y + p2 * gl_TessCoord.z;

    attribsOut.worldPos = pos;
    gl_Position = mvp * vec4(pos, 1.0);
}
