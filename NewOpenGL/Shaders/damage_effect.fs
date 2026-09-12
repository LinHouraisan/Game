#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float intensity;  // 控制效果强度

void main() {
    // 计算到屏幕边缘的距离
    vec2 center = vec2(0.5, 0.5);
    float distance = length(TexCoords - center);
    
    // 创建边缘渐变效果
    float edgeEffect = smoothstep(0.2, 0.5, distance);
    
    // 应用强度参数
    edgeEffect *= intensity;
    
    // 红色效果
    vec4 redEffect = vec4(1.0, 0.0, 0.0, edgeEffect);
    
    // 输出颜色
    FragColor = redEffect;
}