#include "BaseGame.h"

using namespace NCL;
using namespace CSC8503;
using namespace OpenGL;

BaseGame::BaseGame(GLFWwindow* window, OpenGL::GameTechRenderer* renderer, OpenGL::ResourceManager* resourceManager) {
    this->window = window;

    inSelectionMode = false;

    // 初始化物理系统
    btWorldManager = new BulletWorldManager();
    physics = new PhysicsSystem(btWorldManager);

    this->resourceManager = resourceManager;

    // 初始化场景管理器
    sceneManager = new SceneManager();
    sceneManager->InitScene(Vector3(100, 100, 100), 6, 5, 100, 100, btWorldManager);

    // 初始化渲染准备器
    RenderPrepare::GetInstance().Initialize(sceneManager);
    renderPrepare = &RenderPrepare::GetInstance();

    // 保存渲染器引用
    this->render = renderer;

    // 获取特效管理器
    effekseerManager = EffekseerManager::GetInstance();

    // 初始化游戏状态
    gameTime = 0.0f;
    dt = 0.0f;
    useGravity = true;
    playerMode = false;
    player = nullptr;
}

BaseGame::~BaseGame() {
    // 清理资源 - 避免内存泄漏
    delete physics;
    delete btWorldManager;
    // 注意：resourceManager、renderPrepare 和 effekseerManager 是单例或外部传入，不需要在此释放
}

// 类型转换函数
glm::vec3 BaseGame::ToGLMVec3(const NCL::Maths::Vector3& vec) {
    return glm::vec3(vec.x, vec.y, vec.z);
}

bool BaseGame::isKeyJustPressed(int key) {
    int state = glfwGetKey(window, key);
    if (state == GLFW_PRESS && !keyStates[key]) {
        keyStates[key] = true;
        return true;
    }
    if (state == GLFW_RELEASE) {
        keyStates[key] = false;
    }
    return false;
}

