#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in ivec4 boneIds; 
layout(location = 6) in vec4 weights;

struct RenderObjectData {
    mat4 transformMatrix;
    vec4 colorFactor;
    int isActive;
    int animationIndex;  
    int frame;
    mat4 boneMatrix[38];  // 骨骼变换矩阵
};

layout(std430, binding = 0) buffer RenderObjectBuffer {
    RenderObjectData renderObjects[1000];
};

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 lightSpaceMatrix;

void main()
{

    if (renderObjects[gl_InstanceID].isActive == 0) {
        gl_Position = vec4(0.0);
        return;
    }else {
        // 获取当前实例的变换矩阵
        mat4 model = renderObjects[gl_InstanceID].transformMatrix;

        vec4 totalPosition = vec4(0.0f);

        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
        {
            if(boneIds[i] == -1) 
                continue;

            vec4 localPosition = renderObjects[gl_InstanceID].boneMatrix[ boneIds[i] ] * vec4(aPos, 1.0f);
            totalPosition += localPosition * weights[i];
       }

        // 现在再进行坐标修正
        mat4 rotationFix = mat4(
            1.0,  0.0,  0.0,  0.0,
            0.0,  0.0, -1.0,  0.0,
            0.0,  1.0,  0.0,  0.0,
            0.0,  0.0,  0.0,  1.0
        );

        totalPosition = rotationFix * totalPosition;  // 这里再做旋转修正


        // 最终顶点位置
        gl_Position = lightSpaceMatrix * model * totalPosition;
    }
}
