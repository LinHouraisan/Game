#pragma once
#include "bullet/btBulletDynamicsCommon.h"
#include <vector>

namespace NCL {
    namespace CSC8503 {
        class BulletWorldManager {
        public:
            BulletWorldManager();
            ~BulletWorldManager();

            void AddRigidBody(btRigidBody* body);
            void RemoveRigidBody(btRigidBody* body);
            void ClearBulletConstraints();

            static btDiscreteDynamicsWorld* GetPhysicsWorld() { return bulletWorld; }

            struct CollisionPoint {
                btVector3 position;      // 碰撞点位置
                btVector3 normal;        // 碰撞法线
                float distance;          // 穿透深度
            };

            struct CollisionPair {
                const btCollisionObject* objectA;  // 碰撞物体A
                const btCollisionObject* objectB;  // 碰撞物体B
                std::vector<CollisionPoint> points; // 碰撞点列表
            };

            static std::vector<CollisionPair> GetCollisionDetails() {
                std::vector<CollisionPair> collisions;

                if (!bulletDispatcher) return collisions;

                int numManifolds = bulletDispatcher->getNumManifolds();
                for (int i = 0; i < numManifolds; i++) {
                    btPersistentManifold* contactManifold = bulletDispatcher->getManifoldByIndexInternal(i);

                    int numContacts = contactManifold->getNumContacts();
                    if (numContacts > 0) {
                        CollisionPair pair;
                        pair.objectA = contactManifold->getBody0();
                        pair.objectB = contactManifold->getBody1();

                        for (int j = 0; j < numContacts; j++) {
                            btManifoldPoint& pt = contactManifold->getContactPoint(j);

                            CollisionPoint point;
                            point.position = pt.getPositionWorldOnA();
                            point.normal = pt.m_normalWorldOnB;
                            point.distance = pt.getDistance();

                            pair.points.push_back(point);
                        }

                        collisions.push_back(pair);
                    }
                }

                return collisions;
            }

        private:
            static btDefaultCollisionConfiguration* bulletConfig;
            static btCollisionDispatcher* bulletDispatcher;
            static btBroadphaseInterface* bulletBroadphase;
            static btSequentialImpulseConstraintSolver* bulletSolver;
            static btDiscreteDynamicsWorld* bulletWorld;

            std::vector<btTypedConstraint*> bulletConstraints;
        };
    }
}