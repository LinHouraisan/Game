#pragma once
#include "GameObject.h"
#include "NetworkBase.h"
#include "NetworkState.h"

namespace NCL {
    namespace CSC8503 {
        struct FullPacket : public GamePacket {
            int networkID;
            NetworkState fullState;
            
            FullPacket() {
                type = Full_State;
                size = sizeof(int) + sizeof(NetworkState);
            }
        };
        
        struct DeltaPacket : public GamePacket {
            int networkID;
            int stateID;
            char pos[3];
            char orientation[4];
            
            DeltaPacket() {
                type = Delta_State;
                size = sizeof(int) * 2 + sizeof(char) * 7;
            }
        };

        class NetworkObject {
        public:
            NetworkObject(GameObject& o, int id);
            ~NetworkObject();
            
            // 获取此网络对象的ID
            int GetNetworkID() const {
                return networkID;
            }
            
            // 读取网络包并更新状态
            bool ReadPacket(GamePacket& p);
            
            // 创建网络包以发送状态
            bool WritePacket(GamePacket** p, bool deltaFrame, int stateID);
            
            // 获取最新的网络状态
            NetworkState& GetLatestNetworkState();
            
            // 清理旧状态历史
            void UpdateStateHistory(int minID);
            
        protected:
            bool ReadDeltaPacket(DeltaPacket &p);
            bool ReadFullPacket(FullPacket &p);
            
            bool WriteDeltaPacket(GamePacket** p, int stateID);
            bool WriteFullPacket(GamePacket** p, int stateID);
            
            bool GetNetworkState(int frameID, NetworkState& state);
            
            GameObject& object;
            
            NetworkState lastFullState;
            std::vector<NetworkState> stateHistory;
            
            int networkID;
            int lastStateID;
            
            int deltaErrors;
            int fullErrors;
        };
    }
}