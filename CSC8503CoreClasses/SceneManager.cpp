#include "SceneManager.h"
#include "RenderObject.h"
#include "ResourceManager.h"
#include "Weapon.h"
#include "SingleHitWeapon.h"
#include "Monster.h"
#include "GameManager.h"

using namespace NCL;
using namespace CSC8503;
using namespace OpenGL;

//2025.3.10 Forcce rebase
SceneManager::SceneManager() : sceneOctree(nullptr), objectPool(nullptr), nextGameObjectID(0) {
}

SceneManager::~SceneManager() {
    // 销毁场景
    DestroyScene();
}

// 初始化场景的方法
void SceneManager::InitScene(const Vector3& sceneSize, int maxDepth, int maxSize, size_t poolInitialSize, size_t poolMaxSize, BulletWorldManager* btWorldManager) {
    // 如果已经存在八叉树，先删除它
    if (sceneOctree) {
        delete sceneOctree;
    }

    // 创建八叉树用于空间对象管理
    sceneOctree = new Octree<GameObject*>(sceneSize, maxDepth, maxSize);

    // 如果已经存在对象池，先删除它
    if (objectPool) {
        delete objectPool;
    }

    // 创建对象池用于C++对象管理
    objectPool = new ObjectPool<GameObject>(poolInitialSize, poolMaxSize);

    // 清空字典、数组和 ID 计数器
    gameObjectMap.clear();
    gameObjectsCache.clear();
    nextGameObjectID = 0;

    // 清空 RenderObject 数组
    renderObjects.clear();

    this->btWorldManager = btWorldManager;
}

// 销毁场景的方法
void SceneManager::DestroyScene() {
    ClearAndErasePhysics();

    // 如果八叉树存在，遍历八叉树中的所有对象
    if (sceneOctree) {
        sceneOctree->OperateOnContents([](std::list<OctreeEntry<GameObject*>>& entries) {
            for (auto& entry : entries) {
                // 直接销毁对象
                delete entry.object;
            }
        });

        // 删除八叉树
        delete sceneOctree;
        sceneOctree = nullptr;
    }

    // 删除对象池
    if (objectPool) {
        delete objectPool;
        objectPool = nullptr;
    }

    // 清空字典
    gameObjectMap.clear();

    // 清空缓存数组
    gameObjectsCache.clear();

    // 清空 RenderObject 数组
    renderObjects.clear();
}

void SceneManager::ClearAndErasePhysics() {
    btWorldManager->ClearBulletConstraints();

    for (auto& obj : gameObjectMap) {
        if (obj.second->GetPhysicsObject() && obj.second->GetPhysicsObject()->GetBulletBody()) {
            btRigidBody* body = obj.second->GetPhysicsObject()->GetBulletBody();
            if (body->getMotionState()) {
                delete body->getMotionState();
            }
            btWorldManager->RemoveRigidBody(body);
            delete body;
        }
        delete obj.second;
    }

    btWorldManager->GetPhysicsWorld()->clearForces();

    std::vector<btCollisionObject*> collisionObjects;
    for (int i = 0; i < btWorldManager->GetPhysicsWorld()->getNumCollisionObjects(); i++) {
        collisionObjects.push_back(btWorldManager->GetPhysicsWorld()->getCollisionObjectArray()[i]);
    }

    for (btCollisionObject* obj : collisionObjects) {
        btWorldManager->GetPhysicsWorld()->removeCollisionObject(obj);
        delete obj;
    }
}

// 从对象池中请求一个对象
GameObject* SceneManager::AcquirePooledObject() {
    if (objectPool) {
        return objectPool->Acquire();
    }
    return nullptr;
}

// 释放对象池对象
void SceneManager::ReleasePooledObject(GameObject* object) {
    if (objectPool && object) {
        objectPool->Release(object);
    }
}

// 为 GameObject 分配 ID
int SceneManager::AllocateGameObjectID() {
    return nextGameObjectID++;
}

