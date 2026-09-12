
#include "ResourceManager.h"

using namespace OpenGL;


Shader* ResourceManager::modelShader_tiger = nullptr;
Shader* ResourceManager::modelTestShader = nullptr;
Shader* ResourceManager::skyboxShader = nullptr;
Shader* ResourceManager::floorShader = nullptr;
Shader* ResourceManager::shadowShader = nullptr;
Shader* ResourceManager::pathDebugShader = nullptr;
Shader* ResourceManager::staticModelShader = nullptr;
Shader* ResourceManager::arrowShader = nullptr;
Shader* ResourceManager::crosshairShader = nullptr;
Shader* ResourceManager::boxDebugShader = nullptr;
Shader* ResourceManager::shadowShader_tiger = nullptr;
Shader* ResourceManager::mapShader = nullptr;
Shader* ResourceManager::waterShader = nullptr;
Shader* ResourceManager::postToneMappingShader = nullptr;
Shader* ResourceManager::postBloomShader = nullptr;
Shader* ResourceManager::postRimLightShader = nullptr;
Shader* ResourceManager::postFinalShader = nullptr;
Shader* ResourceManager::postEcoShader = nullptr;

Model* ResourceManager::animationModel_tiger = nullptr;
Model* ResourceManager::Model_plane = nullptr;
Model* ResourceManager::Model_player = nullptr;
Model* ResourceManager::Model_netPlayer = nullptr;
std::vector<Model*>  ResourceManager::Model_netPlayers;
Model* ResourceManager::Model_arrow = nullptr;
Model* ResourceManager::Model_map1 = nullptr;
Model* ResourceManager::Model_map2 = nullptr;
Model* ResourceManager::Model_map3 = nullptr;
Model* ResourceManager::Model_map4 = nullptr;
Model* ResourceManager::Model_map5 = nullptr;
Model* ResourceManager::Model_pistol = nullptr;
Model* ResourceManager::Model_gun1 = nullptr;
Model* ResourceManager::Model_gun2 = nullptr;
Model* ResourceManager::Model_gun3 = nullptr;
Model* ResourceManager::Model_switchMapArrow = nullptr;
Model* ResourceManager::Model_gun4 = nullptr;
Model* ResourceManager::Model_coin = nullptr;
Model* ResourceManager::Model_barrel = nullptr;
Model* ResourceManager::Model_water = nullptr;
Model* ResourceManager::Model_gun5 = nullptr;


Animation* ResourceManager::animation_player = nullptr;
AnimationController* ResourceManager::animationController_player = nullptr;
Animation* ResourceManager::animation_tiger = nullptr;
AnimationController* ResourceManager::animationController_tiger = nullptr;

Effekseer::EffectRef ResourceManager::laserEffect = nullptr;
Effekseer::EffectRef ResourceManager::pistolBulletEffect = nullptr;
Effekseer::EffectRef ResourceManager::shotGunBulletEffect = nullptr;
Effekseer::EffectRef ResourceManager::bounceLaserEffect = nullptr;
Effekseer::EffectRef ResourceManager::healingEffect = nullptr;
Effekseer::EffectRef ResourceManager::explodeEffect = nullptr;
Effekseer::EffectRef ResourceManager::gravityFieldEffect = nullptr;

//技能特效
Effekseer::EffectRef ResourceManager::BlackHoleEffect = nullptr;//黑洞技能特效
Effekseer::EffectRef ResourceManager::ShockwaveEffect = nullptr;//震荡波技能特效
Effekseer::EffectRef ResourceManager::DashingEffect = nullptr;//冲刺技能特效
Effekseer::EffectRef ResourceManager::TurretSpawnEffect = nullptr;//炮台本体特效
Effekseer::EffectRef ResourceManager::TurretEffect = nullptr;//炮台技能特效

Instantiate* ResourceManager::instanceRenderer_tiger = nullptr;

std::vector<std::vector<int>> ResourceManager::map1Grid(900, std::vector<int>());
std::vector<std::vector<int>> ResourceManager::map2Grid(900, std::vector<int>());
std::vector<std::vector<int>> ResourceManager::map3Grid(900, std::vector<int>());
std::vector<std::vector<int>> ResourceManager::map4Grid(900, std::vector<int>());
std::vector<std::vector<int>> ResourceManager::map5Grid(900, std::vector<int>());

