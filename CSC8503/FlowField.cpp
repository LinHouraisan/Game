#include "TutorialGame.h"

using namespace NCL;
using namespace CSC8503;
using namespace OpenGL;

void BaseGame::CreateArrow() {
    // 清除当前所有的箭头，避免重复生成
    for (GameObject* arrow : gridArrows) {
        sceneManager->RemoveObjectFromScene(arrow);
    }
    gridArrows.clear();

    float gridSize = mapSize.x / 30.0f;
    int numCellsX = 30;
    int numCellsZ = 30;
    float halfSizeX = mapSize.x * 0.5f;
    float halfSizeZ = mapSize.z * 0.5f;

    // 确保 switchMapCount 在合法范围内
    if (switchMapCount < 0 || switchMapCount >= mapGrids.size()) {
        return;
    }

    int mapIndex = switchMapCount; // 只处理当前关卡的箭头

    // 如果当前地图网格为空，跳过
    if (mapGrids[mapIndex].empty()) {
        return;
    }

    float centerX = mapIndex * 50.0f;
    float centerZ = mapIndex * 50.0f;

    for (int i = 0; i < numCellsX; i++) {
        for (int j = 0; j < numCellsZ; j++) {
            float x = centerX + (-halfSizeX + (i + 0.5f) * gridSize);
            float z = centerZ + (-halfSizeZ + (j + 0.5f) * gridSize);

            // 如果网格值为 0，创建障碍物
            if (mapGrids[mapIndex][i][j] == 0) {
                GameObject* obstacle = sceneManager->EmptyPreform(
                    Vector3(x, 0, z), Quaternion(),
                    BulletShapeType::Box,
                    Vector3(gridSize, 1.0f, gridSize),
                    Vector3(gridSize, 1.0f, gridSize),
                    0.0f
                );
                continue;
            }

            // 生成箭头
            GameObject* arrow = sceneManager->ArrowPreform(
                Vector3(x, 0, z), Quaternion(), BulletShapeType::Sphere,
                Vector3(0.2, 0.2, 0.2), Vector3(0, 0, 0), 1.0f
            );
            //std::cout << "Arrow at (" << x << ", " << z << ") for mapGrid[" << mapIndex << "]" << std::endl;

            arrow->GetRenderObject()->SetIsActive(false);
            gridArrows.push_back(arrow);
        }
    }
}


