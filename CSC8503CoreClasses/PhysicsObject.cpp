#include "PhysicsObject.h"
#include "PhysicsSystem.h"
#include "Transform.h"
using namespace NCL;
using namespace CSC8503;

PhysicsObject::PhysicsObject(Transform* parentTransform, BulletShapeType shapeType, const Vector3& shapeSize, float mass, BulletWorldManager* btWorldManager)
    : shapeSize(shapeSize) { // 直接存储 shapeSize
    transform = parentTransform;

    switch (shapeType) {
    case BulletShapeType::Box:
        bulletShape = new btBoxShape(btVector3(shapeSize.x * 0.5f, shapeSize.y * 0.5f, shapeSize.z * 0.5f));
        break;
    case BulletShapeType::Sphere:
        bulletShape = new btSphereShape(shapeSize.x * 0.5f);
        break;
    case BulletShapeType::Capsule:
        bulletShape = new btCapsuleShape(shapeSize.x * 0.5f, shapeSize.y);
        break;
    case BulletShapeType::Cylinder:
        bulletShape = new btCylinderShape(btVector3(shapeSize.x * 0.5f, shapeSize.y * 0.5f, shapeSize.z * 0.5f));
        break;
    default:
        bulletShape = new btBoxShape(btVector3(shapeSize.x * 0.5f, shapeSize.y * 0.5f, shapeSize.z * 0.5f));
        break;
    }

    btTransform bulletTransform;
    bulletTransform.setIdentity();
    bulletTransform.setOrigin(btVector3(transform->GetPosition().x, transform->GetPosition().y, transform->GetPosition().z));
    bulletTransform.setRotation(btQuaternion(transform->GetOrientation().x, transform->GetOrientation().y, transform->GetOrientation().z, transform->GetOrientation().w));

    btVector3 localInertia(0, 0, 0);
    if (mass > 0.0f) {
        bulletShape->calculateLocalInertia(mass, localInertia);
    }

    btDefaultMotionState* motionState = new btDefaultMotionState(bulletTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, bulletShape, localInertia);
    bulletBody = new btRigidBody(rbInfo);
    //btWorldManager->GetPhysicsWorld()->addRigidBody(bulletBody);//改
    btWorldManager = btWorldManager;
}
PhysicsObject::~PhysicsObject() {
    //delete bulletBody->getMotionState();
    //delete bulletBody->getCollisionShape();
    //delete bulletBody;

}

void PhysicsObject::BTSetRotation(const Quaternion& newRotation) {
    transform->SetOrientation(newRotation);
    btQuaternion btRot(newRotation.x, newRotation.y, newRotation.z, newRotation.w);
    btTransform bulletTransform;
    bulletTransform = bulletBody->getWorldTransform();
    bulletTransform.setRotation(btRot);
    btDefaultMotionState* motionState = static_cast<btDefaultMotionState*>(bulletBody->getMotionState());
    if (motionState) {
        motionState->setWorldTransform(bulletTransform);
    }
    bulletBody->setWorldTransform(bulletTransform);
    bulletBody->activate();
}

Quaternion PhysicsObject::BTGetRotation() const {
    btTransform bulletTransform;
    bulletBody->getMotionState()->getWorldTransform(bulletTransform);
    btQuaternion btRot = bulletTransform.getRotation();

    return Quaternion(btRot.getX(), btRot.getY(), btRot.getZ(), btRot.getW());
}

void PhysicsObject::BTSetPosition(const Vector3& newPosition) {
    transform->SetPosition(newPosition);
    btTransform bulletTransform = bulletBody->getWorldTransform();
    bulletTransform.setOrigin(btVector3(newPosition.x, newPosition.y, newPosition.z));
    btDefaultMotionState* motionState = static_cast<btDefaultMotionState*>(bulletBody->getMotionState());
    if (motionState) {
        motionState->setWorldTransform(bulletTransform);
    }
    bulletBody->setWorldTransform(bulletTransform);
    bulletBody->activate();
}

Vector3 PhysicsObject::BTGetPosition() const {
    btTransform bulletTransform;
    bulletBody->getMotionState()->getWorldTransform(bulletTransform);
    btVector3 btPos = bulletTransform.getOrigin();
    return Vector3(btPos.getX(), btPos.getY(), btPos.getZ());
}


