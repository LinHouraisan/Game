#pragma once
using namespace NCL::Maths;
#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "BulletWorldManager.h"
//#include "GameObject.h"
#include "Transform.h"

namespace NCL {
	class CollisionVolume;


	namespace CSC8503 {
		//class Transform;

		enum class BulletShapeType {
			Box,        // 立方体
			Sphere,     // 球体
			Capsule,    // 胶囊体
			Cylinder   // 圆柱体
		};

		class PhysicsObject {
		public:
			/**
			 * @brief 构造一个物理对象，并在 Bullet 物理引擎中创建对应的刚体。
			 * @param parentTransform 绑定的 Transform 组件
			 * @param shapeType 碰撞体形状类型
			 * @param shapeSize 碰撞体大小
			 * @param mass 质量（0 表示静态物体）
			 */
			PhysicsObject(Transform* parentTransform, BulletShapeType shapeType, const Vector3& shapeSize, float mass, BulletWorldManager* btWorldManager);
			
			/**
			 * @brief 析构函数，释放 Bullet 物理对象。
			 */
			~PhysicsObject();


			/**
			 * @brief 获取刚体的线性速度。
			 */
			Vector3 GetLinearVelocity() const {
				btVector3 velocity = bulletBody->getLinearVelocity();
				return Vector3(velocity.getX(), velocity.getY(), velocity.getZ());
			}

			/**
			 * @brief 设置刚体的线性速度。
			 */
			void SetLinearVelocity(const Vector3& v) {
				bulletBody->setLinearVelocity(btVector3(v.x, v.y, v.z));
			}

			/**
			 * @brief 获取刚体的角速度。
			 */
			Vector3 GetAngularVelocity() const {
				btVector3 angVelocity = bulletBody->getAngularVelocity();
				return Vector3(angVelocity.getX(), angVelocity.getY(), angVelocity.getZ());
			}

			/**
			 * @brief 设置刚体的角速度。
			 */
			void SetAngularVelocity(const Vector3& v) {
				bulletBody->setAngularVelocity(btVector3(v.x, v.y, v.z));
			}

			/**
			 * @brief 获取当前作用的力。
			 */
			Vector3 GetForce() const;

			/**
			 * @brief 获取当前作用的扭矩。
			 */
			Vector3 GetTorque() const;

			/**
			 * @brief 获取刚体的逆质量（质量的倒数）。
			 */
			float GetInverseMass() const;

			/**
			 * @brief 施加一个角动量冲量。
			 * @param force 角动量冲量向量
			 */
			void ApplyAngularImpulse(const Vector3& force);

			/**
			 * @brief 施加一个线性动量冲量。
			 * @param force 线性动量冲量向量
			 */
			void ApplyLinearImpulse(const Vector3& force);

			/**
			 * @brief 施加一个力到刚体中心。
			 * @param force 作用力
			 */
			void AddForce(const Vector3& force);

			/**
			 * @brief 在指定位置对刚体施加一个力。
			 * @param force 作用力
			 * @param position 作用点（相对物体）
			 */
			void AddForceAtPosition(const Vector3& force, const Vector3& position);

			/**
			 * @brief 施加一个扭矩到刚体。
			 * @param torque 作用扭矩
			 */
			void AddTorque(const Vector3& torque);

			/**
			 * @brief 清除当前所有作用在刚体上的力。
			 */
			void ClearForces();

			/**
			 * @brief 获取 Bullet 刚体对象。
			 */
			btRigidBody* GetBulletBody();


			/**
			 * @brief 设置刚体的旋转。
			 * @param newRotation 目标旋转值
			 */
			void BTSetRotation(const Quaternion& newRotation);

			/**
			 * @brief 获取刚体的旋转。
			 */
			Quaternion BTGetRotation() const;

			/**
			 * @brief 设置刚体的位置。
			 * @param newPosition 目标位置
			 */
			void BTSetPosition(const Vector3& newPosition);

			/**
			 * @brief 获取刚体的位置。
			 */
			Vector3 BTGetPosition() const;

			/**
			 * @brief 添加吸附固定约束，使两个刚体保持固定吸附位置。
			 * @param other 另一个物理对象
			 */
			void AddFixedConstraint(PhysicsObject* other);

			/**
			 * @brief 添加铰链约束，使刚体绕固定轴旋转，适用于门、机械臂等。
			 * @param other 另一个物理对象
			 * @param pivotA 当前刚体上的铰链位置
			 * @param pivotB 另一个刚体上的铰链位置
			 * @param axis 旋转轴
			 */
			void AddHingeConstraint(PhysicsObject* other, const Vector3& pivotA, const Vector3& pivotB, const Vector3& axis);