void BaseGame::UpdateFlowFieldArrows() {
    if (!player || gridArrows.empty()) return;

    Vector3 playerPos = player->GetTransform().GetPosition();
    float gridSize = mapSize.x / 30.0f;
    float halfSizeX = mapSize.x / 2.0f;
    float halfSizeZ = mapSize.z / 2.0f;

    flowFieldDirections.resize(5, std::vector<std::vector<Vector3>>(30,
        std::vector<Vector3>(30, Vector3(0, 0, 0))));

    for (int mapIndex = 0; mapIndex < 5; mapIndex++) {
        float centerX = mapIndex * 50.0f;
        float centerZ = mapIndex * 50.0f;


        if (playerPos.x < centerX - halfSizeX || playerPos.x > centerX + halfSizeX ||
            playerPos.z < centerZ - halfSizeZ || playerPos.z > centerZ + halfSizeZ) {
            continue;
        }

        Vector3 localPlayerPos = playerPos - Vector3(centerX, 0, centerZ);
        int playerGridX = std::clamp(static_cast<int>((localPlayerPos.x + halfSizeX) / gridSize), 0, 29);
        int playerGridZ = std::clamp(static_cast<int>((localPlayerPos.z + halfSizeZ) / gridSize), 0, 29);

        std::vector<std::vector<int>> heatMap(30, std::vector<int>(30, INT_MAX));
        for (int i = 0; i < 30; ++i) {
            for (int j = 0; j < 30; ++j) {
                if (mapGrids[mapIndex][i][j] == 0) heatMap[i][j] = INT_MAX;
            }
        }

        using GridCost = std::pair<int, std::pair<int, int>>;
        std::priority_queue<GridCost, std::vector<GridCost>, std::greater<GridCost>> queue;

        heatMap[playerGridX][playerGridZ] = 0;
        queue.push({ 0, {playerGridX, playerGridZ} });

        while (!queue.empty()) {
            auto current = queue.top();
            queue.pop();
            int x = current.second.first;
            int z = current.second.second;

            for (int dx = -1; dx <= 1; dx++) {
                for (int dz = -1; dz <= 1; dz++) {
                    if (dx == 0 && dz == 0) continue;

                    int nx = x + dx;
                    int nz = z + dz;
                    if (nx < 0 || nx >= 30 || nz < 0 || nz >= 30) continue;

                    int cost = (dx != 0 && dz != 0) ? 14 : 10;
                    int newCost = heatMap[x][z] + cost;

                    if (mapGrids[mapIndex][nx][nz] == 1 && newCost < heatMap[nx][nz]) {
                        heatMap[nx][nz] = newCost;
                        queue.push({ newCost, {nx, nz} });
                    }
                }
            }
        }

        for (GameObject* arrow : gridArrows) {
            Vector3 arrowPos = arrow->GetTransform().GetPosition();
            if (arrowPos.x < centerX - halfSizeX || arrowPos.x > centerX + halfSizeX ||
                arrowPos.z < centerZ - halfSizeZ || arrowPos.z > centerZ + halfSizeZ) {
                continue;
            }

            float localX = arrowPos.x - centerX;
            float localZ = arrowPos.z - centerZ;
            int gridX = std::clamp(static_cast<int>((localX + halfSizeX) / gridSize), 0, 29);
            int gridZ = std::clamp(static_cast<int>((localZ + halfSizeZ) / gridSize), 0, 29);

            int minCost = INT_MAX;
            Vector3 minDir(0, 0, 0);

            for (int dx = -1; dx <= 1; dx++) {
                for (int dz = -1; dz <= 1; dz++) {
                    if (dx == 0 && dz == 0) continue;

                    int nx = gridX + dx;
                    int nz = gridZ + dz;
                    if (nx < 0 || nx >= 30 || nz < 0 || nz >= 30) continue;

                    if (heatMap[nx][nz] < minCost) {
                        minCost = heatMap[nx][nz];
                        minDir = Vector::Normalise(Vector3(dx, 0, dz));
                    }
                }
            }

            Quaternion targetRot = Quaternion::FromTwoVectors(Vector3(1, 0, 0), minDir);
            UniformRotationInterpolation(arrow, targetRot, 5);

            flowFieldDirections[mapIndex][gridX][gridZ] = targetRot * Vector3(1, 0, 0);
        }
    }
}


Vector3 BaseGame::SnapTo8Directions(const Vector3& rawDir) {
	if (Vector::Length(rawDir) < 0.01f) return rawDir; // 零向量不处理

	// 计算角度并四舍五入到45度的倍数
	float angle = atan2(rawDir.z, rawDir.x); // 弧度（范围：-π到π）
	float snappedAngle = round(angle / (PI * 0.25f)) * (PI * 0.25f);

	// 转换为单位向量
	return Vector3(cos(snappedAngle), 0, sin(snappedAngle));
}

std::vector<std::vector<int>> TutorialGame::LoadMap(const std::string& filename) {
    std::vector<std::vector<int>> grid;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open: " << filename << std::endl;
        return grid; //返回空
    }

    std::string line;
    while (std::getline(file, line)) {
        std::vector<int> row;
        std::istringstream iss(line);
        int value;
        while (iss >> value) {
            row.push_back(value);
        }
        grid.push_back(row);
    }

    file.close();
    return grid; //返回二维地图
}

