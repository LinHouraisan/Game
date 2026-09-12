#pragma once
#include "Octree.h"
#include "ObjectPool.h"
#include "GameObject.h"
#include <unordered_map>
#include <vector>
#include "ResourceManager.h"
#include "Monster.h"
#include "GameWorld.h"
#include "BulletWorldManager.h"
#include "Weapon.h"
#include "SingleHitWeapon.h"
#include "WideHitWeapon.h"
#include "LaserWeapon.h"
#include "BounceWeapon.h"
#include "GravityWeapon.h"
#include "Player.h"


namespace NCL
{
	namespace CSC8503
	{
		class Monster;
		class SingleHitWeapon;
		class WideHitWeapon;
		class LaserWeapon;
		class BounceWeapon;
		class GravityWeapon;

		class Player;

		using GameObjectIterator = std::vector<GameObject*>::iterator;
		using GameObjectConstIterator = std::vector<GameObject*>::const_iterator;

		class SceneManager {
		public:
			SceneManager();
			~SceneManager();

			// 初始化场景的方法
			void InitScene(const Vector3& sceneSize, int maxDepth = 6, int maxSize = 5, size_t poolInitialSize = 10, size_t poolMaxSize = 100, BulletWorldManager* btWorldManager = nullptr);



			// 销毁场景的方法
			void DestroyScene();

			// 清空物理绑定
			void ClearAndErasePhysics();

			// 从对象池中请求一个对象
			GameObject* AcquirePooledObject();

			// 释放对象池对象
			void ReleasePooledObject(GameObject* object);

			// 为 GameObject 分配 ID 并添加到场景中
			GameObject* AddGameObjectToScene(GameObject* object);

			// 根据 GameObject 引用移除对象
			void RemoveObjectFromScene(GameObject* object);

			// 根据 GameObject ID 移除对象
			void RemoveObjectFromScene(int objectID);

			Monster* Enemy_TigerPreform(
				const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass,
				const float health,
				const float damage);

			Monster* Enemy_SpeedTigerPreform(
				const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass,
				const float health,
				const float damage);

			/*Monster* Enemy_TigerPreform(
				const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass,
				const float health,
				const float damage);*/

			Monster* Final_TigerPreform(
				const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass,
				const float health,
				const float damage);

			Monster* Enemy_EliteTigerPreform(
				const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass,
				const float health,
				const float damage);
			Monster* Enemy_BossMonsterPreform(
				const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass,
				const float health,
				const float damage
			);

			GameObject* Test_PlanePreform(
				const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass);

			Player* PlayerPreform(
				const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass);

			Player* NetworkedPlayerPreform(
				const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass);

			GameObject* EmptyPreform(const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass);

			GameObject* ArrowPreform(const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass);



			SingleHitWeapon* PistolPreform(const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass);

			WideHitWeapon* ShotGunPreform(const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass);

			LaserWeapon* LaserGunPreform(const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass);

			BounceWeapon* BounceGunPreform(const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass);

			GravityWeapon* GravityGunPreform(const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass);

			GameObject* mapArrowPreform(const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass, const Vector3& offset);

			GameObject* mapCoinPreform(const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass);

			GameObject* mapBarrelPreform(const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass);

			GameObject* waterPreform(const Vector3& position,
				const Quaternion& rotation,
				BulletShapeType shapeType,
				const Vector3& objectScale,
				const Vector3& rigidbodySize,
				const float mass);


			// 返回包含场景中的全部 GameObject 数组
			std::vector<GameObject*>& GetAllGameObjects();
			// 返回 RenderObject 数组
			const std::vector<RenderObject*>& GetRenderObjects() const;
			// 返回 RenderObject 数组
			const std::vector<PhysicsObject*>& GetPhysicsObjects() const;

			// 收集符合条件的 RenderObject
			void CollectRenderObjects(const Vector3& position, float radius);

			BulletWorldManager* GetBulletWorldManager() {
				return btWorldManager;
			}

			void GetObjectIterators(
				GameObjectConstIterator& first,
				GameObjectConstIterator& last) const;

			std::vector<Monster*> GetMonsters() {
				return monsters;
			}

			void AddToMonsters(Monster* monster) {
				monsters.push_back(monster);
			}

			void RemoveFromMonsters(Monster* monster) {
				auto it = std::find(monsters.begin(), monsters.end(), monster);
				if (it != monsters.end()) {
					monsters.erase(it);
				}
			}

			Player* GetPlayer() const { return player; } // 获取玩家对象指针

		protected:
			Octree<GameObject*>* sceneOctree; // 用于存储场景的八叉树
			ObjectPool<GameObject>* objectPool; // 用于存储场景对象的对象池
			std::unordered_map<int, GameObject*> gameObjectMap; // 用于存储场景中全部对象的字典
			std::vector<GameObject*> gameObjectsCache;
			std::vector<Monster*> monsters;
			int nextGameObjectID; // 下一个可用的 GameObject ID

			std::vector<RenderObject*> renderObjects; // 用于存储 GameObject 中的 RenderObject 对象
			std::vector<PhysicsObject*> physicsObjects; // 用于存储 GameObject 中的 RenderObject 对象


			// 为 GameObject 分配 ID
			int AllocateGameObjectID();
			BulletWorldManager* btWorldManager;

		private:
			// 从所有数据结构中移除 GameObject
			void RemoveGameObjectFromMap(GameObject* object);

			Player* player = nullptr; // 存储玩家的指针
		};
	}
}
