#include "GameTechRenderer.h"
#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iomanip>
#include "RenderPrepare.h"
#include "AudioManager.h"
#include "GameManager.h"
#include "TutorialGame.h"


using namespace OpenGL;

unsigned int GameTechRenderer::globalFrame = 0;

GameTechRenderer* GameTechRenderer::instance = nullptr;

GameTechRenderer* GameTechRenderer::GetInstance(GLFWwindow* window) {
	if (!instance) {
		instance = new GameTechRenderer(window);
	}
	return instance;
}

GameTechRenderer* GameTechRenderer::GetInstance() {
	return instance;
}

GameTechRenderer::GameTechRenderer(GLFWwindow* window) : window(window) {
	SetDirectLight();

	renderPrepare = &RenderPrepare::GetInstance();

	// 初始化ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	uiManager = new UIManager();
	uiManager->Initialize();
	uiManager->SetWindow(window);

	GameManager::GetInstance()->SetUIManager(uiManager);

	effekseerManager = EffekseerManager::GetInstance();

	glfwSetWindowUserPointer(window, this);
	// 使用UIManager的鼠标回调而不是GameTechRenderer的回调
	glfwSetMouseButtonCallback(window, UIManager::MouseButtonCallbackProxy);
}

GameTechRenderer::~GameTechRenderer() {
	instance = nullptr;
	delete uiManager;
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void GameTechRenderer::Update(float dt) {
	globalFrame++; // 帧计数继续增加

	// 检查游戏是否暂停
	bool gamePaused = GameManager::GetInstance()->IsPaused();

	//if (uiManager) {  //效果淡出
	//	uiManager->UpdateDamageEffect(dt);
	//}

	// 即使在暂停状态，也更新光照以保持视觉一致性
	UpdateLight();

	if (isDamageEffect) {
		damageEffectIntensity -= damageEffectDecayRate * dt;
		if (damageEffectIntensity <= 0.0f) {
			damageEffectIntensity = 0.0f;
			isDamageEffect = false;
		}
	}

	if (!gamePaused) {
		SetSSBO(renderPrepare->GetSSBO());
		SetArrowSSBO(renderPrepare->GetArrowSSBO());
		effekseerManager->Update();

		// 检查当前游戏模式
		GameType currentGameType = GameManager::GetInstance()->GetCurrentGameType();

		if (currentGameType == GameType::NETWORKED) {
			// 网络游戏模式 - 获取所有网络玩家的动画数据
			auto& netPlayers = renderPrepare->GetNetPlayersRender();
			if (!netPlayers.empty()) {
				// 使用第一个玩家的动画矩阵代表所有玩家
				// 注意：所有玩家使用相同的动画系统，但可能处于不同的动画状态
				RenderObject* firstPlayer = netPlayers[0];

				if (firstPlayer && firstPlayer->GetAnimationSpeed() != 0) {
					playerAnimas = ResourceManager::animationController_player->GetPrecomputedMatrices(1);
					hasStoppedAnimation = false;
				}
				else if (!hasStoppedAnimation) {
					playerAnimas = ResourceManager::animationController_player->GetPrecomputedMatrices(1);
					hasStoppedAnimation = true;
				}
			}
		}
		else {
			// 单机游戏模式 - 只处理本地玩家
			auto player = renderPrepare->GetPlayerRender();

			if (player && player->GetAnimationSpeed() != 0) {
				// 如果动画速度不为0，正常获取动画矩阵集
				playerAnimas = ResourceManager::animationController_player->GetPrecomputedMatrices(1);
				hasStoppedAnimation = false; // 重置标志变量
			}
			else {
				// 如果动画速度为0，并且之前没有获取过停止时的动画矩阵集
				if (!hasStoppedAnimation) {
					playerAnimas = ResourceManager::animationController_player->GetPrecomputedMatrices(1);
					hasStoppedAnimation = true; // 设置标志变量为true，表示已经获取过
				}
				// 如果已经获取过停止时的动画矩阵集，则不再重新获取，保持playerAnimas不变
			}
		}
	}

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
		static double lastToggleTime = 0.0;
		double currentTime = glfwGetTime();
		if (currentTime - lastToggleTime > 0.3) { // 防抖
			isToneMapping = !isToneMapping;
			lastToggleTime = currentTime;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		static double lastToggleTime = 0.0;
		double currentTime = glfwGetTime();
		if (currentTime - lastToggleTime > 0.3) { // 防抖
			isRimLighting = !isRimLighting;
			lastToggleTime = currentTime;
		}
	}
}


int GameTechRenderer::GetGlobalFrame() {
	return globalFrame;
}


void GameTechRenderer::Render() {

	bool gamePaused = GameManager::GetInstance()->IsPaused(); // 检查游戏是否暂停

	if (uiManager->ShouldExit()) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		return;
	}

	HandleMouseControl();
	uiManager->CheckContinuousFiring(window);
	HandleDebugUIInput();

	// 使用UIManager的计时器记录整个帧的渲染时间
	auto frameTimer = uiManager->CreateTimer("Frame Time");

	// 1. 首先绑定到后处理帧缓冲
	glBindFramebuffer(GL_FRAMEBUFFER, ResourceManager::postProcessFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (uiManager->GetState() != UIManager::UIState::GAME && uiManager->GetState() != UIManager::UIState::ONLINE_GAME) {
		// 非游戏状态：渲染 UI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		{
			auto uiTimer = uiManager->CreateTimer("UI Rendering");
			UIRenderPass();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		/*if (uiManager && uiManager->HasActiveDamageEffect()) {
			uiManager->RenderDamageEffect();
		}*/
	}
	else {
		// 新增对渲染对象是否为空的判断
		bool canRenderGameObjects = (renderPrepare->GetPlayerRender() != nullptr || !renderPrepare->GetNetPlayersRender().empty()) &&
			renderPrepare->GetWeaponRender() != nullptr;
		// 游戏状态：只渲染游戏场景
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		projection = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = camera->GetViewMatrix();

		if (canRenderGameObjects) {
			{
				auto shadowTimer = uiManager->CreateTimer("Shadow Pass");
				ShadowRenderPass();
			}

			{
				auto mainTimer = uiManager->CreateTimer("Main Rendering Pass");
				MainRenderPass();
			}

			{
				auto skyboxTimer = uiManager->CreateTimer("Skybox Rendering");
				SkyBoxRenderPass();
			}

			{
				auto postTimer = uiManager->CreateTimer("Post Processing");
				PostRenderPass();
			}

			CollectPerformanceMetrics();
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			auto hudTimer = uiManager->CreateTimer("HUD Rendering");
			if (uiManager->GetState() == UIManager::UIState::GAME) {
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y));
				ImGui::Begin("HUDWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);

				uiManager->RenderHUD();

				ImGui::End();
			}

			if (uiManager->IsPauseMenuVisible()) {
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y));
				ImGui::Begin("PauseMenuWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);

				uiManager->RenderPauseMenu();

				ImGui::End();
			}
		
			if (uiManager->IsPlayerProfileVisible()) {
				uiManager->RenderPlayerProfile();
			}

			if (uiManager->IsDebugWindowVisible()) {
				uiManager->RenderDebugWindow();
			}

			if (uiManager->IsScoreBoardVisible()) {
				uiManager->RenderScoreBoard();
			}
		}


		ImGui::Render();
	}

	// 2. 现在渲染到默认帧缓冲并应用后处理效果
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	/*if (uiManager && uiManager->HasActiveDamageEffect()) {
		uiManager->RenderDamageEffect();
	}*/

	if (isToneMapping) {
		// 应用色调映射后处理
		{
			auto postProcessTimer = uiManager->CreateTimer("Post Processing - Tone Mapping");
			ResourceManager::postToneMappingShader->use();
			ResourceManager::postToneMappingShader->setFloat("exposure", 0.3f);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ResourceManager::postColorBuffers[0]);
			glBindVertexArray(ResourceManager::postQuadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
	}
	else {
		// 直接渲染后处理FBO的内容到屏幕（无色调映射）
		{
			auto postProcessTimer = uiManager->CreateTimer("Post Processing - Direct Render");
			ResourceManager::postEcoShader->use(); // 使用简单的全屏着色器
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ResourceManager::postColorBuffers[0]);
			glBindVertexArray(ResourceManager::postQuadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
		// 重置 OpenGL 状态
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	}

	// 渲染伤害效果（屏幕红色效果）
	/*if (uiManager && uiManager->HasActiveDamageEffect()) {
		uiManager->RenderDamageEffect();
	}*/

	// 3. 渲染ImGui在色调映射之后
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(window);
	glfwPollEvents();
}


void GameTechRenderer::ShadowRenderPass() {
	auto setupTimer = uiManager->CreateTimer("Shadow - Setup");

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, ResourceManager::depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	ResourceManager::shadowShader_tiger->use();
	ResourceManager::shadowShader_tiger->setMat4("lightSpaceMatrix", ResourceManager::lightSpaceMatrix);

	{
		auto tigerTimer = uiManager->CreateTimer("Shadow - Tiger Rendering");
		DrawTiger(ResourceManager::shadowShader_tiger, renderPrepare->GetTigerNumber());
	}


	ResourceManager::shadowShader->use();
	ResourceManager::shadowShader->setMat4("lightSpaceMatrix", ResourceManager::lightSpaceMatrix);

	{
		auto playerTimer = uiManager->CreateTimer("Shadow - Player Rendering");
		DrawPlayer(ResourceManager::shadowShader);
		// Added network player rendering
		DrawNetPlayers(ResourceManager::shadowShader);
	}

	//{
	//	auto weaponTimer = uiManager->CreateTimer("Shadow - Weapon Rendering");
	//	DrawWeapon(ResourceManager::shadowShader);
	//}



	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void GameTechRenderer::MainRenderPass() {
	glBindFramebuffer(GL_FRAMEBUFFER, ResourceManager::postProcessFBO);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(0);

	{
		auto tigerTimer = uiManager->CreateTimer("Main - Tiger Rendering");
		//老虎模型专属-------------------------------------------------------------------------------------
		glEnable(GL_CULL_FACE); // 启用面剔除
		DrawTiger(ResourceManager::modelShader_tiger, renderPrepare->GetTigerNumber());
	}

	{
		auto arrowTimer = uiManager->CreateTimer("Main - Arrow Rendering");
		//Debug箭头专属-------------------------------------------------------------------------------------
		DrawArrow(ResourceManager::arrowShader, renderPrepare->GetArrowNumber());
	}

	{
		auto playerTimer = uiManager->CreateTimer("Main - Player Rendering");
		//玩家专属-------------------------------------------------------------------------------------
		DrawPlayer(ResourceManager::modelTestShader);
		DrawNetPlayers(ResourceManager::modelTestShader);
		glDisable(GL_CULL_FACE); // 停用面剔除
	}

	{
		auto weaponTimer = uiManager->CreateTimer("Main - Weapon Rendering");
		//武器专属-------------------------------------------------------------------------------------
		DrawWeapon(ResourceManager::modelTestShader);
		glDisable(GL_CULL_FACE); // 停用面剔除
	}

	{
		auto floorTimer = uiManager->CreateTimer("Main - Map Rendering");
		//场景地图-------------------------------------------------------------------------------------
		if (switchMapCount == 0) {
			RenderLevel_1(ResourceManager::mapShader);
		}
		else if (switchMapCount == 1) {
			RenderLevel_2(ResourceManager::mapShader);
			if (!CoinRemoved) {
				DrawCoin(ResourceManager::staticModelShader, 0, -5.8f, 1.0f, 2.48f);
			}
		}
		else if (switchMapCount == 2) {
			RenderLevel_3(ResourceManager::mapShader);
			DrawWater(ResourceManager::waterShader);
			if (!CoinRemoved) {
				DrawCoin(ResourceManager::staticModelShader, 0, 2.2f, 1.0f, 4.5f);
			}
		}
		else if (switchMapCount == 3) {
			RenderLevel_4(ResourceManager::mapShader);
			if (!CoinRemoved) {
				DrawCoin(ResourceManager::staticModelShader, 0, 2.47f, 1.3f, 4.47f);
				DrawCoin(ResourceManager::staticModelShader, 0, -5.54f, 1.3f, -4.545f); 
			}
		}
		else if (switchMapCount == 4) {
			RenderLevel_5(ResourceManager::mapShader);
		}
	}

	if (monsterDead) {
		auto mapArrowTimer = uiManager->CreateTimer("Main - MapArrow Rendering");
		if(switchMapCount < 3) {
			DrawMapArrow(ResourceManager::staticModelShader, 180, 13.5 + 50.0f * switchMapCount, 0 + 50.0f * switchMapCount);
			DrawMapArrow(ResourceManager::staticModelShader, 90, 0 +50.0f * switchMapCount, 13.5 + 50.0f * switchMapCount);
			DrawMapArrow(ResourceManager::staticModelShader, -90, 0 + 50.0f * switchMapCount, -13.5 + 50.0f * switchMapCount);
		}
		else if (switchMapCount == 3) {
			DrawMapArrow(ResourceManager::staticModelShader, 180, 0 + 50.0f * switchMapCount, 0 + 50.0f * switchMapCount);
		}
	}

	{
		auto barrelTimer = uiManager->CreateTimer("Main - barrel Rendering");
		DrawBarrel(ResourceManager::staticModelShader);
		glDisable(GL_CULL_FACE); // 停用面剔除
	}

}

void GameTechRenderer::PostRenderPass() {
	// 确保绑定到后处理帧缓冲
	glBindFramebuffer(GL_FRAMEBUFFER, ResourceManager::postProcessFBO);

	{
		auto effectTimer = uiManager->CreateTimer("Post - Effects Rendering");
		// 渲染特效
		effekseerManager->RenderEffFrame(view, projection);
	}

	// 渲染debug
	if (isDebugPath) {
		auto debugPathTimer = uiManager->CreateTimer("Post - Debug Path");
		RenderDebugPath();
		RenderDebugRays();
	}

	{
		auto crosshairTimer = uiManager->CreateTimer("Post - Crosshair");
		// 渲染鼠标
		RenderCrosshair();
	}
	// 新增边缘光后处理
	if (isRimLighting) {
		auto rimLightTimer = uiManager->CreateTimer("Post - Rim Lighting");
		glBindFramebuffer(GL_FRAMEBUFFER, ResourceManager::postProcessFBO);

		ResourceManager::postRimLightShader->use();
		ResourceManager::postRimLightShader->setBool("isRimLighting", true);
		ResourceManager::postRimLightShader->setFloat("rimPower", rimPower);
		ResourceManager::postRimLightShader->setVec3("rimColor", rimColor);
		ResourceManager::postRimLightShader->setVec3("viewPos", camera->Position);

		// 计算并设置逆矩阵
		glm::mat4 invProj = glm::inverse(projection);
		glm::mat4 invView = glm::inverse(view);
		ResourceManager::postRimLightShader->setMat4("invProjection", invProj);
		ResourceManager::postRimLightShader->setMat4("invView", invView);

		// 绑定纹理
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ResourceManager::postColorBuffers[0]);
		ResourceManager::postRimLightShader->setInt("sceneTexture", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ResourceManager::depthTexture);
		ResourceManager::postRimLightShader->setInt("depthTexture", 1);

		// 渲染全屏四边形
		glBindVertexArray(ResourceManager::postQuadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

	// 添加受伤效果后处理
	if (isDamageEffect && damageEffectIntensity > 0.01f) {
		auto damageEffectTimer = uiManager->CreateTimer("Post - Damage Effect");

		// 计算脉动效果 - 使红色更明显地闪烁
		float time = glfwGetTime();
		float pulse = (sin(time * 8.0f) * 0.4f + 0.8f) * damageEffectIntensity;

		// 获取屏幕尺寸
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		// 保存当前OpenGL状态
		GLboolean blendEnabled;
		glGetBooleanv(GL_BLEND, &blendEnabled);
		GLint blendSrc, blendDst;
		glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrc);
		glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDst);

		// 启用混合
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// 使用ImGui绘制到当前帧缓冲区
		ImDrawList* drawList = ImGui::GetBackgroundDrawList();

		// 计算边框宽度 
		float borderWidth = std::min(width, height) * (0.15f + pulse * 0.1f);

		ImU32 outerColor = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, pulse * 0.9f));
		ImU32 midColor = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, pulse * 0.5f));
		ImU32 innerColor = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, 0.0f));

		// 中间过渡点
		float midPoint = borderWidth * 0.6f;

		// 上边缘 - 双重渐变效果
		drawList->AddRectFilledMultiColor(ImVec2(0, 0),ImVec2(width, midPoint),outerColor, outerColor, midColor, midColor);
		drawList->AddRectFilledMultiColor(ImVec2(0, midPoint),ImVec2(width, borderWidth),midColor, midColor, innerColor, innerColor);

		// 下边缘 - 双重渐变
		drawList->AddRectFilledMultiColor(ImVec2(0, height - borderWidth),ImVec2(width, height - midPoint),innerColor, innerColor, midColor, midColor);
		drawList->AddRectFilledMultiColor(ImVec2(0, height - midPoint),ImVec2(width, height),midColor, midColor, outerColor, outerColor);

		// 左边缘 - 双重渐变
		drawList->AddRectFilledMultiColor(ImVec2(0, borderWidth),ImVec2(midPoint, height - borderWidth),outerColor, midColor, midColor, outerColor);
		drawList->AddRectFilledMultiColor(ImVec2(midPoint, borderWidth),ImVec2(borderWidth, height - borderWidth),midColor, innerColor, innerColor, midColor);

		// 右边缘 - 双重渐变
		drawList->AddRectFilledMultiColor(ImVec2(width - borderWidth, borderWidth),ImVec2(width - midPoint, height - borderWidth),innerColor, midColor, midColor, innerColor);
		drawList->AddRectFilledMultiColor(ImVec2(width - midPoint, borderWidth),ImVec2(width, height - borderWidth),midColor, outerColor, outerColor, midColor);

		// 角落处额外加强（更明显的红色角落）
		float cornerSize = borderWidth * 1.4f;

		// 左上角
		drawList->AddRectFilledMultiColor(ImVec2(0, 0),ImVec2(cornerSize, cornerSize),outerColor, midColor, midColor, midColor);

		// 右上角
		drawList->AddRectFilledMultiColor(ImVec2(width - cornerSize, 0),ImVec2(width, cornerSize),midColor, outerColor, midColor, midColor);

		// 左下角
		drawList->AddRectFilledMultiColor(ImVec2(0, height - cornerSize),ImVec2(cornerSize, height),midColor, midColor, midColor, outerColor);

		// 右下角
		drawList->AddRectFilledMultiColor(ImVec2(width - cornerSize, height - cornerSize),ImVec2(width, height),midColor, midColor, outerColor, outerColor);

		// 轻微的全屏红色叠加
		ImU32 overlayColor = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, pulse * 0.25f));
		drawList->AddRectFilled(ImVec2(0, 0),ImVec2(width, height),overlayColor);

		// 红色圆形晕开效果 
		float vignettePower = damageEffectIntensity * 0.7f;
		int numSegments = 40;
		ImVec2 center(width / 2, height / 2);
		float maxRadius = std::sqrt(width * width + height * height) * 0.75f;

		drawList->AddCircleFilled(center,maxRadius,ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, pulse * vignettePower * 0.3f)),numSegments);

		if (!blendEnabled) {
			glDisable(GL_BLEND);
		}
		else {
			glBlendFunc(blendSrc, blendDst);
		}
	}
}


