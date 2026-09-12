#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"
#include "Config.h" 

using namespace NCL;
using namespace CSC8503;
using namespace OpenGL;

NetworkedGame::NetworkedGame(GLFWwindow* window, OpenGL::GameTechRenderer* renderer, OpenGL::ResourceManager* resourceManager)
    : BaseGame(window, renderer, resourceManager) {
    // 初始化网络游戏特有的属性
    thisServer = nullptr;
    thisClient = nullptr;
    isNetworkActive = false;
    timeToNextPacket = 0.0f;
    packetsToSnapshot = 0;
    networkUpdateRate = 60.0f; // 60Hz更新频率
    fullSnapshotIntervalFrames = 120; // 每120帧发送一次完整状态
    player = nullptr;

    NetworkBase::Initialise();

    // 生成当前游戏实例的唯一ID：时间戳(高24位) + 随机数(低8位)
    instanceID = ((static_cast<uint32_t>(std::time(nullptr)) & 0xFFFFFF) << 8) |
        (static_cast<uint8_t>(std::rand() % 256));

    // std::cout << "Game instance ID: " << instanceID << std::endl;

    AttemptConnection();
}

NetworkedGame::~NetworkedGame() {
    if (thisClient) {
        thisClient->SafeDisconnect();
        delete thisClient;
        thisClient = nullptr;
    }

    // 清理服务器
    if (thisServer) {
        delete thisServer;
        thisServer = nullptr;
    }

    // 释放网络对象
    for (auto& netObj : networkObjects) {
        delete netObj;
    }
    networkObjects.clear();
}

void NetworkedGame::AttemptConnection() {
    std::cout << "Attempting to connect to the server..." << std::endl;
    GameClient* tempClient = new GameClient();
    bool connectionInitiated = tempClient->Connect(127, 0, 0, 1, NetworkBase::GetDefaultPort());

    if (!connectionInitiated) {
        std::cout << "Failed to initiate connection request, switching to server mode..." << std::endl;
        delete tempClient;
        StartAsServer();
        return;
    }

    // 设置一个短暂的超时，尝试连接
    float connectionTimeout = 1.0f; // 1秒超时
    float timer = 0.0f;
    bool connectionEstablished = false;

    while (timer < connectionTimeout) {
        // 检查是否有连接成功事件
        if (tempClient->CheckForConnectEvent()) {
            connectionEstablished = true;
            break;
        }
        // 休眠一小段时间避免CPU过载
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        timer += 0.1f;
    }
    // 根据连接结果决定角色
    if (connectionEstablished) {
        //std::cout << "成功连接到服务器！" << std::endl;

        // 安全地释放临时客户端
        tempClient->SafeDisconnect();
        delete tempClient;

        // 创建新的客户端实例
        StartAsClient(127, 0, 0, 1);
    }
    else {
        std::cout << "Unable to connect to an existing server, switching to server mode..." << std::endl;

        // 安全地释放临时客户端
        tempClient->SafeDisconnect();
        delete tempClient;

        StartAsServer();
    }
}

void NetworkedGame::InitWorld() {
    if (!sceneManager) {
        //std::cerr << "错误：sceneManager 为空！" << std::endl;
        return;
    }

    // 清除现有场景
    sceneManager->ClearAndErasePhysics();

    mapGrids.resize(1);
    mapGrids[0] = ResourceManager::map1Grid;
    // mapGrid = ResourceManager::map1Grid; 
    
    // 创建地面
    sceneManager->EmptyPreform(
        Vector3(0, -0.75, 0),
        Quaternion(),
        BulletShapeType::Box,
        Vector3(0.001, 0.001, 0.001),
        mapSize,
        0.0f
    );
    playerMode = true;
	// isPaused = false;

    CreateArrow();
    StartMonsterGeneration();
}

void NetworkedGame::StartAsServer() {
    // 先清除可能存在的客户端连接
    if (thisClient) {
        thisClient->SafeDisconnect();
        delete thisClient;
        thisClient = nullptr;
    }

    if (thisServer) {
        delete thisServer;  // 清理之前可能存在的服务器实例
    }

    thisServer = new GameServer(NetworkBase::GetDefaultPort(), 4, this);
    thisServer->RegisterPacketHandler(Received_State, this);
    thisServer->RegisterPacketHandler(Player_Update, this);
    thisServer->RegisterPacketHandler(Player_Connected, this);
    thisServer->RegisterPacketHandler(Monster_Death, this);

    isNetworkActive = true;
    StartLevel();
}

