#version 430 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 fragColorFactor;
in float fragIsActive;
in vec4 FragPosLightSpace;

uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap; 

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

out vec4 FragColor;

float ShadowCalculation(vec4 fragPosLightSpace) {
    //透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0) return 0.0;

    //偏移
    float bias = max(0.05 * (1.0 - dot(normalize(Normal), normalize(lightPos - FragPos))), 0.005);
    
    //PCF采样
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += (projCoords.z - bias) > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    return shadow;
}

void main() {
    if (fragIsActive < 0.1) discard;

    vec4 texColor = texture(texture_diffuse1, TexCoords);
    if(texColor.a < 0.5) discard;

    //光照
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    //漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    
    //镜面光
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    
    //环境光
    vec3 ambient = 0.08 * lightColor;
    vec3 diffuse = diff * lightColor;
    vec3 specular = spec * lightColor;

    //阴影计算
    float shadow = ShadowCalculation(FragPosLightSpace);

    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * texColor.rgb * fragColorFactor.rgb;
    FragColor = vec4(lighting, texColor.a);
}