			/**
			 * @brief 添加滑动约束，使刚体沿一个轴滑动，适用于滑轨、活塞等。
			 * @param other 另一个物理对象
			 * @param frameInA 当前刚体的初始变换
			 * @param frameInB 另一个刚体的初始变换
			 */
			void AddSliderConstraint(PhysicsObject* other, const Matrix4& frameInA, const Matrix4& frameInB);

			/**
			 * @brief 添加双向面向约束
			 * @param other 另一个物理对象
			 * @param otherForward other 物体的默认前向方向（单位向量）
			 */
			void AddEachFacingConstraint(PhysicsObject* other, const Vector3& otherForward);

			/**
			 * @brief 添加单向面向约束
			 * @param other 另一个物理对象
			 * @param otherForward other 物体的默认前向方向（单位向量）
			 */
			void ApplyFacingConstraint(PhysicsObject* other, const Vector3& otherForward);

			/**
			 * @brief 添加距离约束，使 other 物体跟当前物体保持设定距离
			 * @param other 另一个物理对象
			 */
			void AddDistanceConstraint(PhysicsObject* other, float fixedDistance);


			/**
			 * @brief 将刚体设置为运动学模式，使其不受物理引擎控制，但可以手动设置位置和旋转。
			 * 运动学刚体通常用于角色控制器或动画驱动的物体。
			 * @param isKinematic 是否设置为运动学模式
			 */
			void SetKinematic(bool isKinematic);


			/**
			 * @brief 允许或禁止刚体进入休眠模式，以减少物理计算。
			 * 如果启用休眠，则静止的物体不会被模拟，直到受到外力影响。
			 * @param canSleep 是否允许刚体进入休眠状态
			 */
			void SetSleepingEnabled(bool canSleep);


			/**
			 * @brief 检测当前物理对象是否与另一个物理对象发生碰撞。
			 * @param other 另一个物理对象
			 * @return 如果发生碰撞返回 true，否则返回 false
			 */
			bool IsColliding(const PhysicsObject* other) const;

			/**
			 * @brief 获取当前物理对象的所有接触点。
			 * @return 包含接触点位置的 Vector3 列表
			 */
			std::vector<Vector3> GetContactPoints() const;


			/**
			 * @brief 移除调用者与目标对象之间的所有约束
			 * @param other 另一个物理对象
			 */
			void RemoveAllConstraintsBetween(PhysicsObject* other);


			/**
			 * @brief 仅作用于调用者自身，使其被约束在指定平面上
			 * @param planeType 约束的平面（选择："XZ", "XY", "ZY"）
			 */
			void AddPlaneConstraint(const std::string& planeType);

			/**
			 * @brief 移除当前物体的所有约束
			 */
			void RemoveSelfConstraints();

			/**
			 * @brief 启用CCD连续碰撞检测(注意，性能有开销，非必要不启用)
			 */
			void EnableCCD(); 
			// 取消 CCD
			void DisableCCD(); 

			/**
			 * @brief 范围碰撞检测，检测指定地点的半径范围内的所有碰撞对象（爆炸或者AOE可用）
			 * @param position 期望检测地点
			 * @param radius 检测地点半径
			 */
			std::vector<PhysicsObject*> GetObjectsInRadius(const Vector3& position, float radius);

			/**
			 * @brief 可控的冲量（可用于击退，短暂拉近）
			 * @param force 冲量向量
			 * @param duration 冲量作用时间
			 */
			void ApplyKnockback(const Vector3& force, float duration);

			/**
			 * @brief bullet碰撞过滤（可以过滤指定对象的碰撞）
			 * 作用：子弹只与敌人碰撞，不与玩家碰撞，或者让敌人不会互相推挤，但仍能碰撞玩家
				物体属于哪个组（Group）。
				例如：玩家属于 Group 1，敌人属于 Group 2，子弹属于 Group 3。
				物体可以与哪些组发生碰撞（Mask）。
				例如：子弹(Group 3) 只能碰撞敌人(Group 2)，则：
				子弹的 mask 设为 2（仅与 Group 2 敌人碰撞）。
				敌人的 mask 设为 3（允许 Group 3 子弹碰撞）。
			 */
			void SetCollisionFilter(int group, int mask);

			/**
			* 添加阻尼
			*/
			void SetAngularDamping(float damping) {
				bulletBody->setDamping(0.0f, damping); // 第二个参数为角阻尼
			}

			Transform* GetTransform() const {
				return transform;
			}

			btCollisionShape* GetBulletShape() const {
				return bulletShape;
			}

			Vector3 GetShapeSize() const {
				return shapeSize;
			}

			

		protected:
			const CollisionVolume* volume;
			Transform* transform;
			float inverseMass;
			float elasticity;
			float friction;
			Vector3 force;
			Vector3 torque;
			Matrix3 inverseInteriaTensor;
			btRigidBody* bulletBody;
			btCollisionShape* bulletShape;
			Vector3 shapeSize;
			BulletWorldManager* btWorldManager;
		};
	}
}