void TutorialGame::SpawnFinalTigers(int mapIndex) {
    std::random_device rd;
    std::mt19937 gen(rd());

    // 确保 mapIndex 合法
    if (mapIndex < 0 || mapIndex >= mapGrids.size() || mapGrids[mapIndex].empty()) {
        std::cerr << "[ERROR] Invalid mapIndex: " << mapIndex << std::endl;
        return;
    }

    // 获取玩家位置
    Vector3 playerPos = player->GetTransform().GetPosition();
    const float minDistanceFromPlayer = 5.0f; // 最小距离
    const float minDistanceSquared = minDistanceFromPlayer * minDistanceFromPlayer;

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
                Vector3 spawnPos(worldX, 0.2f, worldZ);

                // 检查与玩家的距离
                Vector3 offset = spawnPos - playerPos;
                float distanceSquared = offset.LengthSquared();

                if (distanceSquared >= minDistanceSquared) {
                    validSpawnPositions.emplace_back(spawnPos);
                }
            }
        }
    }

    // 如果没有有效生成点，直接返回
    if (validSpawnPositions.empty()) {
        return;
    }

    std::uniform_int_distribution<int> numTigersDist(30, 50);
    int numTigers = numTigersDist(gen);

    // 随机打乱有效生成点
    std::shuffle(validSpawnPositions.begin(), validSpawnPositions.end(), gen);

    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float X = std::round(dist(gen) * 10) / 10; // 保留一位小数
    float Y = std::round(dist(gen) * 10) / 10;
    float Z = std::round(dist(gen) * 10) / 10;
    float W = std::round(dist(gen) * 10) / 10;

    if (X < 0.6) {
        X = 0.6;
    }


    // 生成老虎，最多生成 validSpawnPositions.size() 和 numTigers 中较小的值
    std::uniform_real_distribution<float> eliteChanceDist(0.0f, 1.0f);
    for (int i = 0; i < std::min(numTigers, static_cast<int>(validSpawnPositions.size())); ++i) {
        Vector3 spawnPos = validSpawnPositions[i];

        Monster* tiger = nullptr;

        tiger = sceneManager->Final_TigerPreform(
            spawnPos, Quaternion(), BulletShapeType::Box,
            Vector3(X, X, X), Vector3(X, X, X),
            1.5f, 2, 2);
        tiger->GetRenderObject()->SetColorFactor(Vector4( X*3,Y * 4,Z * 2,W * 1));

        btRigidBody* btTiger = tiger->GetPhysicsObject()->GetBulletBody();
        btTiger->getBroadphaseHandle()->m_collisionFilterGroup = TIGER_GROUP;
        // 设置老虎的碰撞遮罩 - 与所有组碰撞，除了TIGER_GROUP
        btTiger->getBroadphaseHandle()->m_collisionFilterMask = ~TIGER_GROUP | RAYCAST_GROUP;

        // 将老虎添加到怪物管理器中
        sceneManager->AddToMonsters(tiger);
    }
}

