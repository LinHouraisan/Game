#include "GameClient.h"
#include "./enet/enet.h"
using namespace NCL::CSC8503;
using namespace NCL::Maths;

GameClient::GameClient(NetworkedGame* game) {
	netHandle = enet_host_create(nullptr, 1, 1, 0, 0);
	this->game = game;
}

GameClient::~GameClient() {
	// enet_host_destroy(netHandle);
    
    // 确保在析构前安全断开连接
    SafeDisconnect();

    if (netHandle) {
        enet_host_destroy(netHandle);
        netHandle = nullptr;
    }
}
//参数为4位ip地址和端口号
bool GameClient::Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum) {
	ENetAddress address;
	address.port = portNum;
	address.host = (d << 24) | (c << 16) | (b << 8) | (a);
	netPeer = enet_host_connect(netHandle, &address, 2, 0);

	return netPeer != nullptr;
}

void GameClient::UpdateClient() {
	if (netHandle == nullptr) return;

	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0) {
        int type = event.type;
        ENetPeer* p = event.peer;
        int peer = p->incomingPeerID;
		if (event.type == ENET_EVENT_TYPE_CONNECT) {
			// std::cout << "Handling event.type: ENET_EVENT_TYPE_CONNECT" << std::endl;
            // Client只保持与服务器的单点双向连接，此处ENET_EVENT_TYPE_CONNECT
			// 只在与服务器连接成功时触发，随后其他玩家的加入并不会触发，因此直接生成本地玩家
            // TODO: 后续有必要做防止重复生成的判断
			game->SpawnPlayer(true);
		}
		else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            // std::cout << "Handling event.type: ENET_EVENT_TYPE_RECEIVE" << std::endl;

            // DEBUG OUTPUT
            /*std::cout << "客户端网络层: 收到数据包，大小="
                << event.packet->dataLength << "字节" << std::endl;*/

            GamePacket* packet = (GamePacket*)event.packet->data;

			// DEBUG OUTPUT
            /*std::cout << "客户端网络层: 解析为类型=" << packet->type
                << "，大小=" << packet->size << "字节的游戏包" << std::endl;*/

            ProcessPacket(packet, peer);
		}
		enet_packet_destroy(event.packet);
	}
}

void GameClient::SendPacket(GamePacket& payload) {
	ENetPacket* dataPacket = enet_packet_create(&payload, payload.GetTotalSize(), 0);
	enet_peer_send(netPeer, 0, dataPacket);
    // std::cout << "客户端: 发送包类型: " << payload.type << ", 大小: " << payload.size << std::endl;
}

bool GameClient::CheckForConnectEvent() {
    if (netHandle == nullptr) return false;

    ENetEvent event;
    while (enet_host_service(netHandle, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_CONNECT) {
            std::cout << "Connected to server!" << std::endl;
            enet_packet_destroy(event.packet); // 处理可能的包数据
            return true;
        }
        else if (event.packet) {
            enet_packet_destroy(event.packet); // 处理其他包，但不改变状态
        }
    }

    return false;
}

void GameClient::SafeDisconnect() {
    if (netHandle != nullptr && netPeer != nullptr &&
        netPeer->state == ENET_PEER_STATE_CONNECTED) {

        // 优雅地断开连接
        enet_peer_disconnect(netPeer, 0);

        // 等待断开确认或超时
        ENetEvent event;
        bool disconnected = false;

        // 最多等待100ms
        for (int i = 0; i < 10; i++) {
            while (enet_host_service(netHandle, &event, 10) > 0) {
                if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                    disconnected = true;
                    break;
                }

                if (event.packet) {
                    enet_packet_destroy(event.packet);
                }
            }

            if (disconnected) break;
        }

        // 如果优雅断开失败，强制断开
        if (!disconnected) {
            enet_peer_reset(netPeer);
        }

        netPeer = nullptr;
    }
}

int GameClient::GetPeerID() const {
    // 如果netPeer存在，则返回其incomingPeerID作为客户端ID
    return netPeer ? netPeer->incomingPeerID : -1;
}