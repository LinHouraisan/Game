#include "NetworkObject.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;


NetworkObject::NetworkObject(GameObject& o, int id) : object(o) {
    deltaErrors = 0;
    fullErrors = 0;
    networkID = id;
    lastStateID = 0; // 初始化状态ID
}

NetworkObject::~NetworkObject() {
}

bool NetworkObject::ReadPacket(GamePacket& p) {
	// DEBUG OUTPUT
    /*std::cout << "NetworkObject(ID=" << networkID << "): 尝试读取类型="
        << p.type << "的包" << std::endl;*/

    if (p.type == Full_State) {
        // return ReadFullPacket((FullPacket&)p);

		// DEBUG OUTPUT
        bool result = ReadFullPacket((FullPacket&)p);
        /*std::cout << "NetworkObject(ID=" << networkID << "): "
            << (result ? "成功" : "失败") << "应用完整状态更新" << std::endl;*/
        return result;
    }
    else if (p.type == Delta_State) {
        // return ReadDeltaPacket((DeltaPacket&)p);

		// DEBUG OUTPUT
        bool result = ReadDeltaPacket((DeltaPacket&)p);
        /*std::cout << "NetworkObject(ID=" << networkID << "): "
            << (result ? "成功" : "失败") << "应用增量状态更新" << std::endl;*/
        return result;
    }

    return false; // 不是我们关心的数据包类型
}

bool NetworkObject::WritePacket(GamePacket** p, bool deltaFrame, int stateID) {
    // DEBUG OUTPUT
    /*std::cout << "NetworkObject(ID=" << networkID << "): 尝试写入"
        << (deltaFrame ? "增量" : "完整") << "状态包，状态ID=" << stateID << std::endl;*/

    if (deltaFrame) {
        if (!WriteDeltaPacket(p, stateID)) {
            // DEBUG OUTPUT
            /*std::cout << "NetworkObject(ID=" << networkID
                << "): 创建增量包失败，改为创建完整包" << std::endl;*/

            return WriteFullPacket(p, stateID);
        }

        // DEBUG OUTPUT
        /*std::cout << "NetworkObject(ID=" << networkID
            << "): 成功创建增量包" << std::endl;*/

        return true;
    }
    // return WriteFullPacket(p);

	// DEBUG OUTPUT
    bool result = WriteFullPacket(p, stateID);
    /*std::cout << "NetworkObject(ID=" << networkID
        << "): " << (result ? "成功" : "失败") << "创建完整包" << std::endl;*/
    return result;
}

bool NetworkObject::ReadDeltaPacket(DeltaPacket &p) {
    // 获取上一个状态
    NetworkState previousState;
    if (!GetNetworkState(p.stateID - 1, previousState)) {
        // 如果没有上一个状态，无法应用增量更新
        deltaErrors++;
        return false;
    }
    
    // 从紧凑格式解析变化数据
    Vector3 posDelta;
    posDelta.x = ((float)p.pos[0]) / 127.0f; // 假设使用-127到127的范围
    posDelta.y = ((float)p.pos[1]) / 127.0f;
    posDelta.z = ((float)p.pos[2]) / 127.0f;
    
    Quaternion rotDelta;
    rotDelta.x = ((float)p.orientation[0]) / 127.0f;
    rotDelta.y = ((float)p.orientation[1]) / 127.0f;
    rotDelta.z = ((float)p.orientation[2]) / 127.0f;
    rotDelta.w = ((float)p.orientation[3]) / 127.0f;
    rotDelta.Normalise(); // 确保四元数是单位四元数
    
    // 计算新状态
    NetworkState newState;
    newState.stateID = p.stateID;
    newState.position = previousState.position + posDelta;
    newState.orientation = rotDelta * previousState.orientation;
	// 注：当前DeltaPacket中不处理速度信息
    /*if (object.GetPhysicsObject()) {
        object.GetPhysicsObject()->SetLinearVelocity(newState.linearVelocity);
        object.GetPhysicsObject()->SetAngularVelocity(newState.angularVelocity);
    }*/

    object.GetPhysicsObject()->BTSetPosition(newState.position);
    object.GetPhysicsObject()->BTSetRotation(newState.orientation);
    
    // 存储新状态
    if (stateHistory.size() > 60) { // 保持历史记录的合理大小
        stateHistory.erase(stateHistory.begin());
    }
    stateHistory.push_back(newState);
    
    return true;
}

