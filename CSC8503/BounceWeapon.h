#pragma once
#include "Weapon.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "RenderObject.h"
#include "Quaternion.h"
#include "Matrix.h"
#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "BulletWorldManager.h"
#include "Monster.h"
//#include "SceneManager.h"
#include "Effeksser.h"

namespace NCL::CSC8503 {
	class Weapon;
	class Monster;
	class SceneManager;

	class BounceWeapon : public Weapon
	{
	public:
		BounceWeapon();
		~BounceWeapon();

		void Initialize(float damage, float range, float cd, int maxBounce, SceneManager* sceneMgr);
		void Attack() override;

	private:
		int maxBounceCount;
		int currentBounceCount = 0;
		SceneManager* sceneManager = nullptr;

		std::vector<Vector3> bouncePath;//弹射路径
		std::vector<Monster*> hitMonsters;

		Monster* FindNextTarget(const std::vector<Monster*>& hitMonsters);
	};
}


