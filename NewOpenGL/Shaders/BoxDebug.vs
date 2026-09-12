#version 430 core

layout(location = 0) in vec3 inPosition; // 输入顶点位置（单位立方体的顶点）

// DebugRenderObjectData 结构体
struct DebugRenderObjectData {
    mat4 transformMatrix; // 物体的变换矩阵
    vec4 shapeColor;      // 碰撞盒的颜色
    int shapeType;        // 碰撞盒的类型（Box, Sphere, Capsule, Cylinder）
    vec3 shapeSize;       // 碰撞盒的大小
};

layout(std430, binding = 7) buffer DebugRenderObjectBuffer {
    DebugRenderObjectData debugRenderObjects[1000]; // 最多支持 1000 个 debug 对象
};

uniform int objectIndex; // 当前渲染的对象索引

out vec4 fragColor; // 传递颜色到片段着色器

void main() {
    // 获取当前对象的变换矩阵和形状大小
    mat4 transform = debugRenderObjects[objectIndex].transformMatrix;
    vec3 shapeSize = debugRenderObjects[objectIndex].shapeSize;

    // 根据形状大小缩放顶点位置
    vec3 scaledPosition = inPosition * shapeSize;

    // 应用变换矩阵
    gl_Position = transform * vec4(scaledPosition, 1.0);

    // 传递颜色到片段着色器
    fragColor = debugRenderObjects[objectIndex].shapeColor;
}