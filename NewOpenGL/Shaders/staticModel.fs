#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;  // 光源视角下的顶点位置

uniform sampler2D texture_diffuse1; // 纹理采样器
uniform sampler2D shadowMap;       // 阴影贴图
uniform vec3 lightDir;             // 光源方向
uniform vec3 lightColor;           // 光源颜色
uniform vec3 viewPos;              // 摄像机位置

out vec4 FragColor;

float ShadowCalculation(vec4 fragPosLightSpace) {
    // 透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 将坐标从[-1, 1]转换到[0, 1]
    projCoords = projCoords * 0.5 + 0.5;

    // 如果投影坐标超出[0,1]，则说明片元不在光源视野内，不计算阴影
    if (projCoords.z > 1.0)
        return 0.0;

    // 当前片元在光源视角下的深度值
    float currentDepth = projCoords.z;

    // 添加一个很小的偏移，防止自阴影伪影
    float bias = 0.005;

    // PCF 滤波
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0); // 获取阴影贴图的纹素大小
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            // 对周围 3x3 的纹素进行采样
            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0; // 取平均值

    return shadow;
}
void main() {
    // 读取纹理颜色
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    // 计算法线
    vec3 norm = normalize(Normal);
        
    // 计算视线方向
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // 计算光照方向
    vec3 lightDirection = normalize(-lightDir);

    // 漫反射
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;

    // 镜面高光 (Blinn-Phong 模型)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    
    // 组合光照颜色
    vec3 ambient = 0.1 * lightColor;
    vec3 specular = spec * lightColor;
    
    // 计算阴影因子
    float shadow = ShadowCalculation(FragPosLightSpace);
    
    // 计算最终颜色
    vec3 finalColor = (ambient + (1.0 - shadow) ) * texColor.rgb;

    FragColor = vec4(finalColor, texColor.a);
}