unsigned int ResourceManager::depthMapFBO = 0;
unsigned int ResourceManager::depthMap = 0;
unsigned int ResourceManager::skyboxVAO = 0;
unsigned int ResourceManager::skyboxVBO = 0;
unsigned int ResourceManager::cubemapTexture = 0;
unsigned int ResourceManager::planeVAO = 0;
unsigned int ResourceManager::planeVBO = 0;
unsigned int ResourceManager::floorTexture = 0;
unsigned int ResourceManager::gridVAO = 0;
unsigned int ResourceManager::gridVBO = 0;
unsigned int ResourceManager::crosshairVAO = 0;
unsigned int ResourceManager::crosshairVBO = 0;
unsigned int ResourceManager::crosshairTexture = 0;
unsigned int ResourceManager::rayVAO = 0;
unsigned int ResourceManager::rayVBO = 0;
unsigned int ResourceManager::postProcessFBO = 0;
unsigned int ResourceManager::postColorBuffers[2] = { 0, 0 };
unsigned int ResourceManager::pingpongFBO[2] = { 0, 0 };
unsigned int ResourceManager::pingpongColorbuffers[2] = { 0, 0 };
unsigned int ResourceManager::postQuadVAO = 0;
unsigned int ResourceManager::postQuadVBO = 0;
unsigned int ResourceManager::depthTexture = 0;
glm::mat4 ResourceManager::lightSpaceMatrix = glm::mat4(1.0f);	

std::vector<ResourceManager::Ray> ResourceManager::debugRays;

Shader* ResourceManager::backgroundShader = nullptr;
unsigned int ResourceManager::backgroundTexture = 0;
unsigned int ResourceManager::backgroundVAO = 0;
unsigned int ResourceManager::backgroundVBO = 0;

Shader* ResourceManager::damageEffectShader = nullptr;

ResourceManager::ResourceManager() {
	effekseerManager = EffekseerManager::GetInstance();
	InitialiseAssets();
}


void ResourceManager::InitialiseAssets() {

	SetupShaders();
	SetupSkybox();
	SetupModels();
	SetupNetworkPlayersModel();
	SetupFloor();
	SetupShadow();
	SetupPathDebug();
	SetupEffects();
	SetupCrosshair();
	SetupMap();
	SetupRayDebug(); 
	SetupBackground();
	SetupPostProcessing();
}


ResourceManager::~ResourceManager() {
	delete modelShader_tiger;
	delete skyboxShader;
	delete floorShader;
	delete shadowShader;
	delete animationModel_tiger;
	delete animation_tiger;
	delete instanceRenderer_tiger;
	delete pathDebugShader;
	delete staticModelShader;
	delete arrowShader;
	delete Model_arrow;
	delete boxDebugShader;
	delete backgroundShader;
	delete shadowShader_tiger;
	delete mapShader;
	delete damageEffectShader;
}



