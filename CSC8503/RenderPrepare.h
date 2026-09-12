// RenderPrepare.h
#pragma once
#include "model_animation.h"
#include "shader.h"
#include "Instantiate.h"
#include <random>
#include "ResourceManager.h"
#include "animation_controller.h"
#include "SceneManager.h"
#include "shader_c.h"
#include "RenderObject.h"
#include "Config.h"

struct alignas(16) RenderObjectData {
    glm::mat4 transformMatrix;
    glm::vec4 colorFactor;
    int isActive;
    int animationIndex;
    int frame;
    glm::mat4 boneMatrix[38];
};

//struct alignas(16) DebugRenderObjectData {
//    glm::mat4 transformMatrix; // 物体的变换矩阵
//    glm::vec4 shapeColor;      // 碰撞盒的颜色
//    int shapeType;             // 碰撞盒的类型（Box, Sphere, Capsule, Cylinder）
//    glm::vec3 shapeSize;       // 碰撞盒的大小
//};

using namespace NCL;
using namespace CSC8503;

namespace OpenGL {
    class RenderPrepare {
    public:
        static RenderPrepare& GetInstance();

        void Initialize(SceneManager* sceneManager);
        void RenderingRequest();
        void Update();
        void SeparateObjects();
        void UpdateAnimation();
        void UpdateRenderData();
        void CreateTigerComputerShader();
        void UseComputeShader();
        int GetTigerNumber() { return tigerRenderObjects.size(); }
        int GetArrowNumber() { return arrowRenderObjects.size(); }
        RenderObject* GetPlayerRender(){ return playerR; }
        // RenderObject* GetNetPlayersRender(){ return netPlayerR; }
        std::vector<RenderObject*>& GetNetPlayersRender() { return netPlayersR; }

        std::vector<RenderObject*> GetWeaponsRender() const { return weapons; }
        void AddWeapon(RenderObject* weapon) { weapons.push_back(weapon); }
        void RemoveWeapon(RenderObject* weapon) 
        {
            weapons.erase(std::remove(weapons.begin(), weapons.end(), weapon), weapons.end());
        }

        //油桶渲染相关
        std::vector<RenderObject*> GetBarrelsRender() const { return barrels; }
        void AddBarrel(RenderObject* barrel) { barrels.push_back(barrel); }
        void RemoveSingleBarrel(RenderObject* barrel) {
            auto it = std::find(barrels.begin(), barrels.end(), barrel);
            if (it != barrels.end()) {
                barrels.erase(it);
            }
        }
        void RemoveBarrel()
        {
            barrels.clear();
        }

        RenderObject* GetWeaponRender() { return weaponR; }

        GLuint GetArrowSSBO() const { return arrowSSBO; }  // 获取 arrow SSBO
        GLuint GetDebugSSBO() const { return debugSSBO; }  // 新增：获取 debug SSBO

        GLuint GetSSBO() const { return ssbo; }
        std::unordered_map<Model*, int> GetRenderObjectsMap() const { return renderPipelineTable; }

    private:
        RenderPrepare() = default;
        ~RenderPrepare();

        // 禁止拷贝和赋值
        RenderPrepare(const RenderPrepare&) = delete;
        RenderPrepare& operator=(const RenderPrepare&) = delete;

        std::unordered_map<Model*, int> renderPipelineTable;
        SceneManager* sceneManager = nullptr;
        std::vector<RenderObject*> renderObjects;
        std::vector<RenderObject*> tigerRenderObjects;
        std::vector<RenderObject*> arrowRenderObjects;
        std::vector<PhysicsObject*> debugRenderObjects;

        GLuint computeShader;
        GLuint computeProgram;
        GLuint ssbo;
        GLuint boneSSBOs[6];
        GLuint arrowSSBO;  // Arrow 专属 SSBO
        GLuint debugSSBO; // 新增：Debug 专属 SSBO

        bool renderDataDirty = true;
        bool animationDataDirty = true;

        RenderObject* playerR;
        // RenderObject* netPlayerR;
		std::vector<RenderObject*> netPlayersR;
        RenderObject* weaponR;
        std::vector<RenderObject*> weapons;
        std::vector<RenderObject*> barrels;

    };
}