void NetworkedGame::StartAsClient(char a, char b, char c, char d) {

    // 先清除可能存在的服务器
    if (thisServer) {
        delete thisServer;
        thisServer = nullptr;
    }

    if (thisClient) {
        thisClient->SafeDisconnect();
        delete thisClient;  // 清理之前可能存在的客户端实例
    }

    thisClient = new GameClient(this);
    bool connected = thisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

    if (!connected) {
        std::cerr << "Error: Failed to connect to server!" << std::endl;
        delete thisClient;
        thisClient = nullptr;
        return;
    }

    thisClient->RegisterPacketHandler(Delta_State, this);
    thisClient->RegisterPacketHandler(Full_State, this);
    thisClient->RegisterPacketHandler(Player_Connected, this);
    thisClient->RegisterPacketHandler(Player_Disconnected, this);
    thisClient->RegisterPacketHandler(Player_Update, this);
    thisClient->RegisterPacketHandler(Monster_Spawn, this);
    thisClient->RegisterPacketHandler(Monster_Death, this);

    isNetworkActive = true;
    StartLevel();
}

void NetworkedGame::UpdateGame(float dt) {
    static float verificationTimer = 0.0f;
    verificationTimer += dt;

    // 每5秒验证一次远程玩家状态
    if (verificationTimer > 30.0f) {
        ValidateRemotePlayers();
        verificationTimer = 0.0f;
    }

    // 更新按键状态
    UpdateKeys();

    // 更新网络状态
    if (isNetworkActive) {
        UpdateNetworkState(dt);
    }

    // 更新游戏时间
    gameTime += dt;
    this->dt = dt;

    // 更新物理系统
    UpdatePhysics(dt);

    // 如果是玩家模式，更新相机位置
    if (playerMode && player) {
        SetPlayerCamera();
        ClampMousePosition();
        PlayerControl();
        UpdatePlayer(dt);
        UpdateFlowFieldArrows();
    }

    // 更新武器跟随玩家
	UpdateWeapon(pistol, player);

    // 更新渲染系统
    UpdateRender(dt);

    if (thisServer) {
		// 只在服务器端生成怪物和更新寻路
        UpdateMonsterGeneration(dt);
        for (auto tiger : sceneManager->GetMonsters()) {
            UpdateEnemyMovement(static_cast<Monster*>(tiger), dt);
            // 更新状态
            static_cast<Monster*>(tiger)->UpdateState(dt);
            static_cast<Monster*>(tiger)->MonsterUpdate(dt);
        }
    }
}

void NetworkedGame::UpdateNetworkState(float dt) {
    if (!isNetworkActive) return;

    timeToNextPacket -= dt;
    if (timeToNextPacket < 0) {
        if (thisServer) {
            UpdateAsServer(dt);
        }
        else if (thisClient) {
            UpdateAsClient(dt);
        }
        timeToNextPacket += 1.0f / networkUpdateRate;
    }

    // 服务器需要处理连接请求
    if (thisServer) {
        thisServer->UpdateServer();
    }

    // 客户端需要处理服务器响应
    if (thisClient) {
        thisClient->UpdateClient();
    }
}

void NetworkedGame::UpdateAsServer(float dt) {
    // 服务器当前状态ID
    static int serverStateID = 0;
    serverStateID++;

    packetsToSnapshot--;
    if (packetsToSnapshot < 0) {
        BroadcastSnapshot(false, serverStateID); // 发送完整状态
        packetsToSnapshot = fullSnapshotIntervalFrames;
    }
    else {
        BroadcastSnapshot(true, serverStateID);  // 发送增量状态
    }
}

void NetworkedGame::UpdateAsClient(float dt) {
    if (!thisClient || !player) 
        return;
    // 创建并发送玩家位置更新包
    PlayerUpdatePacket updatePacket;
    updatePacket.playerID = localPlayerNetworkID;
    updatePacket.playerPosition = player->GetTransform().GetPosition();
    updatePacket.playerOrientation = player->GetTransform().GetOrientation();
    // 获取本地玩家对应的武器并添加其位置和旋转
    auto weaponIt = playerWeaponMap.find(localPlayerNetworkID);
    if (weaponIt != playerWeaponMap.end() && weaponIt->second) {
        Weapon* playerWeapon = weaponIt->second;
        updatePacket.weaponPosition = playerWeapon->GetTransform().GetPosition();
        updatePacket.weaponOrientation = playerWeapon->GetTransform().GetOrientation();
    }
    thisClient->SendPacket(updatePacket);
}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame, int currentStateID) {
    // DEBUG OUTPUT
    static int broadcastCounter = 0;
    // if (++broadcastCounter % 60 == 0) { // 每30次广播才输出一次
    //     std::cout << "服务器: 广播" << (deltaFrame ? "增量" : "完整") << "状态更新..." << std::endl;
    // }

    std::vector<GameObject*> gameObjects = sceneManager->GetAllGameObjects();
    for (auto gameObject : gameObjects) {
        NetworkObject* netObj = gameObject->GetNetworkObject();
        if (!netObj) {
            continue;
        }

        // 确定每个客户端的状态ID
        int playerState = 0;
        for (auto& id : stateIDs) {
            playerState = std::max(playerState, id.second);
        }

        GamePacket* newPacket = nullptr;
        if (netObj->WritePacket(&newPacket, deltaFrame, currentStateID)) {
            thisServer->SendGlobalPacket(*newPacket);
            delete newPacket;
        }
    }
}