void GameTechRenderer::SkyBoxRenderPass() {
	////天空盒-------------------------------------------------------------------------------------
	//ResourceManager::skyboxShader->use();
	//glm::mat4 view = glm::mat4(glm::mat3(camera->GetViewMatrix())); // 移除平移部分
	//// 翻转视图矩阵的Z分量
	//view[0][2] *= -1;
	//view[1][2] *= -1;
	//view[2][2] *= -1;
	//glm::mat4 projection = glm::perspective(-glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	//ResourceManager::skyboxShader->setMat4("view", view);
	//ResourceManager::skyboxShader->setMat4("projection", projection);

	//// 渲染天空盒
	//glDepthMask(GL_FALSE);
	//glDepthFunc(GL_LEQUAL);
	//glBindVertexArray(ResourceManager::skyboxVAO);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, ResourceManager::cubemapTexture);
	//glDrawArrays(GL_TRIANGLES, 0, 36);
	//glBindVertexArray(0);
	//glDepthFunc(GL_LESS);
	//glDepthMask(GL_TRUE);
	////天空盒-------------------------------------------------------------------------------------
}


void GameTechRenderer::RenderLevel_1(Shader* shader) {
	ResourceManager::mapShader->use();
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f)); // 再放大

	float angle = glm::radians(-180.0f); // 将角度转换为弧度
	glm::vec3 axis = glm::vec3(1.0f, 0.0f, 0.0f); // 旋转轴为X轴
	//glm::vec3 axis = glm::vec3(0.0f, 0.0f, 0.0f); // 旋转轴为X轴

	model = glm::rotate(model, angle, axis);
	angle = glm::radians(roratemap); // 将角度转换为弧度
	axis = glm::vec3(0.0f, 0.0f, 1.0f); // 旋转轴为X轴

	model = glm::rotate(model, angle, axis);
	model = glm::translate(model, glm::vec3(maptransX, maptransY,maptransZ)); // 先位移

	// 轴对称翻转（绕Y轴翻转）
	model = glm::scale(model, glm::vec3(scalX, scalY, scalZ)); 

	//镜面翻转
	float mirrorAngle = glm::radians(-45.0f);
	model = glm::rotate(model, mirrorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));


	shader->setMat4("model", model);
	shader->setMat4("view", camera->GetViewMatrix());
	shader->setMat4("projection", projection);
	shader->setVec3("lightDirection", lightDirection);
	shader->setVec3("lightColor", lightColor);
	shader->setVec3("viewPos", camera->Position);

	shader->setMat4("lightSpaceMatrix", ResourceManager::lightSpaceMatrix);
	shader->setInt("shadowMap", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ResourceManager::depthMap);
	ResourceManager::Model_map1->Draw(*shader);
}