void PhysicsObject::ApplyAngularImpulse(const Vector3& force) {
    bulletBody->applyTorqueImpulse(btVector3(force.x, force.y, force.z));
}

void PhysicsObject::ApplyLinearImpulse(const Vector3& force) {
    bulletBody->applyCentralImpulse(btVector3(force.x, force.y, force.z));
}

void PhysicsObject::AddForce(const Vector3& addedForce) {
    bulletBody->applyCentralForce(btVector3(addedForce.x, addedForce.y, addedForce.z));
}

void PhysicsObject::AddForceAtPosition(const Vector3& addedForce, const Vector3& position) {
    btVector3 relativePos = btVector3(position.x, position.y, position.z) - bulletBody->getCenterOfMassPosition();
    bulletBody->applyForce(btVector3(addedForce.x, addedForce.y, addedForce.z), relativePos);
}

void PhysicsObject::AddTorque(const Vector3& addedTorque) {
    bulletBody->applyTorque(btVector3(addedTorque.x, addedTorque.y, addedTorque.z));
}

void PhysicsObject::ClearForces() {
    bulletBody->clearForces();
}


Vector3 PhysicsObject::GetForce() const {
    btVector3 btForce = bulletBody->getTotalForce();
    return Vector3(btForce.getX(), btForce.getY(), btForce.getZ());
}

Vector3 PhysicsObject::GetTorque() const {
    btVector3 btTorque = bulletBody->getTotalTorque();
    return Vector3(btTorque.getX(), btTorque.getY(), btTorque.getZ());
}


float PhysicsObject::GetInverseMass() const {
    return bulletBody->getInvMass();
}


btRigidBody* PhysicsObject::GetBulletBody() {
    return bulletBody;
}


void PhysicsObject::AddFixedConstraint(PhysicsObject* other) {
    btRigidBody* bodyA = this->GetBulletBody();
    btRigidBody* bodyB = other->GetBulletBody();

    btTransform frameInA, frameInB;
    frameInA.setIdentity();
    frameInB.setIdentity();

    btFixedConstraint* fixedConstraint = new btFixedConstraint(*bodyA, *bodyB, frameInA, frameInB);
    // GameWorld::bulletWorld->addConstraint(fixedConstraint);
    btWorldManager->GetPhysicsWorld()->addConstraint(fixedConstraint);
}

void PhysicsObject::AddHingeConstraint(PhysicsObject* other, const Vector3& pivotA, const Vector3& pivotB, const Vector3& axis) {
    btRigidBody* bodyA = this->GetBulletBody();
    btRigidBody* bodyB = other->GetBulletBody();

    btVector3 btPivotA(pivotA.x, pivotA.y, pivotA.z);
    btVector3 btPivotB(pivotB.x, pivotB.y, pivotB.z);
    btVector3 btAxis(axis.x, axis.y, axis.z);

    btHingeConstraint* hingeConstraint = new btHingeConstraint(*bodyA, *bodyB, btPivotA, btPivotB, btAxis, btAxis);
    // GameWorld::bulletWorld->addConstraint(hingeConstraint);
    btWorldManager->GetPhysicsWorld()->addConstraint(hingeConstraint);
}

void PhysicsObject::AddSliderConstraint(PhysicsObject* other, const Matrix4& frameInA, const Matrix4& frameInB) {
    btRigidBody* bodyA = this->GetBulletBody();
    btRigidBody* bodyB = other->GetBulletBody();

    btTransform btFrameA, btFrameB;
    btFrameA.setFromOpenGLMatrix(frameInA.GetData());
    btFrameB.setFromOpenGLMatrix(frameInB.GetData());

    btSliderConstraint* sliderConstraint = new btSliderConstraint(*bodyA, *bodyB, btFrameA, btFrameB, true);

    // GameWorld::bulletWorld->addConstraint(sliderConstraint);
    btWorldManager->GetPhysicsWorld()->addConstraint(sliderConstraint);
}


