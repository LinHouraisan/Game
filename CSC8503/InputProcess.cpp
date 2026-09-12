#include "TutorialGame.h"
#include "BaseGame.h"
#include "Monster.h"

using namespace NCL;
using namespace CSC8503;
using namespace OpenGL;

void TutorialGame::UpdateKeys() {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// 冲刺技能检测（Q键）
	if (isKeyJustPressed(GLFW_KEY_Q)) {
		TriggerDashing(); // 触发冲刺技能
	}


	// 震荡波技能检测（Y键）
	if (isKeyJustPressed(GLFW_KEY_Y) && !isShockwaveOnCooldown) {
		TriggerShockwave();
	}


	// 黑洞技能检测（J键）
	if (isKeyJustPressed(GLFW_KEY_J) && !isBlackHoleOnCooldown) {
		TriggerBlackHole();
	}

	//炮台技能检测(K键)
	if (isKeyJustPressed(GLFW_KEY_K) && !isTurretOnCooldown) {
		TriggerTurret();
	}
	


	if (isKeyJustPressed(GLFW_KEY_G)) {
		useGravity = !useGravity;
		ControlGravity();
	}

	if (isKeyJustPressed(GLFW_KEY_P)) {
		if (player) {
			playerMode = !playerMode;
		}
		if (playerMode)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	}
	if (isKeyJustPressed(GLFW_KEY_U)) {
		isPressure = !isPressure;
	}

	// 按下M键时生成tiger
	if (isKeyJustPressed(GLFW_KEY_M)) {
		//SpawnTigers(0);
	}
	if (isKeyJustPressed(GLFW_KEY_N)) {
		//SpawnTigers(2);
	}

	//关卡切换调试
	if (isKeyJustPressed(GLFW_KEY_B)) {
		levelIndex++;
		switchMapCount++;
		if (levelIndex == 5 || switchMapCount == 5) {
			levelIndex = 0;
			switchMapCount = 0;
		}
		UpdatePlayerPosition();
	}

	//绘制关卡切换箭头
	if (isKeyJustPressed(GLFW_KEY_V)) {
		UpdateMapArrow(switchMapCount);
		isSpawningMonsters = false;
		monsterDead = !monsterDead; 
	}


	/*if (isKeyJustPressed(GLFW_KEY_F3)) {
		render->isDebugPath = !(render->isDebugPath);
		if (render->isDebugPath) {
			for (auto arrow : gridArrows) {
				arrow->GetRenderObject()->SetIsActive(true);
			}
		}
		else {
			for (auto arrow : gridArrows) {
				arrow->GetRenderObject()->SetIsActive(false);
			}
		}
	}*/

	
	if (isKeyJustPressed(GLFW_KEY_L)) {
		for (auto& monster : monsters) {
			monster->MonsterFinish = !monster->MonsterFinish;
		}
	}


	if (isKeyJustPressed(GLFW_KEY_T)) {
		effekseerManager->PlayEffect(ResourceManager::laserEffect, 0.0f, 1.0f, 0.0f);
	}

	bool currentEState = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
	eKeyPressed = currentEState && !eKeyWasPressed; // 仅在按下瞬间触发
	eKeyWasPressed = currentEState;

	if (!playerMode)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera->ProcessKeyboard(FORWARD, dt);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera->ProcessKeyboard(BACKWARD, dt);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera->ProcessKeyboard(LEFT, dt);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera->ProcessKeyboard(RIGHT, dt);
	}

}

