#version 330 core

// 顶点属性
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

// 实例化的变换矩阵，mat4 占用 4 个连续的属性位置（7, 8, 9, 10）
layout(location = 7) in vec4 instanceModel0;
layout(location = 8) in vec4 instanceModel1;
layout(location = 9) in vec4 instanceModel2;
layout(location = 10) in vec4 instanceModel3;

// Uniform 变量
uniform mat4 projection;
uniform mat4 view;

// 骨骼相关常量和矩阵
const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

// 输出变量
out vec2 TexCoords;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >=MAX_BONES) 
        {
            totalPosition = vec4(pos,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos,1.0f);
        totalPosition += localPosition * weights[i];
        vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * norm;
   }
	
    // 组装实例化矩阵
    mat4 instanceModel = mat4(instanceModel0, instanceModel1, instanceModel2, instanceModel3);
    // 应用实例化的变换矩阵
    mat4 viewModel = view * instanceModel;

    
    gl_Position =  projection * viewModel * totalPosition;

	TexCoords = tex;
}
