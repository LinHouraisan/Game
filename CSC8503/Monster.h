#pragma once 

#include <string>
#include "GameObject.h" 
#include <thread>
#include <chrono>
#include "Vector.h"
#include "SceneManager.h"
#include "Player.h"

using namespace NCL::CSC8503;
using namespace NCL::Maths;

namespace NCL {
    namespace CSC8503 {

        class SceneManager; // 添加前向声明

        enum class MonsterState {
            IDLE,
            ATTACK
        };

        class Monster : public GameObject {
        public:
            Monster(const std::string& name, float health, float damage, const Vector3& position, SceneManager* scene)
                : GameObject(name), health(health), damage(damage), state(MonsterState::IDLE) {
                GetTransform().SetPosition(position);
                this->sceneManager = scene;
            }

            virtual ~Monster() = default;

            // 新增方法：检查怪物是否死亡
            bool IsDead() const { return isDead; }

            float GetHealth() const { return health; }
            void SetHealth(float h) { health = h; }

            float GetDamage() const { return damage; }
            void SetDamage(float d) { damage = d; }
            std::string GetName() const { return name; }

            void UpdateState(float deltaTime);//更新状态机

            void HandleHurt(float weaponDamage, const btVector3& startPos, const btVector3& btEndPos);//处理受伤（武器）

            void HandleDeath();//处理死亡（武器）

            void MonsterUpdate(float deltaTime);//怪物更新

            bool MonsterFinish = false;

            void AttackPlayer();

            // 新增静态计数方法
            static int GetKillCount() { return s_killCount; }
            static void ResetKillCount() { s_killCount = 0; }

            void SetAsBoss() { isBoss = true; } // 让特定怪物变成Boss
            bool IsBoss() const { return isBoss; }
            static bool GetBossDeathStatus() { return isBossDead; }

        private:
            float health;
            float damage;
            MonsterState state;
            Vector3 playerPosition;
            Vector3 patrolDirection;
            float patrolTimer;
            float patrolInterval;
            SceneManager* sceneManager; // 存储 SceneManager 指针

            float deathTimer = -1.0f;  // 计时器，-1 表示未激活
            bool isDead = false;       // 是否已进入死亡状态

            Vector3 knockbackVelocity; // 存储击退速度
            float knockbackTime = 0.0f; // 击退剩余时间
            float attackAnimationTimer = 0.0f; // 记录攻击动画播放剩余时间

            // 静态击杀计数器
            static int s_killCount;

            bool isBoss = false; // 默认不是Boss
            static bool isBossDead; // 静态变量，标记Boss是否死亡
        };
    }
}