void TutorialGame::SpawnFirstTigers(int mapIndex) {
    std::random_device rd;
    std::mt19937 gen(rd());

    // 确保 mapIndex 合法
    if (mapIndex < 0 || mapIndex >= mapGrids.size() || mapGrids[mapIndex].empty()) {
        std::cerr << "[ERROR] Invalid mapIndex: " << mapIndex << std::endl;
        return;
    }

    if (mapIndex == 0 || mapIndex == 1 || mapIndex == 2) {
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

        // 根据 mapIndex 设定不同的老虎生成范围
        std::uniform_int_distribution<int> numTigersDist(mapIndex == 1 ? 6 : 3, mapIndex == 1 ? 10 : 6);
        int numTigers = numTigersDist(gen);

        // 随机打乱有效生成点
        std::shuffle(validSpawnPositions.begin(), validSpawnPositions.end(), gen);

        // 生成老虎，最多生成 validSpawnPositions.size() 和 numTigers 中较小的值
        std::uniform_real_distribution<float> eliteChanceDist(0.0f, 1.0f);
        for (int i = 0; i < std::min(numTigers, static_cast<int>(validSpawnPositions.size())); ++i) {
            Vector3 spawnPos = validSpawnPositions[i];

            Monster* tiger = nullptr;

            // 20% 概率生成精英老虎
            if (eliteChanceDist(gen) < 0.2f) {
                tiger = sceneManager->Enemy_EliteTigerPreform(
                    spawnPos, Quaternion(), BulletShapeType::Box,
                    Vector3(0.8, 0.8, 0.8), Vector3(0.8, 0.8, 0.8),
                    1.5f, 50, 20);
            }
            else {
                // 80% 概率生成普通老虎
                tiger = sceneManager->Enemy_TigerPreform(
                    spawnPos, Quaternion(), BulletShapeType::Box,
                    Vector3(0.5, 0.5, 0.5), Vector3(0.5, 0.8, 0.5),
                    1.0f, 30, 10);
            }


            // 将老虎添加到怪物管理器中
            sceneManager->AddToMonsters(tiger);
        }
    }
}
void TutorialGame::SpawnBossTigers(int mapIndex) {
    if (mapIndex < 0 || mapIndex >= mapGrids.size() || mapGrids[mapIndex].empty()) {
        std::cerr << "[ERROR] Invalid mapIndex for boss spawn: " << mapIndex << std::endl;
        return;
    }
    if (mapIndex == 3) {
        Vector3 spawnPos(160.0f, 0.2f, 145.0f);

        Monster* tiger = sceneManager->Enemy_BossMonsterPreform(
            spawnPos, Quaternion(), BulletShapeType::Box,
           Vector3(2.0f, 2.0f, 2.0f),  // 较大体型
            Vector3(0.5, 0.5, 3.0),  // 较大碰撞体
            1.0f,                       // 质量
            500.0f,                     // 血量
            80.0f                       // 伤害
        );
        //Vector3 spawnPos2(145.0f, 0.2f, 145.0f);

        //// 生成第一只 Boss Tiger
        //Monster* bossTiger1 = sceneManager->Enemy_BossMonsterPreform(
        //    spawnPos1, Quaternion(), BulletShapeType::Box,
        //    Vector3(2.0f, 2.0f, 2.0f),  // 较大体型
        //    Vector3(0.5, 0.5, 3.0),  // 较大碰撞体
        //    1.0f,                       // 质量
        //    500.0f,                     // 血量
        //    80.0f                       // 伤害
        //);

        //// 生成第二只 Boss Tiger
        //Monster* bossTiger2 = sceneManager->Enemy_BossMonsterPreform(
        //    spawnPos2, Quaternion(), BulletShapeType::Box,
        //    Vector3(2.0f, 2.0f, 2.0f),  // 较大体型
        //    Vector3(0.5, 0.5, 3.0),  // 较大碰撞体
        //    1.0f,                       // 质量
        //    500.0f,                     // 血量
        //    80.0f                       // 伤害
        //);

        //// 添加到怪物管理器
        sceneManager->AddToMonsters(tiger);
        //sceneManager->AddToMonsters(bossTiger2);

        hasSpawnedBoss = true;
    }


}

void TutorialGame::SpawnSpeedTigers(int mapIndex) {
    std::random_device rd;
    std::mt19937 gen(rd());

    // 确保 mapIndex 合法
    if (mapIndex < 0 || mapIndex >= mapGrids.size() || mapGrids[mapIndex].empty()) {
        std::cerr << "[ERROR] Invalid mapIndex: " << mapIndex << std::endl;
        return;
    }

    if (mapIndex == 2 || mapIndex == 3) {
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

        // 随机生成 3-7 只老虎
        std::uniform_int_distribution<int> numTigersDist(15, 30);
        int numTigers = numTigersDist(gen);

        // 随机打乱有效生成点
        std::shuffle(validSpawnPositions.begin(), validSpawnPositions.end(), gen);

        // 生成老虎，最多生成 validSpawnPositions.size() 和 numTigers 中较小的值
        for (int i = 0; i < std::min(numTigers, static_cast<int>(validSpawnPositions.size())); ++i) {
            Vector3 spawnPos = validSpawnPositions[i];

            // 始终生成普通老虎
            Monster* tiger = sceneManager->Enemy_SpeedTigerPreform(
                spawnPos, Quaternion(), BulletShapeType::Box,
                Vector3(0.3, 0.3, 0.3), Vector3(0.2, 0.8, 0.2),
                0.75f, 15, 5);

            // 将老虎添加到怪物管理器中
            sceneManager->AddToMonsters(tiger);
        }
    }
}