bool NetworkObject::ReadFullPacket(FullPacket &p) {
    object.GetPhysicsObject()->BTSetPosition(p.fullState.position);
    object.GetPhysicsObject()->BTSetRotation(p.fullState.orientation);
    if (object.GetPhysicsObject()) {
        object.GetPhysicsObject()->SetLinearVelocity(p.fullState.linearVelocity);
        object.GetPhysicsObject()->SetAngularVelocity(p.fullState.angularVelocity);
    }
    
    // 存储状态
    lastFullState = p.fullState;
    // 更新历史记录
    if (stateHistory.size() > 60) {
        stateHistory.erase(stateHistory.begin());
    }
    stateHistory.push_back(p.fullState);
    
    return true;
}

bool NetworkObject::WriteDeltaPacket(GamePacket** p, int stateID) {
    // 获取当前状态
    NetworkState currentState;
    currentState.position = object.GetTransform().GetPosition();
    currentState.orientation = object.GetTransform().GetOrientation();
    currentState.stateID = stateID;
    
    // 检查历史状态
    NetworkState lastState;
    if (!GetNetworkState(stateID - 1, lastState)) {
        // 如果没有上次状态，无法创建增量更新
        return false;
    }
    
    // 计算位置和方向的变化
    Vector3 posDelta = currentState.position - lastState.position;
    Quaternion rotDelta = currentState.orientation * lastState.orientation.Conjugate();
    
    // 判断变化是否足够大，需要发送更新
    float posSquared = posDelta.LengthSquared();
    float rotAngle = abs(rotDelta.GetAngle());
    
    // 如果变化太小，不需要发送更新
    if (posSquared < 0.01f && rotAngle < 0.1f) {
        return false;
    }
    
    // 创建增量包
    DeltaPacket* deltaPacket = new DeltaPacket();
    deltaPacket->networkID = this->networkID;
    deltaPacket->stateID = stateID;
    
    // 压缩位置变化到紧凑格式
    deltaPacket->pos[0] = (char)(posDelta.x * 127.0f);
    deltaPacket->pos[1] = (char)(posDelta.y * 127.0f);
    deltaPacket->pos[2] = (char)(posDelta.z * 127.0f);
    
    // 压缩旋转变化到紧凑格式
    deltaPacket->orientation[0] = (char)(rotDelta.x * 127.0f);
    deltaPacket->orientation[1] = (char)(rotDelta.y * 127.0f);
    deltaPacket->orientation[2] = (char)(rotDelta.z * 127.0f);
    deltaPacket->orientation[3] = (char)(rotDelta.w * 127.0f);
    
    // 存储当前状态用于历史记录
    if (stateHistory.size() > 60) {
        stateHistory.erase(stateHistory.begin());
    }
    stateHistory.push_back(currentState);
    
    *p = deltaPacket;
    return true;
}

// TODO: 进行速度同步
bool NetworkObject::WriteFullPacket(GamePacket** p, int stateID) {
    // 获取当前状态
    NetworkState currentState;
    currentState.position = object.GetTransform().GetPosition();
    currentState.orientation = object.GetTransform().GetOrientation();
    currentState.stateID = stateID;
    
    if (object.GetPhysicsObject()) {
        currentState.linearVelocity = object.GetPhysicsObject()->GetLinearVelocity();
        currentState.angularVelocity = object.GetPhysicsObject()->GetAngularVelocity();
    }
    
    // 创建完整状态包
    FullPacket* fullPacket = new FullPacket();
    fullPacket->networkID = this->networkID;
    fullPacket->fullState = currentState;
    
    // 存储当前状态用于历史记录
    lastFullState = currentState;
    if (stateHistory.size() > 60) {
        stateHistory.erase(stateHistory.begin());
    }
    stateHistory.push_back(currentState);
    
    *p = fullPacket;
    return true;
}

NetworkState& NetworkObject::GetLatestNetworkState() {
    return lastFullState;
}

bool NetworkObject::GetNetworkState(int frameID, NetworkState& state) {
    // 在历史记录中查找匹配的状态ID
    for (auto& s : stateHistory) {
        if (s.stateID == frameID) {
            state = s;
            return true;
        }
    }

    //std::cout << "NetworkObject(ID=" << networkID
    //    << "): 警告 - 找不到状态ID=" << frameID
    //    << "，历史记录包含" << stateHistory.size() << "个状态" << std::endl;

    //if (!stateHistory.empty()) {
    //    // std::cout << "可用的状态ID: ";
    //    for (auto& s : stateHistory) {
    //        std::cout << s.stateID << " ";
    //    }
    //    std::cout << std::endl;
    //}

    return false;
}

void NetworkObject::UpdateStateHistory(int minID) {
    // 删除过时的状态历史
    auto it = stateHistory.begin();
    while (it != stateHistory.end()) {
        if (it->stateID < minID) {
            it = stateHistory.erase(it);
        }
        else {
            ++it;
        }
    }
}