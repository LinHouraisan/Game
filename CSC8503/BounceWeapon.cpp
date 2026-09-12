#include "BounceWeapon.h"
#include "TutorialGame.h"

using namespace OpenGL;

namespace NCL::CSC8503
{
	BounceWeapon::BounceWeapon()
	{
	}
	BounceWeapon::~BounceWeapon()
	{
	}

	void BounceWeapon::Initialize(float damage, float range, float cd, int maxBounce, SceneManager* sceneMgr)
	{
		basicDamage = damage;
		attackRange = range;
		attackCooldown = cd;
		maxBounceCount = maxBounce;
		sceneManager = sceneMgr;
	}

	//获取下一个弹射的目标
	Monster* BounceWeapon::FindNextTarget(const std::vector<Monster*>& hitMonsters)
	{
		//获取所有怪物
		std::vector<Monster*> allMonsters = sceneManager->GetMonsters();

		//优先选择未被命中的怪物
		std::vector<Monster*> preferTargets;
		for (auto* monster : allMonsters) 
		{
			if (std::find(hitMonsters.begin(), hitMonsters.end(), monster) == hitMonsters.end())
			{
				preferTargets.push_back(monster);
			}
		}

		if (preferTargets.empty())
			return nullptr;

		//在偏好目标中随机选择一个
		int randomIndex = std::rand() % preferTargets.size();
		return preferTargets[randomIndex];
	}

	//攻击模式为：单次射线检测，命中后反弹给其他目标，直到反弹次数耗尽
	void BounceWeapon::Attack()
	{
		//更新冷却时间
		float currentTime = glfwGetTime();
		if (currentTime - lastAttackTime < attackCooldown)
		{
			return;
		}
		lastAttackTime = currentTime;

		//清空弹射计数和命中怪物列表
		bouncePath.clear();
		currentBounceCount = 0;
		hitMonsters.clear();

		if (Vector::Length(weaponLookDir) < SIMD_EPSILON) {
			return;
		}
		//计算子弹终点
		Vector3 startPos = this->GetPhysicsObject()->BTGetPosition();
		btVector3 btStartPos(startPos.x, startPos.y, startPos.z);
		btVector3 forwardDir = btVector3(weaponLookDir.x, weaponLookDir.y, weaponLookDir.z);
		btVector3 btEndPos = btStartPos + forwardDir * attackRange;//和武器射程相关
		//bouncePath.push_back(startPos);
		
		//获取射线调用环境
		BulletWorldManager* btWorldManager = new BulletWorldManager();
		btDiscreteDynamicsWorld* world = btWorldManager->GetPhysicsWorld();

		//播放第一次特效
		Vector3 effPosition = startPos + weaponLookDir * 0.35f;
		Quaternion effRotation = Quaternion::FromTwoVectors(Vector3(0, 0, 1),
			Vector3(forwardDir.x(), forwardDir.y(), forwardDir.z()));
		Vector3 eulerAngles = effRotation.ToEuler();
		float effRotX = glm::radians(eulerAngles.x);
		float effRotY = glm::radians(eulerAngles.y);
		float effRotZ = glm::radians(eulerAngles.z);
		bulletHandle = EffekseerManager::GetInstance()->PlayEffect(
			ResourceManager::bounceLaserEffect, 
			effPosition.x, effPosition.y + 0.4f, effPosition.z,  //修正特效坐标位置
			effRotX, effRotY, effRotZ);
		if (bulletHandle >= 0)
		{
			EffekseerManager::GetInstance()->SetEffectScale(bulletHandle, 1.0f, 1.0f, 2.0f);
		}
	

		//第一次攻击射线检测
		{
			btCollisionWorld::ClosestRayResultCallback rayCallback(btStartPos, btEndPos);
			if (switchMapCount == 4) {
				rayCallback.m_collisionFilterGroup = RAYCAST_GROUP;
				rayCallback.m_collisionFilterMask = TIGER_GROUP;
			}
			world->rayTest(btStartPos, btEndPos, rayCallback);

			//射线可视化调试
			glm::vec3 start = glm::vec3(startPos.x, startPos.y, startPos.z);
			glm::vec3 end = glm::vec3(btEndPos.x(), btEndPos.y(), btEndPos.z());
			ResourceManager::AddDebugRay(start, end);

			if (rayCallback.hasHit())
			{
				GameObject* hitObject = static_cast<GameObject*>(rayCallback.m_collisionObject->getUserPointer());
				Monster* hitMonster = dynamic_cast<Monster*>(hitObject);

				if (hitMonster)
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

					hitMonsters.push_back(hitMonster); //将被击中的怪物纳入列表
					bouncePath.push_back(hitMonster->GetPhysicsObject()->BTGetPosition()); //计入怪物位置
				}
			}
		}

