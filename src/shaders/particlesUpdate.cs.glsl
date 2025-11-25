#version 430 core

layout(local_size_x = 1) in;

struct Particle
{
    vec3 position;
    float zOrientation;
    vec3 velocity;
    vec4 color;
    vec2 size;
    float timeToLive;
    float maxTimeToLive;
};

layout(std140, binding = 0) readonly restrict buffer ParticlesInputBlock
{
    Particle particles[];
} dataIn;

layout(std140, binding = 1) writeonly restrict buffer ParticlesOutputBlock
{
    Particle particles[];
} dataOut;

uniform float time;
uniform float deltaTime;
uniform vec3 emitterPosition;
uniform vec3 emitterDirection;

float rand01()
{
    return fract(sin(dot(vec2(time*100, gl_GlobalInvocationID.x), vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    uint id = gl_GlobalInvocationID.x;

    Particle p = dataIn.particles[id];

    if (p.timeToLive <= 0.0)
    {
        float r = rand01();

        p.position = emitterPosition;
        p.zOrientation = r * 6.2831853;
        p.velocity = emitterDirection * 0.3 + vec3(0, 0.2, 0);
        p.color = vec4(0.5, 0.5, 0.5, 0.2);
        p.size = vec2(0.2, 0.2);

        p.maxTimeToLive = 1.5 + r * 0.5;
        p.timeToLive = p.maxTimeToLive;

        dataOut.particles[id] = p;
        return;
    }

    p.timeToLive -= deltaTime;

    float life01 = p.timeToLive / p.maxTimeToLive;

    p.position += p.velocity * deltaTime;
    p.zOrientation += 0.5 * deltaTime;
    p.color.rgb = mix(vec3(1.0), vec3(0.5), life01);

    float s = smoothstep(0.0, 1.0, life01);
    float opacity = s * (1.0 - s);
    p.color.a = opacity * 2.0;

    p.size = mix(vec2(0.5), vec2(0.2), life01);

    dataOut.particles[id] = p;
}