// 检测鼠标按键是否被按下
bool BaseGame::isMousePressed(int button) {
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

// 限制鼠标位置在窗口内
void BaseGame::ClampMousePosition() {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // 限制范围
    double minX = 0.0, maxX = SCR_WIDTH;
    double minY = 0.0, maxY = SCR_HEIGHT;

    bool clamped = false;

    if (xpos < minX) { xpos = minX; clamped = true; }
    if (xpos > maxX) { xpos = maxX; clamped = true; }
    if (ypos < minY) { ypos = minY; clamped = true; }
    if (ypos > maxY) { ypos = maxY; clamped = true; }

    // 如果超出边界，就重置鼠标位置
    if (clamped) {
        glfwSetCursorPos(window, xpos, ypos);
    }
}

// 设置玩家摄像机
void BaseGame::SetPlayerCamera() {

    if (!player) {
        return; // 如果 player 为空，则直接返回，不执行后续代码
    }

    Vector3 playerPos = player->GetPhysicsObject()->BTGetPosition();


    float cameraHeight = 10.0f;// 相机离玩家的高度
    // 相机就在玩家正上方
    Vector3 cameraPos = Vector3(playerPos.x - 8, playerPos.y + cameraHeight, playerPos.z);



    if (player->IsDead() && !isCameraTransitioning) {

        // 记录死亡时摄像机位置为起点
        transitionStartPos = cameraPos;

        // 计算重生点的目标摄像机位置（需提前获取重生点坐标）
        Vector3 respawnPoint = player->GetRespawnPos();
        transitionTargetPos = Vector3(
            respawnPoint.x - 8,
            respawnPoint.y + cameraHeight,
            respawnPoint.z
        );

        // 初始化过渡
        isCameraTransitioning = true;
        transitionTimer = 0.0f;

    }

    if (isCameraTransitioning) {
        transitionTimer += dt;

        // 计算插值比例（0~1）,持续5s
        float t = transitionTimer / transitionDuration; 
        t = glm::clamp(t, 0.0f, 1.0f); // 确保不超界

        // 使用平滑过渡（示例使用smoothstep缓动,后续可以尝试阻尼平滑
        t = t * t * (3.0f - 2.0f * t);

        // 使用线性插值（Lerp）更新摄像机位置
        glm::vec3 newPosGLM = glm::mix(
            ToGLMVec3(transitionStartPos),
            ToGLMVec3(transitionTargetPos),
            t
        );

        // 将结果转回NCL::Vector3（若摄像机接口需要）
        Vector3 newPos(newPosGLM.x, newPosGLM.y, newPosGLM.z);
        camera->SetPosition(newPos);

        // 如果过渡完成，恢复控制
        if (t >= 1.0f) {
            isCameraTransitioning = false;
        }
        return;
    }


    camera->SetPosition(cameraPos);

    // 纯俯视 => Pitch = -90, 不转动水平角度 => Yaw = 0
    //float fixedPitch = -70.0f;
    float fixedPitch = -50.0f;
    float fixedYaw = 0.0f;
    camera->SetPitch(fixedPitch);
    camera->SetYaw(fixedYaw);
}

void BaseGame::UpdatePlayer(float dt) {
    if (!player) return;

    // 1. 检查死亡状态
    player->CheckDeath();

    // 2. 更新血条 UI
    if (UIManager::s_Instance) {
        UIManager::s_Instance->UpdatePlayerHealth(player->GetHealth(), player->GetMaxHealth());
    }

    // 3. 玩家死亡逻辑处理
    if (player->IsDead()) {
        render->isToneMapping = true;
        if (player->GetRespawnTimer() > 0) {
            player->ReduceRespawnTimer(dt);
            if (player->GetRespawnTimer() <= 0) {
                player->Respawn();
                render->isToneMapping = false;
            }
        }

        // 死亡状态下无需执行后续逻辑
        return;
    }

    // 4. 减少无敌时间
    if (player->GetInvincibleTimer() > 0) {
        player->ReduceInvincibleTimer(dt);
    }
}

// 更新物理
void BaseGame::UpdatePhysics(float dt) {
    if (isPaused) return; // 暂停时不更新物理
    physics->Update(dt);
}

// 更新渲染
void BaseGame::UpdateRender(float dt) {

    if (isPaused) {
        // 暂停时只渲染，不更新
        render->Render();
        return;
    }

    renderPrepare->Update();
    render->Update(dt);
    render->Render();
}

void BaseGame::UpdateWeapon()
{
    if (!player || !pistol)
        return;

    pistol->GetPhysicsObject()->BTSetRotation(
        player->GetPhysicsObject()->BTGetRotation() * Quaternion(Vector3(0, 1, 0), -SIMD_PI / 2.0f));

    pistol->GetPhysicsObject()->BTSetPosition(
        player->GetPhysicsObject()->BTGetPosition() + Vector3(1, 0, 1));
}


//玩家角色技能组

void BaseGame::TriggerDashing() {
    if (isDashing || isDashOnCooldown) return; // 防止重复触发

    // 获取鼠标指向的世界位置
    Vector3 mouseWorldPos = GetMouseWorldPosition();

    // 计算从玩家到鼠标的方向
    Vector3 playerPos = player->GetTransform().GetPosition();

    const float MIN_DASH_DISTANCE = 1.0f;
    if ((mouseWorldPos - playerPos).LengthSquared() < MIN_DASH_DISTANCE * MIN_DASH_DISTANCE) {
        mouseWorldPos = playerPos +
            player->GetTransform().GetOrientation() * Vector3(0, 0, -1) * MIN_DASH_DISTANCE;
    }

    dashDirection = Vector::Normalise(mouseWorldPos - playerPos); // 冲刺方向朝向鼠


    // ==== 2. 播放特效 ====
    //Vector3 effPosition = playerPos + Vector3(0, -0.5f, 0); // 玩家脚下

    Vector3 effDirection = dashDirection * 0.2f;
    Quaternion effRotation = Quaternion::FromTwoVectors(
        Vector3(0, 0, 1),  // 默认前向为Z轴
        // effDirection
        dashDirection
    );

    Vector3 euler = effRotation.ToEuler(); // 获取弧度值

    DashingEffectHandle = EffekseerManager::GetInstance()->PlayEffect(
        ResourceManager::DashingEffect,
        playerPos.x, playerPos.y - 0.5f, playerPos.z,
        euler.x, euler.y, euler.z // 直接传递弧度
    );



    // 重置计时器
    dashTimer = 0.0f;

    // 状态标记
    isDashing = true;
    isDashOnCooldown = true;
    dashCooldownTimer = 0.0f;
}

void BaseGame::UpdateDashing(float dt) {
    if (!isDashing) return;

    dashTimer += dt;

    if (dashTimer < dashDuration) {
        Vector3 playerPos = player->GetTransform().GetPosition();
        Vector3 effPosition = playerPos + Vector3(0, -0.5f, 0);

        // 更新位置
        EffekseerManager::GetInstance()->SetEffectPosition(DashingEffectHandle, effPosition.x, effPosition.y, effPosition.z);

        Vector3 effDirection = dashDirection * 0.2f;
        Quaternion effRotation = Quaternion::FromTwoVectors(Vector3(0, 0, 1), dashDirection);
        Vector3 euler = effRotation.ToEuler();

        // 正确设置旋转参数（弧度）
        EffekseerManager::GetInstance()->SetEffectRotation(
            DashingEffectHandle,
            euler.x, euler.y, euler.z
        );

        // 持续施加冲刺力
        PhysicsObject* physObj = player->GetPhysicsObject();
        physObj->SetLinearVelocity(Vector3(0, 0, 0)); // 清除惯性
        physObj->AddForce(dashDirection * dashSpeed);

    }
    else {
        // 冲刺结束
        EffekseerManager::GetInstance()->StopEffect(DashingEffectHandle);
        isDashing = false;
        player->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
    }


}

void BaseGame::UpdateDashCooldown(float dt) {
    if (isDashOnCooldown) {
        dashCooldownTimer += dt;
        if (dashCooldownTimer >= DASH_COOLDOWN) {
            isDashOnCooldown = false;
            dashCooldownTimer = 0.0f;
        }
    }
}




void BaseGame::TriggerShockwave() {//震荡波实现

    // 初始化震荡波状态
    isShockwaveActive = true;
    shockwaveRadius = 0.0f;
    shockwaveDuration = 0.0f;
    shockedMonsters.clear();

    // 获取玩家位置
    Vector3 playerPos = player->GetTransform().GetPosition();
    Vector3 playerForward = player->GetTransform().GetOrientation() * Vector3(0, 0, -1); // 玩家正前方方向

    Vector3 spawnOffset = Vector3(0, 0, 0); // Y 轴向下偏移
    Vector3 effPosition = playerPos + spawnOffset;

    Quaternion effRotation = Quaternion::FromTwoVectors(
        Vector3(0, 0, 1),  // 特效默认前向
        playerForward
    );

    // 转换为欧拉角（弧度）
    Vector3 euler = effRotation.ToEuler();
    float effRotX = glm::radians(euler.x);
    float effRotY = glm::radians(euler.y);
    float effRotZ = glm::radians(euler.z);
    // 播放特效
    ShockwaveEffectHandle = EffekseerManager::GetInstance()->PlayEffect(ResourceManager::ShockwaveEffect, effPosition.x, effPosition.y, effPosition.z, effRotX, effRotY, effRotZ);
    /*ShockwaveEffectHandle = EffekseerManager::GetInstance()->PlayEffect(
        ResourceManager::ShockwaveEffect,
        effPosition.x, effPosition.y, effPosition.z,
        euler.x, euler.y, euler.z
    );*/

    // 检测周围怪物
    //for (auto obj : sceneManager->GetAllGameObjects()) {
    //    if (auto monster = dynamic_cast<Monster*>(obj)) {
    //        if (monster->IsDead()) continue; // 跳过已死亡怪物

    //        // 计算距离
    //        Vector3 monsterPos = monster->GetTransform().GetPosition();

    //        float distance = NCL::Maths::Vector::Length(monsterPos - playerPos);

    //        if (distance < SHOCKWAVE_RADIUS) {
    //            // 调用击退和伤害逻辑
    //            btVector3 startPos(playerPos.x, playerPos.y, playerPos.z);
    //            btVector3 endPos(monsterPos.x, monsterPos.y, monsterPos.z);
    //            // 调用 HandleHurt，复用武器已有的伤害逻辑
    //            monster->HandleHurt(SHOCKWAVE_DAMAGE, startPos, endPos);
    //        }
    //    }
    //}

    isShockwaveOnCooldown = true;
    shockwaveCooldownTimer = 0.0f;
}

void BaseGame::UpdateShockwave(float dt) {
    if (!isShockwaveActive) return;

    // 更新扩散半径和持续时间
    shockwaveDuration += dt;
    shockwaveRadius = (shockwaveDuration / SHOCKWAVE_DURATION) * SHOCKWAVE_RADIUS; // 线性插值

    // 动态设置特效缩放（根据半径调整）
   // float scale = shockwaveRadius / SHOCKWAVE_RADIUS; // 归一化比例
    //EffekseerManager::GetInstance()->SetEffectScale(ShockwaveEffectHandle, scale, scale, scale);
    Vector3 playerPos = player->GetTransform().GetPosition();
    Vector3 playerForward = player->GetTransform().GetOrientation() * Vector3(0, 0, -1); // 玩家正前方方向

    Vector3 spawnOffset = Vector3(0, 0, 0); // Y 轴向下偏移
    Vector3 effPosition = playerPos + spawnOffset;

    Quaternion effRotation = Quaternion::FromTwoVectors(
        Vector3(0, 0, 1),  // 特效默认前向
        playerForward
    );

    Vector3 euler = effRotation.ToEuler();
    float effRotX = glm::radians(euler.x);
    float effRotY = glm::radians(euler.y);
    float effRotZ = glm::radians(euler.z);
    // 播放特效
    ShockwaveEffectHandle = EffekseerManager::GetInstance()->PlayEffect(ResourceManager::ShockwaveEffect, effPosition.x, effPosition.y, effPosition.z, effRotX, effRotY, effRotZ);

    // 检测当前半径内的新怪物
//    Vector3 playerPos = player->GetTransform().GetPosition();
    for (auto obj : sceneManager->GetAllGameObjects()) {
        if (auto monster = dynamic_cast<Monster*>(obj)) {
            if (monster->IsDead() || shockedMonsters.count(monster) > 0) continue;

            Vector3 monsterPos = monster->GetTransform().GetPosition();
            float distance = Vector::Length(monsterPos - playerPos);

            if (distance <= shockwaveRadius) {
                // 对怪物造成伤害
                btVector3 startPos(playerPos.x, playerPos.y, playerPos.z);
                btVector3 endPos(monsterPos.x, monsterPos.y, monsterPos.z);
                monster->HandleHurt(SHOCKWAVE_DAMAGE, startPos, endPos);

                // 记录已伤害的怪物
                shockedMonsters.insert(monster);
            }
        }
    }

    // 扩散结束条件
    if (shockwaveDuration >= SHOCKWAVE_DURATION) {
        isShockwaveActive = false;
        isShockwaveOnCooldown = true;
        shockwaveCooldownTimer = 0.0f;
        EffekseerManager::GetInstance()->StopEffect(ShockwaveEffectHandle);
    }
}

void BaseGame::UpdateShockwaveCooldown(float dt) {//震荡波冷却
    if (isShockwaveOnCooldown) {
        shockwaveCooldownTimer += dt;
        if (shockwaveCooldownTimer >= SHOCKWAVE_COOLDOWN) {
            EffekseerManager::GetInstance()->StopEffect(ShockwaveEffectHandle);
            isShockwaveOnCooldown = false;
            shockwaveCooldownTimer = 0.0f;
        }
    }
}







//触发黑洞技能
void BaseGame::TriggerBlackHole() {
    if (isBlackHoleActive || isBlackHoleOnCooldown) return; // 黑洞已激活或冷却中，不触发

    // 1. 在玩家面前生成黑洞对象
    Vector3 spawnPos = player->GetTransform().GetPosition() +
        player->GetTransform().GetOrientation() * Vector3(0, 0, -3);
    blackHole = new GameObject();
    //blackHole->SetName("BlackHole");
    blackHole->GetTransform().SetPosition(spawnPos)
        .SetScale(Vector3(0.5f, 0.5f, 0.5f)); // 设置黑洞的初始大小

    // 2. 添加一个球形碰撞体（仅用于检测敌人）
    SphereVolume* volume = new SphereVolume(BLACKHOLE_RADIUS);
    //blackHole->SetBoundingVolume((CollisionVolume*)volume);

    // 3.黑洞特效
    // 计算特效位置
    Vector3 playerForward = player->GetTransform().GetOrientation() * Vector3(0, 0, -1); // 玩家正前方方向
    Vector3 spawnOffset = playerForward * 3.0f; // 在玩家前方3米生成
    Vector3 effPosition = player->GetTransform().GetPosition() + spawnOffset;


    // 计算特效旋转
    // 将特效的默认前向方向 (0,0,1) 对齐到玩家正前方
    Quaternion effRotation = Quaternion::FromTwoVectors(
        Vector3(0, 0, 1),  // 特效默认前向
        playerForward
    );

    // 转换为欧拉角（弧度）
    Vector3 euler = effRotation.ToEuler();
    float effRotX = glm::radians(euler.x);
    float effRotY = glm::radians(euler.y);
    float effRotZ = glm::radians(euler.z);

    // 播放特效
    BlackHoleEffectHandle = EffekseerManager::GetInstance()->PlayEffect(ResourceManager::BlackHoleEffect, effPosition.x, effPosition.y, effPosition.z, effRotX, effRotY, effRotZ); // 特效资源



    // === 新增：创建物理对象 ===
    PhysicsObject* physicsObj = new PhysicsObject(
        &blackHole->GetTransform(),
        BulletShapeType::Sphere, // 球形刚体
        Vector3(BLACKHOLE_RADIUS, BLACKHOLE_RADIUS, BLACKHOLE_RADIUS),
        0.0f,                          // 质量设为0（静态刚体）
        btWorldManager
    );
    blackHole->SetPhysicsObject(physicsObj);
    blackHole->BindBulletToObject();   // 绑定到物理世界

    //将黑洞添加到场景管理器
    sceneManager->AddGameObjectToScene(blackHole);

    // 4. 激活黑洞逻辑
    blackHole->GetTransform().SetPosition(effPosition);
    blackHoleDuration = 5.0f; // 重置持续时间
    isBlackHoleActive = true;
    blackHoleCooldownTimer = 0.0f;
}




void BaseGame::UpdateBlackHole(float dt) {
    if (!isBlackHoleActive || blackHole == nullptr) {
        return;
    }
    // 每帧减少黑洞持续时间
    blackHoleDuration -= dt;
    // 获取黑洞中心位置
    Vector3 center = blackHole->GetTransform().GetPosition();
    float radius = BLACKHOLE_RADIUS;

    auto allObjects = sceneManager->GetAllGameObjects();
    for (auto obj : allObjects) {
        Vector3 objPos = obj->GetTransform().GetPosition();
        float distance = Vector::Length(objPos - center);
        if (distance <= radius) {
            if (auto monster = dynamic_cast<Monster*>(obj)) {
                // 计算方向向量
                Vector3 rawDir = center - objPos;
                float length = Vector::Length(rawDir);

                // 定义 dir 并初始化
                Vector3 dir = Vector3(0, 0, 0);
                if (length > 0.0f) {
                    dir = rawDir / length; // 手动归一化
                }

                // 施加向心力
                monster->GetPhysicsObject()->AddForce(dir * 200.0f);

                // 施加持续伤害（不触发击退）
                btVector3 monsterBtPos(objPos.x, objPos.y, objPos.z);
                monster->HandleHurt(BLACKHOLE_DAMAGE * dt, monsterBtPos, monsterBtPos);
            }
        }
    }
    // 销毁黑洞对象
    if (blackHoleDuration <= 0.0f) {
        // 从物理世界移除刚体
        if (blackHole->GetPhysicsObject() && btWorldManager) {
            auto bulletBody = blackHole->GetPhysicsObject()->GetBulletBody();
            if (bulletBody) {
                btWorldManager->GetPhysicsWorld()->removeRigidBody(bulletBody);
                delete bulletBody->getMotionState();//删除运动状态
                delete bulletBody;//删除刚体
            }
        }

        // 同步特效位置
        Vector3 currentPos = blackHole->GetTransform().GetPosition();
        EffekseerManager::GetInstance()->SetEffectPosition(BlackHoleEffectHandle, currentPos.x, currentPos.y, currentPos.z
        );

        // 销毁逻辑
        blackHoleDuration -= dt;
        if (blackHoleDuration <= 0.0f) {
            // 停止特效
            EffekseerManager::GetInstance()->StopEffect(BlackHoleEffectHandle);
            // 从场景管理器移除
            //sceneManager->RemoveObjectFromScene(blackHole);

            // 删除对象并置空指针
            //delete blackHole;
            blackHole = nullptr;

            //重置状态
            isBlackHoleActive = false;
            isBlackHoleOnCooldown = true;
            //blackHoleCooldownTimer = BLACKHOLE_COOLDOWN;
            blackHoleCooldownTimer = 0.0f;//重置即使冷却器
            blackHoleDuration = 5.0f;
        }
    }
}

//更新黑洞技能冷却
void BaseGame::UpdateBlackHoleCooldown(float dt) {
    if (!isBlackHoleOnCooldown) return;

    blackHoleCooldownTimer += dt;
    if (blackHoleCooldownTimer >= BLACKHOLE_COOLDOWN) {
        isBlackHoleOnCooldown = false;
        blackHoleCooldownTimer = 0.0f;
    }
}





void BaseGame::TriggerTurret() {
    if (isTurretOnCooldown || isTurretActive) return;

    //加入炮台本体特效
       // ===== 1. 计算特效位置 =====
    Vector3 playerForward = player->GetTransform().GetOrientation() * Vector3(0, 0, -1); // 玩家正前方方向
    Vector3 spawnOffset = playerForward * 3.0f; // 在玩家前方3米生成
    Vector3 effPosition = player->GetTransform().GetPosition() + spawnOffset;


    // ===== 2. 计算特效旋转 =====
    // 将特效的默认前向方向 (0,0,1) 对齐到玩家正前方
    Quaternion effRotation = Quaternion::FromTwoVectors(
        Vector3(0, 0, 1),  // 特效默认前向
        playerForward
    );

    // 转换为欧拉角（弧度）
    Vector3 euler = effRotation.ToEuler();
    float effRotX = glm::radians(euler.x);
    float effRotY = glm::radians(euler.y);
    float effRotZ = glm::radians(euler.z);

    // 播放特效
    TurretSpawnEffectHandle = EffekseerManager::GetInstance()->PlayEffect(ResourceManager::TurretSpawnEffect, effPosition.x, effPosition.y, effPosition.z, effRotX, effRotY, effRotZ);

    // 生成位置（玩家前方3米）
    Vector3 spawnPos = player->GetTransform().GetPosition() +
        player->GetTransform().GetOrientation() * Vector3(0, 0, -3);

    // 创建炮台对象（与黑洞对象创建逻辑一致）
    currentTurret = new GameObject();

    currentTurret->GetTransform()
        .SetPosition(spawnPos)
        .SetScale(Vector3(0.5f, 1.2f, 0.5f)); // 圆柱体型

    // 物理组件（静态碰撞体）
    PhysicsObject* physicsObj = new PhysicsObject(
        &currentTurret->GetTransform(),
        BulletShapeType::Sphere, // 球形刚体
        Vector3(0.5f, 1.2f, 0.5f),
        0.0f,                          // 质量设为0（静态刚体）
        btWorldManager
    );
    currentTurret->SetPhysicsObject(physicsObj);
    currentTurret->BindBulletToObject();   // 绑定到物理世界


    sceneManager->AddGameObjectToScene(currentTurret);

    // 初始化数据（与黑洞的blackHoleDuration逻辑一致）

    //turret->GetTransform().SetPosition(effPosition);
    isTurretActive = true;
    turretDuration = 8.0f; // 重置持续时间
    isTurretOnCooldown = true;
    turretCooldownTimer = 0.0f;


}







void BaseGame::UpdateTurret(float dt) {
    if (!isTurretActive || currentTurret == nullptr) {
        return; // 如果炮塔不活跃或者炮塔指针为空，直接返回
    }

    turretDuration -= dt;

    // 当炮塔时间结束时，清理并重置状态
    if (turretDuration <= 0) {
        //移除特效

        EffekseerManager::GetInstance()->StopEffect(TurretEffectHandle);
        // EffekseerManager::GetInstance()->StopEffect(TurretSpawnEffectHandle);
         // 从物理世界移除刚体，确保物理对象存在且有效
        if (currentTurret->GetPhysicsObject() && btWorldManager) {
            auto bulletBody = currentTurret->GetPhysicsObject()->GetBulletBody();
            if (bulletBody) {
                // 确保物理世界存在并移除刚体
                btWorldManager->GetPhysicsWorld()->removeRigidBody(bulletBody);
                delete bulletBody->getMotionState();
                delete bulletBody;

            }
        }

        // 重置状态
        currentTurret = nullptr; // 这时炮塔指针为 nullptr，不再指向已销毁的对象
        isTurretActive = false;
        isTurretOnCooldown = true;
        turretCooldownTimer = 0.0f;
        turretDuration = 8.0f;
        EffekseerManager::GetInstance()->StopEffect(TurretSpawnEffectHandle);
        return; // 关键：立即退出，避免后续代码访问空指针
    }

    // 自动寻找目标
    if (!turretTarget ||
        (currentTurret->GetTransform().GetPosition() - turretTarget->GetTransform().GetPosition()).LengthSquared() > TURRET_RANGE * TURRET_RANGE) {
        turretTarget = FindNearestEnemyForTurret(currentTurret->GetTransform().GetPosition());
    }

    // 攻击逻辑
    turretAttackTimer -= dt;

    // 检查炮塔目标并且确认炮塔指针有效
    if (turretTarget && turretAttackTimer <= 0) {
        if (currentTurret == nullptr) {
            return; // 如果炮塔已经为空，则退出
        }

        // 获取炮塔的起始位置
        Vector3 startPos = currentTurret->GetTransform().GetPosition() + Vector3(0, 1.0f, 0);
        Vector3 targetPos = turretTarget->GetTransform().GetPosition();
        Vector3 attackDir = Vector::Normalise(targetPos - startPos);


        //加入子弹特效
        Vector3 adjustedAttackDir = Vector3(-attackDir.z, attackDir.y, attackDir.x); // 根据坐标系差异调整
        Quaternion bulletRot = Quaternion::FromTwoVectors(Vector3(0, 0, 1), adjustedAttackDir);
        Vector3 euler = bulletRot.ToEuler();
        TurretEffectHandle = EffekseerManager::GetInstance()->PlayEffect(
            ResourceManager::pistolBulletEffect, // 使用手枪子弹特效
            startPos.x, startPos.y, startPos.z,
            glm::radians(euler.x),
            glm::radians(euler.y),
            glm::radians(euler.z)
        );

        btVector3 btStart(startPos.x, startPos.y, startPos.z);
        btVector3 btEnd = btStart + btVector3(attackDir.x, attackDir.y, attackDir.z) * TURRET_RANGE;

        if (Vector::Length(attackDir) < SIMD_EPSILON) {
            return;
        }
        // 执行射线检测
        btCollisionWorld::ClosestRayResultCallback rayCallback(btStart, btEnd);
        btWorldManager->GetPhysicsWorld()->rayTest(btStart, btEnd, rayCallback);

        // 如果有碰撞，进行伤害处理
        if (rayCallback.hasHit()) {
            if (auto monster = dynamic_cast<Monster*>(static_cast<GameObject*>(rayCallback.m_collisionObject->getUserPointer()))) {
                if (monster) {
                    monster->HandleHurt(TURRET_DAMAGE, btStart, rayCallback.m_hitPointWorld);
                }
            }
        }

        // 重置攻击时间
        turretAttackTimer = TURRET_ATTACK_INTERVAL;
    }
}





void BaseGame::UpdateTurretCooldown(float dt) {
    if (!isTurretOnCooldown) return;

    turretCooldownTimer += dt;
    if (turretCooldownTimer >= TURRET_COOLDOWN) {
        isTurretOnCooldown = false;
        turretCooldownTimer = 0.0f;
    }
}

GameObject* BaseGame::FindNearestEnemyForTurret(const Vector3& center) {
    float minSquaredDist = FLT_MAX; // 使用平方距离优化
    GameObject* nearest = nullptr;
    const float TURRET_RANGE_SQUARED = TURRET_RANGE * TURRET_RANGE; // 预计算平方值

    for (auto obj : sceneManager->GetAllGameObjects()) {
        if (dynamic_cast<Monster*>(obj)) {
            Vector3 objPos = obj->GetTransform().GetPosition();
            float squaredDist = (center - objPos).LengthSquared();

            if (squaredDist < minSquaredDist && squaredDist <= TURRET_RANGE_SQUARED) {
                minSquaredDist = squaredDist;
                nearest = obj;
            }
        }
    }
    return nearest;
}

// 冲刺技能接口
bool NCL::CSC8503::BaseGame::IsDashOnCooldown() const {
    return isDashOnCooldown; 
}

float NCL::CSC8503::BaseGame::GetDashCooldownTimer() const { 
    return dashCooldownTimer; 
}

float NCL::CSC8503::BaseGame::GetDashCooldownTime() const { 
    return DASH_COOLDOWN; 
}

// 震荡波技能接口
bool NCL::CSC8503::BaseGame::IsShockwaveOnCooldown() const { 
    return isShockwaveOnCooldown; 
}

float NCL::CSC8503::BaseGame::GetShockwaveCooldownTimer() const { 
    return shockwaveCooldownTimer; 
}

float NCL::CSC8503::BaseGame::GetShockwaveCooldownTime() const { 
    return SHOCKWAVE_COOLDOWN; 
}

// 黑洞技能接口
bool NCL::CSC8503::BaseGame::IsBlackHoleOnCooldown() const { 
    return isBlackHoleOnCooldown; 
}

float NCL::CSC8503::BaseGame::GetBlackHoleCooldownTimer() const { 
    return blackHoleCooldownTimer; 
}

float NCL::CSC8503::BaseGame::GetBlackHoleCooldownTime() const { 
    return BLACKHOLE_COOLDOWN; 
}

// 炮台技能接口
bool NCL::CSC8503::BaseGame::IsTurretOnCooldown() const { 
    return isTurretOnCooldown; 
}

float NCL::CSC8503::BaseGame::GetTurretCooldownTimer() const { 
    return turretCooldownTimer; 
}

float NCL::CSC8503::BaseGame::GetTurretCooldownTime() const { 
    return TURRET_COOLDOWN; 
}