void GameTechRenderer::RenderLevel_2(Shader* shader) {
	ResourceManager::mapShader->use();
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

	float angle = glm::radians(-180.0f);
	glm::vec3 axis = glm::vec3(1.0f, 0.0f, 0.0f);
	//glm::vec3 axis = glm::vec3(0.0f, 0.0f, 0.0f); 

	model = glm::rotate(model, angle, axis);
	angle = glm::radians(roratemap);
	axis = glm::vec3(0.0f, 0.0f, 1.0f);

	model = glm::rotate(model, angle, axis);
	model = glm::translate(model, glm::vec3(50, 0, -50));

	// 轴对称翻转（绕Y轴翻转）
	model = glm::scale(model, glm::vec3(scalX, scalY, scalZ));

	//镜面翻转
	float mirrorAngle = glm::radians(-45.0f);
	model = glm::rotate(model, mirrorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));


	shader->setMat4("model", model);
	shader->setMat4("view", camera->GetViewMatrix());
	shader->setMat4("projection", projection);
	shader->setVec3("lightDirection", lightDirection);
	shader->setVec3("lightColor", lightColor);
	shader->setVec3("viewPos", camera->Position);

	shader->setMat4("lightSpaceMatrix", ResourceManager::lightSpaceMatrix);
	shader->setInt("shadowMap", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ResourceManager::depthMap);
	ResourceManager::Model_map2->Draw(*shader);
}

