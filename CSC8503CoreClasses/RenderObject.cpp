#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "RenderObject.h"
#include <random>

using namespace OpenGL;
using namespace NCL::CSC8503;
using namespace NCL;

RenderObject::RenderObject(Transform* parentTransform, Model* model, AnimationController* animationController) {

	this->transform	= parentTransform;


	this->model		= model;

	this->animationController = animationController;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> distX(0, 30); // 限制偏移帧范围

	offsetFrame = distX(gen);

	this->isActive = true;
}


RenderObject::RenderObject( Transform* parentTransform, Model* model) {

	this->transform = parentTransform;


	this->model = model;

	this->animationController = nullptr;

	this->isActive = true;
}

RenderObject::~RenderObject() {

}

void RenderObject::UpdateFrame() {
    if (!isActive) {
        return; // 如果对象非激活状态，立即停止帧更新
    }

    // 处理非循环模式
    if (!isRepeat && frame >= 119) {
        frame = 119; // 停止在最后一帧

        // 受伤动画 4 播放完后，回到动画 6
        if (animationIndex == 4) {
            animationIndex = 6;
            isRepeat = true; // 让动画 6 循环播放
            isTransitioning = false; // 允许下一次受伤播放动画 4
        }

        // **死亡动画 3 播放完后，不再更新怪物**
        if (animationIndex == 3) {
            isActive = false; // 彻底停止动画更新
        }

        return;
    }

    // 检测是否需要重置帧
    static int lastAnimationIndex = animationIndex;
    if (lastAnimationIndex != animationIndex) {
        frame = 0;
        lastAnimationIndex = animationIndex;
    }

    // 计算新的帧值
    frame = ((GameTechRenderer::GetGlobalFrame() + offsetFrame) * animationSpeed) % 120;
}

