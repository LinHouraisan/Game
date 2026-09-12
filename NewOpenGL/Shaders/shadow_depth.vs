#version 330 core

layout(location = 0) in vec3 pos; // 顶点位置
layout(location = 5) in ivec4 boneIds; // 骨骼ID
layout(location = 6) in vec4 weights; // 骨骼权重

uniform mat4 lightSpaceMatrix; // 光源空间矩阵
uniform mat4 model; // 模型矩阵

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES]; // 骨骼变换矩阵

void main()
{
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >= MAX_BONES) 
        {
            totalPosition = vec4(pos, 1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos, 1.0f);
        totalPosition += localPosition * weights[i];
    }

    // 定义修正矩阵
    mat4 rotationFix = mat4(
        1.0,  0.0,  0.0,  0.0,
        0.0,  0.0, -1.0,  0.0,
        0.0,  1.0,  0.0,  0.0,
        0.0,  0.0,  0.0,  1.0
    );

    // 定义额外的旋转矩阵（绕 Y 轴旋转 180 度）
    float angle = radians(180.0); // 180 度，转换为弧度
    mat4 additionalRotation = mat4(
        cos(angle),  0.0,  sin(angle),  0.0,
        0.0,         1.0,  0.0,         0.0,
        -sin(angle), 0.0,  cos(angle),  0.0,
        0.0,         0.0,  0.0,         1.0
    );

    // 结合修正矩阵和额外旋转矩阵
    // 注意顺序：先应用 rotationFix，再应用 additionalRotation
    mat4 finalRotation = additionalRotation * rotationFix;

    // 应用最终的旋转矩阵
    totalPosition = finalRotation * totalPosition;

    // 将顶点转换到光源空间
    gl_Position = lightSpaceMatrix * model * totalPosition;
}