// 为 GameObject 分配 ID 并添加到场景中
GameObject* SceneManager::AddGameObjectToScene(GameObject* object) {
    if (!object) {
        return nullptr;
    }
    // 分配 ID
    int id = AllocateGameObjectID();
    object->SetWorldID(id);
    gameObjectsCache.push_back(object);

    // 添加到字典
    gameObjectMap[id] = object;

    if (object->GetPhysicsObject()) {
        btWorldManager->AddRigidBody(object->GetPhysicsObject()->GetBulletBody());
        physicsObjects.push_back(object->GetPhysicsObject());
    }

    // 获取对象的位置和大小
    Vector3 position = object->GetTransform().GetPosition();
    Vector3 size = object->GetTransform().GetScale();

    // 将对象插入到八叉树中
    sceneOctree->Insert(object, position, size);

    // 如果对象有 RenderObject，将其添加到 RenderObject 数组中
    if (object->GetRenderObject()) {
        object->GetRenderObject()->SetWorldID(id);
        renderObjects.push_back(object->GetRenderObject());

    }
    return object;
}

// 根据 GameObject 引用移除对象
void SceneManager::RemoveObjectFromScene(GameObject* object) {
    if (!object) {
        return;
    }

    Monster* monster = dynamic_cast<Monster*>(object);
    if (monster) {

        // 从monsters数组中移除
        RemoveFromMonsters(monster);
    }

    // 从主容器 gameObjectsCache 中移除
    auto& gameObjects = GetAllGameObjects();
    auto it = std::find(gameObjects.begin(), gameObjects.end(), object);
    if (it != gameObjects.end()) {
        gameObjects.erase(it);
    }

    RemoveGameObjectFromMap(object);
}



// 从所有数据结构中移除 GameObject
void SceneManager::RemoveGameObjectFromMap(GameObject* object) {
    if (!object) {
        return;
    }


    int objectID = object->GetWorldID();

    // 从八叉树中移除对象
    Vector3 position = object->GetTransform().GetPosition();
    Vector3 size = object->GetTransform().GetScale();
    sceneOctree->Remove(object);

    // 从 RenderObject 数组中移除对象的 RenderObject
    RenderObject* renderObject = object->GetRenderObject();
    if (renderObject) {
        auto renderIt = std::find(renderObjects.begin(), renderObjects.end(), renderObject);
        if (renderIt != renderObjects.end()) {
            renderObjects.erase(renderIt);
        }
    }

    // 从物理世界中移除对象的物理对象
    if (object->GetPhysicsObject() && object->GetPhysicsObject()->GetBulletBody()) {
        btRigidBody* body = object->GetPhysicsObject()->GetBulletBody();
        if (body->getMotionState()) {
            delete body->getMotionState();
        }
        btWorldManager->RemoveRigidBody(body);
        delete body;
    }
   

    // 添加:从缓存数组中移除
    auto cacheIt = std::find(gameObjectsCache.begin(), gameObjectsCache.end(), object);
    if (cacheIt != gameObjectsCache.end()) {
        gameObjectsCache.erase(cacheIt);
    }

    // 删除对象
    delete object;

    // 从字典中移除对象
    gameObjectMap.erase(objectID);
}



// 返回 RenderObject 数组
const std::vector<RenderObject*>& SceneManager::GetRenderObjects() const {
    return renderObjects;
}

// 返回 PhysicsObject 数组
const std::vector<PhysicsObject*>& SceneManager::GetPhysicsObjects() const {
    return physicsObjects;
}


// 收集符合条件的 RenderObject
void SceneManager::CollectRenderObjects(const Vector3& position, float radius) {
    // TODO
}

