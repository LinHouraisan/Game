//#include "GameWorld.h"
//#include "GameObject.h"
//#include "Constraint.h"
//#include "CollisionDetection.h"
//#include "Camera.h"
//#include "PhysicsObject.h"
//
//using namespace NCL;
//using namespace NCL::CSC8503;
//
////btDefaultCollisionConfiguration* GameWorld::bulletConfig = nullptr;
////btCollisionDispatcher* GameWorld::bulletDispatcher = nullptr;
////btBroadphaseInterface* GameWorld::bulletBroadphase = nullptr;
////btSequentialImpulseConstraintSolver* GameWorld::bulletSolver = nullptr;
////btDiscreteDynamicsWorld* GameWorld::bulletWorld = nullptr;
//
//GameWorld::GameWorld() {
//	shuffleConstraints = false;
//	shuffleObjects = false;
//	worldIDCounter = 0;
//	worldStateCounter = 0;
//
//	/*if (!bulletWorld) {
//		bulletConfig = new btDefaultCollisionConfiguration();
//		bulletDispatcher = new btCollisionDispatcher(bulletConfig);
//		bulletBroadphase = new btDbvtBroadphase();
//		bulletSolver = new btSequentialImpulseConstraintSolver();
//
//		bulletWorld = new btDiscreteDynamicsWorld(bulletDispatcher, bulletBroadphase, bulletSolver, bulletConfig);
//		bulletWorld->setGravity(btVector3(0, 0, 0));
//
//
//	}*/
//}
//
//GameWorld::~GameWorld() {
//}
//
//void GameWorld::Clear() {
//	gameObjects.clear();
//	constraints.clear();
//	worldIDCounter = 0;
//	worldStateCounter = 0;
//
//	if (bulletWorld) {
//		delete bulletWorld;
//		delete bulletSolver;
//		delete bulletBroadphase;
//		delete bulletDispatcher;
//		delete bulletConfig;
//
//		bulletWorld = nullptr;
//	}
//}
//
//
//void GameWorld::ClearAndErase() {
//
//	ClearBulletConstraints();
//
//	for (auto& obj : gameObjects) {
//		if (obj->GetPhysicsObject() && obj->GetPhysicsObject()->GetBulletBody()) {
//			btRigidBody* body = obj->GetPhysicsObject()->GetBulletBody();
//			if (body->getMotionState()) {
//				delete body->getMotionState();
//			}
//			bulletWorld->removeRigidBody(body);
//			delete body;
//		}
//		delete obj;
//	}
//	gameObjects.clear();
//
//	bulletWorld->clearForces();
//
//	std::vector<btCollisionObject*> collisionObjects;
//	for (int i = 0; i < bulletWorld->getNumCollisionObjects(); i++) {
//		collisionObjects.push_back(bulletWorld->getCollisionObjectArray()[i]);
//	}
//
//	for (btCollisionObject* obj : collisionObjects) {
//		bulletWorld->removeCollisionObject(obj);
//		delete obj;
//	}
//}
//
//
//void GameWorld::AddGameObject(GameObject* o) {
//	gameObjects.emplace_back(o);
//	o->SetWorldID(worldIDCounter++);
//	worldStateCounter++;
//
//	if (o->GetPhysicsObject() && o->GetPhysicsObject()->GetBulletBody()) {
//		AddRigidBody(o->GetPhysicsObject()->GetBulletBody());
//	}
//}
//
//void GameWorld::RemoveGameObject(GameObject* o, bool andDelete) {
//	gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), o), gameObjects.end());
//
//	if (o->GetPhysicsObject() && o->GetPhysicsObject()->GetBulletBody()) {
//		RemoveRigidBody(o->GetPhysicsObject()->GetBulletBody());
//	}
//
//	if (andDelete) {
//		delete o;
//	}
//	worldStateCounter++;
//}
//
//void GameWorld::AddRigidBody(btRigidBody* body) {
//	bulletWorld->addRigidBody(body);
//}
//
//// 在删除单个刚体前遍历其所持约束并先行删除
//void GameWorld::RemoveRigidBody(btRigidBody* body) {
//	std::vector<btTypedConstraint*> constraintsToRemove;
//
//	int numConstraints = bulletWorld->getNumConstraints();
//	for (int i = numConstraints - 1; i >= 0; --i) {
//		btTypedConstraint* constraint = bulletWorld->getConstraint(i);
//		if (&(constraint->getRigidBodyA()) == body || &(constraint->getRigidBodyB()) == body) {
//			constraintsToRemove.push_back(constraint);
//		}
//	}
//
//	for (btTypedConstraint* constraint : constraintsToRemove) {
//		bulletWorld->removeConstraint(constraint);
//		delete constraint;
//	}
//
//	bulletConstraints.erase(
//		std::remove_if(bulletConstraints.begin(), bulletConstraints.end(),
//			[&](btTypedConstraint* c) {
//				return &(c->getRigidBodyA()) == body || &(c->getRigidBodyB()) == body;
//			}),
//		bulletConstraints.end()
//	);
//
//	bulletWorld->removeRigidBody(body);
//	delete body;
//}
//
//
//void GameWorld::GetObjectIterators(
//	GameObjectIterator& first,
//	GameObjectIterator& last) const {
//
//	first = gameObjects.begin();
//	last = gameObjects.end();
//}
//
//void GameWorld::OperateOnContents(GameObjectFunc f) {
//	for (GameObject* g : gameObjects) {
//		f(g);
//	}
//}
//
//void GameWorld::UpdateWorld(float dt) {
//	auto rng = std::default_random_engine{};
//
//	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
//	std::default_random_engine e(seed);
//
//	if (shuffleObjects) {
//		std::shuffle(gameObjects.begin(), gameObjects.end(), e);
//	}
//
//	if (shuffleConstraints) {
//		std::shuffle(constraints.begin(), constraints.end(), e);
//	}
//}
//
//bool GameWorld::Raycast(Ray& r, RayCollision& closestCollision, bool closestObject, GameObject* ignoreThis) const {
//	RayCollision collision;
//
//	for (auto& i : gameObjects) {
//		if (!i->GetBoundingVolume()) { 
//			continue;
//		}
//		if (i == ignoreThis) {
//			continue;
//		}
//		RayCollision thisCollision;
//		if (CollisionDetection::RayIntersection(r, *i, thisCollision)) {
//
//			if (!closestObject) {
//				closestCollision = collision;
//				closestCollision.node = i;
//				return true;
//			}
//			else {
//				if (thisCollision.rayDistance < collision.rayDistance) {
//					thisCollision.node = i;
//					collision = thisCollision;
//				}
//			}
//		}
//	}
//	if (collision.node) {
//		closestCollision = collision;
//		closestCollision.node = collision.node;
//		return true;
//	}
//	return false;
//}
//
////// 失效
////void GameWorld::AddConstraint(Constraint* c) {
////	constraints.emplace_back(c);
////}
////
////// 失效
////void GameWorld::RemoveConstraint(Constraint* c, bool andDelete) {
////	constraints.erase(std::remove(constraints.begin(), constraints.end(), c), constraints.end());
////	if (andDelete) {
////		delete c;
////	}
////}
////
////// 失效
////void GameWorld::GetConstraintIterators(
////	std::vector<Constraint*>::const_iterator& first,
////	std::vector<Constraint*>::const_iterator& last) const {
////	first	= constraints.begin();
////	last	= constraints.end();
////}
//
////// 失效
////void GameWorld::AddBulletConstraint(btTypedConstraint* constraint) {
////	if (!constraint) return;
////	bulletWorld->addConstraint(constraint);
////	bulletConstraints.emplace_back(constraint);
////}
////
////// 失效
////void GameWorld::RemoveBulletConstraint(btTypedConstraint* constraint, bool andDelete) {
////	if (!constraint) return;
////
////	auto it = std::find(bulletConstraints.begin(), bulletConstraints.end(), constraint);
////	if (it != bulletConstraints.end()) {
////		bulletWorld->removeConstraint(constraint);
////		if (andDelete) {
////			delete constraint;
////		}
////		bulletConstraints.erase(it);
////	}
////}
//
//void GameWorld::ClearBulletConstraints() {
//	int numConstraints = bulletWorld->getNumConstraints();
//
//	std::vector<btTypedConstraint*> constraintsToRemove;
//
//	for (int i = numConstraints - 1; i >= 0; --i) {
//		btTypedConstraint* constraint = bulletWorld->getConstraint(i);
//		constraintsToRemove.push_back(constraint);
//	}
//
//	for (btTypedConstraint* constraint : constraintsToRemove) {
//		bulletWorld->removeConstraint(constraint);
//		delete constraint;
//	}
//
//	bulletConstraints.clear();
//}
