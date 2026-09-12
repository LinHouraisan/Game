#pragma once

#include "GameTechRenderer.h"
#include "PhysicsSystem.h"
#include "BaseGame.h"
#include "Quaternion.h"
#include "StateGameObject.h"
#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "ResourceManager.h"
#include "RenderPrepare.h"
#include "SceneManager.h"
#include <unordered_map>
#include "Effeksser.h"
#include "Config.h"

#include "GameWorld.h"
#include "PhysicsObject.h"
#include <random>
#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include <string>
#include <queue>

#include <windows.h>
#include <psapi.h>
#include <iostream>


extern OpenGL::Camera* camera;


namespace NCL {
    namespace CSC8503 {
        // 重构后的TutorialGame，现在继承自BaseGame
        class TutorialGame : public BaseGame {
        public:
            TutorialGame(GLFWwindow* window, OpenGL::GameTechRenderer* renderer, OpenGL::ResourceManager* resourceManager);
            ~TutorialGame();

            void UpdateGame(float dt) override; // 覆盖基类方法

            //爆炸事件
            void HandleBarrelExplode(const Vector3& position);
            void SetupWeapon(SingleHitWeapon* weapon) {
                weapon->SetGameContext(this);
                weapon->SetBarrels(Barrels);
            }
            float GetMemoryUsage();

            bool GetIsWin() { return isWin; }

        protected:
            void UpdateKeys();
            // void PlayerControl();
            void InitWorld() override; // 实现基类的纯虚函数
            void ControlGravity();
            void CheckLevelCompletion();

            void SpawnFirstTigers(int mapIndex);
            void SpawnFinalTigers(int mapIndex);
            void SpawnBossTigers(int mapIndex);
            void SpawnSpeedTigers(int mapIndex);

            void UpdateEvent();
            void CreateCoin();
            void UpdateHealingEvent();
            void UpdateBarrelEvent(int mapIndex);
            
            //void LoadMap(const std::string& filename);
            std::vector<std::vector<int>> LoadMap(const std::string& filename);

            void SwitchLevel();

            //void UpdateWeapon(Weapon* weapon);
            void GetWeapon();
            int WeaponCollected = 0;
           
            // TutorialGame特有的属性
            bool isPressure;

    
            void SkillEnhancement();//震荡波技能自动释放
            
           
            
            float minPitch = -80.0f;
            float maxPitch = 80.0f;
            float rotationSpeed = 2.0f;
            float rotationSmoothFactor = 20.0f; // 转向平滑系数（越大转向越快）
            float maxAngularVelocity = 10.0f;

            int countP = 0;
            int levelIndex = 0; //目前的关卡位置
            bool eKeyPressed = false; //跟踪E键状态
            bool eKeyWasPressed = false; //上一帧E键状态
            void UpdatePlayerPosition();

            void UpdateMonsterGeneration(float deltaTime);
            void UpdateEffectPostion(Effekseer::Handle EffectHandle);  //特效跟随玩家设置

            Effekseer::Handle healingEffectHandle = -1; //治疗特效跟随标志


            GameObject* testobj_tiger;
            GameObject* testobj_gun;

            GameObject* Coin;
            GameObject* Barrel;
            GameObject* Water;
            int lastCoinMapCount = -1; //记录金币生成
            
            std::vector<GameObject*> Coins; //金币
            std::vector<GameObject*> Barrels; //油桶

            bool hasSpawnedBoss = false; // 是否已生成boss
            bool isWin = false;
        };
    }
}

extern bool monsterDead;
extern bool CoinRemoved;
extern bool switchMap;//是否切换关卡
extern int switchMapCount;
