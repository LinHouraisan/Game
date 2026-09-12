#pragma once
#include "../NewOpenGL/camera.h"
#include "animator.h"
#include "model_animation.h"
#include "Instantiate.h"
#include "../NewOpenGL/shader.h"
#include <vector>
#include "GameMaterialLoad.h"
#include "ResourceManager.h"
#include "UIManager.h"
#include <chrono>
#include "Effeksser.h"
#include "Config.h"
#include "RenderObject.h"


extern OpenGL::Camera* camera;

namespace OpenGL {  // 前向声明
	class RenderPrepare;  
}

namespace OpenGL {

	class GameTechRenderer {
	public:
		static GameTechRenderer* GetInstance(GLFWwindow* window);
		static GameTechRenderer* GetInstance();  // 添加无参数的获取实例方法
		~GameTechRenderer();

		void Update(float dt);
		void Render();
		void ShadowRenderPass();
		void MainRenderPass();
		void PostRenderPass();
		void SkyBoxRenderPass();
		void RenderLevel_1(Shader* shader);
		void RenderLevel_2(Shader* shader);
		void RenderLevel_3(Shader* shader);
		void RenderLevel_4(Shader* shader);
		void RenderLevel_5(Shader* shader);
		void SetDirectLight();
		static int GetGlobalFrame();
		const glm::mat4& GetProjection() const { return projection; }
		const glm::mat4& GetView() const { return view; }
		void DrawTiger(Shader* shader, int count);
		void DrawArrow(Shader* shader, int count);
		void DrawPlayer(Shader* shader);
		void DrawNetPlayers(Shader* shader);
		void DrawWeapon(Shader* shader);
		void DrawMapArrow(Shader* shader, float rotationAngle, float offsetX, float offsetZ);
		void DrawCoin(Shader* shader, float rotationAngle, float offsetX, float offsetY, float offsetZ);
		void DrawBarrel(Shader* shader);
		void DrawWater(Shader* shader);

		void SetSSBO(GLuint newSsbo) { ssbo = newSsbo; }
		UIManager* GetUIManager() { return uiManager; }
		void SetArrowSSBO(GLuint ssbo) { arrowSSBO = ssbo; }
		void RenderDebugRays();
		void UpdateLight();
		GameTechRenderer(GLFWwindow* window);

		static GameTechRenderer* instance;

		GLFWwindow* window;

		RenderPrepare* renderPrepare;

		GLuint ssbo;
		GLuint arrowSSBO;  // 专用于 Arrow

		glm::vec3 lightDirection;
		glm::vec3 lightColor;
		glm::vec3 lightPos;
		glm::mat4 lightProjection;
		glm::mat4 lightView;
		const glm::vec3 DEFAULT_LIGHT_DIRECTION = glm::vec3(0.0f, -1.0f, 0.0f); // 默认斜向下方向
		const float LIGHT_ANGLE = glm::radians(45.0f); // 光线与垂直方向的夹角（45度）

		glm::mat4 projection;
		glm::mat4 view;


		bool isDebugPath = false;

		int frameCount = 0;

		std::vector<glm::mat4> boneMatrices;
		float g_Time = 0;

		static unsigned int globalFrame;

		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

		UIManager* uiManager;
		void UIRenderPass();
		void HandleMouseControl();
		void CollectPerformanceMetrics();
		void HandleDebugUIInput();
		void RenderDebugPath();
		void RenderCrosshair();
		//glm::mat4 WeaponAdjust(glm::mat4 model, RenderObject* weapon);

		float lastToggleTime = 0.0f; //防止按键过快
		int drawCallCount = 0;
		int triangleCount = 0;

		EffekseerManager* effekseerManager;

		float maptransZ = 0;
		float maptransX = 0;
		float maptransY = 0;
		float roratemap = 0;

		float scalX = 1;
		float scalY = -1;
		float scalZ = -1;

		std::vector<glm::mat4> playerAnimas;
		bool hasStoppedAnimation = false;
		bool isToneMapping = false; // 控制颜色映射开关
		bool isRimLighting = false; // 控制边缘光开关
		float rimPower = 1.0f;    
		glm::vec3 rimColor = glm::vec3(1.0, 0.0, 0.0); // 边缘光颜色

		bool isDamageEffect = false;
		float damageEffectIntensity = 0.0f;
		float damageEffectDecayRate = 0.08f;
		void ActivateDamageEffect(float intensity);
	};

}