void BaseGame::PlayerControl() {

	if (isPaused) return; // 暂停状态下不处理玩家控制

	if (!playerMode || !player || player->IsDead() || isCameraTransitioning) return;

	PhysicsObject* physObj = player->GetPhysicsObject();
	Vector3 forwardDir = projectedDir;  // W前进方向
	Vector3 rightDir(-projectedDir.z, 0, projectedDir.x);  // D右

	float groundForce = 200.0f;

	Vector3 applyDir(0, 0, 0);
	bool applyForce = false;

	Vector3 toTarget = GetMouseWorldPosition() - player->GetPhysicsObject()->BTGetPosition(); // 玩家到鼠标位置
	toTarget.y = 0;

	if (isAutoFire) // 自动操控
	{
		// 设定一个停靠范围，避免卡顿
		if (Vector::Length(toTarget) > 0.5f)
		{
			applyDir = Vector::Normalise(toTarget);
			applyForce = true;
		}
		else // 重置速度，消除惯性
		{
			physObj->SetLinearVelocity(Vector3(0, 0, 0)); // 停止移动
			physObj->SetAngularVelocity(Vector3(0, 0, 0)); // 停止旋转
		}
	}
	else // 手动操控
	{
		// 获取按键输入
		bool W = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
		bool A = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
		bool S = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
		bool D = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

		// 获取按键松开状态
		bool W_Release = glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE;
		bool A_Release = glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE;
		bool S_Release = glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE;
		bool D_Release = glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE;

		int moveX = 0;
		int moveZ = 0;
		if (W) { moveZ += 1; applyForce = true; }
		if (S) { moveZ -= 1; applyForce = true; }
		if (D) { moveX += 1; applyForce = true; }
		if (A) { moveX -= 1; applyForce = true; }

		// 生成移动方向向量
		applyDir = forwardDir * (float)moveZ + rightDir * (float)moveX;

		// 重置速度，消除惯性
		if (W_Release && A_Release && S_Release && D_Release) {
			physObj->SetLinearVelocity(Vector3(0, 0, 0)); // 停止移动
			physObj->SetAngularVelocity(Vector3(0, 0, 0)); // 停止旋转
		}
	}

	// 施加力
	if (applyForce) {
		physObj->SetLinearVelocity(Vector3(0, 0, 0)); // 清除当前速度，防止惯性影响
		groundForce = HandleGroundForce(130.0f);    //是否遇到地形减速
		physObj->AddForce(Vector::Normalise(applyDir) * groundForce);
		player->GetRenderObject()->SetAnimationSpeed(1);
	}
	else {
		player->GetRenderObject()->SetAnimationSpeed(0);
	}

	// 计算旋转方向
	Vector3 playerPos = player->GetTransform().GetPosition();
	Vector3 mouseWorldPos = GetMouseWorldPosition();
	Vector3 lookDir;

	if (isAutoFire)
	{
		lookDir = Vector::Normalise(toTarget);
	}
	else
	{
		lookDir = Vector::Normalise(mouseWorldPos - playerPos);
	}

	lookDir.y = 0; // 确保绕 Y 轴旋转

	// 计算目标旋转（仅绕Y轴）
	Quaternion targetRotation = Quaternion::FromTwoVectors(Vector3(0, 0, -1), lookDir);
	player->GetPhysicsObject()->BTSetRotation(targetRotation);

	// 切换自动开火模式
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		isAutoFire = !isAutoFire;
	}

	//按住鼠标左键开火（受attackCooldown影响）
	if ((isAutoFire && FindNearestTarget()) || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		for (Weapon* weapon : weapons)
		{
			if (weapon)
			{
				weapon->Attack();
			}
		}
	}
}

Monster* BaseGame::FindNearestTarget(Player* playerObj)
{
	// 如果没有指定玩家对象，则使用BaseGame的player变量
	if (!playerObj) 
	{
		playerObj = player;
	}

	std::vector<Monster*> monsters = sceneManager->GetMonsters();

	Monster* nearestMonster = nullptr;
	float minDistance = FLT_MAX;
	float mapSize = 30.0f;

	Vector3 playerPos = player->GetPhysicsObject()->BTGetPosition();

	for (Monster* monster : monsters) 
	{
		if (!monster) continue;

		Vector3 monsterPos = monster->GetPhysicsObject()->BTGetPosition();
		float distance = Vector::Length(playerPos - monsterPos);

		if (distance < minDistance && distance < mapSize) 
		{
			minDistance = distance;
			nearestMonster = monster;
		}
	}

	return nearestMonster;
}

// 获取所有武器造成的总伤害
float BaseGame::GetFinalDamage()
{
	float finalDamage = 0.0f;
	for (Weapon* weapon : weapons)
	{
		finalDamage += weapon->GetTotalDamage();
	}

	return finalDamage;
}

void BaseGame::EquipWeapon(Weapon* newWeapon)
{
	weapons.push_back(newWeapon);
}

