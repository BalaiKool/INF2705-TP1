#version 400 core

layout(vertices = 3) out;

uniform mat4 modelView;

const float MIN_TESS = 2.0;
const float MAX_TESS = 32.0;

const float MIN_DIST = 10.0;
const float MAX_DIST = 40.0;

float tessLevelForEdge(vec4 p0, vec4 p1)
{
    vec4 mid = (p0 + p1) * 0.5;
    float dist = length( (modelView * mid).xyz ); // distance à la caméra

    float f = clamp((dist - MIN_DIST) / (MAX_DIST - MIN_DIST), 0.0, 1.0);

    return mix(MAX_TESS, MIN_TESS, f);
}

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (gl_InvocationID == 0)
    {
        float t0 = tessLevelForEdge(gl_in[1].gl_Position, gl_in[2].gl_Position);
        float t1 = tessLevelForEdge(gl_in[2].gl_Position, gl_in[0].gl_Position);
        float t2 = tessLevelForEdge(gl_in[0].gl_Position, gl_in[1].gl_Position);

        gl_TessLevelOuter[0] = t0;
        gl_TessLevelOuter[1] = t1;
        gl_TessLevelOuter[2] = t2;

        gl_TessLevelInner[0] = max(t0, max(t1, t2));
    }
}
