#pragma once
#include <random>
#include "bullet/btBulletDynamicsCommon.h"

#include "Ray.h"
#include "CollisionDetection.h"
#include "QuadTree.h"


namespace NCL {
	class Camera;
	using Maths::Ray;
	namespace CSC8503 {
		class GameObject;
		class Constraint;

		/*typedef std::function<void(GameObject*)> GameObjectFunc;
		typedef std::vector<GameObject*>::const_iterator GameObjectIterator;*/

		class GameWorld {
		public:
			GameWorld();
			~GameWorld();

			void Clear();
			void ClearAndErase();

			void AddGameObject(GameObject* o);
			void RemoveGameObject(GameObject* o, bool andDelete = false);


			PerspectiveCamera& GetMainCamera() {
				return mainCamera;
			}

			void ShuffleConstraints(bool state) {
				shuffleConstraints = state;
			}

			void ShuffleObjects(bool state) {
				shuffleObjects = state;
			}

			bool Raycast(Ray& r, RayCollision& closestCollision, bool closestObject = false, GameObject* ignore = nullptr) const;

			virtual void UpdateWorld(float dt);

			/*void OperateOnContents(GameObjectFunc f);

			void GetObjectIterators(
				GameObjectIterator& first,
				GameObjectIterator& last) const;*/

			int GetWorldStateID() const {
				return worldStateCounter;
			}

			// Bullet 物理对象管理
			void AddRigidBody(btRigidBody* body);
			void RemoveRigidBody(btRigidBody* body);

			// 提供 Bullet 物理世界访问接口
			static btDiscreteDynamicsWorld* GetPhysicsWorld() { return bulletWorld; }


			void ClearBulletConstraints();

			// 存储 Bullet 物理约束
			std::vector<btTypedConstraint*> bulletConstraints;

			// Bullet 物理引擎
			static btDefaultCollisionConfiguration* bulletConfig;
			static btCollisionDispatcher* bulletDispatcher;
			static btBroadphaseInterface* bulletBroadphase;
			static btSequentialImpulseConstraintSolver* bulletSolver;
			static btDiscreteDynamicsWorld* bulletWorld;

			std::vector<GameObject*> gameObjects;

		protected:

			std::vector<Constraint*> constraints;

			PerspectiveCamera mainCamera;

			bool shuffleConstraints;
			bool shuffleObjects;
			int		worldIDCounter;
			int		worldStateCounter;


		};
	}
}

