#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 texCoords;

layout(std430, binding = 1) buffer ArrowTransformBuffer {
    mat4 arrowTransforms[2000]; // 从 SSBO 读取变换矩阵
};

uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;

void main() {
    int id = gl_InstanceID;
    mat4 model = arrowTransforms[id]; // 读取 SSBO 中的模型变换矩阵

    //  只旋转局部坐标，而不是 model
    mat4 rotationFix = mat4(
        1.0,  0.0,  0.0,  0.0,
        0.0,  0.0, -1.0,  0.0,
        0.0,  1.0,  0.0,  0.0,
        0.0,  0.0,  0.0,  1.0
    );

    vec4 fixedLocalPos = rotationFix * vec4(aPos, 1.0);  //  只影响局部坐标

    gl_Position = projection * view * model * fixedLocalPos; // 现在 `model` 还是世界坐标系，不受 rotationFix 影响
    TexCoords = texCoords;
}