void NetworkedGame::UpdateMinimumState() {
    // 定期从服务器移除旧数据
    int minID = INT_MAX;
    int maxID = 0;

    for (auto i : stateIDs) {
        minID = std::min(minID, i.second);
        maxID = std::max(maxID, i.second);
    }

    // 所有客户端都已确认达到至少minID状态
    // 因此我们可以删除任何旧状态
    std::vector<GameObject*> gameObjects = sceneManager->GetAllGameObjects();

    for (auto gameObject : gameObjects) {
        NetworkObject* netObj = gameObject->GetNetworkObject();
        if (!netObj) {
            continue;
        }
        netObj->UpdateStateHistory(minID);
    }
}

void NetworkedGame::UpdateMonsterGeneration(float deltaTime) {
    // 如果当前不在生成怪物的状态，跳过
    if (!isSpawningMonsters) return;
    // 检查玩家数量是否大于1
    if (netPlayerMap.size() <= 1) return;

    // 增加经过的时间
    timePassed += deltaTime;
    totalTimePassed += deltaTime;

    // 已生成的波数
    static int waveCount = 0;

    // 每隔 spawnInterval 时间生成一批怪物
    if (timePassed >= spawnInterval) {
        SpawnTigers(0); // 生成到地图索引0的位置
        timePassed = 0.0f; // 重置时间，开始计算下一个间隔
        waveCount++; // 增加已生成的波数
    }
    // 如果已生成指定时间（spawnDuration），停止生成
    //if (totalTimePassed >= spawnDuration) {
    //    std::cout << "停止生成怪物，总共生成 " << waveCount << " 波" << std::endl;
    //    isSpawningMonsters = false; // 停止生成怪物
    //    waveCount = 0; // 重置波数计数器，以便下次从头开始
    //}
}

void NetworkedGame::SpawnTigers(int mapIndex) {
    std::random_device rd;
    std::mt19937 gen(rd());
    // 确保 mapIndex 合法
    if (mapIndex < 0 || mapIndex >= mapGrids.size() || mapGrids[mapIndex].empty()) {
        // std::cerr << "[ERROR] Invalid mapIndex: " << mapIndex << std::endl;
        return;
    }
    if (mapIndex != 3) {
        // 存储值为 1 的有效生成点
        std::vector<Vector3> validSpawnPositions;
        // 确保 switchMapCount 在合法范围内
        float gridSize = mapSize.x / 30.0f;
        float centerX = mapIndex * 50.0f;  // 当前地图的 X 轴中心偏移
        float centerZ = mapIndex * 50.0f;  // 当前地图的 Z 轴中心偏移
        float halfSizeX = mapSize.x * 0.5f;
        float halfSizeZ = mapSize.z * 0.5f;

        // 遍历 mapGrid，记录所有值为 1 的位置
        for (int i = 0; i < 30; ++i) {
            for (int j = 0; j < 30; ++j) {
                if (mapGrids[mapIndex][i][j] == 1) {
                    // 转换为世界坐标
                    float worldX = centerX + (-halfSizeX + (i + 0.5f) * gridSize);
                    float worldZ = centerZ + (-halfSizeZ + (j + 0.5f) * gridSize);
                    validSpawnPositions.emplace_back(worldX, 0.2f, worldZ);
                }
            }
        }
        // 如果没有有效生成点，直接返回
        if (validSpawnPositions.empty()) {
            // std::cerr << "[WARNING] No valid spawn positions in mapIndex: " << mapIndex << std::endl;
            return;
        }
        // 随机生成老虎
        std::uniform_int_distribution<int> numTigersDist(3, 5);
        int numTigers = numTigersDist(gen);
        // 随机打乱有效生成点
        std::shuffle(validSpawnPositions.begin(), validSpawnPositions.end(), gen);
        // 生成老虎，最多生成 validSpawnPositions.size() 和 numTigers 中较小的值
        std::uniform_real_distribution<float> eliteChanceDist(0.0f, 1.0f);
        for (int i = 0; i < std::min(numTigers, static_cast<int>(validSpawnPositions.size())); ++i) {
            Vector3 spawnPos = validSpawnPositions[i];
            Monster* tiger = nullptr;
            bool isElite = false;
            float health;
            float damage;
            // 20% 概率生成精英老虎
            if (eliteChanceDist(gen) < 0.2f) {
                isElite = true;
                health = 50.0f;
                damage = 20.0f;
                tiger = sceneManager->Enemy_EliteTigerPreform(
                    spawnPos, Quaternion(), BulletShapeType::Box,
                    Vector3(0.8, 0.8, 0.8), Vector3(0.8, 0.8, 0.8),
                    1.5f, health, damage);
            }
            else {
                health = 30.0f;
                damage = 10.0f;
                // 80% 概率生成普通老虎
                tiger = sceneManager->Enemy_TigerPreform(
                    spawnPos, Quaternion(), BulletShapeType::Box,
                    Vector3(0.5, 0.5, 0.5), Vector3(0.5, 0.8, 0.5),
                    1.0f, health, damage);
            }

            // 为怪物创建网络对象
            int monsterNetID = GenerateUniqueNetworkID();
            NetworkObject* netObj = new NetworkObject(*tiger, monsterNetID);
            networkObjects.push_back(netObj);
            tiger->SetNetworkObject(netObj);
            // 将老虎添加到怪物管理器中
            sceneManager->AddToMonsters(tiger);

            // 广播怪物生成消息
            MonsterSpawnPacket spawnPacket;
            spawnPacket.monsterID = monsterNetID;
            spawnPacket.position = spawnPos;
            spawnPacket.health = health;
            spawnPacket.damage = damage;
            spawnPacket.isElite = isElite;

            thisServer->SendGlobalPacket(spawnPacket);
        }
    }
}

