#include "NetworkBase.h"
#include "./enet/enet.h"
NetworkBase::NetworkBase()	{
	netHandle = nullptr;
}

NetworkBase::~NetworkBase()	{
	if (netHandle) {
		enet_host_destroy(netHandle);
	}
}

void NetworkBase::Initialise() {
	enet_initialize();
}

void NetworkBase::Destroy() {
	enet_deinitialize();
}

bool NetworkBase::ProcessPacket(GamePacket* packet, int peerID) {
    // 获取包类型
    int type = packet->type;
    
    // 查找为该类型消息注册的处理器
    PacketHandlerIterator first, last;
    if (!GetPacketHandlers(type, first, last)) {
        // 没有处理器处理这种类型的消息
        // std::cout << "No handlers for packet type: " << type << std::endl;
        return false;
    }
    
    // 调用所有注册的处理器
    bool handled = false;
    for (auto i = first; i != last; ++i) {
        i->second->ReceivePacket(type, packet, peerID);
        handled = true;
    }
    
    return handled;
}