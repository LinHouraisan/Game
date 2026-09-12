#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "CollisionDetection.h"
#include "Quaternion.h"

#include "Constraint.h"

#include "Debug.h"
#include "Window.h"
#include <functional>
using namespace NCL;
using namespace CSC8503;

PhysicsSystem::PhysicsSystem(BulletWorldManager* btWorldManager) {
	applyGravity = false;
	useBroadPhase = false;
	dTOffset = 0.0f;
	globalDamping = 0.995f;
	this->btWorldManager = btWorldManager;
	SetGravity(Vector3(0.0f, -9.8f, 0.0f));
}

PhysicsSystem::~PhysicsSystem() {

}

void PhysicsSystem::SetGravity(const Vector3& g) {
	gravity = g;
}


void PhysicsSystem::Clear() {
	allCollisions.clear();
}


bool useSimpleContainer = false;

int constraintIterationCount = 10;

const int   idealHZ = 120;
const float idealDT = 1.0f / idealHZ;


int realHZ = idealHZ;
float realDT = idealDT;


void PhysicsSystem::Update(float dt) {

	btWorldManager->GetPhysicsWorld()->stepSimulation(dt, 10);

	int numObjects = btWorldManager->GetPhysicsWorld()->getNumCollisionObjects();

	for (int i = 0; i < numObjects; i++) {
		btCollisionObject* obj = btWorldManager->GetPhysicsWorld()->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);

		void* userPointer = body->getUserPointer();
		GameObject* gameObj = static_cast<GameObject*>(userPointer);
		if (gameObj) {
			btTransform bulletTransform;
			body->getMotionState()->getWorldTransform(bulletTransform);
			btVector3 pos = bulletTransform.getOrigin();
			btQuaternion rot = bulletTransform.getRotation();

			gameObj->GetTransform().SetPosition(Vector3(pos.x(), pos.y(), pos.z()));
			gameObj->GetTransform().SetOrientation(Quaternion(rot.x(), rot.y(), rot.z(), rot.w()));

		}
	}
}

void PhysicsSystem::AddRigidBody(btRigidBody* body) {
	btWorldManager->GetPhysicsWorld()->addRigidBody(body);
}

void PhysicsSystem::UpdateCollisionList() {
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = allCollisions.begin(); i != allCollisions.end(); ) {
		if ((*i).framesLeft == numCollisionFrames) {
			i->a->OnCollisionBegin(i->b);
			i->b->OnCollisionBegin(i->a);
		}

		CollisionDetection::CollisionInfo& in = const_cast<CollisionDetection::CollisionInfo&>(*i);
		in.framesLeft--;

		if ((*i).framesLeft < 0) {
			i->a->OnCollisionEnd(i->b);
			i->b->OnCollisionEnd(i->a);
			i = allCollisions.erase(i);
		}
		else {
			++i;
		}
	}
}

void PhysicsSystem::UpdateObjectAABBs() {
	/*gameWorld.OperateOnContents(
		[](GameObject* g) {
			g->UpdateBroadphaseAABB();
		}
	);*/
}

void PhysicsSystem::BasicCollisionDetection() {
}
void PhysicsSystem::ImpulseResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {

}
void PhysicsSystem::BroadPhase() {

}
void PhysicsSystem::NarrowPhase() {

}
void PhysicsSystem::IntegrateAccel(float dt) {

}

void PhysicsSystem::IntegrateVelocity(float dt) {

}

// 弃用
void PhysicsSystem::ClearForces() {

}

// 弃用
void PhysicsSystem::UpdateConstraints(float dt) {
}