#pragma once
#include "GameObject.h"
#include "GameClient.h"
#include "Player.h"

namespace NCL {
	namespace CSC8503 {
		class NetworkedGame;

		class NetworkPlayer : public Player {
		public:
			NetworkPlayer(NetworkedGame* game, int num);
			~NetworkPlayer();

			int GetPlayerNum() const {
				return playerNum;
			}

		protected:
			NetworkedGame* game;
			int playerNum;
		};
	}
}

