#include "RenderPrepare.h"
#include <glm/gtc/matrix_transform.hpp>
#include "TutorialGame.h"

using namespace OpenGL;
using namespace NCL;
using namespace CSC8503;



// 获取单例实例
RenderPrepare& RenderPrepare::GetInstance() {
    static RenderPrepare instance;
    return instance;
}

void RenderPrepare::Initialize(SceneManager* sceneManager) {
    this->sceneManager = sceneManager;

    CreateTigerComputerShader();

    // 创建 Arrow SSBO
    glGenBuffers(1, &arrowSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, arrowSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_ARROW_OBJECTS * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}


RenderPrepare::~RenderPrepare() {
    delete sceneManager;
    glDeleteBuffers(1, &ssbo);
    glDeleteProgram(computeProgram);
}


void RenderPrepare::Update() {

    RenderingRequest();

    // 全体可渲染对象帧同步
    for (auto* obj : renderObjects) {
        obj->UpdateFrame();
    }

    SeparateObjects();

    UpdateRenderData();

    UseComputeShader();
}


void RenderPrepare::RenderingRequest() {
    renderObjects.clear();
	renderObjects = sceneManager->GetRenderObjects();

}

void RenderPrepare::SeparateObjects() {
    tigerRenderObjects.clear();
    arrowRenderObjects.clear();  
    netPlayersR.clear();

    for (auto* obj : renderObjects) {
        if (obj->GetModel() == ResourceManager::animationModel_tiger) {
            tigerRenderObjects.push_back(obj);
        }
        if (obj->GetModel() == ResourceManager::Model_arrow && obj->GetIsActive()) {
            arrowRenderObjects.push_back(obj);
        }
        if (obj->GetModel() == ResourceManager::Model_player ) {
            playerR = obj;
        }
        if (obj->GetModel() == ResourceManager::Model_netPlayer) {
            netPlayersR.push_back(obj);
            // netPlayerR = obj;
        }
        //if (obj->GetModel() == ResourceManager::Model_gun1) {
        //    weaponR = obj;
        //}
        //if (obj->GetModel() == ResourceManager::Model_gun2) {
        //    weaponR = obj;
        //}
        //if (obj->GetModel() == ResourceManager::Model_gun3) {
        //    weaponR = obj;
        //}
        if (obj->GetModel() == ResourceManager::Model_gun1 ||
            obj->GetModel() == ResourceManager::Model_gun2 ||
            obj->GetModel() == ResourceManager::Model_gun3) 
        {
            //weapons.push_back(obj);
            weaponR = obj;
        }

    }
}



void RenderPrepare::CreateTigerComputerShader() {
    // 创建怪物 Compute Shader
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    const char* computeShaderSource = R"(

#version 430 core

layout(local_size_x = 16) in;

struct RenderObjectData {
    mat4 transformMatrix;
    vec4 colorFactor;
    int isActive;
    int animationIndex;  
    int frame;
    mat4 boneMatrix[38]; // 38个骨骼矩阵
};

layout(std430, binding = 0) buffer RenderObjectBuffer {
    RenderObjectData renderObjects[10000];
};

// 6 个骨骼动画轨道的 SSBO
layout(std430, binding = 1) buffer BoneMatrices1 { mat4 boneMatrices1[120 * 38]; };
layout(std430, binding = 2) buffer BoneMatrices2 { mat4 boneMatrices2[120 * 38]; };
layout(std430, binding = 3) buffer BoneMatrices3 { mat4 boneMatrices3[120 * 38]; };
layout(std430, binding = 4) buffer BoneMatrices4 { mat4 boneMatrices4[120 * 38]; };
layout(std430, binding = 5) buffer BoneMatrices5 { mat4 boneMatrices5[120 * 38]; };
layout(std430, binding = 6) buffer BoneMatrices6 { mat4 boneMatrices6[120 * 38]; };

void main() {

    uint id = gl_GlobalInvocationID.x;

    if (id > 10000 || renderObjects[id].isActive == 0 || renderObjects[id].animationIndex < 1) return; // 直接跳过非法id，未激活的对象以及非动画对象

    int animationIndex = renderObjects[id].animationIndex;

    int frame = renderObjects[id].frame;

    if (animationIndex == 1) {
        for (int b = 0; b < 38; ++b) {
            renderObjects[id].boneMatrix[b] = boneMatrices1[frame * 38 + b];
        }
    } else if (animationIndex == 2) {  
        for (int b = 0; b < 38; ++b) {
            renderObjects[id].boneMatrix[b] = boneMatrices2[frame * 38 + b];
        }
    } else if (animationIndex == 3) {   
        for (int b = 0; b < 38; ++b) {
            renderObjects[id].boneMatrix[b] = boneMatrices3[frame * 38 + b];
        }
    } else if (animationIndex == 4) {
        for (int b = 0; b < 38; ++b) {
            renderObjects[id].boneMatrix[b] = boneMatrices4[frame * 38 + b];
        }
    } else if (animationIndex == 5) {
        for (int b = 0; b < 38; ++b) {
            renderObjects[id].boneMatrix[b] = boneMatrices5[frame * 38 + b];
        }
    } else {
        for (int b = 0; b < 38; ++b) {
            renderObjects[id].boneMatrix[b] = boneMatrices6[frame * 38 + b];
        }
    }
}

)";

    glShaderSource(computeShader, 1, &computeShaderSource, NULL);
    glCompileShader(computeShader);

    // 检查编译错误
    GLint success;
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(computeShader, 1024, NULL, infoLog);
        // std::cout << "ERROR::COMPUTE_SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);

    // 检查链接错误
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(computeProgram, 1024, NULL, infoLog);
        // std::cout << "ERROR::COMPUTE_SHADER::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(computeShader);

    // 创建并初始化 SSBO
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

    // 初始化 1000 个对象的默认值
    std::vector<RenderObjectData> defaultData(MAX_RENDER_OBJECTS);
    for (int i = 0; i < MAX_RENDER_OBJECTS; ++i) {
        defaultData[i].isActive = 0;  // 默认所有对象未激活
        defaultData[i].animationIndex = -1; // 负数代表无效动画
        defaultData[i].transformMatrix = glm::mat4(0.0f); // 清零矩阵
    }

    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_RENDER_OBJECTS * sizeof(RenderObjectData), defaultData.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);    

    // 创建 6 个骨骼 SSBO
    glGenBuffers(6, boneSSBOs);

    GLsizeiptr totalMat4 = tiger_frame * tiger_boneCount; // 2280

    for (int i = 0; i < 6; ++i) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, boneSSBOs[i]);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
            totalMat4 * sizeof(glm::mat4),
            nullptr,
            GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    UpdateAnimation();

    // 绑定 6 个骨骼 SSBO
    for (int i = 0; i < 6; ++i) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i + 1, boneSSBOs[i]);
    }

}


