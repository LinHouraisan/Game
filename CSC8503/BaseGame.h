#pragma once

#include "GameTechRenderer.h"
#include "PhysicsSystem.h"
#include "BulletWorldManager.h"
#include "ResourceManager.h"
#include "RenderPrepare.h"
#include "SceneManager.h"
#include "Effeksser.h"
#include "Config.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <GLFW/glfw3.h>
#include "Weapon.h"
#include "SingleHitWeapon.h"
#include "WideHitWeapon.h"
#include "LaserWeapon.h"
#include "BounceWeapon.h"
#include "GravityWeapon.h"

extern OpenGL::Camera* camera;

namespace NCL {
    namespace CSC8503 {
        // 定义基础游戏类，提供 TutorialGame 和 NetworkedGame 的共同基础功能
        class BaseGame {
        public:
            BaseGame(GLFWwindow* window, OpenGL::GameTechRenderer* renderer, OpenGL::ResourceManager* resourceManager);
            virtual ~BaseGame();

            // 游戏主循环更新函数 - 纯虚函数，子类必须实现
            virtual void UpdateGame(float dt) = 0;

            virtual void SetPaused(bool paused) {  // 子类可以扩展此方法以处理特定暂停逻辑
                isPaused = paused;      
            }


            // 冲刺技能接口
            bool IsDashOnCooldown() const;
            float GetDashCooldownTimer() const;
            float GetDashCooldownTime() const;

            // 震荡波技能接口
            bool IsShockwaveOnCooldown() const;
            float GetShockwaveCooldownTimer() const;
            float GetShockwaveCooldownTime() const;

            // 黑洞技能接口
            bool IsBlackHoleOnCooldown() const;
            float GetBlackHoleCooldownTimer() const;
            float GetBlackHoleCooldownTime() const;

            // 炮台技能接口
            bool IsTurretOnCooldown() const;
            float GetTurretCooldownTimer() const;
            float GetTurretCooldownTime() const;

        protected:
            // 共享组件与资源
            GLFWwindow* window;
            BulletWorldManager* btWorldManager;
            PhysicsSystem* physics;
            SceneManager* sceneManager;
            OpenGL::GameTechRenderer* render;
            OpenGL::ResourceManager* resourceManager;
            OpenGL::RenderPrepare* renderPrepare;
            EffekseerManager* effekseerManager;

            bool inSelectionMode;
            bool isAutoFire = false; //是否自动开火
            bool useGravity;
            bool playerMode;
            float gameTime;
            float dt;

            // 公共游戏对象
            Player* player;
            std::vector<Weapon*> weapons;
            SingleHitWeapon* pistol;
            WideHitWeapon* shotGun;
            LaserWeapon* laserGun;
            BounceWeapon* bounceGun;
            GravityWeapon* gravityGun;

            // 通用工具函数
            bool isKeyJustPressed(int key);
            bool isMousePressed(int key);
            void ClampMousePosition();
            
            // 内部按键状态跟踪
            std::unordered_map<int, bool> keyStates;
            
            // 虚函数，供子类实现游戏初始化
            virtual void InitWorld() = 0;
            
            // 基础游戏逻辑组件
            // void UpdateKeys();
            void SetPlayerCamera();
            
            // 基础物理系统更新
            void UpdatePhysics(float dt);
            
            // 基础渲染逻辑
            void UpdateRender(float dt);

            // 寻路与怪物移动
            std::vector<GameObject*> mapArrows; //地图箭头
            std::vector<GameObject*> gridArrows; // 存储所有箭头对象
            std::vector<GameObject*> tigers; // 用于存储生成的tiger
            std::vector<std::shared_ptr<Monster>> monsters;
            //std::vector<std::vector<int>> mapGrid; // 存储地图网格信息
            std::vector<std::vector<std::vector<int>>> mapGrids; //多个地图的网格信息
            //std::vector<std::vector<Vector3>> flowFieldDirections; // 存储每个网格的流场方向
            std::vector<std::vector<std::vector<Vector3>>> flowFieldDirections;

