#include "LaserWeapon.h"
#include "TutorialGame.h"

using namespace OpenGL;

namespace NCL::CSC8503
{
	LaserWeapon::LaserWeapon()
	{
	}
	LaserWeapon::~LaserWeapon()
	{
	}

	void LaserWeapon::Initialize(int id, float damage, float range, float cd)
	{
		weaponID = id;
		basicDamage = damage;
		attackRange = range;
		attackCooldown = cd;
	}

	//攻击模式为：单次射线检测，可穿透射线上的所有目标，如激光
	void LaserWeapon::Attack()
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
		btVector3 forwardDir = btVector3(weaponLookDir.x, weaponLookDir.y, weaponLookDir.z);
		btVector3 btEndPos = btStartPos + forwardDir * attackRange;//和武器射程相关

		//播放特效
		Vector3 effPosition = startPos + weaponLookDir * 0.6f; //特效位置向发射方向偏移少许

		Quaternion effRotation = Quaternion::FromTwoVectors(Vector3(0, 0, 1),
			Vector3(forwardDir.x(), forwardDir.y(), forwardDir.z())); //修正特效坐标位置
		//Quaternion fixedEffRotation = Quaternion(Vector3(0, 1, 0), 1.0f * SIMD_PI / 2.0f) * effRotation;

		Vector3 eulerAngles = effRotation.ToEuler();
		float effRotX = glm::radians(eulerAngles.x);
		float effRotY = glm::radians(eulerAngles.y);
		float effRotZ = glm::radians(eulerAngles.z);

		bulletHandle = EffekseerManager::GetInstance()->PlayEffect(ResourceManager::laserEffect, effPosition.x, effPosition.y, effPosition.z, effRotX, effRotY, effRotZ);
		if (Vector::Length(weaponLookDir) < SIMD_EPSILON) {
			return;
		}
		//射线检测：获取射线所有命中
		btCollisionWorld::AllHitsRayResultCallback allHitsCallback(btStartPos, btEndPos);
		if (switchMapCount == 4) {
			allHitsCallback.m_collisionFilterGroup = RAYCAST_GROUP;
			allHitsCallback.m_collisionFilterMask = TIGER_GROUP;
		}
		BulletWorldManager* btWorldManager = new BulletWorldManager();
		btDiscreteDynamicsWorld* world = btWorldManager->GetPhysicsWorld();
		world->rayTest(btStartPos, btEndPos, allHitsCallback);

		//射线可视化调试
		glm::vec3 start = glm::vec3(startPos.x, startPos.y, startPos.z);
		glm::vec3 end = glm::vec3(btEndPos.x(), btEndPos.y(), btEndPos.z());
		ResourceManager::AddDebugRay(start, end);

		//命中逻辑
		if (allHitsCallback.hasHit())
		{
			for (int i = 0; i < allHitsCallback.m_collisionObjects.size(); ++i) //处理每个命中物体
			{
				GameObject* hitObject = static_cast<GameObject*>(allHitsCallback.m_collisionObjects[i]->getUserPointer());
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

		//应用后坐力
		ApplyRecoil();

	}
}



