#include "GravityWeapon.h"

using namespace OpenGL;

namespace NCL::CSC8503
{
	GravityWeapon::GravityWeapon()
	{
	}
	GravityWeapon::~GravityWeapon()
	{
	}

	void GravityWeapon::Initialize(int id, float range, float cd, float radius, float duration, float force, SceneManager* sceneMgr)
	{
		weaponID = id;
		attackRange = range;
		attackCooldown = cd;
		gravityRadius = radius;
		gravityDuration = duration;
		gravityForce = force;
		sceneManager = sceneMgr;
	}
	
	void GravityWeapon::ApplyGravityField(float dt) 
	{
		if (!isGravityActive) return;

		elapsedTime += dt;

		if (elapsedTime >= gravityDuration) 
		{
			isGravityActive = false;
			return;
		}

		// 对范围内的所有怪物施加引力
		std::vector<Monster*> monsters = sceneManager->GetMonsters();
		for (auto monster : monsters)
		{
			Vector3 monsterPos = monster->GetPhysicsObject()->BTGetPosition();
			Vector3 toCenter = gravityCenter - monsterPos;
			float distance = Vector::Length(toCenter);

			if (distance < gravityRadius)
			{ // 仅影响范围内的怪物
				Vector3 pullForce = Vector::Normalise(toCenter) * (gravityForce / (distance + 1.0f));
				monster->GetPhysicsObject()->AddForce(pullForce);
			}
		}
	}

	//攻击模式为：释放引力场
	void GravityWeapon::Attack()
	{
		//更新冷却时间
		float currentTime = glfwGetTime();
		if (currentTime - lastAttackTime < attackCooldown)
		{
			return;
		}
		lastAttackTime = currentTime;

		//计算生成终点
		Vector3 startPos = this->GetPhysicsObject()->BTGetPosition();
		Vector3 targetPos = startPos + weaponLookDir * attackRange;

		gravityCenter = targetPos;
		isGravityActive = true;
		elapsedTime = 0.0f;

		// 播放特效
		bulletHandle = EffekseerManager::GetInstance()->PlayEffect(ResourceManager::gravityFieldEffect, targetPos.x, targetPos.y, targetPos.z);

		//应用后坐力
		ApplyRecoil();

	}
}