void GameTechRenderer::RenderLevel_3(Shader* shader) {
	ResourceManager::mapShader->use();
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

	float angle = glm::radians(-180.0f);
	glm::vec3 axis = glm::vec3(1.0f, 0.0f, 0.0f);
	//glm::vec3 axis = glm::vec3(0.0f, 0.0f, 0.0f); 

	model = glm::rotate(model, angle, axis);
	angle = glm::radians(roratemap);
	axis = glm::vec3(0.0f, 0.0f, 1.0f);

	model = glm::rotate(model, angle, axis);
	model = glm::translate(model, glm::vec3(100, 0, -100));

	// 轴对称翻转（绕Y轴翻转）
	model = glm::scale(model, glm::vec3(scalX, scalY, scalZ));

	//镜面翻转
	float mirrorAngle = glm::radians(-45.0f);
	model = glm::rotate(model, mirrorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));


	shader->setMat4("model", model);
	shader->setMat4("view", camera->GetViewMatrix());
	shader->setMat4("projection", projection);
	shader->setVec3("lightDirection", lightDirection);
	shader->setVec3("lightColor", lightColor);
	shader->setVec3("viewPos", camera->Position);

	shader->setMat4("lightSpaceMatrix", ResourceManager::lightSpaceMatrix);
	shader->setInt("shadowMap", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ResourceManager::depthMap);
	ResourceManager::Model_map3->Draw(*shader);
}