void ResourceManager::SetupShaders() {
	modelShader_tiger = new Shader("../../NewOpenGL/Shaders/computerAim.vs", "../../NewOpenGL/Shaders/computerAim.fs");
	modelTestShader = new Shader("../../NewOpenGL/Shaders/anim_model.vs", "../../NewOpenGL/Shaders/anim_model.fs");
	skyboxShader = new Shader("../../NewOpenGL/Shaders/skybox.vs", "../../NewOpenGL/Shaders/skybox.fs");
	floorShader = new Shader("../../NewOpenGL/Shaders/floor.vs", "../../NewOpenGL/Shaders/floor.fs");
	shadowShader = new Shader("../../NewOpenGL/Shaders/shadow_depth.vs", "../../NewOpenGL/Shaders/shadow_depth.fs");
	shadowShader_tiger = new Shader("../../NewOpenGL/Shaders/shadow_depth_Instance.vs", "../../NewOpenGL/Shaders/shadow_depth.fs");
	pathDebugShader = new Shader("../../NewOpenGL/Shaders/PathDebug.vs", "../../NewOpenGL/Shaders/PathDebug.fs");
	staticModelShader = new Shader("../../NewOpenGL/Shaders/staticModel.vs", "../../NewOpenGL/Shaders/staticModel.fs");
	arrowShader = new Shader("../../NewOpenGL/Shaders/pathDebug_arrow.vs", "../../NewOpenGL/Shaders/pathDebug_arrow.fs");
	crosshairShader = new Shader("../../NewOpenGL/Shaders/crosshair.vs", "../../NewOpenGL/Shaders/crosshair.fs");
	boxDebugShader = new Shader("../../NewOpenGL/Shaders/BoxDebug.vs", "../../NewOpenGL/Shaders/BoxDebug.fs");
	mapShader = new Shader("../../NewOpenGL/Shaders/map.vs", "../../NewOpenGL/Shaders/map.fs");
	waterShader = new Shader("../../NewOpenGL/Shaders/water.vs", "../../NewOpenGL/Shaders/water.fs");

	postToneMappingShader = new Shader("../../NewOpenGL/Shaders/post_tonemapping.vs", "../../NewOpenGL/Shaders/post_tonemapping.fs");
	postBloomShader = new Shader("../../NewOpenGL/Shaders/post_bloom.vs", "../../NewOpenGL/Shaders/post_bloom.fs");
	postRimLightShader = new Shader("../../NewOpenGL/Shaders/post_rimlight.vs", "../../NewOpenGL/Shaders/post_rimlight.fs");
	postEcoShader = new Shader("../../NewOpenGL/Shaders/simple_post.vs", "../../NewOpenGL/Shaders/simple_post.fs");
	//postFinalShader = new Shader("../../NewOpenGL/Shaders/post_final.vs", "../../NewOpenGL/Shaders/post_final.fs");
	damageEffectShader = new Shader("../../NewOpenGL/Shaders/damage_effect.vs", "../../NewOpenGL/Shaders/damage_effect.fs");
}


