#include "TutorialGame.h"
#include "BaseGame.h"
#include "Monster.h"
#include "Player.h"
#include "Vector.h"
#include "AudioManager.h"

using namespace NCL;
using namespace CSC8503;
using namespace OpenGL;

void BaseGame::UpdateMapArrow(int mapIndex) {
	if (mapIndex < 3) {
		for (GameObject* arrow : mapArrows) {
			sceneManager->RemoveObjectFromScene(arrow);
		}
		mapArrows.clear();

		mapArrows.push_back(sceneManager->mapArrowPreform(Vector3(0, 0.02, 0), Quaternion(), BulletShapeType::Box, Vector3(1, 1, 1), Vector3(1, 1, 1), 0.0f, Vector3(13.5 + 50.0f * mapIndex, 0, 0 + 50.0f * mapIndex)));
		mapArrows.push_back(sceneManager->mapArrowPreform(Vector3(0, 0.02, 0), Quaternion(), BulletShapeType::Box, Vector3(1, 1, 1), Vector3(1, 1, 1), 0.0f, Vector3(0 + 50.0f * mapIndex, 0, 13.5 + 50.0f * mapIndex)));
		mapArrows.push_back(sceneManager->mapArrowPreform(Vector3(0, 0.02, 0), Quaternion(), BulletShapeType::Box, Vector3(1, 1, 1), Vector3(1, 1, 1), 0.0f, Vector3(0 + 50.0f * mapIndex, 0, -13.5 + 50.0f * mapIndex)));
	}
	else if(mapIndex == 3){
		mapArrows.push_back(sceneManager->mapArrowPreform(Vector3(0, 0.02, 0), Quaternion(), BulletShapeType::Box, Vector3(1, 1, 1), Vector3(1, 1, 1), 0.0f, Vector3(0 + 50.0f * mapIndex, 0, 0 + 50.0f * mapIndex)));
	}
}

//²»Í¬µØÍ¼µÄ½ð±ÒÖ¸Ê¾ÎïÉú³É
void TutorialGame::CreateCoin() {
	if (switchMapCount == 1 || switchMapCount == 2 || switchMapCount == 3) {
		//Ö»ÓÐµ±Ç°ÖµÓëÉÏÒ»´Î²»Í¬Ê±²ÅÖ´ÐÐ¸üÐÂ
		if (lastCoinMapCount != switchMapCount) {
			Coins.clear();
			//½ð±ÒÉú³ÉµÄ¹Ø¿¨ºÍÎ»ÖÃ
			if (switchMapCount == 1) {
				Coins.push_back(sceneManager->mapCoinPreform
				(Vector3(-5.8 + 50 * switchMapCount, 0.2, 2.48 + 50 * switchMapCount), Quaternion(), BulletShapeType::Sphere, Vector3(1, 1, 1), Vector3(0.6, 0.6, 0.6), 0));
			}
			else if (switchMapCount == 2) {
				Coins.push_back(sceneManager->mapCoinPreform
				(Vector3(2.2 + 50 * switchMapCount, 0.2, 4.5 + 50 * switchMapCount), Quaternion(), BulletShapeType::Sphere, Vector3(1, 1, 1), Vector3(0.6, 0.6, 0.6), 0));
			}
			else if (switchMapCount == 3) {
				Coins.push_back(sceneManager->mapCoinPreform
				(Vector3(2.47f + 50 * switchMapCount, 0.2, 4.47f + 50 * switchMapCount), Quaternion(), BulletShapeType::Sphere, Vector3(1, 1, 1), Vector3(0.6, 0.6, 0.6), 0));
				Coins.push_back(sceneManager->mapCoinPreform
				(Vector3(-5.54f + 50 * switchMapCount, 0.2, -4.545f + 50 * switchMapCount), Quaternion(), BulletShapeType::Sphere, Vector3(1, 1, 1), Vector3(0.6, 0.6, 0.6), 0));
			}

			lastCoinMapCount = switchMapCount; //¼ÇÂ¼µ±Ç°×´Ì¬
		}
	}
	else {
		lastCoinMapCount = -1; //ÖØÖÃ
	}
}

