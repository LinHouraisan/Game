#include "GameManager.h"
#include "AudioManager.h"
#include "UIManager.h" 

using namespace NCL;
using namespace CSC8503;

GameManager* GameManager::instance = nullptr;

GameManager* GameManager::GetInstance() {
    if (!instance) {
        instance = new GameManager();
    }
    return instance;
}

GameManager::GameManager() : currentGame(nullptr), currentGameType(GameType::NONE) {
    resourceManager = new OpenGL::ResourceManager();
}

GameManager::~GameManager() {
    DestroyCurrentGame();
    delete resourceManager;
    instance = nullptr;
}

void GameManager::CreateGame(GameType type, GLFWwindow* window, OpenGL::GameTechRenderer* renderer) {
    DestroyCurrentGame();

    switch (type) {
    case GameType::TUTORIAL:
        currentGame = new TutorialGame(window, renderer, resourceManager);
        currentGameType = GameType::TUTORIAL;
        break;
    case GameType::NETWORKED:
        currentGame = new NetworkedGame(window, renderer, resourceManager);
        currentGameType = GameType::NETWORKED;
        break;
    default:
        currentGame = nullptr;
        currentGameType = GameType::NONE;
        break;
    }
}

void GameManager::DestroyCurrentGame() {
    if (currentGame) {
        delete currentGame;
        currentGame = nullptr;
    }
    currentGameType = GameType::NONE;
}

BaseGame* GameManager::GetCurrentGame() {
    return currentGame;
}

void GameManager::SetPaused(bool paused) {
    // 如果状态没有变化则不处理
    if (paused == isPaused) return;

    isPaused = paused;

    // 暂停/恢复音频
    if (isPaused) {
        AudioManager::GetInstance()->PauseBGM();
    }
    else {
        AudioManager::GetInstance()->ResumeBGM();
    }

    // 通知当前游戏实例暂停状态变更
    if (currentGame) {
        currentGame->SetPaused(paused);
    }
}

UIManager* GameManager::GetUIManager() const {
    return uiManager;
}

void GameManager::SetUIManager(UIManager* uiManager) {
    this->uiManager = uiManager;
}

bool GameManager::IsDashOnCooldown() const {
    if (!currentGame) return false;
    return currentGame->IsDashOnCooldown();
}

float GameManager::GetDashCooldownTimer() const {
    if (!currentGame) return 0.0f;
    return currentGame->GetDashCooldownTimer();
}

float GameManager::GetDashCooldownTime() const {
    if (!currentGame) return 0.0f;
    return currentGame->GetDashCooldownTime();
}

bool GameManager::IsShockwaveOnCooldown() const {
    if (!currentGame) return false;
    return currentGame->IsShockwaveOnCooldown();
}

float GameManager::GetShockwaveCooldownTimer() const {
    if (!currentGame) return 0.0f;
    return currentGame->GetShockwaveCooldownTimer();
}

float GameManager::GetShockwaveCooldownTime() const {
    if (!currentGame) return 0.0f;
    return currentGame->GetShockwaveCooldownTime();
}

bool GameManager::IsBlackHoleOnCooldown() const {
    if (!currentGame) return false;
    return currentGame->IsBlackHoleOnCooldown();
}

float GameManager::GetBlackHoleCooldownTimer() const {
    if (!currentGame) return 0.0f;
    return currentGame->GetBlackHoleCooldownTimer();
}

float GameManager::GetBlackHoleCooldownTime() const {
    if (!currentGame) return 0.0f;
    return currentGame->GetBlackHoleCooldownTime();
}

bool GameManager::IsTurretOnCooldown() const {
    if (!currentGame) return false;
    return currentGame->IsTurretOnCooldown();
}

float GameManager::GetTurretCooldownTimer() const {
    if (!currentGame) return 0.0f;
    return currentGame->GetTurretCooldownTimer();
}

float GameManager::GetTurretCooldownTime() const {
    if (!currentGame) return 0.0f;
    return currentGame->GetTurretCooldownTime();
}