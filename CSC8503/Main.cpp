#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "../NewOpenGL/camera.h"
#include "animator.h"
#include "model_animation.h"
#include "Instantiate.h"

#include <stb_image.h>

#include <vector>
#include <iostream>
#include <random>

//#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "GameServer.h"
#include "GameClient.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

#include "PushdownMachine.h"

#include "PushdownState.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

#include "Config.h"

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>
#include "GameManager.h"

using namespace OpenGL;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

OpenGL::Camera* camera = new OpenGL::Camera(glm::vec3(0.0f, 0.0f, 3.0f));

float deltaTime = 0.0f;
float lastFrame = 0.0f;
//全局设置-------------------------------------------------------------------------------------


int main() {

	//OpenGL初始化设置-------------------------------------------------------------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CSC8508", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	stbi_set_flip_vertically_on_load(true);
	glEnable(GL_DEPTH_TEST);
	//OpenGL初始化设置-------------------------------------------------------------------------------------

	auto effekseerManager = EffekseerManager::GetInstance();

	effekseerManager->Initialize();

	OpenGL::GameTechRenderer* renderer = GameTechRenderer::GetInstance(window);

	// TutorialGame* g = new TutorialGame(window, renderer);
	GameManager* gameManager = GameManager::GetInstance();

	// 设置UI状态变更回调
	renderer->GetUIManager()->SetStateChangeCallback([renderer, gameManager, window](UIManager::UIState state) {
		if (state == UIManager::UIState::GAME) {
			// 单人游戏模式
			gameManager->DestroyCurrentGame();
			gameManager->CreateGame(GameType::TUTORIAL, window, renderer);
		}
		else if (state == UIManager::UIState::ONLINE_GAME) {
			// 多人游戏模式
			gameManager->DestroyCurrentGame();
			gameManager->CreateGame(GameType::NETWORKED, window, renderer);
		}
	});

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// g->UpdateGame(deltaTime);
		// 获取当前活动的游戏实例
		BaseGame* currentGame = gameManager->GetCurrentGame();
		if (currentGame) {
			currentGame->UpdateGame(deltaTime);
		}
		else {
			// 无活动游戏实例，仅渲染UI
			renderer->Render();
		}
	}

	// delete g;
	gameManager->DestroyCurrentGame();
	effekseerManager->Shutdown();
	glfwTerminate();
	return 0;

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; 

	lastX = xpos;
	lastY = ypos;

	camera->ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera->ProcessMouseScroll(yoffset);
}

