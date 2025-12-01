#version 330 core
in vec3 fragColor;
in vec2 fragTexCoords;
in vec3 fragNormal;

out vec4 FragColor;

uniform sampler2D diffuseSampler;

void main()
{
    // Simple shader - just show the texture or color
    vec4 texColor = texture(diffuseSampler, fragTexCoords);
    
    // If texture is mostly black/default, use vertex color or normal visualization
    if (texColor.rgb == vec3(0.0)) {
        if (fragColor != vec3(0.0)) {
            FragColor = vec4(fragColor, 1.0);
        } else {
            // Visualize normals if no color
            FragColor = vec4(fragNormal * 0.5 + 0.5, 1.0);
        }
    } else {
        FragColor = texColor;
    }
}