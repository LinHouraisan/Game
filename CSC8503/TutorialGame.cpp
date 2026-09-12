#include "TutorialGame.h"
#include "UIManager.h"


using namespace NCL;
using namespace CSC8503;
using namespace OpenGL;

bool monsterDead = false;
bool CoinRemoved = false;
bool switchMap = false;
int switchMapCount = 0;

TutorialGame::TutorialGame(GLFWwindow* window, OpenGL::GameTechRenderer* renderer, OpenGL::ResourceManager* resourceManager)
	: BaseGame(window, renderer, resourceManager) {
	// 初始化TutorialGame特有的属性
	isPressure = false;
	testobj_tiger = nullptr;
	testobj_gun = nullptr;
	pistol = nullptr;
	Water = nullptr;

	// 已在BaseGame中初始化的属性不需要重复初始化
	// 例如 window, physics, sceneManager 等

	// 设置playerMode为true（在BaseGame中默认为false）
	playerMode = true;

	// 调用InitWorld完成游戏世界初始化
	InitWorld();
}


TutorialGame::~TutorialGame() {
	// 只释放TutorialGame特有的资源
}

void TutorialGame::UpdateGame(float dt) {
	// 始终更新按键状态，确保即使在暂停状态下也能处理UI输入和暂停相关快捷键
	UpdateKeys();

	// 检查暂停状态
	if (isPaused) {
		// 在暂停状态下，只进行渲染而不更新游戏状态
		render->Render();
		return;
	}

	CheckLevelCompletion();
	// 处理玩家控制
	PlayerControl();

	// 更新游戏时间
	gameTime += dt;
	this->dt = dt;

	// 更新物理系统 - 调用基类方法
	UpdatePhysics(dt);

	// 更新渲染 - 调用基类方法
	UpdateRender(dt);

	// 特定于TutorialGame的更新逻辑
	if (playerMode && player) {
		SetPlayerCamera(); // 基类方法
		ClampMousePosition(); // 基类方法
		UpdateFlowFieldArrows();
	}

	SwitchLevel(); //关卡切换
	GetWeapon();//武器随关卡推进自动获得
	CreateCoin();  //金币生成
	UpdateEvent(); //随机事件更新
	UpdateEffectPostion(healingEffectHandle);  //特效跟随处理

	UpdatePlayer(dt);

	//更新冲刺技能
	UpdateDashing(dt); // 每帧更新冲刺状态
	UpdateDashCooldown(dt); // 更新冲刺冷却计时

	//更新震荡波技能
	UpdateShockwave(dt);
	UpdateShockwaveCooldown(dt); // 每帧更新冷却
	SkillEnhancement();//自动释放震荡波技能

	// 更新黑洞技能
	UpdateBlackHole(dt);
	UpdateBlackHoleCooldown(dt);

	//更新炮台技能
	UpdateTurret(dt);
	UpdateTurretCooldown(dt);

	for (Weapon* weapon : weapons)
	{
		if (weapon)
		{
			UpdateWeapon(weapon);
		}
	}
	
	UpdateMonsterGeneration(dt);

	// 维护寻路&更新敌人移动
	for (auto tiger : sceneManager->GetMonsters()) {
		UpdateEnemyMovement(static_cast<Monster*>(tiger), dt);
		// 更新状态
		static_cast<Monster*>(tiger)->UpdateState(dt);
		static_cast<Monster*>(tiger)->MonsterUpdate(dt);

	}

	if (render->GetUIManager()) {
		render->GetUIManager()->UpdateDebugInfo();
	}
}

void TutorialGame::UpdatePlayerPosition() {
	player->GetPhysicsObject()->BTSetPosition(Vector3(switchMapCount * 50.0f, 0.1, switchMapCount * 50.0f));
	player->SetRespawnPos(Vector3(switchMapCount * 50.0f, 0.1, switchMapCount * 50.0f));
	switchMap = false;
}