Monster* SceneManager::Enemy_TigerPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass,
    const float health,
    const float damage)
{
    // 创建一个新的怪物实例
    Monster* tiger = new Monster("Monster", health, damage, position, this);

    // 设置物体位置、缩放和旋转
    tiger->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    // 添加渲染
    RenderObject* renderObject = new RenderObject(&tiger->GetTransform(), ResourceManager::animationModel_tiger, ResourceManager::animationController_tiger);
    renderObject->SetAnimationIndex(6);
    tiger->SetRenderObject(renderObject);

    // 创建物理对象
    PhysicsObject* physicsObj = new PhysicsObject(&tiger->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
	tiger->SetPhysicsObject(physicsObj);//把刚才创建的 PhysicsObject 绑定到老虎 Monster
	tiger->BindBulletToObject(); //绑定到 Bullet 物理引擎

    // 限制刚体在 Y 轴上的移动
    physicsObj->GetBulletBody()->setLinearFactor(btVector3(1, 0, 1)); // 允许 X 和 Z 轴移动，禁止 Y 轴移动

    // 限制刚体绕 X 和 Z 轴的旋转
    physicsObj->GetBulletBody()->setAngularFactor(btVector3(0, 1, 0)); // 只允许绕 Y 轴旋转

    // 将物体添加到场景中
    AddGameObjectToScene(tiger);
 
    return tiger;
}

Monster* SceneManager::Enemy_SpeedTigerPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass,
    const float health,
    const float damage)
{
    // 创建一个新的怪物实例
    Monster* tiger = new Monster("Monster", health, damage, position, this);

    // 设置物体位置、缩放和旋转
    tiger->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    // 添加渲染
    RenderObject* renderObject = new RenderObject(&tiger->GetTransform(), ResourceManager::animationModel_tiger, ResourceManager::animationController_tiger);
    renderObject->SetAnimationIndex(6);
    tiger->SetRenderObject(renderObject);
    tiger->GetRenderObject()->SetColorFactor(Vector4(3.0f, 0.0f, 5.0f, 0.3f));
    // 创建物理对象
    PhysicsObject* physicsObj = new PhysicsObject(&tiger->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    tiger->SetPhysicsObject(physicsObj);//把刚才创建的 PhysicsObject 绑定到老虎 Monster
    tiger->BindBulletToObject(); //绑定到 Bullet 物理引擎

    // 限制刚体在 Y 轴上的移动
    physicsObj->GetBulletBody()->setLinearFactor(btVector3(1, 0, 1)); // 允许 X 和 Z 轴移动，禁止 Y 轴移动

    // 限制刚体绕 X 和 Z 轴的旋转
    physicsObj->GetBulletBody()->setAngularFactor(btVector3(0, 1, 0)); // 只允许绕 Y 轴旋转

    // 将物体添加到场景中
    AddGameObjectToScene(tiger);

    return tiger;
}


Monster* SceneManager::Final_TigerPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass,
    const float health,
    const float damage)
{
    // 创建一个新的怪物实例
    Monster* tiger = new Monster("Monster", health, damage, position, this);

    // 设置物体位置、缩放和旋转
    tiger->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    // 添加渲染
    RenderObject* renderObject = new RenderObject(&tiger->GetTransform(), ResourceManager::animationModel_tiger, ResourceManager::animationController_tiger);
    renderObject->SetAnimationIndex(6);
    tiger->SetRenderObject(renderObject);

    // 创建物理对象
    PhysicsObject* physicsObj = new PhysicsObject(&tiger->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    tiger->SetPhysicsObject(physicsObj);//把刚才创建的 PhysicsObject 绑定到老虎 Monster
    tiger->BindBulletToObject(); //绑定到 Bullet 物理引擎

    // 限制刚体在 Y 轴上的移动
    physicsObj->GetBulletBody()->setLinearFactor(btVector3(1, 0, 1)); // 允许 X 和 Z 轴移动，禁止 Y 轴移动

    // 限制刚体绕 X 和 Z 轴的旋转
    physicsObj->GetBulletBody()->setAngularFactor(btVector3(0, 1, 0)); // 只允许绕 Y 轴旋转

    // 将物体添加到场景中
    AddGameObjectToScene(tiger);

    return tiger;
}


Monster* SceneManager::Enemy_EliteTigerPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass,
    const float health,
    const float damage)
{
    // 创建一个新的怪物实例
    Monster* tiger = new Monster("Monster", health, damage, position, this);

    // 设置物体位置、缩放和旋转
    tiger->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    // 添加渲染
    RenderObject* renderObject = new RenderObject(&tiger->GetTransform(), ResourceManager::animationModel_tiger, ResourceManager::animationController_tiger);
    renderObject->SetAnimationIndex(6);
    tiger->SetRenderObject(renderObject);
   tiger->GetRenderObject()->SetColorFactor(Vector4(0.0f, 10.0f, 0.0f, 1.0f));


    // 创建物理对象
    PhysicsObject* physicsObj = new PhysicsObject(&tiger->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    tiger->SetPhysicsObject(physicsObj);
    tiger->BindBulletToObject();

    // 限制刚体在 Y 轴上的移动
    physicsObj->GetBulletBody()->setLinearFactor(btVector3(1, 0, 1)); // 允许 X 和 Z 轴移动，禁止 Y 轴移动

    // 限制刚体绕 X 和 Z 轴的旋转
    physicsObj->GetBulletBody()->setAngularFactor(btVector3(0, 1, 0)); // 只允许绕 Y 轴旋转

    // 将物体添加到场景中
    AddGameObjectToScene(tiger);
    // std::cout << "EliteTiger monster generated successfully!" << std::endl;
    // std::cout << "Name: " << tiger->GetName() << std::endl;
    // std::cout << "Health: " << tiger->GetHealth() << std::endl;
    // std::cout << "Damage: " << tiger->GetDamage() << std::endl;

    return tiger;
}
Monster* SceneManager::Enemy_BossMonsterPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass,
    const float health,
    const float damage)
{
    // 创建一个新的怪物实例
    Monster* tiger = new Monster("Monster", health, damage, position, this);
	tiger->SetAsBoss(); // 设置为 Boss 怪物

    // 设置物体位置、缩放和旋转
    tiger->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    // 添加渲染
    RenderObject* renderObject = new RenderObject(&tiger->GetTransform(), ResourceManager::animationModel_tiger, ResourceManager::animationController_tiger);
    renderObject->SetAnimationIndex(6);
    tiger->SetRenderObject(renderObject);
    tiger->GetRenderObject()->SetColorFactor(Vector4(10.0f, 10.0f, 0.0f, 10.0f));


    // 创建物理对象
    PhysicsObject* physicsObj = new PhysicsObject(&tiger->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    tiger->SetPhysicsObject(physicsObj);
    tiger->BindBulletToObject();

    // 限制刚体在 Y 轴上的移动
    physicsObj->GetBulletBody()->setLinearFactor(btVector3(1, 0, 1)); // 允许 X 和 Z 轴移动，禁止 Y 轴移动

    // 限制刚体绕 X 和 Z 轴的旋转
    physicsObj->GetBulletBody()->setAngularFactor(btVector3(0, 1, 0)); // 只允许绕 Y 轴旋转

    // 将物体添加到场景中
    AddGameObjectToScene(tiger);
   /* std::cout << "BossTiger monster generated successfully!" << std::endl;
    std::cout << "Name: " << tiger->GetName() << std::endl;
    std::cout << "Health: " << tiger->GetHealth() << std::endl;
    std::cout << "Damage: " << tiger->GetDamage() << std::endl;*/

    return tiger;
}

GameObject* SceneManager::Test_PlanePreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass)
{
    GameObject* bulletObject = new GameObject();

    // 设置物体位置、缩放和旋转
    bulletObject->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    // 添加渲染
    RenderObject* renderObject = new RenderObject(&bulletObject->GetTransform(), ResourceManager::Model_plane);

    bulletObject->SetRenderObject(renderObject);

    // 创建物理对象
    PhysicsObject* physicsObj = new PhysicsObject(&bulletObject->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    bulletObject->SetPhysicsObject(physicsObj);
    bulletObject->BindBulletToObject();

    // 将物体添加到场景中
    AddGameObjectToScene(bulletObject);

    return bulletObject;
}

Player* SceneManager::PlayerPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass)
{
    Player* bulletObject = new Player();

    // 设置物体位置、缩放和旋转
    bulletObject->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    // 添加渲染
    RenderObject* renderObject = new RenderObject(&bulletObject->GetTransform(), ResourceManager::Model_player);

    bulletObject->SetRenderObject(renderObject);

    // 创建物理对象
    PhysicsObject* physicsObj = new PhysicsObject(&bulletObject->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    bulletObject->SetPhysicsObject(physicsObj);
    bulletObject->BindBulletToObject();

    // 限制刚体在 Y 轴上的移动
    physicsObj->GetBulletBody()->setLinearFactor(btVector3(1, 0, 1)); // 允许 X 和 Z 轴移动，禁止 Y 轴移动

    // 限制刚体绕 X 和 Z 轴的旋转
    physicsObj->GetBulletBody()->setAngularFactor(btVector3(0, 1, 0)); // 只允许绕 Y 轴旋转

    // 将物体添加到场景中
    AddGameObjectToScene(bulletObject);

    player = bulletObject; // 保存玩家对象

    return bulletObject;
}

Player* SceneManager::NetworkedPlayerPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass)
{
    // 创建一个特定的远程网络玩家对象（使用不同的模型和颜色区分）
    Player* networkPlayer = new Player();

    // 设置物体位置、缩放和旋转
    networkPlayer->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    // 添加渲染
    RenderObject* renderObject = new RenderObject(&networkPlayer->GetTransform(), ResourceManager::Model_netPlayer);
    // renderObject->SetColorFactor(Vector4(1.0f, 0.5f, 0.5f, 1.0f)); // 远程玩家显示为红色
    networkPlayer->SetRenderObject(renderObject);

    // 创建物理对象
    PhysicsObject* physicsObj = new PhysicsObject(
        &networkPlayer->GetTransform(),
        shapeType,
        rigidbodySize,
        mass,
        btWorldManager
    );
    networkPlayer->SetPhysicsObject(physicsObj);
    networkPlayer->BindBulletToObject();

    // 限制刚体在 Y 轴上的移动
    physicsObj->GetBulletBody()->setLinearFactor(btVector3(1, 0, 1));
    physicsObj->GetBulletBody()->setAngularFactor(btVector3(0, 1, 0));

    AddGameObjectToScene(networkPlayer);
    return networkPlayer;
}

