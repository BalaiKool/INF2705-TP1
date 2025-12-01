#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 3) out;

in ATTRIBS_TES_OUT
{
    vec3 worldPos;
} attribsIn[];

out ATTRIBS_GS_OUT
{
    float heightRatio;
} attribsOut;


uniform mat4 mvp;

// Fonction pseudo aléatoire, utiliser le paramètre co pour avoir une valeur différente en sortie
float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec3 base = attribsIn[0].worldPos;

    float r = rand(base.xz);

    const float baseWidth = 0.05;
    const float varWidth  = 0.04;
    float width  = baseWidth + varWidth * r;

    const float baseHeight = 0.4;
    const float varHeight  = 0.4;
    float height = baseHeight + varHeight * r;

    float angleY = r * 6.28318;      // 0..2π
    float angleX = r * 0.1 * 3.1415; // courbure

    vec3 up = vec3(0, height, 0);

    mat3 rotY = mat3(
        cos(angleY), 0, sin(angleY),
        0, 1, 0,
       -sin(angleY), 0, cos(angleY)
    );
    
    mat3 rotX = mat3(
        1, 0, 0,
        0, cos(angleX), -sin(angleX),
        0, sin(angleX), cos(angleX)
    );

    up = rotY * (rotX * up);

    vec3 right = normalize(vec3(up.z, 0, -up.x)) * width;

    vec3 v0 = base - right;
    vec3 v1 = base + right;
    vec3 v2 = base + up;

    attribsOut.heightRatio = 0.0;
    gl_Position = mvp * vec4(v0, 1.0);
    EmitVertex();

    attribsOut.heightRatio = 0.0;
    gl_Position = mvp * vec4(v1, 1.0);
    EmitVertex();

    attribsOut.heightRatio = 1.0;
    gl_Position = mvp * vec4(v2, 1.0);
    EmitVertex();

    EndPrimitive();
}