void GameTechRenderer::RenderLevel_4(Shader* shader) {
	ResourceManager::mapShader->use();
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

	float angle = glm::radians(-180.0f);
	glm::vec3 axis = glm::vec3(1.0f, 0.0f, 0.0f);

	model = glm::rotate(model, angle, axis);
	angle = glm::radians(roratemap);
	axis = glm::vec3(0.0f, 0.0f, 1.0f);

	model = glm::rotate(model, angle, axis);
	model = glm::translate(model, glm::vec3(150, 0, -150));

	// 轴对称翻转（绕Y轴翻转）
	model = glm::scale(model, glm::vec3(scalX, scalY, scalZ));

	//镜面翻转
	float mirrorAngle = glm::radians(-45.0f);
	model = glm::rotate(model, mirrorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));


	shader->setMat4("model", model);
	shader->setMat4("view", camera->GetViewMatrix());
	shader->setMat4("projection", projection);
	shader->setVec3("lightDirection", lightDirection);
	shader->setVec3("lightColor", lightColor);
	shader->setVec3("viewPos", camera->Position);

	shader->setMat4("lightSpaceMatrix", ResourceManager::lightSpaceMatrix);
	shader->setInt("shadowMap", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ResourceManager::depthMap);
	ResourceManager::Model_map4->Draw(*shader);
}
void GameTechRenderer::RenderLevel_5(Shader* shader) {
	ResourceManager::mapShader->use();
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

	float angle = glm::radians(-180.0f);
	glm::vec3 axis = glm::vec3(1.0f, 0.0f, 0.0f);

	model = glm::rotate(model, angle, axis);
	angle = glm::radians(roratemap);
	axis = glm::vec3(0.0f, 0.0f, 1.0f);

	model = glm::rotate(model, angle, axis);
	model = glm::translate(model, glm::vec3(200, 0, -200));

	// 轴对称翻转（绕Y轴翻转）
	model = glm::scale(model, glm::vec3(scalX, scalY, scalZ));

	//镜面翻转
	float mirrorAngle = glm::radians(-45.0f);
	model = glm::rotate(model, mirrorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));


	shader->setMat4("model", model);
	shader->setMat4("view", camera->GetViewMatrix());
	shader->setMat4("projection", projection);
	shader->setVec3("lightDirection", lightDirection);
	shader->setVec3("lightColor", lightColor);
	shader->setVec3("viewPos", camera->Position);

	shader->setMat4("lightSpaceMatrix", ResourceManager::lightSpaceMatrix);
	shader->setInt("shadowMap", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ResourceManager::depthMap);
	ResourceManager::Model_map5->Draw(*shader);
}

void GameTechRenderer::DrawMapArrow(Shader* shader, float rotationAngle, float offsetX, float offsetZ) {
	shader->use();

	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(0.0f + offsetX, 0.05f, 0.0f + offsetZ));

	//旋转
	model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	shader->setMat4("model", model);
	shader->setMat4("view", camera->GetViewMatrix());
	shader->setMat4("projection", projection);
	shader->setVec3("lightDirection", lightDirection);
	shader->setVec3("lightColor", lightColor);
	shader->setVec3("viewPos", camera->Position);
	shader->setMat4("lightSpaceMatrix", ResourceManager::lightSpaceMatrix);

	shader->setInt("shadowMap", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ResourceManager::depthMap);

	ResourceManager::Model_switchMapArrow->Draw(*shader);
}

void GameTechRenderer::DrawCoin(Shader* shader, float rotationAngle, float offsetX, float offsetY, float offsetZ) {
	shader->use();

	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(50.0f * switchMapCount + offsetX, offsetY, 50.0f * switchMapCount + offsetZ));

	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));

	//旋转
	model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	shader->setMat4("model", model);
	shader->setMat4("view", camera->GetViewMatrix());
	shader->setMat4("projection", projection);
	shader->setVec3("lightDirection", lightDirection);
	shader->setVec3("lightColor", lightColor);
	shader->setVec3("viewPos", camera->Position);
	shader->setMat4("lightSpaceMatrix", ResourceManager::lightSpaceMatrix);

	shader->setInt("shadowMap", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ResourceManager::depthMap);

	ResourceManager::Model_coin->Draw(*shader);
}

void GameTechRenderer::DrawBarrel(Shader* shader) {
	shader->use();
	shader->setMat4("view", view);
	shader->setMat4("projection", projection);

	for (auto barrel : renderPrepare->GetBarrelsRender())
	{
		if (!barrel) continue;

		glm::mat4 model = glm::mat4(1.0f);

		Vector3 position = barrel->GetTransform()->GetPosition();
		Vector3 scale = barrel->GetTransform()->GetScale();

		model = glm::translate(model, glm::vec3(position.x, position.y, position.z)); // 平移
		float scal = 1;
		model = glm::scale(model, glm::vec3(scale.x * scal, scale.y * scal, scale.z * scal)); // 缩放
		
		shader->setMat4("model", model);

		barrel->GetModel()->Draw(*shader);
	}
}

void GameTechRenderer::DrawWater(Shader* shader) {
	shader->use();

	float currentTime = static_cast<float>(glfwGetTime());
	shader->setFloat("time", currentTime);

	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(5.5f + 50.0f * switchMapCount, 0, 6.5f + 50.0f * switchMapCount));

	model = glm::scale(model, glm::vec3(1.01f, 1.0f, 1.01f));

	shader->setMat4("model", model);
	shader->setMat4("view", camera->GetViewMatrix());
	shader->setMat4("projection", projection);
	shader->setVec3("lightDirection", lightDirection);
	shader->setVec3("lightColor", lightColor);
	shader->setVec3("viewPos", camera->Position);
	shader->setMat4("lightSpaceMatrix", ResourceManager::lightSpaceMatrix);

	shader->setInt("shadowMap", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ResourceManager::depthMap);

	ResourceManager::Model_water->Draw(*shader);
}



void GameTechRenderer::SetDirectLight() {
	lightDirection = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));
	lightColor = glm::vec3(1.0f, 0.95f, 0.9f); // 轻微偏暖色
	lightPos = glm::vec3(-1.0f, 40.0f, -1.0f);
	lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
	lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	ResourceManager::lightSpaceMatrix = lightProjection * lightView;
}