void BaseGame::UpdateEnemyMovement(Monster* enemy, float dt) {
	Vector3 enemyPos = enemy->GetTransform().GetPosition();
	Vector3 direction = CalculateFlowDirection(enemyPos);

	PhysicsObject* physObj = enemy->GetPhysicsObject();
	physObj->SetLinearVelocity(Vector3(0, 0, 0)); // 清除惯性
	physObj->AddForce(direction * enemyMoveForce);

	// 调整敌人朝向
	if (Vector::Length(direction) > 0.1f) {
		Quaternion targetRot = Quaternion::FromTwoVectors(Vector3(0, 0, 1), Vector::Normalise(direction));
        UniformRotationInterpolation(enemy,targetRot,10);
	}
}

Vector3 BaseGame::CalculateFlowDirection(const Vector3& position) {
    float gridSize = mapSize.x / 30.0f;
    float halfSizeX = mapSize.x / 2.0f;
    float halfSizeZ = mapSize.z / 2.0f;

    for (int mapIndex = 0; mapIndex < 5; mapIndex++) {

        if (mapIndex >= mapGrids.size() || mapGrids[mapIndex].empty()) continue;

        float centerX = mapIndex * 50.0f;
        float centerZ = mapIndex * 50.0f;

        if (position.x >= centerX - halfSizeX && position.x <= centerX + halfSizeX &&
            position.z >= centerZ - halfSizeZ && position.z <= centerZ + halfSizeZ) {

            Vector3 localPos = position - Vector3(centerX, 0, centerZ);
            int x0 = std::clamp(static_cast<int>((localPos.x + halfSizeX - 0.5f) / gridSize), 0, 29);
            int z0 = std::clamp(static_cast<int>((localPos.z + halfSizeZ - 0.5f) / gridSize), 0, 29);

            float xFrac = fmod(localPos.x + halfSizeX - 0.5f, gridSize) / gridSize;
            float zFrac = fmod(localPos.z + halfSizeZ - 0.5f, gridSize) / gridSize;

            Vector3 dir00 = GetGridDirection(mapIndex, x0, z0);
            Vector3 dir01 = GetGridDirection(mapIndex, x0, z0 + 1);
            Vector3 dir10 = GetGridDirection(mapIndex, x0 + 1, z0);
            Vector3 dir11 = GetGridDirection(mapIndex, x0 + 1, z0 + 1);

            return Vector::Normalise(
                dir00 * (1 - xFrac) * (1 - zFrac) +
                dir10 * xFrac * (1 - zFrac) +
                dir01 * (1 - xFrac) * zFrac +
                dir11 * xFrac * zFrac
            );
        }
    }
    return Vector3(0, 0, 0);
}

Vector3 BaseGame::GetGridDirection(int mapIndex, int x, int z) {
    // 添加三重检查
    if (mapIndex < 0 || mapIndex >= mapGrids.size()) return Vector3(0, 0, 0);
    if (x < 0 || x >= mapGrids[mapIndex].size()) return Vector3(0, 0, 0);
    if (z < 0 || z >= mapGrids[mapIndex][x].size()) return Vector3(0, 0, 0);

    return (mapGrids[mapIndex][x][z] == 1) ?
        flowFieldDirections[mapIndex][x][z] : Vector3(0, 0, 0);
}

// 新增的旋转插值函数组
void BaseGame::StartRotationInterpolation(GameObject* obj, const Quaternion& targetRot, float duration) {
    if (duration <= 0) {
        obj->GetPhysicsObject()->BTSetRotation(targetRot);
        return;
    }

    rotatingObjects[obj] = {
        obj->GetTransform().GetOrientation(), // 当前旋转作为起点
        targetRot,
        duration,
        0.0f
    };
}

