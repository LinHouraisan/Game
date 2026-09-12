// GameManager.h
#pragma once
#include "BaseGame.h"
#include "TutorialGame.h"
#include "NetworkedGame.h"
#include "UIManager.h"
#include <GLFW/glfw3.h>
#include <memory>

namespace NCL {
    namespace CSC8503 {
        enum class GameType {
            NONE,
            TUTORIAL,  // 单人游戏
            NETWORKED  // 网络游戏
        };

        class GameManager {
        public:
            static GameManager* GetInstance();
            ~GameManager();

            // 创建/销毁游戏实例
            void CreateGame(GameType type, GLFWwindow* window, OpenGL::GameTechRenderer* renderer);
            void DestroyCurrentGame();

            // 获取当前游戏实例
            BaseGame* GetCurrentGame();
            GameType GetCurrentGameType() const { return currentGameType; }

            bool IsPaused() const { return isPaused; }
            void SetPaused(bool paused);

            // 记分板
            UIManager* GetUIManager() const;  
            void SetUIManager(UIManager* uiManager);  

            // 技能状态访问接口
            bool IsDashOnCooldown() const;
            float GetDashCooldownTimer() const;
            float GetDashCooldownTime() const;

            bool IsShockwaveOnCooldown() const;
            float GetShockwaveCooldownTimer() const;
            float GetShockwaveCooldownTime() const;

            bool IsBlackHoleOnCooldown() const;
            float GetBlackHoleCooldownTimer() const;
            float GetBlackHoleCooldownTime() const;

            bool IsTurretOnCooldown() const;
            float GetTurretCooldownTimer() const;
            float GetTurretCooldownTime() const;

        private:
            OpenGL::ResourceManager* resourceManager;
            GameManager();
            static GameManager* instance;

            BaseGame* currentGame;
            GameType currentGameType;

            bool isPaused = false;

            // 记分板
            UIManager* uiManager = nullptr;  
        };
    }
}