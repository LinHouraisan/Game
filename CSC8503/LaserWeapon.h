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

	class LaserWeapon : public Weapon
	{
	public:
		LaserWeapon();
		~LaserWeapon();

		void Initialize(int id, float damage, float range, float cd);
		void Attack() override;
	};
}


