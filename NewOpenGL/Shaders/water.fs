#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;
in vec2 FlowUV; //接收流动UV

uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;

out vec4 FragColor;

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0) return 0.0;

    float currentDepth = projCoords.z;
    vec3 norm = normalize(Normal);
    
    float depthBias = max(0.005 * (1.0 - dot(norm, lightDir)), 0.001);
    depthBias *= 1.0 + 2.0 * smoothstep(0.0, 0.2, abs(FragPos.y - 0.1)); 

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x,y)*texelSize).r;
            shadow += (currentDepth - depthBias > closestDepth) ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return shadow;
}

void main() {
    vec4 texColor = texture(texture_diffuse1, FlowUV);
    texColor.a = 0.4;

    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir);
    
    //光照
    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor * 1.5;
    vec3 ambient = 1.2 * lightColor; 
    
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDirNorm, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = spec * lightColor * 1.0;
    
    float shadow = ShadowCalculation(FragPosLightSpace);
    
    //阴影应用失败
    vec3 directLight = (1.0 - shadow) * (diffuse + specular);
    vec3 finalColor = (ambient + directLight) * texColor.rgb;
    
    FragColor = vec4(finalColor, texColor.a);
}