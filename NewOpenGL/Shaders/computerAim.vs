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
    mat4 boneMatrix[38];
};

layout(std430, binding = 0) buffer RenderObjectBuffer {
    RenderObjectData renderObjects[1000];
};

uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix; 

out vec4 fragColorFactor;
out float fragIsActive;
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace; 

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

void main() {
    int id = gl_InstanceID;  

    if (renderObjects[id].isActive == 0) {
        gl_Position = vec4(0.0);
        fragIsActive = 0.0;
        fragColorFactor = vec4(1.0);
        return;
    } else {
        mat4 model = renderObjects[id].transformMatrix;
        vec4 totalPosition = vec4(0.0f);

        // 骨骼变换
        for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if (boneIds[i] == -1) continue;
            vec4 localPosition = renderObjects[id].boneMatrix[boneIds[i]] * vec4(aPos, 1.0);
            totalPosition += localPosition * weights[i];
        }

        // 坐标系修正矩阵
        mat4 rotationFix = mat4(
            1.0,  0.0,  0.0,  0.0,
            0.0,  0.0, -1.0,  0.0,
            0.0,  1.0,  0.0,  0.0,
            0.0,  0.0,  0.0,  1.0
        );

        // 修正变换顺序: 先修正坐标系再应用模型矩阵
        vec4 correctedPosition = rotationFix * totalPosition;
        vec4 worldPosition = model * correctedPosition;

        // 常规渲染位置
        gl_Position = projection * view * worldPosition;
        
        //阴影
        FragPosLightSpace = lightSpaceMatrix * worldPosition;

        // 法线变换
        mat3 normalMatrix = mat3(transpose(inverse(model * rotationFix)));
        Normal = normalMatrix * norm;

        FragPos = vec3(worldPosition);
        fragIsActive = 1.0;
        fragColorFactor = renderObjects[id].colorFactor;
        TexCoords = tex;
    }
}
