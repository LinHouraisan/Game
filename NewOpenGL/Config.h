#pragma once

#include "Vector.h"
using namespace NCL;
using namespace Maths;

// 全局配置，慎重修改


// 阴影配置
constexpr const unsigned int SHADOW_WIDTH = 2000, SHADOW_HEIGHT = 2000;

// 屏幕配置
constexpr const unsigned int SCR_WIDTH = 1920;
constexpr const unsigned int SCR_HEIGHT = 1080;

// ssbo大小配置
#define MAX_RENDER_OBJECTS 1000
#define MAX_ARROW_OBJECTS 1000  

// 老虎模型配置
constexpr int tiger_boneCount = 38;
constexpr int tiger_frame = 120;

// 鼠标配置
extern float lastX;
extern float lastY;
extern bool firstMouse;

// 直射光配置
constexpr float near_plane = 1.0f, far_plane = 60.5f;

// 地图配置
const Vector3 mapSize(30, 0.5, 30);

// 渲染用固定顶点配置
constexpr float quadVertices[] = {
	// 位置       // 纹理坐标
	-0.025f, -0.025f,  0.0f, 0.0f, 1.0f,
	 0.025f, -0.025f,  0.0f, 1.0f, 1.0f,
	 0.025f,  0.025f,  0.0f, 1.0f, 0.0f,

	-0.025f, -0.025f,  0.0f, 0.0f, 1.0f,
	 0.025f,  0.025f,  0.0f, 1.0f, 0.0f,
	-0.025f,  0.025f,  0.0f, 0.0f, 0.0f
};

constexpr float planeVertices[] = {
	// 位置               // 纹理坐标
	5.0f,  0.0f,  5.0f,   5.0f, 0.0f,
   -5.0f,  0.0f,  5.0f,   0.0f, 0.0f,
   -5.0f,  0.0f, -5.0f,   0.0f, 5.0f,

	5.0f,  0.0f,  5.0f,   5.0f, 0.0f,
   -5.0f,  0.0f, -5.0f,   0.0f, 5.0f,
	5.0f,  0.0f, -5.0f,   5.0f, 5.0f
};

constexpr float skyboxVertices[] = {
	// Positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

const short TIGER_GROUP = 1 << 0;  // 使用位掩码定义老虎组

const short RAYCAST_GROUP = 1 << 15; // 使用高位避免冲突