void NetworkedGame::NotifyMonsterDeath(Monster* monster, int killerPlayerID) {
    if (!monster || !monster->GetNetworkObject()) return;

    int monsterNetID = monster->GetNetworkObject()->GetNetworkID();

    // 如果是客户端，向服务器发送消息
    if (thisClient) {
        MonsterDeathPacket deathPacket;
        deathPacket.monsterID = monsterNetID;
        deathPacket.killerPlayerID = killerPlayerID != -1 ? killerPlayerID : localPlayerNetworkID;

        thisClient->SendPacket(deathPacket);
    }
    // 如果是服务器，广播怪物死亡消息
    else if (thisServer) {
        MonsterDeathPacket deathPacket;
        deathPacket.monsterID = monsterNetID;
        deathPacket.killerPlayerID = killerPlayerID;

        thisServer->SendGlobalPacket(deathPacket);
    }
}


void NetworkedGame::SpawnPlayer(bool isLocalPlayer, int sourceNetworkID, int weaponNetID) {
    int playerNetID; // 全局唯一的网络ID
    Vector3 spawnPos = Vector3(0, 0.5, 0); // 生成初始位置

    Player* newPlayer = sceneManager->NetworkedPlayerPreform(
        spawnPos,
        Quaternion(),
        BulletShapeType::Sphere,
        Vector3(0.5f, 0.5f, 0.5f),
        Vector3(0.5f, 0.5f, 0.5f),
        1.0f
    );
    if (isLocalPlayer) {
        playerNetID = GenerateUniqueNetworkID();
        this->player = newPlayer;
		localPlayerNetworkID = playerNetID;
        std::cout << "Creating [local] player, NetworkID: " << playerNetID << std::endl;
        Weapon* newPistol = CreateWeaponForPlayer(playerNetID, 1, newPlayer);

        if (isLocalPlayer && newPistol) {
            pistol = static_cast<SingleHitWeapon*>(newPistol);
            weaponNetID = newPistol->GetNetworkObject()->GetNetworkID();
        }
    }
    else {
        playerNetID = sourceNetworkID;
        std::cout << "Creating [remote-mapped] player, NetworkID: " << playerNetID << std::endl;
        Weapon* newPistol = CreateWeaponForPlayer(playerNetID, 1, newPlayer, weaponNetID);
    }

    netPlayerMap[playerNetID] = newPlayer;
    NetworkObject* netObj = new NetworkObject(*newPlayer, playerNetID);
    networkObjects.push_back(netObj);
    newPlayer->SetNetworkObject(netObj);

    // 广播新玩家连接事件
    NewPlayerPacket connectPacket;
    connectPacket.playerID = playerNetID;
    connectPacket.weaponID = weaponNetID;
    connectPacket.position = spawnPos;

    if (thisServer) {
        // 此时战局中并没有其他玩家连接，因此无需广播
        // thisServer->SendGlobalPacket(connectPacket);
	} else if (thisClient) {
        thisClient->SendPacket(connectPacket);
    }
}

