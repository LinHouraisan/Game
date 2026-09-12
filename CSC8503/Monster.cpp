#include "Monster.h"
#include "Vector.h"
#include "RenderObject.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "GameManager.h"

#include <cstdlib>
using namespace NCL::CSC8503;
using namespace NCL::Maths;

// 初始化静态计数器
int NCL::CSC8503::Monster::s_killCount = 0;

void Monster::MonsterUpdate(float deltaTime) {
    // 检查游戏是否暂停
    if (GameManager::GetInstance()->IsPaused()) {
        return; // 暂停时不更新怪物行为
    }

    if (isDead) {
        if (deathTimer > 0) {
            deathTimer -= deltaTime;  // 减少死亡计时器
        }
        if (GetPhysicsObject()) {
            GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));  // 清除线性速度
            GetPhysicsObject()->SetKinematic(true);  // 设置为运动学刚体，停止物理引擎控制
        }
        if (deathTimer <= 0) {
            // 死亡计时器到期，移除怪物
            sceneManager->RemoveObjectFromScene(this);
            isDead = false;  // 防止重复删除
        }
    }

    // 处理击退效果，每帧更新
    if (knockbackTime > 0.0f) {
        knockbackTime -= deltaTime; // 减少击退时间
        if (GetPhysicsObject() && GetPhysicsObject()->GetBulletBody()) {
            GetPhysicsObject()->GetBulletBody()->activate(); // 确保刚体活动
            GetPhysicsObject()->AddForce(knockbackVelocity);
        }
    }
}

void Monster::UpdateState(float deltaTime) {
    // 检查游戏是否暂停
    if (GameManager::GetInstance()->IsPaused()) {
        return; // 暂停时不更新AI状态
    }

    if (!sceneManager || !sceneManager->GetPlayer()) {
        return; // 如果 sceneManager 或玩家对象无效，提前返回
    }

    Vector3 playerPosition = sceneManager->GetPlayer()->GetTransform().GetPosition();

    // 计算距离
    float distanceToPlayer = Vector::Length(playerPosition - GetTransform().GetPosition());

    if (distanceToPlayer > 1.0f) {
        state = MonsterState::IDLE;
    }
    else {
        state = MonsterState::ATTACK;
    }

    switch (state) {
    case MonsterState::IDLE:
        if (GetRenderObject() && !isDead) {
            GetRenderObject()->SetAnimationIndex(6);  // 受击动画
            GetRenderObject()->SetIsRepeat(true);
        }
        // std::cout << GetName() << " is in IDLE state." << std::endl;
        break;
    case MonsterState::ATTACK:
        //std::cout << GetName() << " is in ATTACK state." << std::endl;
        AttackPlayer();
        break;
    }
}

void Monster::HandleHurt(float weaponDamage, const btVector3& startPos, const btVector3& btEndPos) {
    health -= weaponDamage;
    // std::cout << GetName() << " took " << weaponDamage << " damage! Remaining health: " << health << std::endl;

    if (GetRenderObject() && !isDead) {
        GetRenderObject()->SetAnimationIndex(4);  // 受击动画
        GetRenderObject()->SetIsTransitioning(true);
        GetRenderObject()->SetIsRepeat(false);
    }

    if (health <= 0.0f && !isDead) {
        HandleDeath();
        return;
    }
    else {
        btVector3 knockbackDir = btEndPos - startPos;
        if (knockbackDir.length() > 0.1f) {
            knockbackDir.normalize();
            knockbackVelocity = Vector3(knockbackDir.x(), knockbackDir.y(), knockbackDir.z()) * (weaponDamage * 20.0f);
            knockbackTime = 0.1f;
            GetPhysicsObject()->GetBulletBody()->activate();
        }
    }

    if (GetRenderObject()) {
        if (GetRenderObject()->GetIsTransitioning()) return; // 防止多次触发

        GetRenderObject()->SetIsTransitioning(true);
        GetRenderObject()->SetAnimationIndex(4);  // 切换到受伤动画
        GetRenderObject()->SetIsRepeat(false);    // 只播放一次
    }
}
bool Monster::isBossDead = false; // 初始化 Boss 死亡状态为 false
void Monster::HandleDeath() {

    if (isBoss) {
        isBossDead = true; // 只有Boss死亡时才会变true
        std::cout << "The Boss has been defeated!" << std::endl;
    }



    if (GetPhysicsObject()) {
        GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));  // 清除线性速度
        GetPhysicsObject()->SetKinematic(true); // 设置为运动学刚体，停止物理引擎控制
    }

    if (!isDead) {
        isDead = true;
        deathTimer = 2.0f; // 设置2秒延迟

        // 增加击杀计数
        s_killCount++;

        // 播放死亡动画
        if (GetRenderObject()) {
            GetRenderObject()->SetAnimationIndex(3);
            GetRenderObject()->SetIsRepeat(false);
            GetRenderObject()->SetIsTransitioning(true);
        }
    }
    // 通知游戏系统怪物死亡
    GameManager* gameManager = GameManager::GetInstance();
    if (gameManager->GetCurrentGameType() == GameType::NETWORKED) {
        NetworkedGame* networkedGame = dynamic_cast<NetworkedGame*>(gameManager->GetCurrentGame());
        if (networkedGame) {
            networkedGame->NotifyMonsterDeath(this);
        }
    }
    // std::cout << GetName() << " has died! Total kills: " << s_killCount << std::endl;
}

void Monster::AttackPlayer() {
    if (GetRenderObject()) {
        if (GetRenderObject()->GetIsTransitioning()) return; // 防止多次触发

        GetRenderObject()->SetIsTransitioning(false);
        GetRenderObject()->SetAnimationIndex(2);  // 切换到受伤动画
        GetRenderObject()->SetIsRepeat(false);    // 只播放一次
    }
    sceneManager->GetPlayer()->TakeDamage(damage);  // 让玩家掉血
}