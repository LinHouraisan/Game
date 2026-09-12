#pragma once
#include "BaseGame.h"
#include "NetworkBase.h"
#include "NetworkState.h" 
#include <map>
#include <string>

namespace NCL {
    namespace CSC8503 {
        class GameServer;
        class GameClient;
        class NetworkPlayer;
        class NetworkObject;

        class NetworkedGame : public BaseGame, public PacketReceiver {
        public:
            NetworkedGame(GLFWwindow* window, OpenGL::GameTechRenderer* renderer, OpenGL::ResourceManager* resourceManager);
            ~NetworkedGame();

            void StartAsServer();
            void StartAsClient(char a, char b, char c, char d);

            void SpawnPlayer(bool isLocalPlayer, int sourceNetworkID = -1, int weaponNetID = -1);

            void UpdateGame(float dt) override;
            void ReceivePacket(int type, GamePacket* payload, int source = -1) override;

            bool IsFullyInitialized() const { return isFullyInitialized; }

            int GenerateUniqueNetworkID() const;

            void NotifyMonsterDeath(Monster* monster, int killerPlayerID = -1);

            int GetLocalPlayerNetworkID() const { return localPlayerNetworkID; }

            // 检查武器是否由本地玩家拥有
            bool IsWeaponOwnedByLocalPlayer(Weapon* weapon) const {
                if (!weapon || !weapon->GetNetworkObject()) return false;

                auto it = playerWeaponMap.find(localPlayerNetworkID);
                if (it != playerWeaponMap.end() && it->second == weapon) {
                    return true;
                }
                return false;
            }

        protected:
            void AttemptConnection();
            void InitWorld() override;
            void StartLevel();

            void UpdateAsServer(float dt);
            void UpdateAsClient(float dt);
            void UpdateNetworkState(float dt);

            void BroadcastSnapshot(bool deltaFrame, int currentStateID);
            void UpdateMinimumState();
            void SpawnTigers(int mapIndex);
            void UpdateMonsterGeneration(float deltaTime);

            GameServer* thisServer;
            GameClient* thisClient;

            float timeToNextPacket;
            int packetsToSnapshot;
            int fullSnapshotIntervalFrames;
            float networkUpdateRate;

            std::map<int, int> stateIDs;
            std::vector<NetworkObject*> networkObjects;

            std::vector<std::vector<int>> mapGrid; // 存储地图网格信息

            // 网络游戏特有属性
            bool isNetworkActive;
            bool isFullyInitialized = false; // 游戏是否已完全初始化标志
            
            void UpdateKeys();  // 更新按键状态
            // void UpdateAllWeapons(float dt); // 更新所有玩家的所有武器

            // 存储所有网络玩家对象，包括自身，键为玩家ID
            std::unordered_map<int, Player*> netPlayerMap;
            // 玩家武器映射：键为玩家的网络ID，值为该玩家拥有的武器
            std::unordered_map<int, Weapon*> playerWeaponMap;
            // 为指定玩家创建武器
            Weapon* CreateWeaponForPlayer(int playerNetID, int weaponType, Player* owner, int weaponNetID = -1);

            void ValidateRemotePlayers();

        private:

            uint32_t instanceID;  // 当前游戏实例的唯一ID，基于游戏窗口创建
            
            int localPlayerNetworkID; // 本地玩家的NetworkID

        };
    }
}