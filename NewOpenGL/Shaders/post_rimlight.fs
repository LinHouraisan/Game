#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sceneTexture;    // 场景颜色纹理
uniform sampler2D depthTexture;    // 深度纹理
uniform mat4 invProjection;        // 逆投影矩阵
uniform mat4 invView;              // 逆视图矩阵
uniform vec3 viewPos;              // 相机位置

uniform bool isRimLighting;        // 是否启用边缘光
uniform float rimPower;            // 边缘光强度
uniform vec3 rimColor;             // 边缘光颜色

// 从深度纹理重建世界坐标
vec3 WorldPosFromDepth(float depth) {
    vec4 clipSpacePosition = vec4(TexCoords * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewSpacePosition = invProjection * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = invView * viewSpacePosition;
    return worldSpacePosition.xyz;
}

void main()
{
    // 获取场景颜色
    vec4 sceneColor = texture(sceneTexture, TexCoords);
    
    if (!isRimLighting) {
        FragColor = sceneColor;
        return;
    }
    
    // 获取深度值并重建世界坐标
    float depth = texture(depthTexture, TexCoords).r;
    vec3 worldPos = WorldPosFromDepth(depth);
    
    // 计算视线方向和法线
    vec3 viewDir = normalize(viewPos - worldPos);
    vec3 normal = normalize(cross(dFdx(worldPos), dFdy(worldPos)));
    
    // 计算边缘光效果
    float rim = 1.0 - max(dot(viewDir, normal), 0.0);
    rim = pow(rim, rimPower);
    
    // 混合边缘光效果
    vec3 rimEffect = rim * rimColor;
    FragColor = vec4(sceneColor.rgb + rimEffect, sceneColor.a);
}