Weapon* NetworkedGame::CreateWeaponForPlayer(int playerNetID, int weaponType, Player* owner, int weaponNetID) {
    Weapon* newWeapon = nullptr;
    Vector3 weaponPos = owner->GetTransform().GetPosition() + Vector3(1, 0, 1);

    // 根据武器类型创建不同的武器
    switch (weaponType) {
    case 1: // 手枪
        newWeapon = sceneManager->PistolPreform(
            weaponPos, Quaternion(),
            BulletShapeType::Sphere,
            Vector3(0.3f, 0.3f, 0.3f),
            Vector3(0.5f, 0.5f, 0.5f),
            1.0f
        );
        if (newWeapon) {
            static_cast<SingleHitWeapon*>(newWeapon)->Initialize(1, 15.0f, 5.0f, 0.2f);
        }
        break;

    case 2: // 霰弹枪
        newWeapon = sceneManager->ShotGunPreform(
            weaponPos, Quaternion(),
            BulletShapeType::Sphere,
            Vector3(0.3f, 0.3f, 0.3f),
            Vector3(0.5f, 0.5f, 0.5f),
            1.0f
        );
        if (newWeapon) {
            static_cast<WideHitWeapon*>(newWeapon)->Initialize(2, 5.0f, 4.0f, 0.8f, 8, 20.0f);
        }
        break;

    case 3: // 激光枪
        newWeapon = sceneManager->LaserGunPreform(
            weaponPos, Quaternion(),
            BulletShapeType::Sphere,
            Vector3(0.3f, 0.3f, 0.3f),
            Vector3(0.5f, 0.5f, 0.5f),
            1.0f
        );
        if (newWeapon) {
            static_cast<LaserWeapon*>(newWeapon)->Initialize(3, 30.0f, 20.0f, 2.0f);
        }
        break;

    case 4: // 弹射枪
        newWeapon = sceneManager->BounceGunPreform(
            weaponPos, Quaternion(),
            BulletShapeType::Sphere,
            Vector3(0.3f, 0.3f, 0.3f),
            Vector3(0.5f, 0.5f, 0.5f),
            1.0f
        );
        if (newWeapon) {
            static_cast<BounceWeapon*>(newWeapon)->Initialize(5.0f, 6.0f, 0.5f, 2, sceneManager);
        }
        break;
    }

    if (newWeapon) {
        renderPrepare->AddWeapon(newWeapon->GetRenderObject());

        // 为武器创建网络对象
        int finalWeaponNetID = (weaponNetID > 0) ? weaponNetID : GenerateUniqueNetworkID();
        NetworkObject* netObj = new NetworkObject(*newWeapon, finalWeaponNetID);
        networkObjects.push_back(netObj);
        newWeapon->SetNetworkObject(netObj);

        std::cout << "Creating weapon for player " << playerNetID << " (Type: " << weaponType << "), WeaponID: " << finalWeaponNetID << std::endl;
        EquipWeapon(newWeapon);
        playerWeaponMap[playerNetID] = newWeapon;
    }

    return newWeapon;
}

void NetworkedGame::StartLevel() {
    InitWorld();

    if (thisServer) {
        // 服务器特定初始化
        std::cout << "Server: Initializing network scene..." << std::endl;
        SpawnPlayer(true);
    }
    else if (thisClient) {
        // 客户端特定初始化
        std::cout << "Client: Initializing network scene..." << std::endl;
        // SpawnPlayer推迟到成功收到ENET_EVENT_TYPE_CONNECT事件后
        // SpawnPlayer(true);
    }
}