void TutorialGame::UpdateEvent() {
	static float coinRemoveTime = 0.0f; //¼ÇÂ¼ CoinRemoved ÉèÎª true µÄÊ±¼ä

	if (CoinRemoved) {
		float currentTime = glfwGetTime(); 
		if (currentTime - coinRemoveTime >= 3.0f) { //3ÃëÀäÈ´
			CoinRemoved = false;
			// std::cout << "Buff effect ended." << std::endl;
		}
	}

	for (auto& coin : Coins) {

		Vector3 playerPos = player->GetTransform().GetPosition();
		Vector3 coinPos = coin->GetTransform().GetPosition();

		//¿¿½ü°´ÏÂEºó£¬´¥·¢buffÌØÐ§
		float distance = Vector::Length(playerPos - coinPos);

		if ((distance < 3.0f) && isKeyJustPressed(GLFW_KEY_E) && !CoinRemoved) {
			CoinRemoved = true;

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> distrib(0, 1);

			//Ëæ»úÖ´ÐÐÆäÖÐÒ»¸öÊÂ¼þ
			if (distrib(gen) == 0) {
				//--ÖÎÁÆbuffÐ§¹û--
				UpdateHealingEvent();
			}
			else {
				//--ÓÍÍ°--
				UpdateBarrelEvent(switchMapCount);
			}

			coinRemoveTime = glfwGetTime(); // ¼ÇÂ¼µ±Ç°Ê±¼ä
			// std::cout << "Activate Buff. " << player->GetHealth() << std::endl;
		}
	}
}

void TutorialGame::UpdateEffectPostion(Effekseer::Handle EffectHandle) {
	if (EffectHandle != -1) {
		Vector3 playerPos = player->GetTransform().GetPosition();

		// ¸üÐÂÌØÐ§Î»ÖÃµ½Íæ¼Òµ±Ç°Î»ÖÃ
		effekseerManager->SetEffectPosition(
			EffectHandle,
			playerPos.x,
			0.1f, // ±£³Ö¹Ì¶¨¸ß¶È
			playerPos.z
		);

		// ¼ì²éÌØÐ§ÊÇ·ñ½áÊø
		if (!effekseerManager->IsEffectPlaying(EffectHandle)) {
			EffectHandle = -1;
		}
	}
}

void TutorialGame::UpdateHealingEvent() {
	Vector3 playerPos = player->GetTransform().GetPosition();
	healingEffectHandle = effekseerManager->PlayEffect(
		ResourceManager::healingEffect, playerPos.x, 0.1f, playerPos.z);
	AudioManager::PlaySound("heal");
	player->SetHealth(player->GetHealth() + 30);
}

