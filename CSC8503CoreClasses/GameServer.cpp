#include "GameServer.h"
#include "./enet/enet.h"
#include "math.h"
using namespace NCL::CSC8503;
using namespace NCL::Maths;

GameServer::GameServer(int onPort, int maxClients, NetworkedGame* game) {
	port = onPort;
	clientMax = maxClients;
	clientCount = 0;
	netHandle = nullptr;
	this->game = game;
	Initialise();
}

GameServer::~GameServer() {
	Shutdown();
}

void GameServer::Shutdown() {
	SendGlobalPacket(BasicNetworkMessages::Shutdown);
	enet_host_destroy(netHandle);
	netHandle = nullptr;
}

bool GameServer::Initialise() {
	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = port;

	netHandle = enet_host_create(&address, clientMax, 1, 0, 0);

	if (!netHandle) {
		std::cout << __FUNCTION__ << "failed to create network handle!" << std::endl;
		return false;
	}

	return true;
}

bool GameServer::SendGlobalPacket(int msgID) {
	GamePacket packet;
	packet.type = msgID;
	return SendGlobalPacket(packet);
}

bool GameServer::SendGlobalPacket(GamePacket& packet) {
	ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);
	enet_host_broadcast(netHandle, 0, dataPacket);
	// std::cout << "服务器: 发送包类型: " << packet.type << ", 大小: " << packet.size << std::endl;
	return true;
}

bool GameServer::SendGlobalPacket(GamePacket& packet, int excludePeerID) {
	// 创建数据包
	ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);

	// 遍历所有对等点，除了被排除的那个
	for (size_t i = 0; i < netHandle->peerCount; ++i) {
		if (netHandle->peers[i].incomingPeerID != excludePeerID) {
			enet_peer_send(&netHandle->peers[i], 0, dataPacket);
		}
	}

	return true;
}

bool GameServer::SendPacket(int peerID, GamePacket& payload) {
	// 创建数据包
	ENetPacket* dataPacket = enet_packet_create(&payload, payload.GetTotalSize(), 0);

	// 查找特定的peer
	for (size_t i = 0; i < netHandle->peerCount; ++i) {
		if (netHandle->peers[i].incomingPeerID == peerID) {
			enet_peer_send(&netHandle->peers[i], 0, dataPacket);
			return true;
		}
	}

	// 如果找不到该peer，销毁数据包
	enet_packet_destroy(dataPacket);
	return false;
}

void GameServer::UpdateServer() {
	if (!netHandle) return;
	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0) {
		int type = event.type;
		ENetPeer* p = event.peer;
		int peer = p->incomingPeerID;
		if (type == ENetEventType::ENET_EVENT_TYPE_CONNECT) {
			// std::cout << "Handling event.type: ENET_EVENT_TYPE_CONNECT" << std::endl;
		}
		else if (type == ENetEventType::ENET_EVENT_TYPE_DISCONNECT) {
			std::cout << "Server: A client has disconnected" << std::endl;
		}
		else if (type == ENetEventType::ENET_EVENT_TYPE_RECEIVE) {
			// std::cout << "Handling event.type: ENET_EVENT_TYPE_RECEIVE" << std::endl;
			GamePacket* packet = (GamePacket*)event.packet->data;
			ProcessPacket(packet, peer);
		}
		enet_packet_destroy(event.packet);
	}
}
