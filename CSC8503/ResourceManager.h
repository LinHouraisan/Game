#pragma once
#include "model_animation.h"
#include "shader.h"
#include "Instantiate.h"
#include "GameMaterialLoad.h"
#include <random>
#include "animation_controller.h"
#include "Effeksser.h"
#include "Config.h"

namespace OpenGL {

    class ResourceManager {
    public:

        struct Ray {
            glm::vec3 start;
            glm::vec3 end;
        };


        ResourceManager();
        ~ResourceManager();

        void InitialiseAssets();

        void SetupShaders();
        void SetupSkybox();
        void SetupModels();
        void SetupEffects();
        void SetupFloor();
        void SetupShadow();
        void SetupPathDebug();
        void SetupCrosshair();
        void SetupMap();
        static void SetupRayDebug();
        static void SetupPostProcessing();
		void SetupNetworkPlayersModel();

        static Shader* modelShader_tiger;
        static Shader* modelTestShader;
        static Shader* skyboxShader;
        static Shader* floorShader;
        static Shader* shadowShader;
        static Shader* pathDebugShader;
        static Shader* staticModelShader;
        static Shader* arrowShader;
        static Shader* crosshairShader;
        static Shader* boxDebugShader;        
        static Shader* shadowShader_tiger;
        static Shader* mapShader;
        static Shader* waterShader;
        // 后处理Shader
        static Shader* postToneMappingShader;
        static Shader* postBloomShader;
        static Shader* postRimLightShader;
        static Shader* postFinalShader;
        static Shader* postEcoShader;


        static Model* animationModel_tiger;
        static Model* Model_plane;
        static Model* Model_player;
        static Model* Model_netPlayer;
		static std::vector<Model*> Model_netPlayers;
        static Model* Model_arrow;
        static Model* Model_map1;
        static Model* Model_map2;
        static Model* Model_map3;
        static Model* Model_map4;
        static Model* Model_map5;
        static Model* Model_pistol;
        static Model* Model_gun1;
        static Model* Model_gun2;
        static Model* Model_gun3;
        static Model* Model_switchMapArrow;
        static Model* Model_gun4;
        static Model* Model_coin;
        static Model* Model_barrel;
        static Model* Model_water;
        static Model* Model_gun5;


        static Animation* animation_player;
        static AnimationController* animationController_player;
        static Animation* animation_tiger;
        static AnimationController* animationController_tiger;

        // 注意：特效资源是内部管理的动态指针，这里不要用指针
        static Effekseer::EffectRef laserEffect; 
        static Effekseer::EffectRef pistolBulletEffect;
        static Effekseer::EffectRef shotGunBulletEffect;
        static Effekseer::EffectRef bounceLaserEffect;
        static Effekseer::EffectRef gravityFieldEffect;
        static Effekseer::EffectRef healingEffect;
        static Effekseer::EffectRef explodeEffect;

        //技能特效
        static Effekseer::EffectRef BlackHoleEffect;//黑洞技能特效
        static Effekseer::EffectRef ShockwaveEffect;//震荡波技能特效
        static Effekseer::EffectRef DashingEffect;//冲刺技能特效
        static Effekseer::EffectRef TurretSpawnEffect;//炮台本体特效
        static Effekseer::EffectRef TurretEffect;//炮台技能特效

        static Instantiate* instanceRenderer_tiger;

        static std::vector<std::vector<int>> map1Grid; 
        static std::vector<std::vector<int>> map2Grid;
        static std::vector<std::vector<int>> map3Grid;
        static std::vector<std::vector<int>> map4Grid;
        static std::vector<std::vector<int>> map5Grid;

        static unsigned int depthMapFBO, depthMap;
        static unsigned int skyboxVAO, skyboxVBO, cubemapTexture;
        static unsigned int planeVAO, planeVBO, floorTexture;
        static unsigned int gridVAO, gridVBO;
        static unsigned int crosshairVAO, crosshairVBO, crosshairTexture;
        static unsigned int rayVAO, rayVBO;
        // 后处理帧缓冲对象
        static unsigned int postProcessFBO;
        static unsigned int postColorBuffers[2]; 
        static unsigned int pingpongFBO[2];
        static unsigned int pingpongColorbuffers[2];
        static unsigned int postQuadVAO;
        static unsigned int postQuadVBO;
        static unsigned int depthTexture;

        static glm::mat4 lightSpaceMatrix;

        static void AddDebugRay(const glm::vec3& start, const glm::vec3& end);

        glm::vec3 lightDirection = glm::normalize(glm::vec3(0.5f, -1.0f, 0.5f));
        glm::vec3 lightColor = glm::vec3(1.0f, 0.95f, 0.9f); // 轻微偏暖色
        glm::vec3 lightPos = glm::vec3(-2.0f, 40.0f, -1.0f);

        float lastX = SCR_WIDTH / 2.0f;
        float lastY = SCR_HEIGHT / 2.0f;

        bool firstMouse = true;

        EffekseerManager* effekseerManager;

        static std::vector<Ray> debugRays;

        static Shader* backgroundShader;
        static unsigned int backgroundTexture;
        static unsigned int backgroundVAO, backgroundVBO;
        static void SetupBackground();

        static Shader* damageEffectShader; 
    };
}