#pragma once
#include "NetworkBase.h"
#include <stdint.h>
#include <thread>
#include <atomic>
#include "NetworkedGame.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class GameClient : public NetworkBase {
		public:
			GameClient(NetworkedGame* game = nullptr);
			~GameClient();

			bool Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum);

			void SendPacket(GamePacket&  payload);

			void UpdateClient();

			// 检查是否有连接成功事件
			bool CheckForConnectEvent();

			// 安全断开连接
			void SafeDisconnect();

			// 获取客户端的ID（作为peer在服务器中的标识）
			int GetPeerID() const;

		protected:	
			_ENetPeer*	netPeer;
			NetworkedGame* game;
		};
	}
}