GameObject* SceneManager::EmptyPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass)
{
    GameObject* bulletObject = new GameObject();

    // 设置物体位置、缩放和旋转
    bulletObject->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    // 创建物理对象
    PhysicsObject* physicsObj = new PhysicsObject(&bulletObject->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    bulletObject->SetPhysicsObject(physicsObj);
    bulletObject->BindBulletToObject();

    // 将物体添加到场景中
    AddGameObjectToScene(bulletObject);

    return bulletObject;
}

GameObject* SceneManager::ArrowPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass)
{
    GameObject* bulletObject = new GameObject();

    // 设置物体位置、缩放和旋转
    bulletObject->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    // 添加渲染
    RenderObject* renderObject = new RenderObject(&bulletObject->GetTransform(), ResourceManager::Model_arrow);
    bulletObject->SetRenderObject(renderObject);

  
	// 创建物理对象

	PhysicsObject* physicsObj = new PhysicsObject(&bulletObject->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
	bulletObject->SetPhysicsObject(physicsObj);
	bulletObject->BindBulletToObject();
	physicsObj->SetKinematic(true);
   
    // 将物体添加到场景中
    AddGameObjectToScene(bulletObject);

    return bulletObject;
}

// for WideHitWeapon only (shotGun)
WideHitWeapon* SceneManager::ShotGunPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass)
{
    WideHitWeapon* bulletObject = new WideHitWeapon();

    bulletObject->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    //这里测试模型
    RenderObject* renderObject = new RenderObject(&bulletObject->GetTransform(), ResourceManager::Model_gun1);
    bulletObject->SetRenderObject(renderObject);

    PhysicsObject* physicsObj = new PhysicsObject(&bulletObject->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    bulletObject->SetPhysicsObject(physicsObj);
    bulletObject->BindBulletToObject();
    physicsObj->SetKinematic(true);

    AddGameObjectToScene(bulletObject);

    return bulletObject;
}

// for LaserWeapon only (laserGun)
LaserWeapon* SceneManager::LaserGunPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass)
{
    LaserWeapon* bulletObject = new LaserWeapon();

    bulletObject->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    //这里测试模型
    RenderObject* renderObject = new RenderObject(&bulletObject->GetTransform(), ResourceManager::Model_gun2);
    bulletObject->SetRenderObject(renderObject);

    PhysicsObject* physicsObj = new PhysicsObject(&bulletObject->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    bulletObject->SetPhysicsObject(physicsObj);
    bulletObject->BindBulletToObject();
    physicsObj->SetKinematic(true);

    AddGameObjectToScene(bulletObject);

    return bulletObject;
}

// for SingleHitWeapon only (pistol)
SingleHitWeapon* SceneManager::PistolPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass)
{
    SingleHitWeapon* bulletObject = new SingleHitWeapon();

    bulletObject->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    //这里测试模型
    RenderObject* renderObject = new RenderObject(&bulletObject->GetTransform(), ResourceManager::Model_gun3);
    bulletObject->SetRenderObject(renderObject);

    PhysicsObject* physicsObj = new PhysicsObject(&bulletObject->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    bulletObject->SetPhysicsObject(physicsObj);
    bulletObject->BindBulletToObject();
    physicsObj->SetKinematic(true);

    AddGameObjectToScene(bulletObject);

    return bulletObject;
}

// for BounceWeapon only (bounceGun)
BounceWeapon* SceneManager::BounceGunPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass)
{
    BounceWeapon* bulletObject = new BounceWeapon();

    bulletObject->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    //这里测试模型
    RenderObject* renderObject = new RenderObject(&bulletObject->GetTransform(), ResourceManager::Model_gun4);
    bulletObject->SetRenderObject(renderObject);



    PhysicsObject* physicsObj = new PhysicsObject(&bulletObject->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    bulletObject->SetPhysicsObject(physicsObj);
    bulletObject->BindBulletToObject();
    physicsObj->SetKinematic(true);

    AddGameObjectToScene(bulletObject);

    return bulletObject;
}

// forGravityWeapon only (gravityGun)
GravityWeapon* SceneManager::GravityGunPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass)
{
    GravityWeapon* bulletObject = new GravityWeapon();

    bulletObject->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    //这里测试模型
    RenderObject* renderObject = new RenderObject(&bulletObject->GetTransform(), ResourceManager::Model_gun5);
    bulletObject->SetRenderObject(renderObject);



    PhysicsObject* physicsObj = new PhysicsObject(&bulletObject->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    bulletObject->SetPhysicsObject(physicsObj);
    bulletObject->BindBulletToObject();
    physicsObj->SetKinematic(true);

    AddGameObjectToScene(bulletObject);

    return bulletObject;
}


GameObject* SceneManager::mapArrowPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass, const Vector3& offset)
{
    GameObject* bulletObject = new GameObject();

    // 设置物体位置、缩放和旋转
    bulletObject->GetTransform()
        .SetPosition(position + offset)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    // 添加渲染
    RenderObject* renderObject = new RenderObject(&bulletObject->GetTransform(), ResourceManager::Model_switchMapArrow);
    bulletObject->SetRenderObject(renderObject);

    // 创建物理对象
    PhysicsObject* physicsObj = new PhysicsObject(&bulletObject->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    bulletObject->SetPhysicsObject(physicsObj);
    bulletObject->BindBulletToObject();
    physicsObj->SetKinematic(true);

    // 将物体添加到场景中
    AddGameObjectToScene(bulletObject);

    return bulletObject;
}

GameObject* SceneManager::mapCoinPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass)
{
    GameObject* bulletObject = new GameObject();

    // 设置物体位置、缩放和旋转
    bulletObject->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    // 添加渲染
    RenderObject* renderObject = new RenderObject(&bulletObject->GetTransform(), ResourceManager::Model_coin);
    bulletObject->SetRenderObject(renderObject);

    // 创建物理对象
    PhysicsObject* physicsObj = new PhysicsObject(&bulletObject->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    bulletObject->SetPhysicsObject(physicsObj);
    bulletObject->BindBulletToObject();
    physicsObj->SetKinematic(true);

    // 将物体添加到场景中
    AddGameObjectToScene(bulletObject);

    return bulletObject;
}

GameObject* SceneManager::mapBarrelPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass)
{
    GameObject* bulletObject = new GameObject();

    // 设置物体位置、缩放和旋转
    bulletObject->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    // 添加渲染
    RenderObject* renderObject = new RenderObject(&bulletObject->GetTransform(), ResourceManager::Model_barrel);
    bulletObject->SetRenderObject(renderObject);

    // 创建物理对象
    PhysicsObject* physicsObj = new PhysicsObject(&bulletObject->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    bulletObject->SetPhysicsObject(physicsObj);
    bulletObject->BindBulletToObject();
    physicsObj->SetKinematic(true);

    // 将物体添加到场景中
    AddGameObjectToScene(bulletObject);

    return bulletObject;
}

GameObject* SceneManager::waterPreform(const Vector3& position,
    const Quaternion& rotation,
    BulletShapeType shapeType,
    const Vector3& objectScale,
    const Vector3& rigidbodySize,
    const float mass)
{
    GameObject* bulletObject = new GameObject();

    // 设置物体位置、缩放和旋转
    bulletObject->GetTransform()
        .SetPosition(position)
        .SetScale(objectScale)
        .SetOrientation(rotation);

    // 添加渲染
    RenderObject* renderObject = new RenderObject(&bulletObject->GetTransform(), ResourceManager::Model_water);
    bulletObject->SetRenderObject(renderObject);

    // 创建物理对象
    PhysicsObject* physicsObj = new PhysicsObject(&bulletObject->GetTransform(), shapeType, rigidbodySize, mass, btWorldManager);
    bulletObject->SetPhysicsObject(physicsObj);
    bulletObject->BindBulletToObject();
    physicsObj->SetKinematic(true);

    // 将物体添加到场景中
    AddGameObjectToScene(bulletObject);

    return bulletObject;
}

std::vector<GameObject*>& SceneManager::GetAllGameObjects(){
    return gameObjectsCache;
}

void SceneManager::GetObjectIterators(
    GameObjectConstIterator& first,
    GameObjectConstIterator& last) const {
    first = gameObjectsCache.begin();
    last = gameObjectsCache.end();
}
