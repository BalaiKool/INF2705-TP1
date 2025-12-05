#version 330 core
in vec3 vertexColor;
in vec2 texCoord;
in vec3 fragPos;
in vec3 fragNormal;

uniform sampler2D uTexture;
uniform sampler2D uNormalMap;
uniform sampler2D uRoughnessMap;
uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;
uniform float uLightIntensity;
uniform bool uLightingEnabled;

out vec4 FragColor;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(uNormalMap, texCoord).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(fragPos);
    vec3 Q2 = dFdy(fragPos);
    vec2 st1 = dFdx(texCoord);
    vec2 st2 = dFdy(texCoord);
    
    vec3 N = normalize(fragNormal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    
    return normalize(TBN * tangentNormal);
}

vec3 calculateCrystalLighting(vec3 baseColor, vec3 position, vec3 normal, float roughness) 
{
    if (!uLightingEnabled) {
        return baseColor;
    }

    vec3 lightDir = normalize(-uLightPos);
    
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * uLightColor;
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor * uLightIntensity;
    
    vec3 viewDir = normalize(uViewPos - position);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    float shininess = mix(128.0, 8.0, roughness);
    float spec = pow(max(dot(viewDir, reflect(-lightDir, normal)), 0.0), shininess);
    
    float specularStrength = (1.0 - roughness);
    vec3 specular = vec3(0.8, 0.9, 1.0) * spec * specularStrength;

    vec3 emission = baseColor * 0.1;

    vec3 lighting = ambient + diffuse + specular + emission;
    
    return baseColor * lighting;
}

void main()
{
    vec4 texColor = texture(uTexture, texCoord);
    float roughness = texture(uRoughnessMap, texCoord).r;
    vec3 normal = getNormalFromMap();
    
    vec3 baseColor = texColor.rgb * vertexColor;
    
    vec3 litColor = calculateCrystalLighting(baseColor, fragPos, normal, roughness);
    
    if (uLightingEnabled) {
        vec3 viewDir = normalize(uViewPos - fragPos);
        
        float rim = 1.0 - max(dot(normal, viewDir), 0.0);
        rim = smoothstep(0.5, 1.0, rim);
        litColor += rim * vec3(0.3, 0.4, 0.8) * 0.5;

        float sparkle = sin(fragPos.x * 10.0 + fragPos.y * 8.0 + fragPos.z * 6.0) * 0.1;
        litColor += sparkle * vec3(0.5, 0.7, 1.0);
    }
    
    FragColor = vec4(litColor, texColor.a);
}