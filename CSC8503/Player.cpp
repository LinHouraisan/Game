#include "Player.h"
#include "GameWorld.h"
#include "AudioManager.h"

namespace NCL::CSC8503 {
	Player::Player() {
		health = 100.0f;
		maxHealth = 100.0f;

		lifeSteal = 0.0f;
		cooldownReduction = 0.0f;

		isDead = false;
		respawnTimer = -1.0f;
		respawnPos = Vector3(-10, 0.1, 0);

	}

	Player::~Player() {}


	//  受到怪物攻击
	void Player::TakeDamage(int damage) {
		// 检查是否处于无敌状态或死亡
		if (invincibleTimer > 0 || IsDead()) {
			invincibleTimer -= 0.003;

			return; // 无敌时间内无视伤害
		}

		AudioManager::PlaySound("hurt");

		health -= damage;
		//受击音效，动画

		if (health < 0) health = 0;

		// std::cout << "Player took " << damage << " damage! HP: " << health << std::endl;


		// 触发无敌状态
		invincibleTimer = INVINCIBLE_DURATION;

	}

	// 检查死亡状态
	void Player::CheckDeath() {
		if (isDead || health) return;
		isDead = true;
		// 播放死亡动画

		respawnTimer = 6.0f; // 3秒后重生
		// std::cout << "Player has died! Respawning in 3 seconds..." << std::endl;
		respawnTimer = 5.0f; // 5秒后重生
		// std::cout << "Player has died! Respawning in 5 seconds..." << std::endl;


		if (physicsObject) {
			btRigidBody* body = physicsObject->GetBulletBody();
			if (body)
			{
				//physicsObject->GetBtWorldManager()->RemoveRigidBody(body);
				physicsObject->SetKinematic(true);
			}

		}
		//TODO:停止渲染


	}

	// 新增重生方法
	void Player::Respawn() {
		isDead = false;
		health = maxHealth;
		invincibleTimer = 3.0f; // 重生后3秒无敌

		if (physicsObject) {
			physicsObject->BTSetPosition(respawnPos);
			physicsObject->SetKinematic(false);
		}
		// std::cout << "Player has respawned!" << std::endl;

	}

}
