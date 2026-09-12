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
	class GravityField;
	class SceneManager;

	class GravityWeapon : public Weapon
	{
	public:
		GravityWeapon();
		~GravityWeapon();

		void Initialize(int id, float range, float cd, float radius, float duration, float force, SceneManager* sceneMgr);
		void Attack() override;

		//GravityField* GetGravityField() const { return field; }
		void ApplyGravityField(float dt);

		bool GetGravityState() const { return isGravityActive; }
		void SetGravityState(bool state) { isGravityActive = state; }

	private:
		float gravityRadius;   // 引力场半径
		float gravityDuration; // 引力场持续时间
		float gravityForce;    // 引力

		Vector3 gravityCenter;        // 吸引中心
		bool isGravityActive = false; // 是否正在生效
		float elapsedTime = 0.0f;     // 计时

		SceneManager* sceneManager = nullptr;
	};
}