		//设置 currentPos，如果初次命中存在，则从第一个命中位置开始弹射；否则从武器起点开始
		Vector3 currentPos = hitMonsters.empty() ? 
			startPos : 
			hitMonsters.back()->GetPhysicsObject()->BTGetPosition();

		//向指定目标弹射攻击，直到弹射次数耗尽
		if(hitMonsters.size() > 0)
		{
			while (currentBounceCount < maxBounceCount)
			{
				Monster* nextTarget = FindNextTarget(hitMonsters);//下一个攻击的目标
				if (!nextTarget)
					break; //没有找到下一个目标

				//从当前弹射位置到下一个目标的射线检测
				btVector3 btCurrentPos(currentPos.x, currentPos.y, currentPos.z);
				Vector3 targetPos = nextTarget->GetPhysicsObject()->BTGetPosition();
				btVector3 btTargetPos(targetPos.x, targetPos.y, targetPos.z);
				btCollisionWorld::ClosestRayResultCallback bounceCallback(btCurrentPos, btTargetPos);
				if (switchMapCount == 4) {
					bounceCallback.m_collisionFilterGroup = RAYCAST_GROUP;
					bounceCallback.m_collisionFilterMask = TIGER_GROUP;
				}
				world->rayTest(btCurrentPos, btTargetPos, bounceCallback);

				//射线可视化调试
				ResourceManager::AddDebugRay(
					glm::vec3(btCurrentPos.x(), btCurrentPos.y(), btCurrentPos.z()),
					glm::vec3(btTargetPos.x(), btTargetPos.y(), btTargetPos.z()));

				//处理命中逻辑
				if (bounceCallback.hasHit())
				{
					if (nextTarget->GetHealth() > basicDamage)
					{
						totalDamage += basicDamage;
						nextTarget->HandleHurt(basicDamage, btCurrentPos, btTargetPos);
					}
					else
					{
						totalDamage += nextTarget->GetHealth();
						nextTarget->HandleDeath();
					}

					//更新命中列表、弹射位置和弹射次数
					hitMonsters.push_back(nextTarget);
					bouncePath.push_back(nextTarget->GetPhysicsObject()->BTGetPosition());
					currentPos = nextTarget->GetPhysicsObject()->BTGetPosition();
					currentBounceCount++;
				}
				else
				{
					break; //如果射线没有命中目标，则停止弹射
				}
			}
		}


		//播放后续特效
		if (bouncePath.size() > 0)
		{
			for (int i = 0; i < bouncePath.size() - 1; ++i)
			{
				Vector3 effStart = bouncePath[i];
				Vector3 effEnd = bouncePath[i + 1];
				Vector3 effDir = Vector::Normalise(effEnd - effStart);

				float effLength = Vector::Length(effEnd - effStart); //该段特效长度

				//特效旋转
				Quaternion effRotation = Quaternion::FromTwoVectors(Vector3(0, 0, 1), effDir);
				Vector3 eulerAngles = effRotation.ToEuler();
				float effRotX = glm::radians(eulerAngles.x);
				float effRotY = glm::radians(eulerAngles.y);
				float effRotZ = glm::radians(eulerAngles.z);

				bulletHandle = EffekseerManager::GetInstance()->
					PlayEffect(ResourceManager::bounceLaserEffect, effStart.x, effStart.y, effStart.z, effRotX, effRotY, effRotZ);
				if (bulletHandle >= 0) 
				{
					//设置特效长度（配合effLength）
					EffekseerManager::GetInstance()->SetEffectScale(bulletHandle, 0.2f, 1.0f, effLength * 0.35f);
				}
			}
		}
		
		//应用后坐力
		//ApplyRecoil();
	}


}