void PhysicsObject::AddEachFacingConstraint(PhysicsObject* other, const Vector3& otherForward) {
    btRigidBody* bodyA = this->GetBulletBody();
    btRigidBody* bodyB = other->GetBulletBody();

    if (!bodyA || !bodyB) return;

    btTransform frameInA, frameInB;
    frameInA.setIdentity();
    frameInB.setIdentity();

    btVector3 dir = bodyA->getCenterOfMassPosition() - bodyB->getCenterOfMassPosition();
    if (dir.length() < SIMD_EPSILON) return;

    dir.normalize();

    btVector3 defaultForward(otherForward.x, otherForward.y, otherForward.z);
    if (defaultForward.length() < 1e-6) return;

    defaultForward.normalize();

    btVector3 rotationAxis = defaultForward.cross(dir);
    if (rotationAxis.length() < 1e-6) rotationAxis = btVector3(0, 1, 0);  // 避免奇异性

    float rotationAngle = defaultForward.angle(dir);
    btQuaternion targetRotation(rotationAxis, rotationAngle);

    frameInA.setRotation(btQuaternion::getIdentity()); // 保持 A 的默认旋转
    frameInB.setRotation(targetRotation); // 让 B 旋转到 A 的朝向

    btGeneric6DofConstraint* facingConstraint = new btGeneric6DofConstraint(*bodyA, *bodyB, frameInA, frameInB, false);

    facingConstraint->setLinearLowerLimit(btVector3(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT));
    facingConstraint->setLinearUpperLimit(btVector3(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT));

    facingConstraint->setAngularLowerLimit(btVector3(0, 0, 0));
    facingConstraint->setAngularUpperLimit(btVector3(0, 0, 0));

    // GameWorld::bulletWorld->addConstraint(facingConstraint);
    btWorldManager->GetPhysicsWorld()->addConstraint(facingConstraint);
}

void PhysicsObject::ApplyFacingConstraint(PhysicsObject* other, const Vector3& otherForward)
{
    btRigidBody* bodyA = this->GetBulletBody();
    btRigidBody* bodyB = other->GetBulletBody();
    other->SetSleepingEnabled(false);

    // 1. 计算从 other 到 this 的目标朝向
    btVector3 posA = bodyA->getCenterOfMassPosition();
    btVector3 posB = bodyB->getCenterOfMassPosition();
    btVector3 targetDir = posA - posB;
    if (targetDir.length2() < 1e-6f)
        return;
    targetDir.normalize();

    // 2. 计算 other 的当前前向量
    btTransform transB = bodyB->getWorldTransform();
    btVector3 currentForward = transB.getBasis() * btVector3(otherForward.x, otherForward.y, otherForward.z);
    if (currentForward.length2() < 1e-6f)
        return;
    currentForward.normalize();

    // 3. 算出旋转轴和夹角
    btVector3 rotationAxis = currentForward.cross(targetDir);
    float sinLength = rotationAxis.length();
    if (sinLength < 1e-6f)
        return;
    rotationAxis.normalize();

    // 使用 dot 来快速求夹角（防止一些情况下 cross+angle() 可能出现数值不稳定）
    float dotVal = std::clamp(currentForward.dot(targetDir), -1.0f, 1.0f);
    float angle = acos(dotVal);

    // 4. PD 控制求力矩
    //    Kp 越大，转得越“迅速”；Kd 越大，阻尼越强，越不易抖动，但过大则可能转得慢。
    float Kp = 3.5f;
    float Kd = 1.5f;

    // 取刚体当前的角速度
    btVector3 angVel = bodyB->getAngularVelocity();

    // 只关心绕 rotationAxis 方向的角速度分量
    float angVelAroundAxis = angVel.dot(rotationAxis);

    // PD 控制公式： torque = Kp * angle - Kd * angVelAroundAxis
    float torqueMag = Kp * angle - Kd * angVelAroundAxis;
    btVector3 torque = rotationAxis * torqueMag;

    // 5. 应用力矩
    bodyB->applyTorque(torque);
}



void PhysicsObject::AddDistanceConstraint(PhysicsObject* other, float fixedDistance) {
    btRigidBody* bodyA = this->GetBulletBody();
    btRigidBody* bodyB = other->GetBulletBody();

    if (!bodyA || !bodyB) return;

    btVector3 posA = bodyA->getCenterOfMassPosition();
    btVector3 posB = bodyB->getCenterOfMassPosition();

    btVector3 dir = posB - posA;
    if (dir.length() < 1e-6) return; // 避免奇异情况

    dir.normalize();

    btVector3 anchorA = posA + dir * (fixedDistance * 0.5f);
    btVector3 anchorB = posB - dir * (fixedDistance * 0.5f);

    btVector3 localAnchorA = bodyA->getCenterOfMassTransform().inverse() * anchorA;
    btVector3 localAnchorB = bodyB->getCenterOfMassTransform().inverse() * anchorB;

    btPoint2PointConstraint* distanceConstraint = new btPoint2PointConstraint(*bodyA, *bodyB, localAnchorA, localAnchorB);

    // GameWorld::bulletWorld->addConstraint(distanceConstraint);
    btWorldManager->GetPhysicsWorld()->addConstraint(distanceConstraint);
}