void GameTechRenderer::UpdateLight() {
	auto player = renderPrepare->GetPlayerRender();
	if (!player) return;

	Vector3 playerPos = player->GetTransform()->GetPosition();

	float mapOffsetX = switchMapCount * 50.0f;
	float mapOffsetZ = switchMapCount * 50.0f;
	lightPos = glm::vec3(mapOffsetX, playerPos.y + 20.0f, mapOffsetZ + 3.0f);
	//lightPos = glm::vec3(playerPos.x, playerPos.y + 20.0f, playerPos.z);

	// 获取玩家朝向
	Quaternion orientation = Quaternion();
	glm::quat q(orientation.w, orientation.x, orientation.y, orientation.z);
	glm::vec3 forward = glm::rotate(q, glm::vec3(0.0f, 0.0f, 1.0f)); // 假设玩家面朝Z轴正方向
	// 光线方向调整为玩家朝向
	lightDirection = glm::normalize(forward);

	// 计算光源方向：斜向下，同时跟随玩家朝向
	glm::vec3 horizontalDirection = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z)); // 水平方向
	lightDirection = glm::normalize(horizontalDirection + DEFAULT_LIGHT_DIRECTION * glm::tan(LIGHT_ANGLE));

	// 更新光源视图和空间矩阵
	lightView = glm::lookAt(lightPos, lightPos + lightDirection, glm::vec3(0.0f, 1.0f, 0.0f));

	float lightProjectionSize = 50.0f;

	lightProjection = glm::ortho(-lightProjectionSize, lightProjectionSize,
		-lightProjectionSize, lightProjectionSize,
		near_plane, far_plane);

	ResourceManager::lightSpaceMatrix = lightProjection * lightView;
}

//void GameTechRenderer::DrawTiger(Shader* shader, int count) {
//	shader->use();
//	shader->setMat4("projection", projection);
//	shader->setMat4("view", view);
//	shader->setVec3("lightDirection", lightDirection);
//	shader->setVec3("lightColor", lightColor);
//	shader->setVec3("viewPos", camera->Position);
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
//	ResourceManager::animationModel_tiger->DrawInstancedSSBO(*shader, count);
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
//}
void GameTechRenderer::DrawTiger(Shader* shader, int count) {
	shader->use();

	shader->setMat4("projection", projection);
	shader->setMat4("view", view);
	shader->setMat4("lightSpaceMatrix", ResourceManager::lightSpaceMatrix); //传递动态阴影矩阵
	shader->setVec3("lightPos", lightPos);
	shader->setVec3("lightColor", lightColor);
	shader->setVec3("viewPos", camera->Position);

	//绑定阴影
	glActiveTexture(GL_TEXTURE1); 
	glBindTexture(GL_TEXTURE_2D, ResourceManager::depthMap);
	shader->setInt("shadowMap", 1); 

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ResourceManager::animationModel_tiger->DrawInstancedSSBO(*shader, count);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	//glActiveTexture(GL_TEXTURE0);
	//glDisable(GL_BLEND);
}

void OpenGL::GameTechRenderer::UIRenderPass() {
	if (uiManager->GetState() != UIManager::UIState::GAME && uiManager->GetState() != UIManager::UIState::ONLINE_GAME) {
		uiManager->Render();  // 渲染主菜单等UI
	}
	else {
		// 在游戏状态下，如果调试窗口可见，则渲染调试窗口
		if (uiManager->IsDebugWindowVisible()) {
			uiManager->RenderDebugWindow();
		}
	}
}

