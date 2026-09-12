#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform float exposure;
uniform float contrast = 1.0;  // 新增对比度控制
uniform float saturation = 1.4; // 新增饱和度控制
uniform float vibrance = 2.2;   // 新增鲜艳度控制

// 更夸张的色调映射曲线
vec3 ExaggeratedTonemap(vec3 x) {
    // 更强烈的S曲线
    float a = 1.5;  // 增加对比度
    float b = 0.2;  // 增加暗部
    float c = 1.0;  // 增加亮部
    float d = 0.6;  // 调整中间调
    float e = 0.2;  // 增加暗部细节
    
    vec3 result = (x * (a * x + b)) / (x * (c * x + d) + e);
    return pow(result, vec3(1.0/1.8)); // 使用更低的gamma值增加对比度
}

// 饱和度调整
vec3 applySaturation(vec3 color, float saturation) {
    vec3 luminance = vec3(dot(color, vec3(0.2126, 0.7152, 0.0722)));
    return mix(luminance, color, saturation);
}

// 鲜艳度调整(选择性增加低饱和颜色的饱和度)
vec3 applyVibrance(vec3 color, float vibrance) {
    float average = (color.r + color.g + color.b) / 3.0;
    float maxColor = max(color.r, max(color.g, color.b));
    float amount = (maxColor - average) * (-vibrance * 3.0);
    return color + (color - maxColor) * amount;
}

void main() {
    vec3 hdrColor = texture(scene, TexCoords).rgb;
    
    // 应用曝光
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure * 2.0); // 增加曝光系数
    
    // 应用更夸张的色调映射
    mapped = ExaggeratedTonemap(mapped);
    
    // 应用饱和度调整
    mapped = applySaturation(mapped, saturation);
    
    // 应用鲜艳度调整
    mapped = applyVibrance(mapped, vibrance);
    
    // 应用对比度
    mapped = (mapped - 0.5) * contrast + 0.5;
    
    // 限制范围并输出
    FragColor = vec4(clamp(mapped, 0.0, 1.0), 1.0);
}