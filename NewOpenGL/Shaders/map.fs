#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;  //光源视角下的顶点位置

uniform sampler2D texture_diffuse1; //纹理采样器
uniform sampler2D shadowMap;       //阴影贴图
uniform vec3 lightDir;             //光源方向
uniform vec3 lightColor;         
uniform vec3 viewPos;            

out vec4 FragColor;

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;
    float bias = max(0.0025 * (1.0 - dot(Normal, lightDir)), 0.0005); 

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    int kernelSize = 2; 
    int sampleCount = 0;
    for (int x = -kernelSize; x <= kernelSize; ++x) {
        for (int y = -kernelSize; y <= kernelSize; ++y) {
            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
            sampleCount++;
        }
    }
    shadow /= float(sampleCount);

    //让阴影不完全黑暗
    shadow = mix(0.3, 1.0, shadow); 
    return shadow;
}
//float ShadowCalculation(vec4 fragPosLightSpace) {
//     //透视除法
//    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//     //将坐标从[-1, 1]转换到[0, 1]
//    projCoords = projCoords * 0.5 + 0.5;

//     //如果投影坐标超出[0,1]，则说明片元不在光源视野内，不计算阴影
//    if (projCoords.z > 1.0)
//        return 0.0;

//     //当前片元在光源视角下的深度值
//    float currentDepth = projCoords.z;

//     //添加一个很小的偏移，防止自阴影伪影
//    float bias = 0.005;

//     //PCF 滤波
//    float shadow = 0.0;
//    vec2 texelSize = 1.0 / textureSize(shadowMap, 0); // 获取阴影贴图的纹素大小
//    for (int x = -1; x <= 1; ++x) {
//        for (int y = -1; y <= 1; ++y) {
//             //对周围 3x3 的纹素进行采样
//            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
//            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
//        }
//    }
//    shadow /= 9.0; // 取平均值

//    return shadow;
//}

void main() {
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDirection = normalize(-lightDir);

    //漫反射
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor * 1.2; 

    //镜面高光
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 16.0); 
    vec3 specular = spec * lightColor * 0.2;  //降低高光防止过曝

    //环境光
    vec3 ambient = 1.6 * lightColor;  

    //阴影
    float shadow = ShadowCalculation(FragPosLightSpace);
    
    //最终颜色
    vec3 finalColor = (ambient * (1 - shadow)  + (1.0 - shadow) * diffuse + specular) * texColor.rgb;
    
    FragColor = vec4(finalColor, texColor.a);
}