void GameTechRenderer::HandleDebugUIInput() {
	if (uiManager->GetState() == UIManager::UIState::GAME || uiManager->GetState() == UIManager::UIState::ONLINE_GAME) {
		if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
			float currentTime = glfwGetTime();
			if (currentTime - lastToggleTime > 0.3f) {
				uiManager->ToggleDebugWindow();
				lastToggleTime = currentTime;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) {
			float currentTime = glfwGetTime();
			if (currentTime - lastToggleTime > 0.3f) {
				uiManager->ToggleDebugTab();
				lastToggleTime = currentTime;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
			float currentTime = glfwGetTime();
			if (currentTime - lastToggleTime > 0.3f) {
				uiManager->ToggleScoreBoard();
				lastToggleTime = currentTime;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
			float currentTime = glfwGetTime();
			if (currentTime - lastToggleTime > 0.3f) {
				uiManager->TogglePlayerProfile();
				lastToggleTime = currentTime;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_F12) == GLFW_PRESS) {
			float currentTime = glfwGetTime();
			if (currentTime - lastToggleTime > 0.3f) {
				uiManager->TogglePauseMenu();
				lastToggleTime = currentTime;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_F10) == GLFW_PRESS) {
			float currentTime = glfwGetTime();
			if (currentTime - lastToggleTime > 0.3f) {
				uiManager->TestDamageEffect(0.8f);
				lastToggleTime = currentTime;
			}
		}
	}
}

void GameTechRenderer::CollectPerformanceMetrics() {
	// 计算FPS
	static float lastTime = glfwGetTime();
	float currentTime = glfwGetTime();
	float deltaTime = currentTime - lastTime;
	lastTime = currentTime;

	static float fpsUpdateTime = 0.0f;
	static int frameCount = 0;
	frameCount++;
	fpsUpdateTime += deltaTime;

	if (fpsUpdateTime >= 1.0f) {
		float fps = frameCount / fpsUpdateTime;
		frameCount = 0;
		fpsUpdateTime = 0.0f;

		// 更新UIManager中的FPS显示
		uiManager->UpdateFPS(fps);
	}

	// 添加渲染统计信息到UIManager
	uiManager->AddDebugInfo("Draw Calls", std::to_string(drawCallCount), "Render Stats");
	uiManager->AddDebugInfo("Triangle Count", std::to_string(triangleCount), "Render Stats");

	// 添加相机信息
	std::stringstream posStream;
	posStream << std::fixed << std::setprecision(2)
		<< "X: " << camera->Position.x
		<< " Y: " << camera->Position.y
		<< " Z: " << camera->Position.z;
	uiManager->AddDebugInfo("Camera Position", posStream.str(), "Camera");

	// 添加玩家信息
	auto player = renderPrepare->GetPlayerRender();
	if (player) {
		Vector3 pos = player->GetTransform()->GetPosition();
		std::stringstream playerPosStream;
		playerPosStream << std::fixed << std::setprecision(2)
			<< "X: " << pos.x
			<< " Y: " << pos.y
			<< " Z: " << pos.z;
		uiManager->AddDebugInfo("Player Position", playerPosStream.str(), "Game State");
	}

	// 添加地图变换信息
	uiManager->AddDebugInfo("Map Translation X", std::to_string(maptransX), "Map Transform");
	uiManager->AddDebugInfo("Map Translation Y", std::to_string(maptransY), "Map Transform");
	uiManager->AddDebugInfo("Map Translation Z", std::to_string(maptransZ), "Map Transform");
	uiManager->AddDebugInfo("Map Rotation", std::to_string(roratemap), "Map Transform");

	// 添加虎子信息
	uiManager->AddDebugInfo("Tiger Count", std::to_string(renderPrepare->GetTigerNumber()), "Game State");
	uiManager->AddDebugInfo("BGM Playing", AudioManager::GetInstance()->IsBGMPlaying() ? "Yes" : "No", "Audio");
	uiManager->AddDebugInfo("BGM Volume", std::to_string(AudioManager::GetInstance()->GetBGMVolume()), "Audio");
	uiManager->AddDebugInfo("SFX Volume", std::to_string(AudioManager::GetInstance()->GetSFXVolume()), "Audio");

	// 重置计数器
	drawCallCount = 0;
	triangleCount = 0;
}

void GameTechRenderer::RenderDebugPath() {
	glUseProgram(0);
	ResourceManager::pathDebugShader->use();

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	ResourceManager::pathDebugShader->setMat4("model", model);
	ResourceManager::pathDebugShader->setMat4("view", camera->GetViewMatrix());
	ResourceManager::pathDebugShader->setMat4("projection", projection);

	glBindVertexArray(ResourceManager::gridVAO);
	glDrawArrays(GL_LINES, 0, (NCL::Maths::Vector3(100, 0.5, 100).x / (mapSize.x / 30.0f) + 1) * 4); // 30x30 网格
	glBindVertexArray(0);
}

void GameTechRenderer::DrawArrow(Shader* shader, int count) {
	shader->use();
	shader->setMat4("projection", projection);
	shader->setMat4("view", view);

	// 绑定 Arrow 专属 SSBO
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, arrowSSBO);
	ResourceManager::Model_arrow->DrawInstancedSSBO(*shader, count);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GameTechRenderer::RenderCrosshair() {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// 将鼠标屏幕坐标转换为 NDC（范围 -1.0 到 1.0）
	float ndcX = (xpos / SCR_WIDTH) * 2.0f - 1.0f;
	float ndcY = 1.0f - (ypos / SCR_HEIGHT) * 2.0f;

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(ndcX, ndcY, 0.0f));
	model = glm::scale(model, glm::vec3(1, 1.7778, 1.0f));

	glUseProgram(ResourceManager::crosshairShader->ID);
	ResourceManager::crosshairShader->setMat4("model", model);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ResourceManager::crosshairTexture);

	glBindVertexArray(ResourceManager::crosshairVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void GameTechRenderer::RenderDebugRays() {
	if (!ResourceManager::debugRays.empty()) {
		ResourceManager::pathDebugShader->use();
		ResourceManager::pathDebugShader->setMat4("view", view);
		ResourceManager::pathDebugShader->setMat4("projection", projection);

		// 绑定射线的 VAO
		glBindVertexArray(ResourceManager::rayVAO);

		for (const auto& ray : ResourceManager::debugRays) {
			// 更新射线的顶点数据
			float rayVertices[] = {
				ray.start.x, ray.start.y, ray.start.z,
				ray.end.x, ray.end.y, ray.end.z
			};

			// 将顶点数据上传到 VBO
			glBindBuffer(GL_ARRAY_BUFFER, ResourceManager::rayVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(rayVertices), rayVertices, GL_STATIC_DRAW);

			// 绘制射线
			glDrawArrays(GL_LINES, 0, 2);
		}

		// 解绑 VAO
		glBindVertexArray(0);

		// 清空射线数据，避免重复渲染
		ResourceManager::debugRays.clear();
	}
}

void GameTechRenderer::DrawPlayer(Shader* shader) {
	shader->use();
	shader->setMat4("view", view);
	shader->setMat4("projection", projection);

	auto player = renderPrepare->GetPlayerRender();
	if (!player) return;

	// 获取玩家的 Transform 信息
	Vector3 position = player->GetTransform()->GetPosition();
	Vector3 scale = player->GetTransform()->GetScale();
	Quaternion orientation = player->GetTransform()->GetOrientation();

	// 将四元数转换为旋转矩阵
	glm::mat4 rotationMatrix = glm::mat4_cast(glm::quat(orientation.w, orientation.x, orientation.y, orientation.z));

	// 构建模型矩阵
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position.x, position.y, position.z)); // 平移
	model = model * rotationMatrix; // 旋转
	float scal = 0.004;
	model = glm::scale(model, glm::vec3(scale.x * scal, scale.y * scal, scale.z * scal)); // 缩放

	// 将模型矩阵传递给着色器
	shader->setMat4("model", model);

	// 获取骨骼动画的变换矩阵并传递给着色器
	auto transforms = playerAnimas;
	for (int j = 0; j < transforms.size(); ++j) {
		shader->setMat4("finalBonesMatrices[" + std::to_string(j) + "]", transforms[j]);

	}


	// 绘制玩家模型
	ResourceManager::Model_player->Draw(*shader);
}