void TutorialGame::SwitchLevel() {
	//怪物全部死亡后，关卡切换箭头生成，玩家靠近按E进入下一关
	if (eKeyPressed && player && !mapArrows.empty()) {
		Vector3 playerPos = player->GetTransform().GetPosition();

		for (auto arrow : mapArrows) {
			if (!arrow) continue;

			Vector3 arrowPos = arrow->GetTransform().GetPosition();
			float distance = Vector::Length(playerPos - arrowPos);

			//进入下一关
			if (distance < 1.2f) {
				levelIndex++;//关卡索引
				switchMapCount++;

				StartMonsterGeneration();//怪物生成逻辑
				CreateArrow();//debug箭头（下一关）
				monsterDead = false;//重置怪物生成
				switchMap = true;

				//测试用条件,防止越界
				if (levelIndex == 5) {
					levelIndex = 0;
					switchMapCount = 0; // 确保switchMapCount也被重置
				}

				UpdatePlayerPosition(); //更新玩家位置
				std::cout << "Next Level, This Level is" << switchMapCount + 1 << std::endl;

				// 将当前关卡目标显示在控制台
				UIManager* uiManager = render->GetUIManager();
				int baseTarget = uiManager->GetCurrentLevelKillTarget();
				int nextLevelTarget = baseTarget * (switchMapCount + 1);
				std::cout << "New Score" << nextLevelTarget << " defeat" << std::endl;

				break;
			}
		}
	}
}


void TutorialGame::InitWorld() {
	sceneManager->ClearAndErasePhysics();

	// 加载地图
	mapGrids.resize(5);
	mapGrids[0] = ResourceManager::map1Grid;
	mapGrids[1] = ResourceManager::map2Grid;
	mapGrids[2] = ResourceManager::map3Grid;
	mapGrids[3] = ResourceManager::map4Grid;
	mapGrids[4] = ResourceManager::map5Grid;

	// 创建地面
	sceneManager->EmptyPreform(
		Vector3(0, -0.75, 0), Quaternion(), BulletShapeType::Box, Vector3(0.001, 0.001, 0.001), mapSize, 0.0f);

	// 创建玩家
	player = sceneManager->PlayerPreform(
		Vector3(-10, 0.1, 0), Quaternion(), BulletShapeType::Sphere, Vector3(0.5, 0.5, 0.5), Vector3(0.5, 0.5, 0.5), 1.0f);
	// std::cout << "Player Health: " << player->GetHealth() << std::endl;

	// 测试用手枪
	pistol = sceneManager->PistolPreform(Vector3(1, 0, 1), Quaternion(), BulletShapeType::Sphere, Vector3(0.3, 0.3, 0.3), Vector3(0.5, 0.5, 0.5), 1.0f);
	pistol->Initialize(1, 15.0f, 5.0f, 0.2f); // 设置手枪基础参数
	renderPrepare->AddWeapon(pistol->GetRenderObject());
	EquipWeapon(pistol);
	//----手枪的射击油桶逻辑-----
	pistol->SetGameContext(this);         

	//测试武器渲染
	for (auto weapon : renderPrepare->GetWeaponsRender()) {
		std::cout << weapon->GetModel()->GetModelName() << std::endl;
	}
	// 创建箭头指示器
	CreateArrow();
	StartMonsterGeneration();

	//water
	Water = sceneManager->waterPreform(
		Vector3(105.5f, -0.5f, 106.5f), Quaternion(), BulletShapeType::Box, Vector3(1, 1, 1), Vector3(15, 0.1, 15), 0.0f);
}

void TutorialGame::CheckLevelCompletion() {

	int killCount = NCL::CSC8503::Monster::GetKillCount();
	UIManager* uiManager = render->GetUIManager();

	// 基础目标是20，每关递增20
	int baseTarget = uiManager->GetCurrentLevelKillTarget();
	int currentLevelTarget = baseTarget * (switchMapCount + 1);

	// 每个关卡只触发一次关卡完成
	if (killCount >= currentLevelTarget && !monsterDead) {
		std::cout << "Level completion! Defeat:" << killCount << " Target:" << currentLevelTarget << std::endl;

		// 显示关卡完成弹窗
		uiManager->ShowLevelCompletePopup();

		// 启用关卡转换
		monsterDead = true;

		// 更新地图箭头以引导玩家到下一关
		UpdateMapArrow(switchMapCount);
	}
}

float TutorialGame::GetMemoryUsage() {
#ifdef _WIN32
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
		return	pmc.WorkingSetSize / 1024;
	}
#else
	struct rusage usage;
	if (getrusage(RUSAGE_SELF, &usage) == 0) {
		std::cout << "Memory Usage (Unix):" << std::endl;
		std::cout << "  Resident Set Size: " << usage.ru_maxrss << " KB" << std::endl;
	}
#endif
}