void RenderPrepare::UpdateRenderData() {

    std::vector<RenderObjectData> data(MAX_RENDER_OBJECTS);

    int index = 0;

    for (auto* obj : tigerRenderObjects) {

        if (index >= MAX_RENDER_OBJECTS) continue;

        NCL::Maths::Matrix4 nclMat = obj->GetTransform()->GetMatrix();
        data[index].transformMatrix = glm::make_mat4(nclMat.GetData());

        NCL::Maths::Vector4 color = obj->GetColorFactor();
        data[index].colorFactor = glm::vec4(color.x, color.y, color.z, color.w);

        data[index].isActive = obj->GetIsActive();

        data[index].animationIndex = obj->GetAnimationIndex();

        data[index].frame = obj->GetOwnFrame();

        index++;
    }

    // 更新 SSBO 数据
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, MAX_RENDER_OBJECTS * sizeof(RenderObjectData), data.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


    std::vector<glm::mat4> arrowData(MAX_ARROW_OBJECTS);

    int index2 = 0;
    for (auto* obj : arrowRenderObjects) {
        if (index2 >= MAX_ARROW_OBJECTS) break; // 限制 2000 个

        NCL::Maths::Matrix4 nclMat = obj->GetTransform()->GetMatrix();
        arrowData[index2] = glm::make_mat4(nclMat.GetData());
        index2++;
    }

    // 更新 arrowSSBO
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, arrowSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, MAX_ARROW_OBJECTS * sizeof(glm::mat4), arrowData.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void RenderPrepare::UpdateAnimation() {
    // 1. 计算所需缓冲区大小

    for (int i = 0; i < 6; ++i) {
        std::vector<glm::mat4> totalMatrices(tiger_frame * tiger_boneCount);

        // 2. 收集所有帧的数据
        for (int frame = 0; frame < tiger_frame; ++frame) {
            auto& matrices = ResourceManager::animationController_tiger->GetPrecomputedMatrices(i+1);
            // 将每帧数据存入 totalMatrices
            std::memcpy(&totalMatrices[frame * tiger_boneCount], matrices.data(), tiger_boneCount * sizeof(glm::mat4));
        }

        auto& matrices = totalMatrices;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, boneSSBOs[i]);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, matrices.size() * sizeof(glm::mat4), matrices.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}



void RenderPrepare::UseComputeShader() {

    int workGroupSize = 32; // 视具体情况调整工作组大小
    int numRenderObjects = renderObjects.size();
    int numGroups = (numRenderObjects + workGroupSize - 1) / workGroupSize;

    glUseProgram(computeProgram);

    // 绑定 SSBO
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    // 执行 Compute Shader
    glDispatchCompute(numGroups, 1, 1);

    // 确保计算完成
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);


    // 解绑 SSBO（可选，除非之后有其他需要）
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}