void GameTechRenderer::DrawNetPlayers(Shader* shader) {
	shader->use();
	shader->setMat4("view", view);
	shader->setMat4("projection", projection);

	auto netPlayers = renderPrepare->GetNetPlayersRender();
	if (netPlayers.empty()) return;

    for (size_t i = 0; i < netPlayers.size(); i++) {
        auto* player = netPlayers[i];
		// 获取玩家的 Transform 信息
		Vector3 position = player->GetTransform()->GetPosition();
		Vector3 scale = player->GetTransform()->GetScale();
		Quaternion orientation = player->GetTransform()->GetOrientation();

		// 将四元数转换为旋转矩阵
		glm::mat4 rotationMatrix = glm::mat4_cast(glm::quat(orientation.w, orientation.x, orientation.y, orientation.z));

		// 构建模型矩阵
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(position.x, position.y, position.z)); // 平移
		model = model * rotationMatrix; // 旋转
		float scal = 0.004;
		model = glm::scale(model, glm::vec3(scale.x * scal, scale.y * scal, scale.z * scal)); // 缩放

		// 将模型矩阵传递给着色器
		shader->setMat4("model", model);

		// 获取骨骼动画的变换矩阵并传递给着色器
		auto transforms = playerAnimas;
		for (int j = 0; j < transforms.size(); ++j) {
			shader->setMat4("finalBonesMatrices[" + std::to_string(j) + "]", transforms[j]);
		}

		// 绘制玩家模型
		ResourceManager::Model_netPlayers[i]->Draw(*shader);
	}
}

void GameTechRenderer::DrawWeapon(Shader* shader) {
	shader->use();
	shader->setMat4("view", view);
	shader->setMat4("projection", projection);

	for (auto weapon : renderPrepare->GetWeaponsRender())
	{
		//std::cout << weapon->GetModel()->GetModelName() << std::endl;

		if (!weapon) continue;
	
		glm::mat4 model = glm::mat4(1.0f);

		Vector3 position = weapon->GetTransform()->GetPosition();
		Vector3 scale = weapon->GetTransform()->GetScale();
		Quaternion orientation = weapon->GetTransform()->GetOrientation();

		if (weapon->GetModel() == ResourceManager::Model_gun1) {
			glm::mat4 rotationMatrix = glm::mat4_cast(glm::quat(orientation.w, orientation.x, orientation.y, orientation.z));
			glm::mat4 flipMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			rotationMatrix = rotationMatrix * flipMatrix;
			model = glm::translate(model, glm::vec3(position.x, position.y, position.z)); // 平移
			model = model * rotationMatrix; // 旋转
			float scal = 0.004;
			model = glm::scale(model, glm::vec3(scale.x * scal, scale.y * scal, scale.z * scal)); // 缩放
		}
		else if (weapon->GetModel() == ResourceManager::Model_gun2) {
			glm::mat4 rotationMatrix = glm::mat4_cast(glm::quat(orientation.w, orientation.x, orientation.y, orientation.z));
			glm::mat4 flipMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //调头
			rotationMatrix = rotationMatrix;
			model = glm::translate(model, glm::vec3(position.x, position.y, position.z)); // 平移
			model = model * rotationMatrix; // 旋转
			float scal = 0.008;
			model = glm::scale(model, glm::vec3(scale.x * scal, scale.y * scal, scale.z * scal)); // 缩放
		}
		else if (weapon->GetModel() == ResourceManager::Model_gun3) {
			glm::mat4 rotationMatrix = glm::mat4_cast(glm::quat(orientation.w, orientation.x, orientation.y, orientation.z));
			glm::mat4 flipMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 flipMatrix2 = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));//翻转
			rotationMatrix = rotationMatrix * flipMatrix * flipMatrix2;
			model = glm::translate(model, glm::vec3(position.x, position.y, position.z)); // 平移
			model = model * rotationMatrix; // 旋转
			float scal = 0.002;
			model = glm::scale(model, glm::vec3(scale.x * scal, scale.y * scal, scale.z * scal)); // 缩放
		}
		else if (weapon->GetModel() == ResourceManager::Model_gun4) {
			glm::mat4 rotationMatrix = glm::mat4_cast(glm::quat(orientation.w, orientation.x, orientation.y, orientation.z));
			//glm::mat4 flipMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 flipMatrix2 = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));//翻转
			rotationMatrix = rotationMatrix * flipMatrix2;
			model = glm::translate(model, glm::vec3(position.x, position.y, position.z)); // 平移
			model = model * rotationMatrix; // 旋转
			float scal = 0.007;
			model = glm::scale(model, glm::vec3(scale.x * scal, scale.y * scal, scale.z * scal)); // 缩放
		}
		else if (weapon->GetModel() == ResourceManager::Model_gun5) {
			glm::mat4 rotationMatrix = glm::mat4_cast(glm::quat(orientation.w, orientation.x, orientation.y, orientation.z));
			//glm::mat4 flipMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			//glm::mat4 flipMatrix2 = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));//翻转
			//rotationMatrix = rotationMatrix * flipMatrix2;
			model = glm::translate(model, glm::vec3(position.x, position.y, position.z)); // 平移
			model = model * rotationMatrix; // 旋转
			float scal = 0.00004;
			model = glm::scale(model, glm::vec3(scale.x * scal, scale.y * scal, scale.z * scal)); // 缩放
		}
	
		shader->setMat4("model", model);
	
		weapon->GetModel()->Draw(*shader);
	}

}

void GameTechRenderer::HandleMouseControl() {
	static UIManager::UIState lastState = UIManager::UIState::MAIN_MENU;
	UIManager::UIState currentState = uiManager->GetState();

	// 处理鼠标状态
	switch (currentState) {
	case UIManager::UIState::GAME:
		if (uiManager->IsPauseMenuVisible()) {
			// 当暂停菜单可见时显示鼠标
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else {
			if (lastState != currentState) {
				glfwSetCursorPos(window, SCR_WIDTH / 2.0, SCR_HEIGHT / 2.0);
			}
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		break;
	case UIManager::UIState::ONLINE_GAME:
		if (uiManager->IsPauseMenuVisible()) {
			// 当暂停菜单可见时显示鼠标
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else {
			if (lastState != currentState) {
				glfwSetCursorPos(window, SCR_WIDTH / 2.0, SCR_HEIGHT / 2.0);
			}
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		break;
	default:
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		break;
	}

	lastState = currentState;
}

void GameTechRenderer::ActivateDamageEffect(float intensity) {
	
	// 伤害效果强度
	float enhancedIntensity = std::min(intensity * 3.5f + 0.4f, 1.0f);

	// 如果已有效果，叠加而不是替换
	if (isDamageEffect) {
		damageEffectIntensity = std::min(damageEffectIntensity + enhancedIntensity, 1.0f);
	}
	else {
		damageEffectIntensity = enhancedIntensity;
	}

	isDamageEffect = true;
	// std::cout << "Damage effect activated with intensity: " << damageEffectIntensity << std::endl;
}
