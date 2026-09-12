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
#include "Effeksser.h"

namespace NCL::CSC8503 {
	class Weapon;

	class WideHitWeapon : public Weapon
	{
	public:
		WideHitWeapon();
		~WideHitWeapon();

		void Initialize(int id, float damage, float range, float cd, int pelletNum, float spreadAng);
		void Attack() override;

	private:
		int pelletCount = 8;   //霰弹枪一次攻击发射的子弹数
		float spreadAngle = 15.0f; //子弹散射角度（每条射线随机偏移）
	};
}



