#include "BulletWorldManager.h"

using namespace NCL;
using namespace CSC8503;

btDefaultCollisionConfiguration* BulletWorldManager::bulletConfig = nullptr;
btCollisionDispatcher* BulletWorldManager::bulletDispatcher = nullptr;
btBroadphaseInterface* BulletWorldManager::bulletBroadphase = nullptr;
btSequentialImpulseConstraintSolver* BulletWorldManager::bulletSolver = nullptr;
btDiscreteDynamicsWorld* BulletWorldManager::bulletWorld = nullptr;

BulletWorldManager::BulletWorldManager() {
    if (!bulletWorld) {
        bulletConfig = new btDefaultCollisionConfiguration();
        bulletDispatcher = new btCollisionDispatcher(bulletConfig);
        bulletBroadphase = new btDbvtBroadphase();
        bulletSolver = new btSequentialImpulseConstraintSolver();

        bulletWorld = new btDiscreteDynamicsWorld(bulletDispatcher, bulletBroadphase, bulletSolver, bulletConfig);
        bulletWorld->setGravity(btVector3(0, 0, 0));
    }
}

BulletWorldManager::~BulletWorldManager() {
    delete bulletWorld;
    delete bulletSolver;
    delete bulletBroadphase;
    delete bulletDispatcher;
    delete bulletConfig;
}

void BulletWorldManager::AddRigidBody(btRigidBody* body) {
    bulletWorld->addRigidBody(body);
}

void BulletWorldManager::RemoveRigidBody(btRigidBody* body) {
    bulletWorld->removeRigidBody(body);
}

void BulletWorldManager::ClearBulletConstraints() {
    int numConstraints = bulletWorld->getNumConstraints();
    std::vector<btTypedConstraint*> constraintsToRemove;

    for (int i = numConstraints - 1; i >= 0; --i) {
        btTypedConstraint* constraint = bulletWorld->getConstraint(i);
        constraintsToRemove.push_back(constraint);
    }

    for (btTypedConstraint* constraint : constraintsToRemove) {
        bulletWorld->removeConstraint(constraint);
        delete constraint;
    }

    // bulletConstraints.clear();
}