//负责武器相关更新
void BaseGame::UpdateWeapon(Weapon* weapon, Player* playerObj)
{
	// 如果没有指定玩家对象，则使用BaseGame的player变量
	if (!playerObj) {
		playerObj = player;
	}
	if (!weapon || !playerObj)
		return;

	//不同位置的武器与玩家的相对偏移
	Vector3 posOffset;
	switch (weapon->GetWeaponID())
	{
	case 1: posOffset = Vector3(0.8, 0, 0.8); break;
	case 2: posOffset = Vector3(-0.8, 0, 0.8); break;
	case 3: posOffset = Vector3(-0.8, 0, -0.8); break;
	case 4: posOffset = Vector3(0.8, 0, -0.8); break;
	}

	// 重力枪单独处理
	GravityWeapon* gravityWeapon = dynamic_cast<GravityWeapon*>(weapon);
	if (gravityWeapon && gravityWeapon->GetGravityState())
	{
		gravityWeapon->ApplyGravityField(dt);
	}

	//更新武器位置
	if (!dynamic_cast<BounceWeapon*>(weapon)) // 光棱枪不更新位置
	{
		float recoilTimer = weapon->GetRecoilTimer();
		if (recoilTimer > 0.0f) //处理后坐力导致的位移
		{
			Vector3 currentPos = weapon->GetPhysicsObject()->BTGetPosition();
			Vector3 originalPos = player->GetPhysicsObject()->BTGetPosition() + posOffset;

			//逐渐回到原位
			weapon->GetPhysicsObject()->BTSetPosition(
				currentPos + (originalPos - currentPos) * dt * weapon->GetRecoilRecoverySpeed());

			//更新计时器
			weapon->SetRecoilTimer(recoilTimer - dt);
		}
		else
		{
			weapon->GetPhysicsObject()->BTSetPosition(
				player->GetPhysicsObject()->BTGetPosition() + posOffset);
		}
	}

	//设置武器朝向
	Vector3 targetPos = GetMouseWorldPosition();
	if (isAutoFire)
	{
		Monster* nearestMonster = FindNearestTarget();
		if (nearestMonster)
		{
			targetPos = nearestMonster->GetPhysicsObject()->BTGetPosition();
		}
	}

	Vector3 weaponLookDir = Vector::Normalise(targetPos - weapon->GetPhysicsObject()->BTGetPosition());
	weaponLookDir.y = 0;
	weapon->SetWeaponLookDir(weaponLookDir);

	//更新武器旋转
	Quaternion weaponRot = Quaternion::FromTwoVectors(Vector3(0, 0, -1), weapon->GetWeaponLookDir());
	weapon->GetPhysicsObject()->BTSetRotation(weaponRot * Quaternion(Vector3(0, 1, 0), 1.0f * SIMD_PI / 3.0f));//额外偏移修正
}

void TutorialGame::ControlGravity() {
	if (useGravity) {
		btWorldManager->GetPhysicsWorld()->setGravity(btVector3(0, -9.81f, 0));

		for (GameObject* obj : sceneManager->GetAllGameObjects()) {
			if (obj->GetPhysicsObject() && obj->GetPhysicsObject()->GetBulletBody()) {
				btRigidBody* body = obj->GetPhysicsObject()->GetBulletBody();
				body->setGravity(btVector3(0, -9.81f, 0));
				body->activate();
			}
		}
	}
	else {
		btWorldManager->GetPhysicsWorld()->setGravity(btVector3(0, 0, 0));

		for (GameObject* obj : sceneManager->GetAllGameObjects()) {
			if (obj->GetPhysicsObject() && obj->GetPhysicsObject()->GetBulletBody()) {
				btRigidBody* body = obj->GetPhysicsObject()->GetBulletBody();
				body->setGravity(btVector3(0, 0, 0));
				body->activate();
			}
		}
	}

}

Vector3 BaseGame::GetMouseWorldPosition() {
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	// 获取屏幕尺寸
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	// 归一化鼠标位置
	float x = (2.0f * mouseX) / width - 1.0f;
	float y = 1.0f - (2.0f * mouseY) / height; // OpenGL 屏幕坐标 y 轴向下

	// 获取摄像机的投影矩阵和视图矩阵
	glm::mat4 view = camera->GetViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)width / (float)height, 0.1f, 100.0f);

	// 逆变换到世界空间
	glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);
	glm::vec4 rayEye = glm::inverse(projection) * rayClip;
	rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
	glm::vec3 rayWorld = glm::vec3(glm::inverse(view) * rayEye);
	rayWorld = glm::normalize(rayWorld);

	// 计算鼠标指向的世界坐标（假设鼠标射线与 Y = 0 平面相交）
	float t = -camera->Position.y / rayWorld.y;
	glm::vec3 worldPos = camera->Position + rayWorld * t;

	return Vector3(worldPos.x, 0, worldPos.z);
}

// ClampMousePosition()移动到BaseGame
// void TutorialGame::ClampMousePosition() {}

// isKeyJustPressed()移动到BaseGame
// bool TutorialGame::isKeyJustPressed(int key) {}

// isMousePressed()移动到BaseGame
// bool TutorialGame::isMousePressed(int key) {}

// SetPlayerCamera()移动到BaseGame
// void TutorialGame::SetPlayerCamera() {}