void PhysicsObject::SetKinematic(bool isKinematic) {
    if (isKinematic) {
        bulletBody->setCollisionFlags(bulletBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        bulletBody->setActivationState(DISABLE_DEACTIVATION);
    }
    else {
        bulletBody->setCollisionFlags(bulletBody->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
        bulletBody->setActivationState(ACTIVE_TAG);
    }
}


void PhysicsObject::SetSleepingEnabled(bool canSleep) {
    if (canSleep) {
        bulletBody->forceActivationState(ACTIVE_TAG);
        bulletBody->setSleepingThresholds(0.8f, 1.0f);
    }
    else {
        bulletBody->setActivationState(DISABLE_DEACTIVATION);
    }
}


bool PhysicsObject::IsColliding(const PhysicsObject* other) const {
    // btDispatcher* dispatcher = GameWorld::bulletWorld->getDispatcher();
    btDispatcher* dispatcher = btWorldManager->GetPhysicsWorld()->getDispatcher();
    int numManifolds = dispatcher->getNumManifolds();

    for (int i = 0; i < numManifolds; i++) {
        btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);
        const btCollisionObject* objA = contactManifold->getBody0();
        const btCollisionObject* objB = contactManifold->getBody1();

        if ((objA == bulletBody && objB == other->bulletBody) ||
            (objB == bulletBody && objA == other->bulletBody)) {
            return contactManifold->getNumContacts() > 0;
        }
    }
    return false;
}


std::vector<Vector3> PhysicsObject::GetContactPoints() const {
    std::vector<Vector3> contactPoints;
    // btDispatcher* dispatcher = GameWorld::bulletWorld->getDispatcher();
    btDispatcher* dispatcher = btWorldManager->GetPhysicsWorld()->getDispatcher();
    int numManifolds = dispatcher->getNumManifolds();

    for (int i = 0; i < numManifolds; i++) {
        btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);
        const btCollisionObject* objA = contactManifold->getBody0();
        const btCollisionObject* objB = contactManifold->getBody1();

        if (objA == bulletBody || objB == bulletBody) {
            int numContacts = contactManifold->getNumContacts();
            for (int j = 0; j < numContacts; j++) {
                btManifoldPoint& point = contactManifold->getContactPoint(j);
                btVector3 pos = point.getPositionWorldOnB();
                contactPoints.emplace_back(Vector3(pos.getX(), pos.getY(), pos.getZ()));
            }
        }
    }
    return contactPoints;
}


void PhysicsObject::RemoveAllConstraintsBetween(PhysicsObject* other) {
    if (!other) return;

    btRigidBody* bodyA = this->GetBulletBody();
    btRigidBody* bodyB = other->GetBulletBody();

    // btDiscreteDynamicsWorld* world = GameWorld::bulletWorld;
    btDiscreteDynamicsWorld* world = btWorldManager->GetPhysicsWorld();
    int numConstraints = world->getNumConstraints();

    for (int i = numConstraints - 1; i >= 0; --i) {
        btTypedConstraint* constraint = world->getConstraint(i);

        if ((&constraint->getRigidBodyA() == bodyA && &constraint->getRigidBodyB() == bodyB) ||
            (&constraint->getRigidBodyA() == bodyB && &constraint->getRigidBodyB() == bodyA)) {

            world->removeConstraint(constraint);
            delete constraint;
        }
    }
}