void BaseGame::RotationalInterpolation(float deltaTime) {
    std::vector<GameObject*> completedObjects;

    for (auto& [obj, data] : rotatingObjects) {
        data.elapsedTime += deltaTime;
        float t = std::min(data.elapsedTime / data.lerpTime, 1.0f);

        // 使用球面线性插值
        Quaternion currentRot = Quaternion::Slerp(data.startRot, data.targetRot, t);
        obj->GetPhysicsObject()->BTSetRotation(currentRot);

        if (t >= 1.0f) {
            completedObjects.push_back(obj);
        }
    }

    // 清理完成插值的对象
    for (auto* obj : completedObjects) {
        rotatingObjects.erase(obj);
    }
}

void BaseGame::UniformRotationInterpolation(GameObject* obj, const Quaternion& targetRot, int totalFrames) {
    Quaternion currentRot = obj->GetTransform().GetOrientation();

    // 修正目标旋转方向，确保选择最短路径
    Quaternion adjustedTarget = targetRot;
    if (Quaternion::Dot(currentRot, adjustedTarget) < 0) {
        adjustedTarget = adjustedTarget * -1.0f;
    }

    if (Quaternion::Dot(currentRot, adjustedTarget) > 0.999f) {
        obj->GetPhysicsObject()->BTSetRotation(adjustedTarget);
        return;
    }

    float step = 1.0f / totalFrames;
    Quaternion interpolatedRot = Quaternion::Slerp(currentRot, adjustedTarget, step);

    // 清除角速度后设置旋转
    obj->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
    obj->GetPhysicsObject()->BTSetRotation(interpolatedRot);
}

void BaseGame::StartMonsterGeneration() {
    isSpawningMonsters = true;  // 设置为生成怪物状态
    timePassed = 0.0f;          // 重置计时器
    totalTimePassed = 0.0f;     // 重置总计时器
}
void TutorialGame::UpdateMonsterGeneration(float deltaTime) {
	
    if (switchMapCount != 4) {
        // 如果当前不在生成怪物的状态，跳过
        if (!isSpawningMonsters) return;

        // 增加经过的时间
        timePassed += deltaTime;
        totalTimePassed += deltaTime;

        // 只生成一次Boss
        if (!hasSpawnedBoss) {
            SpawnBossTigers(switchMapCount); // 生成Boss
            //hasSpawnedBoss = true;     
        }
        // 每隔 spawnInterval 时间生成一批怪物
        if (timePassed >= spawnInterval) {
            //SpawnBossTigers(switchMapCount);
            SpawnFirstTigers(switchMapCount);
            SpawnSpeedTigers(switchMapCount);
            timePassed = 0.0f; // 重置时间，开始计算下一个间隔
        }

        // 如果已生成指定时间（spawnDuration），停止生成
        if (totalTimePassed >= spawnDuration) {
            isSpawningMonsters = false; // 停止生成怪物
        }
    }
    else {
        // 如果当前不在生成怪物的状态，跳过
        if (!isSpawningMonsters) return;

        // 增加经过的时间
        timePassed += deltaTime;
        totalTimePassed += deltaTime;

        // 每隔 spawnInterval 时间生成一批怪物
        if (timePassed >= spawnFinalInterval) {

            SpawnFinalTigers(switchMapCount);
            timePassed = 0.0f; // 重置时间，开始计算下一个间隔
        }

        // 如果已生成指定时间（spawnDuration），停止生成
        if (totalTimePassed >= spawnDuration) {
            isSpawningMonsters = false; // 停止生成怪物
            isWin = true;
        }
    }

    if (Monster::GetBossDeathStatus()) {
        std::cout << "Boss is dead! " << std::endl;
        // 这里可以触发胜利逻辑，比如传送到下一个关卡
    }
}