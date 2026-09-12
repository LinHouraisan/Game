#pragma once
#include "NetworkBase.h"
#include "NetworkedGame.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class GameServer : public NetworkBase {
		public:
			GameServer(int onPort, int maxClients, NetworkedGame* game = nullptr);
			~GameServer();

			bool Initialise();
			void Shutdown();

			bool SendGlobalPacket(int msgID);
			bool SendGlobalPacket(GamePacket& packet);
			// 添加支持排除某个peer的全局发送函数
			bool SendGlobalPacket(GamePacket& packet, int excludePeerID);
			// 添加发送到特定客户端的函数
			bool SendPacket(int peerID, GamePacket& payload);

			virtual void UpdateServer();

		protected:
			int			port;
			int			clientMax;
			int			clientCount;
			NetworkedGame*	game;

			int incomingDataRate;
			int outgoingDataRate;
		};
	}
}
