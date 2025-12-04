#version 330 core
in vec3 vertexColor;
in vec2 texCoord;
in vec3 vWorldPos;
in vec3 vNormal;

uniform sampler2D uTexture;
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform float uLightIntensity;
uniform vec3 uCameraPos;
uniform bool uLightingEnabled;

out vec4 FragColor;

vec3 calculateCrystalLighting(vec3 baseColor, vec3 position, vec3 normal) {

    if (!uLightingEnabled) {
        return baseColor;
    }

    vec3 lightDir = normalize(-uLightPos);
    
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * uLightColor;
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor * uLightIntensity;
    
    vec3 viewDir = normalize(uCameraPos - position);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = vec3(0.8, 0.9, 1.0) * spec * 0.5;
    
    vec3 emission = baseColor * 0.1;
    vec3 lighting = ambient + diffuse + specular + emission;
    
    return baseColor * lighting;
}

void main()
{
    vec4 texColor = texture(uTexture, texCoord);
    vec3 baseColor = texColor.rgb * vertexColor;
    vec3 litColor = calculateCrystalLighting(baseColor, vWorldPos, normalize(vNormal));
    
    if (uLightingEnabled) {
        vec3 viewDir = normalize(uCameraPos - vWorldPos);
        vec3 normal = normalize(vNormal);
        float rim = 1.0 - max(dot(normal, viewDir), 0.0);
        rim = smoothstep(0.5, 1.0, rim);
        litColor += rim * vec3(0.3, 0.4, 0.8) * 0.5;
        
        float sparkle = sin(vWorldPos.x * 10.0 + vWorldPos.y * 8.0 + vWorldPos.z * 6.0) * 0.1;
        litColor += sparkle * vec3(0.5, 0.7, 1.0);
    }
    FragColor = vec4(litColor, texColor.a);
}