void PhysicsObject::AddPlaneConstraint(const std::string& planeType) {
    btRigidBody* body = this->GetBulletBody();

    btTransform frameA;
    frameA.setIdentity();

    btGeneric6DofConstraint* planeConstraint = new btGeneric6DofConstraint(*body, frameA, true);

    if (planeType == "XZ") {
        planeConstraint->setLinearLowerLimit(btVector3(-FLT_MAX, 0, -FLT_MAX)); // XZ 平面内可移动
        planeConstraint->setLinearUpperLimit(btVector3(FLT_MAX, 0, FLT_MAX));
    }
    else if (planeType == "XY") {
        planeConstraint->setLinearLowerLimit(btVector3(-FLT_MAX, -FLT_MAX, 0)); // XY 平面内可移动
        planeConstraint->setLinearUpperLimit(btVector3(FLT_MAX, FLT_MAX, 0));
    }
    else if (planeType == "ZY") {
        planeConstraint->setLinearLowerLimit(btVector3(0, -FLT_MAX, -FLT_MAX)); // ZY 平面内可移动
        planeConstraint->setLinearUpperLimit(btVector3(0, FLT_MAX, FLT_MAX));
    }
    else {
        delete planeConstraint;
        return;
    }

    planeConstraint->setAngularLowerLimit(btVector3(0, 0, 0));
    planeConstraint->setAngularUpperLimit(btVector3(0, 0, 0));


    // GameWorld::bulletWorld->addConstraint(planeConstraint);
    btWorldManager->GetPhysicsWorld()->addConstraint(planeConstraint);
}


void PhysicsObject::RemoveSelfConstraints() {
    btRigidBody* body = this->GetBulletBody();
    // btDiscreteDynamicsWorld* world = GameWorld::bulletWorld;
    btDiscreteDynamicsWorld* world = btWorldManager->GetPhysicsWorld();

    int numConstraints = world->getNumConstraints();

    for (int i = numConstraints - 1; i >= 0; --i) {
        btTypedConstraint* constraint = world->getConstraint(i);

        if (&constraint->getRigidBodyA() == body || &constraint->getRigidBodyB() == body) {
            world->removeConstraint(constraint);
            delete constraint; // 释放内存
        }
    }
}

void PhysicsObject::EnableCCD() {
    if (!bulletBody) return;

    float minDimension = std::min({ shapeSize.x, shapeSize.y, shapeSize.z });

    float motionThreshold = 0.2f * minDimension; // CCD 启动阈值
    float sweptRadius = 0.1f * minDimension;     // CCD 球体半径

    bulletBody->setCcdMotionThreshold(motionThreshold);
    bulletBody->setCcdSweptSphereRadius(sweptRadius);
}

void PhysicsObject::DisableCCD() {
    if (!bulletBody) return;

    bulletBody->setCcdMotionThreshold(0.0f);
    bulletBody->setCcdSweptSphereRadius(0.0f);
}

std::vector<PhysicsObject*> PhysicsObject::GetObjectsInRadius(const Vector3& position, float radius) {
    std::vector<PhysicsObject*> results;
    btVector3 btPosition(position.x, position.y, position.z);
    btSphereShape sphere(radius);

    btCollisionWorld::ClosestConvexResultCallback callback(btPosition, btPosition);
    // GameWorld::bulletWorld->convexSweepTest(&sphere, btTransform(btQuaternion::getIdentity(), btPosition), btTransform(btQuaternion::getIdentity(), btPosition), callback);
    btWorldManager->GetPhysicsWorld()->convexSweepTest(&sphere, btTransform(btQuaternion::getIdentity(), btPosition), btTransform(btQuaternion::getIdentity(), btPosition), callback);

    if (callback.hasHit()) {
        results.push_back(static_cast<PhysicsObject*>(callback.m_hitCollisionObject->getUserPointer()));
    }
    return results;
}

void PhysicsObject::ApplyKnockback(const Vector3& force, float duration) {
    bulletBody->applyCentralImpulse(btVector3(force.x, force.y, force.z));
    bulletBody->activate();
    SetSleepingEnabled(false);
}

void PhysicsObject::SetCollisionFilter(int group, int mask) {
    // GameWorld::bulletWorld->removeRigidBody(bulletBody);
    btWorldManager->GetPhysicsWorld()->removeRigidBody(bulletBody);
    bulletBody->getBroadphaseHandle()->m_collisionFilterGroup = group;
    bulletBody->getBroadphaseHandle()->m_collisionFilterMask = mask;
    // GameWorld::bulletWorld->addRigidBody(bulletBody, group, mask);
    btWorldManager->GetPhysicsWorld()->addRigidBody(bulletBody, group, mask);
}