void TutorialGame::UpdateBarrelEvent(int mapIndex) {
	std::random_device rd;
	std::mt19937 gen(rd());

	// ´æ´¢ÖµÎª 1 µÄÓÐÐ§Éú³Éµã
	std::vector<Vector3> validSpawnPositions;
	// È·±£ switchMapCount ÔÚºÏ·¨·¶Î§ÄÚ

	float gridSize = mapSize.x / 30.0f;
	float centerX = mapIndex * 50.0f;  // µ±Ç°µØÍ¼µÄ X ÖáÖÐÐÄÆ«ÒÆ
	float centerZ = mapIndex * 50.0f;  // µ±Ç°µØÍ¼µÄ Z ÖáÖÐÐÄÆ«ÒÆ
	float halfSizeX = mapSize.x * 0.5f;
	float halfSizeZ = mapSize.z * 0.5f;

	// ±éÀú mapGrid£¬¼ÇÂ¼ËùÓÐÖµÎª 1 µÄÎ»ÖÃ
	for (int i = 0; i < 30; ++i) {
		for (int j = 0; j < 30; ++j) {
			if (mapGrids[mapIndex][i][j] == 1) {
				// ×ª»»ÎªÊÀ½ç×ø±ê
				float worldX = centerX + (-halfSizeX + (i + 0.5f) * gridSize);
				float worldZ = centerZ + (-halfSizeZ + (j + 0.5f) * gridSize);
				validSpawnPositions.emplace_back(worldX, 0.2f, worldZ);
			}
		}
	}

	//Èç¹ûÃ»ÓÐÓÐÐ§Éú³Éµã£¬Ö±½Ó·µ»Ø
	if (validSpawnPositions.empty()) {
		std::cerr << "No valid spawn positions in mapIndex: " << mapIndex << std::endl;
		return;
	}

	//Ëæ»úÉú³É8µ½12¸öÓÍÍ°
	std::uniform_int_distribution<int> numBarrelsDist(8, 12);
	int numBarrels = numBarrelsDist(gen);

	// Ëæ»ú´òÂÒÓÐÐ§Éú³Éµã
	std::shuffle(validSpawnPositions.begin(), validSpawnPositions.end(), gen);

	for (GameObject* barrel : Barrels) {
		sceneManager->RemoveObjectFromScene(barrel);
	}
	Barrels.clear(); //Çå¿ÕÖ®Ç°µÄÓÍÍ°
	renderPrepare->RemoveBarrel(); //Çå¿ÕäÖÈ¾


	for (int i = 0; i < std::min(numBarrels, static_cast<int>(validSpawnPositions.size())); ++i) {
		Vector3 spawnPos = validSpawnPositions[i];
		GameObject* newBarrel = sceneManager->mapBarrelPreform(spawnPos, Quaternion(),
			BulletShapeType::Box, Vector3(1, 1, 1), Vector3(1, 1, 1), 0);
		Barrels.push_back(newBarrel);
		renderPrepare->AddBarrel(newBarrel->GetRenderObject());
	}
	pistol->SetBarrels(Barrels);
}

void TutorialGame::HandleBarrelExplode(const Vector3& position) {

	const float EXPLOSION_RADIUS = 3.0f;    //ÉËº¦·¶Î§°ë¾¶
	const float EXPLOSION_DAMAGE = 60.0f;   //»ù´¡ÉËº¦
	const float DAMAGE_FALLOFF_RATE = 0.5f;    //¾àÀëË¥¼õÂÊ

	effekseerManager->PlayEffect(
		ResourceManager::explodeEffect, position.x, position.y, position.z);
	AudioManager::PlaySound("boom");

	for (auto it = Barrels.begin(); it != Barrels.end();) {
		GameObject* barrel = *it;
		Vector3 barrelPos = barrel->GetTransform().GetPosition();

		if (Vector::Length(barrelPos - position) < 0.01f) {
			RenderObject* barrelRender = barrel->GetRenderObject();

			//±éÀú¹ÖÎï
			for (auto obj : sceneManager->GetAllGameObjects()) {
				if (auto monster = dynamic_cast<Monster*>(obj)) {
					if (monster->IsDead()) continue; //Ìø¹ýÒÑËÀÍö¹ÖÎï

					Vector3 monsterPos = monster->GetTransform().GetPosition();
					float distance = Vector::Length(monsterPos - barrelPos);

					//ÉËº¦Ëæ¾àÀëË¥¼õ
					if (distance < EXPLOSION_RADIUS) {
						float damageRatio = 1.0f - (distance / EXPLOSION_RADIUS) * DAMAGE_FALLOFF_RATE;
						float finalDamage = EXPLOSION_DAMAGE * damageRatio;

						btVector3 startPos(barrelPos.x, barrelPos.y, barrelPos.z);
						btVector3 endPos(monsterPos.x, monsterPos.y, monsterPos.z);
						monster->HandleHurt(finalDamage, startPos, endPos); 
					}
				}
			}

			//Ïú»ÙÓÍÍ°
			sceneManager->RemoveObjectFromScene(barrel);
			it = Barrels.erase(it);
			if (barrelRender) {
				renderPrepare->RemoveSingleBarrel(barrelRender);
			}
		}
		else {
			++it;
		}
	}
}

