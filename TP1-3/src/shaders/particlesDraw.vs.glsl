#version 430 core

out VS_OUT {
    vec3 position;
    float zOrientation;
    vec4 color;
    vec2 size;
} vsOut;

layout(std430, binding = 0) readonly buffer ParticlesInputBlock
{
    struct Particle
    {
        vec3 position;
        float zOrientation;
        vec3 velocity;
        vec4 color;
        vec2 size;
        float timeToLive;
        float maxTimeToLive;
    } particles[];
};

void main()
{
    Particle p = particles[gl_VertexID];

    vsOut.position     = p.position;
    vsOut.zOrientation = p.zOrientation;
    vsOut.color        = p.color;
    vsOut.size         = p.size;
}
