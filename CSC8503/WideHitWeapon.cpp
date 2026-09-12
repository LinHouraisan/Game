#include "WideHitWeapon.h"
#include "TutorialGame.h"

using namespace OpenGL;

namespace NCL::CSC8503
{
	WideHitWeapon::WideHitWeapon()
	{
	}
	WideHitWeapon::~WideHitWeapon()
	{
	}

	void WideHitWeapon::Initialize(int id, float damage, float range, float cd, int pelletNum, float spreadAng)
	{
		weaponID = id;
		basicDamage = damage;
		attackRange = range;
		attackCooldown = cd;
		pelletCount = pelletNum;
		spreadAngle = spreadAng;
	}

	//攻击模式为：多次射线检测，不穿透目标，如霰弹枪
	void WideHitWeapon::Attack()
	{
		//更新冷却时间
		float currentTime = glfwGetTime();

		if (currentTime - lastAttackTime < attackCooldown)
		{
			return;
		}

		lastAttackTime = currentTime;

		//计算子弹终点
		Vector3 startPos = this->GetPhysicsObject()->BTGetPosition();
		btVector3 btStartPos(startPos.x, startPos.y, startPos.z);

		for (int i = 0; i < pelletCount; i++)
		{
			if (Vector::Length(weaponLookDir) < SIMD_EPSILON) {
				return;
			}
			float angleOffset = (rand() % 2000 - 1000) / 1000.0f * glm::radians(spreadAngle); // 随机散射角度

			//射击方向
			btVector3 forwardDir = btVector3(weaponLookDir.x, weaponLookDir.y, weaponLookDir.z);
			btMatrix3x3 rotationMatrix;
			rotationMatrix.setEulerYPR(angleOffset, 0.0f, 0.0f); //绕 Y 轴旋转 angleOffset
			forwardDir = rotationMatrix * forwardDir;


			//射击终点
			btVector3 btEndPos = btStartPos + forwardDir * attackRange;

			if (Vector::Length(weaponLookDir) < SIMD_EPSILON) {
				return;
			}
			//射线检测
			btCollisionWorld::ClosestRayResultCallback rayCallback(btStartPos, btEndPos);
			if (switchMapCount == 4) {
				rayCallback.m_collisionFilterGroup = RAYCAST_GROUP;
				rayCallback.m_collisionFilterMask = TIGER_GROUP;
			}
			BulletWorldManager* btWorldManager = new BulletWorldManager();
			btDiscreteDynamicsWorld* world = btWorldManager->GetPhysicsWorld();
			world->rayTest(btStartPos, btEndPos, rayCallback);

			//播放特效
			Vector3 effPosition = startPos;


			Quaternion effRotation = Quaternion::FromTwoVectors(Vector3(0, 0, 1), 
				Vector3(forwardDir.x(), forwardDir.y(), forwardDir.z())); //此处修正与SingleHitWeapon不一样
			Quaternion fixedEffRotation = effRotation * Quaternion(Vector3(0, 1, 0), -1.0f * SIMD_PI / 4.0f); //添加固定角度修正

			Vector3 eulerAngles = fixedEffRotation.ToEuler();
			float effRotX = glm::radians(eulerAngles.x);
			float effRotY = glm::radians(eulerAngles.y);
			float effRotZ = glm::radians(eulerAngles.z);

			bulletHandle = EffekseerManager::GetInstance()->PlayEffect(ResourceManager::shotGunBulletEffect, effPosition.x, effPosition.y, effPosition.z, effRotX, effRotY, effRotZ);

			//射线可视化调试
			glm::vec3 start = glm::vec3(startPos.x, startPos.y, startPos.z);
			glm::vec3 end = glm::vec3(btEndPos.x(), btEndPos.y(), btEndPos.z());
			ResourceManager::AddDebugRay(start, end);

			//命中逻辑
			if (rayCallback.hasHit())
			{
				GameObject* hitObject = static_cast<GameObject*>(rayCallback.m_collisionObject->getUserPointer());
				Monster* hitMonster = dynamic_cast<Monster*>(hitObject);

				if (hitMonster) //命中object是怪物
				{
					if (hitMonster->GetHealth() > basicDamage)
					{
						totalDamage += basicDamage;
						hitMonster->HandleHurt(basicDamage, btVector3(startPos.x, startPos.y, startPos.z), btEndPos);
					}
					else
					{
						totalDamage += hitMonster->GetHealth();
						hitMonster->HandleDeath();
					}

					//命中后停止特效播放（未完成），有bug
					//EffekseerManager::GetInstance()->StopEffect(bulletHandle);
				}
			}
		}
		ApplyRecoil();




	}
}


