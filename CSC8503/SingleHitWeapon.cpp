#include "SingleHitWeapon.h"
#include "TutorialGame.h"
#include "GameManager.h"

using namespace OpenGL;

namespace NCL::CSC8503
{
	SingleHitWeapon::SingleHitWeapon()
	{
	}
	SingleHitWeapon::~SingleHitWeapon()
	{
	}

	void SingleHitWeapon::Initialize(int id, float damage, float range, float cd)
	{
		weaponID = id;
		basicDamage = damage;
		attackRange = range;
		attackCooldown = cd;
	}

	//攻击模式为：单次射线检测，不穿透目标，如手枪
	void SingleHitWeapon::Attack() 
	{
		// 检查当前游戏类型和武器所有权
		GameType currentGameType = GameManager::GetInstance()->GetCurrentGameType();

		// 针对网络游戏模式做额外检查
		if (currentGameType == GameType::NETWORKED) {
			// 获取当前游戏实例
			BaseGame* currentGame = GameManager::GetInstance()->GetCurrentGame();
			NetworkedGame* networkedGame = dynamic_cast<NetworkedGame*>(currentGame);

			if (networkedGame && !networkedGame->IsWeaponOwnedByLocalPlayer(this)) {
				// 如果武器不属于本地玩家，不执行攻击
				return;
			}
		}
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
		Vector3 effPosition = startPos;

		Quaternion effRotation = Quaternion::FromTwoVectors(Vector3(0, 0, 1), 
			Vector3(-forwardDir.z(), forwardDir.y(), forwardDir.x())); //修正特效坐标位置

		Vector3 eulerAngles = effRotation.ToEuler();
		float effRotX = glm::radians(eulerAngles.x);
		float effRotY = glm::radians(eulerAngles.y);
		float effRotZ = glm::radians(eulerAngles.z);

		bulletHandle = EffekseerManager::GetInstance()->PlayEffect(ResourceManager::pistolBulletEffect, effPosition.x, effPosition.y, effPosition.z, effRotX, effRotY, effRotZ);

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

			//油桶爆炸逻辑
			if (gameContext) {
				for (GameObject* barrel : barrels) {
					if (barrel == hitObject) {
						Vector3 barrelPos = barrel->GetTransform().GetPosition();
						gameContext->HandleBarrelExplode(barrelPos);
						break;
					}
				}
			}
		}

		//应用后坐力
		ApplyRecoil();

	}
}
