
#pragma once
#include "GameObject.h"
#include "PhysicsObject.h"
#include "PhysicsSystem.h"
#include "Transform.h"
#include "ResourceManager.h"


using namespace NCL::Maths;

namespace NCL::CSC8503
{
	class Weapon : public GameObject
	{
	public:
		Weapon();
		~Weapon();

		virtual void Attack() = 0;
		virtual void ApplyRecoil();//应用后坐力

		float GetBasicDamage() const { return basicDamage; }
		void SetBasicDamage(float dmg) { basicDamage = dmg; }

		float GetAttackRange() const { return attackRange; }
		void SetAttackRange(float range) { attackRange = range; }

		float GetAttackCooldown() const { return attackCooldown; }
		void SetAttackCooldown(float cd) { attackCooldown = cd; }

		Vector3 GetWeaponLookDir() const { return weaponLookDir; }
		void SetWeaponLookDir(Vector3 dir) { weaponLookDir = dir; }

		int GetWeaponID() const { return weaponID; }
		void SetWeaponID(int id) { weaponID = id; }

		float GetRecoilTimer() const { return recoilTimer; }
		void SetRecoilTimer(float timer) { recoilTimer = timer; }

		float GetRecoilRecoverySpeed() const { return recoilRecoverySpeed; }
		void SetRecoilRecoverySpeed(float recoilSpd) { recoilRecoverySpeed = recoilSpd; }

		float GetTotalDamage() const { return totalDamage; }
		//void SetBasicDamage(float dmg) { basicDamage = dmg; }

		bool GetActivateState() const { return isActivate; }
		void SetActivate(bool state) { isActivate = state; }

	protected:
		float basicDamage;//武器基础伤害
		float attackRange;//武器射程
		float attackCooldown;//武器两次攻击之间的间隔(s)
		float lastAttackTime = 0.0f;//上次攻击的时间
		Vector3 weaponLookDir;//武器指向
		Effekseer::Handle bulletHandle;//管理子弹特效
		int weaponID;//武器id

		float recoilStrength = 0.2f;// 后坐力强度
		float recoilRecoveryTime = 1.0f;// 恢复时间
		float recoilRecoverySpeed = 10.0f;// 恢复速度
		float recoilTimer = 0.0f;//恢复计时

		float totalDamage = 0.0f; //武器造成的总伤害
		bool isActivate;
	};
}