            // 旋转插值数据结构
            struct RotationLerpData {
                Quaternion startRot;
                Quaternion targetRot;
                float lerpTime;
                float elapsedTime;
            };

            std::unordered_map<GameObject*, RotationLerpData> rotatingObjects;
            void RotationalInterpolation(float deltaTime);
            void StartRotationInterpolation(GameObject* obj, const Quaternion& targetRot, float duration);
            Vector3 CalculateFlowDirection(const Vector3& position);
            //Vector3 GetGridDirection(int x, int z);
            Vector3 GetGridDirection(int mapIndex, int x, int z);

            Vector3 SnapTo8Directions(const Vector3& rawDir);
            void UniformRotationInterpolation(GameObject* obj, const Quaternion& targetRot, int totalFrames);

            void CreateArrow();
            void UpdateFlowFieldArrows();
            void UpdateMapArrow(int mapIndex);

            float enemyMoveForce = 100.0f; // 敌人的移动力

            float spawnDuration = 30.0f; // 生成持续时间（秒）
            float spawnInterval = 5.0f; // 每次生成之间的间隔（秒）
            float spawnFinalInterval = 1.0f; // 每次生成之间的间隔（秒）
            float timePassed = 0.0f; // 追踪已过去的时间
            bool isSpawningMonsters = false; // 是否正在生成怪物
            float totalTimePassed;

            void StartMonsterGeneration();
            void UpdateEnemyMovement(Monster* enemy, float dt);

            void UpdatePlayer(float dt);

            // 武器相关
            void EquipWeapon(Weapon* newWeapon); // 装备武器
            void UpdateWeapon();
            void UpdateWeapon(Weapon* weapon, Player* playerObj = nullptr);
            float GetFinalDamage(); // 获取所有武器造成的总伤害
            Monster* FindNearestTarget(Player* playerObj = nullptr); //获取离玩家最近的怪物

            Vector3 projectedDir = Vector3(1, 0, 0);
            void PlayerControl();
            Vector3 GetMouseWorldPosition();
            bool isPaused = false;

            // 相机过渡状态相关
            bool isCameraTransitioning = false;  // 是否正在过渡
            float transitionDuration = 5.0f;     // 过渡时间（秒）
            float transitionTimer = 0.0f;        // 当前过渡计时器
            Vector3 transitionStartPos;          // 过渡起点（死亡时摄像机位置）
            Vector3 transitionTargetPos;         // 过渡终点（重生点摄像机位置）

            float HandleGroundForce(float Force);


            // 冲刺技能参数
            bool isDashing = false;       // 是否正在冲刺                                            DashSkill
            float dashDistance = 1000.0f;   // 冲刺距离
            float dashSpeed = 500.0f;      // 冲刺速度
            float dashDuration = 0.3f;    // 冲刺持续时间
            float dashTimer = 0.0f;       // 冲刺计时器
            const float DASH_COOLDOWN = 1.0f;
            float dashCooldownTimer = 0.0f; // 冷却计时器
            bool isDashOnCooldown = false;  // 是否在冷却中
            Vector3 dashDirection;        // 冲刺方向
            void TriggerDashing();
            void UpdateDashing(float dt); // 声明更新函数
            void UpdateDashCooldown(float dt);
            //冲刺特效
            Effekseer::Handle DashingEffectHandle; // 冲刺特效


            //震荡波技能相关参数                                                                          ShockwaveSkill

         // 自动释放控制变量
            bool  m_InMap4 = false;          // 是否在第四关
            int   m_ShockwaveCount = 0;       // 已释放次数
            double m_LastShockwaveTime = 0;   // 上次释放时间
            const int MAX_SHOCKWAVES = 20;     // 最大释放次数
            const double SHOCKWAVE_INTERVAL = 2.0; // 间隔时间(秒)
            void SkillEnhancement();
     