int NetworkedGame::GenerateUniqueNetworkID() const {
    static uint16_t nextID = 0;

    // 获取当前时间的秒级时间戳（用于可读性，只需要最近的几秒）
    uint32_t timestamp = static_cast<uint32_t>(std::time(nullptr) & 0x7FFF); // 使用15位，保留符号位为0

    // 组合时间戳(15位) + 实例ID(8位) + 序列号(8位)
    int uniqueID = (timestamp << 16) | ((instanceID & 0xFF) << 8) | (nextID++ & 0xFF);

    return uniqueID;
}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
    switch (type) {
    case BasicNetworkMessages::Delta_State: {
        // 处理增量状态更新
        DeltaPacket* dp = (DeltaPacket*)payload;

		// DEBUG OUTPUT
        // 只在特定情况下输出（例如每50个包输出一次）
        // static int deltaPacketCounter = 0;
        // bool shouldLog = (++deltaPacketCounter % 120 == 0);
        // if (shouldLog) {
        //     std::cout << "Client: Processing delta state packet, ID=" << dp->networkID << std::endl;
        // }

        // DEBUG OUTPUT
        bool foundObject = false;
        bool stateApplied = false;

        // 跳过本地玩家的更新
        if (dp->networkID == localPlayerNetworkID) {
            // if (shouldLog) {
            //     std::cout << "Client: Skipping local player state update, ID=" << dp->networkID << std::endl;
            // }
            foundObject = true;
            // 仍然需要确认接收到了数据包
            ClientPacket stateConfirm;
            stateConfirm.lastID = dp->stateID;
            thisClient->SendPacket(stateConfirm);
            break;
        }

        // 检查是否为本地玩家的武器
        auto weaponIt = playerWeaponMap.find(localPlayerNetworkID);
        if (weaponIt != playerWeaponMap.end() && weaponIt->second &&
            weaponIt->second->GetNetworkObject() &&
            weaponIt->second->GetNetworkObject()->GetNetworkID() == dp->networkID) {
            // if (shouldLog) {
            //     std::cout << "Client: Skipping local player weapon state update, ID=" << dp->networkID << std::endl;
            // }
            foundObject = true;
            // 仍然需要确认接收到了数据包
            ClientPacket stateConfirm;
            stateConfirm.lastID = dp->stateID;
            thisClient->SendPacket(stateConfirm);
            break;
        }

        // 查找相应的网络对象
        std::vector<GameObject*> gameObjects = sceneManager->GetAllGameObjects();
        for (auto gameObject : gameObjects) {
            NetworkObject* netObj = gameObject->GetNetworkObject();
            if (netObj && netObj->GetNetworkID() == dp->networkID) {
                // netObj->ReadPacket(*dp);
                
				// DEBUG OUTPUT
                stateApplied = netObj->ReadPacket(*dp);
                // 只在失败时记录
                // if (!stateApplied && shouldLog) {
                //     std::cout << "Client Warning: Object ID=" << dp->networkID << " failed to apply delta state update" << std::endl;
                // }

                foundObject = true;
                break;
            }
        }

		// DEBUG OUTPUT
        // if (!foundObject) {
        //     std::cout << "Client Warning: Could not find object with NetworkID=" << dp->networkID << " to apply state update" << std::endl;
        // }

        // 如果成功应用当前状态，则向服务器确认收到状态
        if (thisClient && stateApplied) {
            ClientPacket stateConfirm;
            stateConfirm.lastID = dp->stateID;
            thisClient->SendPacket(stateConfirm);
        }
        break;
    }
    case BasicNetworkMessages::Full_State: {
        // 处理完整状态更新
        FullPacket* fp = (FullPacket*)payload;

        // DEBUG OUTPUT
        // 只在特定情况下输出（例如每50个包输出一次）
        // static int deltaPacketCounter = 0;
        // bool shouldLog = (++deltaPacketCounter % 50 == 0);
        // if (shouldLog) {
        //     std::cout << "Client: Processing delta state packet, ID=" << fp->networkID << std::endl;
        // }

        // DEBUG OUTPUT
        bool foundObject = false;

        // 跳过本地玩家的更新
        if (fp->networkID == localPlayerNetworkID) {
            // if (shouldLog) {
            //     std::cout << "Client: Skipping local player state update, ID=" << fp->networkID << std::endl;
            // }
            foundObject = true;
            // 仍然需要确认接收到了数据包
            ClientPacket stateConfirm;
            stateConfirm.lastID = fp->fullState.stateID;
            thisClient->SendPacket(stateConfirm);
            break;
        }

        // 检查是否为本地玩家的武器
        auto weaponIt = playerWeaponMap.find(localPlayerNetworkID);
        if (weaponIt != playerWeaponMap.end() && weaponIt->second &&
            weaponIt->second->GetNetworkObject() &&
            weaponIt->second->GetNetworkObject()->GetNetworkID() == fp->networkID) {
            // if (shouldLog) {
            //     std::cout << "Client: Skipping local player weapon state update, ID=" << fp->networkID << std::endl;
            // }
            foundObject = true;
            // 仍然需要确认接收到了数据包
            ClientPacket stateConfirm;
            stateConfirm.lastID = fp->fullState.stateID;
            thisClient->SendPacket(stateConfirm);
            break;
        }

        // 查找相应的网络对象
        std::vector<GameObject*> gameObjects = sceneManager->GetAllGameObjects();
        for (auto gameObject : gameObjects) {
            NetworkObject* netObj = gameObject->GetNetworkObject();
            if (netObj && netObj->GetNetworkID() == fp->networkID) {
				if (netObj->GetNetworkID() == localPlayerNetworkID) {
					// 本地玩家不应更新自己的状态
					continue;
				}
                // netObj->ReadPacket(*fp);
                
				// DEBUG OUTPUT
                bool success = netObj->ReadPacket(*fp);
                
                // 只在失败时记录
                /*if (!success && shouldLog) {
                    std::cout << "Client Warning: Object ID=" << fp->networkID << " failed to apply delta state update" << std::endl;
                }*/

                foundObject = true;
                break;
            }
        }

        // DEBUG OUTPUT
        /*if (!foundObject) {
            std::cout << "Client Warning: Could not find object with NetworkID=" << fp->networkID << " to apply state update" << std::endl;
        }*/

        // 向服务器确认收到状态
        if (thisClient) {
            ClientPacket stateConfirm;
            stateConfirm.lastID = fp->fullState.stateID;
            thisClient->SendPacket(stateConfirm);
        }
        break;
    }
    case BasicNetworkMessages::Received_State: {
        // 服务器收到客户端的状态确认
        ClientPacket* cp = (ClientPacket*)payload;
        stateIDs[source] = cp->lastID;

        // 更新最小状态（所有客户端都已确认的）
        UpdateMinimumState();
        break;
    }
    case BasicNetworkMessages::Player_Connected: {
        // 服务器端与客户端在建立连接时会接收到多条重复的Player_Connected消息，可能是由于ENet底层的
        // 消息重传机制。此处需要加入玩家ID是否已在字典中的判断，避免重复生成玩家或重复广播。
        // std::cout << "Processing Player_Connected message, source: " << source << ", payload size: " << payload->size << std::endl;

        NewPlayerPacket* npp = static_cast<NewPlayerPacket*>(payload);
        if (thisServer) {
            // std::cout << "Server: Received player connection event, NetworkID: " << npp->playerID << ", WeaponID: " << npp->weaponID << std::endl;
			// TODO: SpawnPlayer应该接受position参数，此处应该传递npp->position
            if (netPlayerMap.find(npp->playerID) == netPlayerMap.end()) {
                SpawnPlayer(false, npp->playerID, npp->weaponID);
                std::cout << "Server: Created remote player, NetworkID: " << npp->playerID << std::endl;
                // 向新玩家发送已有玩家的信息
                for (const auto& pair : netPlayerMap) {
                    // 不发送新玩家自己的信息
                    if (pair.first != source) {
                        NewPlayerPacket existingPlayerInfo;
                        existingPlayerInfo.playerID = pair.first;
                        existingPlayerInfo.position = pair.second->GetTransform().GetPosition();
                        // 添加玩家武器ID
                        auto weaponIt = playerWeaponMap.find(pair.first);
                        if (weaponIt != playerWeaponMap.end() && weaponIt->second &&
                            weaponIt->second->GetNetworkObject()) {
                            existingPlayerInfo.weaponID = weaponIt->second->GetNetworkObject()->GetNetworkID();
                        }
                        else {
                            existingPlayerInfo.weaponID = -1;
                        }

                        thisServer->SendPacket(source, existingPlayerInfo);
                        // std::cout << "Server: Sending existing player " << pair.first << " info to new player (PeerID) " << source << std::endl;
                    }
                }
				// 向所有连接的客户端广播新玩家加入游戏消息
                NewPlayerPacket addNewPlayer;
                addNewPlayer.playerID = npp->playerID;
                addNewPlayer.weaponID = npp->weaponID;
                addNewPlayer.position = npp->position;
                thisServer->SendGlobalPacket(addNewPlayer, source);
                // std::cout << "Server: Broadcasting new player (ID: " << npp->playerID << ") join message" << std::endl;
            }
        }
        else if (thisClient) {
            if (npp->playerID != localPlayerNetworkID) { // 不为自己创建远程玩家
                // 客户端本地创建远程玩家
                if (netPlayerMap.find(npp->playerID) == netPlayerMap.end()) {
                    SpawnPlayer(false, npp->playerID, npp->weaponID);
                    std::cout << "Client: Created remote player ID: " << npp->playerID << ", WeaponID: " << npp->weaponID << std::endl;
                }
            }
        }
        break;
    }
    case BasicNetworkMessages::Player_Update: {
        PlayerUpdatePacket* pup = static_cast<PlayerUpdatePacket*>(payload);
        
        if (thisServer) {
            // 广播给所有其他客户端
            thisServer->SendGlobalPacket(*payload, source); // 排除发送者
        }

        auto it = netPlayerMap.find(pup->playerID);
        if (it != netPlayerMap.end()) {
            // 更新远程玩家位置
            Player* remotePlayer = it->second;
            if (remotePlayer && remotePlayer->GetPhysicsObject()) {
                remotePlayer->GetPhysicsObject()->BTSetPosition(pup->playerPosition);
                remotePlayer->GetPhysicsObject()->BTSetRotation(pup->playerOrientation);
            }
            // 更新玩家武器位置和旋转
            auto weaponIt = playerWeaponMap.find(pup->playerID);
            if (weaponIt != playerWeaponMap.end() && weaponIt->second) {
                Weapon* playerWeapon = weaponIt->second;
                if (playerWeapon && playerWeapon->GetPhysicsObject()) {
                    playerWeapon->GetPhysicsObject()->BTSetPosition(pup->weaponPosition);
                    playerWeapon->GetPhysicsObject()->BTSetRotation(pup->weaponOrientation);
                }
            }
        }
        break;
    }
    case BasicNetworkMessages::Player_Disconnected: {
        // 使用自定义断开连接包来避免使用data成员
        int disconnectedPlayerID = source;  // 使用source参数代替从data中读取

        if (thisServer) {
            auto it = netPlayerMap.find(disconnectedPlayerID);
            if (it != netPlayerMap.end()) {
                // 清理玩家的武器
                auto weaponIt = playerWeaponMap.find(disconnectedPlayerID);
                if (weaponIt != playerWeaponMap.end()) {
                    Weapon* weapon = weaponIt->second;
                    if (weapon) {
                        renderPrepare->RemoveWeapon(weapon->GetRenderObject());
                        sceneManager->RemoveObjectFromScene(weapon);
                    }
                    playerWeaponMap.erase(weaponIt);
                }
                // 从场景中移除玩家
                sceneManager->RemoveObjectFromScene(it->second);
                // 从映射中移除
                netPlayerMap.erase(it);
                // 广播玩家断开连接消息
                ServerPacket disconnectMsg;
                disconnectMsg.type = Player_Disconnected;
                thisServer->SendGlobalPacket(disconnectMsg);
            }
        }
        else if (thisClient) {
            auto it = netPlayerMap.find(disconnectedPlayerID);
            if (it != netPlayerMap.end()) {
                // 清理玩家的武器
                auto weaponIt = playerWeaponMap.find(disconnectedPlayerID);
                if (weaponIt != playerWeaponMap.end()) {
                    Weapon* weapon = weaponIt->second;
                    if (weapon) {
                        renderPrepare->RemoveWeapon(weapon->GetRenderObject());
                        sceneManager->RemoveObjectFromScene(weapon);
                    }
                    playerWeaponMap.erase(weaponIt);
                }
                // 从场景中移除
                sceneManager->RemoveObjectFromScene(it->second);
                // 从映射中移除
                netPlayerMap.erase(it);
                std::cout << "Player " << disconnectedPlayerID << " has disconnected" << std::endl;
            }
        }
        break;
    }
    case BasicNetworkMessages::Monster_Spawn: {
        MonsterSpawnPacket* msp = static_cast<MonsterSpawnPacket*>(payload);

        // 如果是客户端，根据收到的怪物信息在本地生成怪物
        if (thisClient) {
            Monster* tiger = nullptr;

            // 根据是否精英怪物选择生成方式
            if (msp->isElite) {
                tiger = sceneManager->Enemy_EliteTigerPreform(
                    msp->position, Quaternion(), BulletShapeType::Box,
                    Vector3(0.8, 0.8, 0.8), Vector3(0.8, 0.8, 0.8),
                    1.5f, msp->health, msp->damage);
            }
            else {
                tiger = sceneManager->Enemy_TigerPreform(
                    msp->position, Quaternion(), BulletShapeType::Box,
                    Vector3(0.5, 0.5, 0.5), Vector3(0.5, 0.8, 0.5),
                    1.0f, msp->health, msp->damage);
            }

            // 为怪物创建网络对象，使用指定的网络ID
            NetworkObject* netObj = new NetworkObject(*tiger, msp->monsterID);
            networkObjects.push_back(netObj);
            tiger->SetNetworkObject(netObj);

            // 将怪物添加到管理器
            sceneManager->AddToMonsters(tiger);
        }
        break;
    }
    case BasicNetworkMessages::Monster_Death: {
        MonsterDeathPacket* mdp = static_cast<MonsterDeathPacket*>(payload);

        // 查找对应的怪物
        Monster* targetMonster = nullptr;
        for (auto monster : sceneManager->GetMonsters()) {
            if (monster->GetNetworkObject() &&
                monster->GetNetworkObject()->GetNetworkID() == mdp->monsterID) {
                targetMonster = monster;
                break;
            }
        }

        if (targetMonster) {
            // 如果是服务器，广播怪物死亡消息
            if (thisServer) {
                // 排除发送方避免重复广播
                thisServer->SendGlobalPacket(*payload, source);
            }

            // 如果还没死亡，处理死亡逻辑
            if (!targetMonster->IsDead()) {
                // 强制设置生命值为0
                targetMonster->SetHealth(0);
                // 触发死亡处理
                targetMonster->HandleDeath();
            }
        }
        break;
    }
    default:{}
    }
}

void NetworkedGame::UpdateKeys() {
    // 基本按键状态更新
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 玩家模式切换
    if (isKeyJustPressed(GLFW_KEY_P)) {
        if (player) {
            playerMode = !playerMode;
        }
        if (playerMode)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void NetworkedGame::ValidateRemotePlayers() {
    if (netPlayerMap.empty()) {
        std::cout << "Currently no online players" << std::endl;
        return;
    }
    std::cout << "There are " << netPlayerMap.size() << " online players in the game" << std::endl;
	for (const auto& pair : netPlayerMap) {
		std::cout << "Online Player ID: " << pair.first << std::endl;
	}
}