//Óöµ½Ë®Ãæ¼õËÙ
float BaseGame::HandleGroundForce(float Force) {
	Vector3 playerPos = player->GetTransform().GetPosition();
	Vector3 waterPos = Vector3(105.5f, -0.5f, 106.5f);
	float distance = Vector::Length(playerPos - waterPos);

	if (distance < 8.5f) {
		return Force;
	}
	else {
		return 200.0f;
	}
}

void TutorialGame::GetWeapon() {
	//WeaponCollected×÷Îª±êÊ¶·û£¬Ö»´´½¨ºÍÊ°È¡Ò»´ÎÎäÆ÷
	if (WeaponCollected != switchMapCount) {
		if (switchMapCount == 1) {
			laserGun = sceneManager->LaserGunPreform(Vector3(50, 0, 50), Quaternion(), BulletShapeType::Sphere, Vector3(0.3, 0.3, 0.3), Vector3(0.5, 0.5, 0.5), 1.0f);
			laserGun->Initialize(3, 50.0f, 20.0f, 2.0f);
			renderPrepare->AddWeapon(laserGun->GetRenderObject());
			EquipWeapon(laserGun);
			WeaponCollected = switchMapCount;
		}
		if (switchMapCount == 2) {
			// 测试引力枪
			gravityGun = sceneManager->GravityGunPreform(Vector3(150, 0, 150), Quaternion(), BulletShapeType::Sphere, Vector3(0.3, 0.3, 0.3), Vector3(0.5, 0.5, 0.5), 1.0f);
			gravityGun->Initialize(4, 5.0f, 3.0f, 5.0f, 1.5f, 300.0f, sceneManager);
			renderPrepare->AddWeapon(gravityGun->GetRenderObject());
			EquipWeapon(gravityGun);
			WeaponCollected = switchMapCount;
		}
		if (switchMapCount == 3) {
			// 测试霰弹枪
			shotGun = sceneManager->ShotGunPreform(Vector3(100, 0, 100), Quaternion(), BulletShapeType::Sphere, Vector3(0.3, 0.3, 0.3), Vector3(0.5, 0.5, 0.5), 1.0f);
			shotGun->Initialize(2, 20.0f, 7.0f, 0.8f, 8, 20.0f);
			renderPrepare->AddWeapon(shotGun->GetRenderObject());
			EquipWeapon(shotGun);
			WeaponCollected = switchMapCount;
		}
		if (switchMapCount == 4) {
			bounceGun = sceneManager->BounceGunPreform(Vector3(200, 0.1, 200), Quaternion(), BulletShapeType::Sphere, Vector3(0.3, 0.3, 0.3), Vector3(0.1, 0.1, 0.1), 0.0f);
			bounceGun->Initialize(50.0f, 6.0f, 0.3f, 15, sceneManager);
			renderPrepare->AddWeapon(bounceGun->GetRenderObject());
			EquipWeapon(bounceGun);
			WeaponCollected = switchMapCount;
		}
	}
}

void TutorialGame::SkillEnhancement() {
	// 进入第四关时初始化
	if (switchMapCount == 4 && !m_InMap4) {
		m_InMap4 = true;
		m_ShockwaveCount = 0;
		m_LastShockwaveTime = glfwGetTime(); // 记录进入时间
	}

	// 离开第四关时重置
	if (switchMapCount != 4 && m_InMap4) {
		m_InMap4 = false;
		m_ShockwaveCount = 0;
	}

	// 在第四关时执行自动释放
	if (m_InMap4 && m_ShockwaveCount < MAX_SHOCKWAVES) {
		double currentTime = glfwGetTime();

		// 首次立即触发，后续间隔3秒
		if ((currentTime - m_LastShockwaveTime) >= SHOCKWAVE_INTERVAL) {
			TriggerShockwave();

			// 更新状态
			m_LastShockwaveTime = currentTime;
			m_ShockwaveCount++;

			// 调试输出
			std::cout << "第 " << m_ShockwaveCount << " 次震荡波已释放 ("
				<< currentTime << ")" << std::endl;
		}
	}
}