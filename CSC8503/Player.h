#pragma once
#include "GameObject.h"
#include <vector>

namespace NCL::CSC8503 {

    class SceneManager;

    class Player : public GameObject {
    public:
        Player();
        ~Player();

        // 角色受击与死亡
        void TakeDamage(int damage);
        void CheckDeath();
        void Respawn();

        float GetHealth() const { return health; }
        void SetHealth(float value) { health = std::clamp(value, 0.0f, maxHealth); }

        float GetMaxHealth() const { return maxHealth; }
        void SetMaxHealth(float value) { maxHealth = value; }

        // 状态相关
        bool IsDead() const { return isDead; }
        bool IsInvincible() const { return invincibleTimer > 0; }
        float GetInvincibleTimer() const { return invincibleTimer; }
        float GetRespawnTimer() const { return respawnTimer; }
        void  ReduceRespawnTimer(float dt) { respawnTimer -= dt; }
        void  ReduceInvincibleTimer(float dt) { invincibleTimer -= dt; }

        void SetRespawnPos(Vector3 pos) {
            respawnPos = pos;
        }

        Vector3 GetRespawnPos() {
            return respawnPos;
        }



    private:
        float health;
        float maxHealth;

        float lifeSteal;
        float cooldownReduction;

        float invincibleTimer;  // 新增：无敌时间计时器
        static constexpr float INVINCIBLE_DURATION = 0.8f; // 无敌时间0.8秒
        float respawnTimer; // 重生计时器

        // 角色状态
        bool isDead;
        Vector3 respawnPos;


    };
}