void ResourceManager::SetupSkybox() {
	// 加载天空盒
	std::vector<std::string> faces = {
		"../../Assets/skybox/right.jpg", "../../Assets/skybox/left.jpg",
		"../../Assets/skybox/bottom.jpg", "../../Assets/skybox/top.jpg",
		"../../Assets/skybox/front.jpg", "../../Assets/skybox/back.jpg"
	};
	cubemapTexture = GameMaterialLoad::LoadCubemap(faces);


	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void ResourceManager::SetupShadow() {
	glGenFramebuffers(1, &depthMapFBO);

	// 创建深度纹理
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// 设置边缘采样为白色（表示不在阴影中）
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// 将深度纹理附加到帧缓冲的深度附件上
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


void ResourceManager::SetupModels() {
	animationModel_tiger = GameMaterialLoad::LoadModel("../../Assets/tiger_monster_Useable/tiger.gltf");
	animation_tiger = GameMaterialLoad::LoadAnimation("../../Assets/tiger_monster_Useable/tiger.gltf", animationModel_tiger);
	animationController_tiger = new AnimationController(animation_tiger);

	Model_player = GameMaterialLoad::LoadModel("../../Assets/legaoPlayer_Useable/legao.gltf");
	animation_player = GameMaterialLoad::LoadAnimation("../../Assets/legaoPlayer_Useable/legao.gltf", Model_player);
	animationController_player = new AnimationController(animation_player);

	//---Map---
	Model_arrow = GameMaterialLoad::LoadModel("../../Assets/direction_arrow/scene.gltf");
	Model_map1 = GameMaterialLoad::LoadModel("../../Assets/map.gltf/map1/map1.gltf");
	Model_map2 = GameMaterialLoad::LoadModel("../../Assets/map.gltf/map2/map2.gltf");
	Model_map3 = GameMaterialLoad::LoadModel("../../Assets/map.gltf/map3/map3WithoutWater/map3.gltf");
	Model_map4 = GameMaterialLoad::LoadModel("../../Assets/map.gltf/map4/map4.gltf");
	Model_map5 = GameMaterialLoad::LoadModel("../../Assets/map.gltf/mapfortest/10000grid.gltf");
	Model_switchMapArrow = GameMaterialLoad::LoadModel("../../Assets/map.gltf/mapGeneral/indicator-special-arrow.obj");
	Model_coin = GameMaterialLoad::LoadModel("../../Assets/map.obj/coin.obj");
	Model_barrel = GameMaterialLoad::LoadModel("../../Assets/map.obj/barrel.obj");
	Model_water = GameMaterialLoad::LoadModel("../../Assets/map.gltf/map3/water.gltf");
	for (auto& mesh : Model_water->meshes) {
		glBindTexture(GL_TEXTURE_2D, mesh.textures[0].id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	//---Map---
	
	Model_gun1 = GameMaterialLoad::LoadModel("../../Assets/Weapon/scifi_gun/scifi_gun.gltf");

	Model_gun2 = GameMaterialLoad::LoadModel("../../Assets/Weapon/cyberpunk_style_gun_pistol/cyberpunk_style_gun_pistol.gltf");

	Model_gun3 = GameMaterialLoad::LoadModel("../../Assets/Weapon/gun3/gun3.gltf");

	Model_gun4 = GameMaterialLoad::LoadModel("../../Assets/Weapon/bounce_laser/bounce_laser.gltf");

	Model_gun5 = GameMaterialLoad::LoadModel("../../Assets/Weapon/gravity_gun/gravity_gun.gltf");

}

void ResourceManager::SetupFloor() {
	floorTexture = GameMaterialLoad::LoadTexture("../../Assets/floor/floor.jpg");

	floorShader->use();
	floorShader->setInt("texture1", 0);


	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glBindVertexArray(0);
}

void ResourceManager::SetupEffects() {
	laserEffect = effekseerManager->LoadEffect(u"../../Assets/Effects/Bullet03.efk", 0.3f);
	pistolBulletEffect = effekseerManager->LoadEffect(u"../../Assets/Effects/Bullet01.efk", 0.1f);
	shotGunBulletEffect = effekseerManager->LoadEffect(u"../../Assets/Effects/Bullet02.efk", 0.2f);
	bounceLaserEffect = effekseerManager->LoadEffect(u"../../Assets/Effects/Bullet04.efk", 0.3f);
	gravityFieldEffect = effekseerManager->LoadEffect(u"../../Assets/Effects/Bullet05.efk", 1.0f);
	healingEffect = effekseerManager->LoadEffect(u"../../Assets/Effects/map/heal.efk", 0.6f);
	explodeEffect = effekseerManager->LoadEffect(u"../../Assets/Effects/map/fire.efk", 0.5f);

	//技能特效
	BlackHoleEffect = effekseerManager->LoadEffect(u"../../Assets/Effects/BlackHoleefk.efk", 0.5f);//黑洞技能特效
	ShockwaveEffect = effekseerManager->LoadEffect(u"../../Assets/Effects/newnewshockwaveefk.efk", 0.8f);//震荡波技能特效
	DashingEffect = effekseerManager->LoadEffect(u"../../Assets/Effects/Dashingefk.efk", 0.3f);//冲刺技能特效
	TurretSpawnEffect = effekseerManager->LoadEffect(u"../../Assets/Effects/TurretSpawnefk.efk", 0.5f);//炮台本体特效
	TurretEffect = effekseerManager->LoadEffect(u"../../Assets/Effects/Bullet01.efk", 0.1f);//炮台子弹特效

}

void ResourceManager::SetupPathDebug() {
	std::vector<float> gridVertices;
	float gridSize = mapSize.x / 30.0f; // 30x30 网格
	float halfSizeX = mapSize.x * 0.5f;
	float halfSizeZ = mapSize.z * 0.5f;

	for (float x = -halfSizeX; x <= halfSizeX; x += gridSize) {
		gridVertices.push_back(x); gridVertices.push_back(0.01f); gridVertices.push_back(-halfSizeZ);
		gridVertices.push_back(x); gridVertices.push_back(0.01f); gridVertices.push_back(halfSizeZ);
	}
	for (float z = -halfSizeZ; z <= halfSizeZ; z += gridSize) {
		gridVertices.push_back(-halfSizeX); gridVertices.push_back(0.01f); gridVertices.push_back(z);
		gridVertices.push_back(halfSizeX); gridVertices.push_back(0.01f); gridVertices.push_back(z);
	}

	glGenVertexArrays(1, &gridVAO);
	glGenBuffers(1, &gridVBO);
	glBindVertexArray(gridVAO);
	glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
	glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

void ResourceManager::SetupCrosshair() {
	crosshairTexture = GameMaterialLoad::LoadTexture("../../Assets/UI/Aim.png");

	glGenVertexArrays(1, &crosshairVAO);
	glGenBuffers(1, &crosshairVBO);
	glBindVertexArray(crosshairVAO);
	glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);
}

void ResourceManager::SetupMap() {
	map1Grid = GameMaterialLoad::LoadMap("../../Assets/map.gltf/map1/grid_data.txt");
	map2Grid = GameMaterialLoad::LoadMap("../../Assets/map.gltf/map2/grid_data1.txt");
	map3Grid = GameMaterialLoad::LoadMap("../../Assets/map.gltf/map3/grid_data.txt");
	map4Grid = GameMaterialLoad::LoadMap("../../Assets/map.gltf/map4/grid_data.txt");
	map5Grid = GameMaterialLoad::LoadMap("../../Assets/map.gltf/mapfortest/900grid_data.txt");
}

void ResourceManager::AddDebugRay(const glm::vec3& start, const glm::vec3& end) {
	debugRays.push_back({ start, end });
}

void ResourceManager::SetupRayDebug() {

	glGenVertexArrays(1, &rayVAO);
	glGenBuffers(1, &rayVBO);
	glBindVertexArray(rayVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rayVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); //顶点属性指针
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void ResourceManager::SetupBackground() {

	backgroundShader = new Shader("../../NewOpenGL/Shaders/background.vs", "../../NewOpenGL/Shaders/background.fs");

	// 定义全屏四边形的顶点数据
	// 格式: 位置(x,y,z), 纹理坐标(u,v)
	float fullscreenQuad[] = {
		// 位置                // 纹理坐标
		-1.0f,  1.0f, 0.0f,   0.0f, 1.0f,  // 左上
		-1.0f, -1.0f, 0.0f,   0.0f, 0.0f,  // 左下
		 1.0f, -1.0f, 0.0f,   1.0f, 0.0f,  // 右下

		-1.0f,  1.0f, 0.0f,   0.0f, 1.0f,  // 左上
		 1.0f, -1.0f, 0.0f,   1.0f, 0.0f,  // 右下
		 1.0f,  1.0f, 0.0f,   1.0f, 1.0f   // 右上
	};

	glGenVertexArrays(1, &backgroundVAO);
	glGenBuffers(1, &backgroundVBO);
	glBindVertexArray(backgroundVAO);
	glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenQuad), fullscreenQuad, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);
}

void ResourceManager::SetupNetworkPlayersModel() {
	// 暂设定除了当前控制角色之外的网络玩家数量上限为3
	for (int i = 0; i < 3; i++) {
		Model_netPlayers.push_back(GameMaterialLoad::LoadModel("../../Assets/legaoPlayer_Useable/legao.gltf"));
	}
}

// 后处理资源初始化实现
void ResourceManager::SetupPostProcessing() {
	// 创建帧缓冲对象
	glGenFramebuffers(1, &postProcessFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, postProcessFBO);

	// 创建两个颜色缓冲（浮点格式）
	glGenTextures(2, postColorBuffers);
	for (unsigned int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, postColorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, postColorBuffers[i], 0);
	}

	// 创建PingPong帧缓冲用于Bloom模糊
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH / 4, SCR_HEIGHT / 4, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
	}

	// 创建屏幕四边形
	float quadVertices[] = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	glGenVertexArrays(1, &postQuadVAO);
	glGenBuffers(1, &postQuadVBO);
	glBindVertexArray(postQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, postQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// 修改帧缓冲附件
	glBindFramebuffer(GL_FRAMEBUFFER, postProcessFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, postProcessFBO);
	unsigned int depthRBO;
	glGenRenderbuffers(1, &depthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);
	// 检查帧缓冲完整性
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		// std::cout << "Post-processing framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}