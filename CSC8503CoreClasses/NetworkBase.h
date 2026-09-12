#pragma once
// #include "./enet/enet.h"
struct _ENetHost;
struct _ENetPeer;
struct _ENetEvent;

using namespace NCL::Maths;


enum BasicNetworkMessages {
	None,
	String_Message,
	Delta_State,	//1 byte per channel since the last state
	Full_State,		//Full transform etc
	Received_State, //received from a client, informs that its received packet n
	Player_Connected,
	Player_Disconnected,
	Player_Update,
	Shutdown,
	Monster_Spawn,   //怪物生成消息
	Monster_Death    //怪物死亡消息
};

struct GamePacket {
	short size;
	short type;

	GamePacket() {
		type = BasicNetworkMessages::None;
		size = 0;
	}

	GamePacket(short type) : GamePacket() {
		this->type = type;
	}

	int GetTotalSize() {
		return sizeof(GamePacket) + size;
	}
};

// 用于传输纯文字消息的数据包
struct StringPacket : public GamePacket {
	char stringData[256];

	StringPacket(const std::string& message) {
		type = BasicNetworkMessages::String_Message;
		size = (short)message.length();
		memcpy(stringData, message.data(), size);
	}

	std::string GetStringFromData() {
		std::string realString(stringData);
		realString.resize(size);
		return realString;
	}
};

struct ClientPacket : public GamePacket {
	int lastID;

	ClientPacket() : lastID(0) {
		type = Received_State;
		size = sizeof(int);
	}
};

struct ServerPacket : public GamePacket {
	ServerPacket() {
		type = None;
		size = 0;
	}
};

// 新玩家连接数据包
struct NewPlayerPacket : public GamePacket {
	int playerID;
	int weaponID;
	Vector3 position;

	NewPlayerPacket() {
		type = Player_Connected;
		size = sizeof(int) * 2 + sizeof(Vector3);
	}
};

// 玩家位置更新数据包
struct PlayerUpdatePacket : public GamePacket {
	int playerID;
	Vector3 playerPosition;
	Vector3 weaponPosition;
	Quaternion playerOrientation;
	Quaternion weaponOrientation;

	PlayerUpdatePacket() {
		type = Player_Update;
		// 将playerID类型 从 int(4字节) 改为 uint64_t(8字节)后，玩家旋转无法正确传输，原因是
		// 内存布局变更导致的数据解析错误。解决办法是通过在结构体的size中填充4字节使内存布局与原
		// 来保持一致。此处改回了int类型以保证实现上的简洁。
		size = sizeof(int) + sizeof(Vector3) * 2 + sizeof(Quaternion) * 2;
	}
};

// 怪物生成数据包
struct MonsterSpawnPacket : public GamePacket {
	int monsterID;          // 怪物的网络ID
	Vector3 position;       // 怪物位置
	float health;           // 怪物生命值
	float damage;           // 怪物伤害值
	bool isElite;           // 是否精英怪物

	MonsterSpawnPacket() {
		type = Monster_Spawn;
		size = sizeof(int) + sizeof(Vector3) + sizeof(float) * 2 + sizeof(bool);
	}
};

// 怪物死亡数据包
struct MonsterDeathPacket : public GamePacket {
	int monsterID;          // 死亡怪物的网络ID
	int killerPlayerID;     // 击杀怪物的玩家ID (如果是-1表示非玩家击杀)

	MonsterDeathPacket() {
		type = Monster_Death;
		size = sizeof(int) * 2;
	}
};

class PacketReceiver {
public:
	virtual void ReceivePacket(int type, GamePacket* payload, int source = -1) = 0;
};

class NetworkBase {
public:
	static void Initialise();
	static void Destroy();

	static int GetDefaultPort() {
		return 1234;
	}

	void RegisterPacketHandler(int msgID, PacketReceiver* receiver) {
		packetHandlers.insert(std::make_pair(msgID, receiver));
	}
protected:
	NetworkBase();
	~NetworkBase();

	bool ProcessPacket(GamePacket* p, int peerID = -1);

	typedef std::multimap<int, PacketReceiver*>::const_iterator PacketHandlerIterator;

	bool GetPacketHandlers(int msgID, PacketHandlerIterator& first, PacketHandlerIterator& last) const {
		auto range = packetHandlers.equal_range(msgID);

		if (range.first == packetHandlers.end()) {
			return false; //no handlers for this message type!
		}
		first = range.first;
		last = range.second;
		return true;
	}

	_ENetHost* netHandle;

	std::multimap<int, PacketReceiver*> packetHandlers;
};