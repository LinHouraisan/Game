#version 430 core

in vec4 fragColor; // 从顶点着色器传递的颜色

out vec4 outColor; // 输出颜色

void main() {
    outColor = fragColor; // 直接使用传递的颜色
}