            bool isShockwaveOnCooldown = false;        // 是否冷却中
            float shockwaveCooldownTimer = 0.0f;       // 冷却计时器
            const float SHOCKWAVE_COOLDOWN = 1.0f;     // 冷却时间1秒
            const float SHOCKWAVE_RADIUS = 3.0f;       // 作用半径
            const float SHOCKWAVE_DURATION = 0.5f;  // 扩散持续时间
            const float SHOCKWAVE_DAMAGE = 30.0f;      // 伤害值
            void TriggerShockwave();                   // 触发震荡波
            void UpdateShockwave(float dt);
            void UpdateShockwaveCooldown(float dt);     // 更新冷却
            // 震荡波扩散相关变量
            bool isShockwaveActive = false;    // 是否正在扩散
            float shockwaveRadius = 0.0f;      // 当前扩散半径
            float shockwaveDuration = 0.5f;    // 扩散持续时间
            std::unordered_set<Monster*> shockedMonsters; // 已受伤害的怪物（避免重复伤害）

            //震荡波特效
            Effekseer::Handle ShockwaveEffectHandle; // 震荡波特效




            // 黑洞技能相关参数                                                                               BlackHole
            bool isBlackHoleActive = false;            // 黑洞是否激活
            bool isBlackHoleOnCooldown = false;        // 黑洞是否在冷却中
            float blackHoleDuration = 5.0f;           // 黑洞持续时间
            float blackHoleCooldownTimer = 0.0f;      // 黑洞冷却计时器
            const float BLACKHOLE_COOLDOWN = 10.0f;    // 黑洞冷却时间
            const float BLACKHOLE_RADIUS = 3.0f;      // 黑洞吸引范围
            const float BLACKHOLE_DAMAGE = 10.0f;     // 黑洞每秒伤害
            GameObject* blackHole = nullptr;          // 黑洞对象

            //黑洞特效
            Effekseer::Handle BlackHoleEffectHandle; // 黑洞特效
            // 辅助函数：Vector3 转 glm::vec3
            glm::vec3 ToGLMVec3(const NCL::Maths::Vector3& vec);
            // 辅助函数：Quaternion 转 glm::quat
            glm::quat ToGLMQuat(const NCL::Maths::Quaternion& quat);

            void TriggerBlackHole();                  // 触发黑洞技能
            void UpdateBlackHole(float dt);           // 更新黑洞逻辑
            void UpdateBlackHoleCooldown(float dt);  // 更新黑洞冷却


            
            // 召唤炮台技能参数                                                                               Turret      
            bool isTurretActive = false;           // 是否有激活的炮台
            bool isTurretOnCooldown = false;       // 是否在冷却中
            float turretCooldownTimer = 0.0f;      // 冷却计时器
            float turretDuration = 8.0f;           // 炮台存在时间
            float turretAttackTimer = 0.0f;        // 攻击计时器

            //const float TURRET_LIFETIME = 8.0f;   // 存在时间
            const float TURRET_COOLDOWN = 10.0f;   // 技能冷却时间
            const float TURRET_RANGE = 15.0f;      // 索敌半径
            const float TURRET_ATTACK_INTERVAL = 1.0f; // 攻击间隔
            const float TURRET_DAMAGE = 20.0f;     // 基础伤害值

            GameObject* currentTurret = new GameObject(); // 当前激活的炮台
            GameObject* turretTarget = nullptr;     // 当前攻击目标

            // 特效句柄
            Effekseer::Handle TurretSpawnEffectHandle;//炮台本体特效
            Effekseer::Handle TurretEffectHandle; // 攻击特效

            // 函数声明
            void TriggerTurret();
            void UpdateTurret(float dt);
            void UpdateTurretCooldown(float dt);
            GameObject* FindNearestEnemyForTurret(const Vector3& center);
        };
    }
}
