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

void main()
{
    vec4 texColor = texture(uTexture, texCoord);
    float roughness = texture(uRoughnessMap, texCoord).r;
    vec3 normal = getNormalFromMap();

    vec3 lightDir = normalize(uLightPos - fragPos);
    vec3 viewDir = normalize(uViewPos - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;
    
    float shininess = mix(128.0, 8.0, roughness);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = spec * uLightColor * (1.0 - roughness);
    
    vec3 ambient = 0.1 * texColor.rgb;
    
    vec3 result = (ambient + diffuse + specular) * texColor.rgb * vertexColor;
    FragColor = vec4(result, texColor.a);
}