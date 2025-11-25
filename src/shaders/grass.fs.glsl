#version 330 core

in ATTRIBS_GS_OUT
{
    float heightRatio;
} attribsIn;

out vec4 FragColor;

void main()
{
    const vec3 COLOR_TOP  = vec3(0.6, 0.86, 0.21);
    const vec3 COLOR_BASE = vec3(0.25, 0.45, 0.05);

    vec3 col = mix(COLOR_BASE, COLOR_TOP, attribsIn.heightRatio);

    FragColor = vec4(col, 1.0);
}
