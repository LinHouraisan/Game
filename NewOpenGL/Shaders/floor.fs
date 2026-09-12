#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;  // 阴影贴图采样器
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 viewPos;

// 根据光源视角下的坐标计算阴影因子（0 表示无阴影，1 表示全阴影）
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // 透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 将坐标从[-1, 1]转换到[0, 1]
    projCoords = projCoords * 0.5 + 0.5;
    // 读取阴影贴图中对应位置的深度值
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // 当前片元在光源视角下的深度值
    float currentDepth = projCoords.z;
    // 添加一个很小的偏移，防止自阴影伪影
    float bias = 0.005;
    // 如果当前深度超过存储的深度，则说明处于阴影中
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    // 如果投影坐标超出[0,1]，则说明片元不在光源视野内，不计算阴影
    if(projCoords.z > 1.0)
        shadow = 0.0;
    return shadow;
}

void main()
{
    // 假设地面法线为正 Y 方向
    vec3 norm = vec3(0.0, 1.0, 0.0);
    vec3 lightDir = normalize(-lightDirection);
    
    // 漫反射分量
    float diff = max(dot(norm, lightDir), 0.0);
    
    // 环境光与漫反射光
    vec3 ambient = 0.2 * lightColor;
    vec3 diffuse = diff * lightColor;
    
    // 计算阴影因子
    float shadow = ShadowCalculation(FragPosLightSpace);
    
    // 最终颜色：被阴影区域的漫反射项减弱
    vec3 color = texture(texture_diffuse1, TexCoords).rgb;
    vec3 lighting = (ambient + (1.0 - shadow) * diffuse) * color;
    
    FragColor = vec4(lighting, 1.0);
}
