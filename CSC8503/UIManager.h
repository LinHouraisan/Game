#pragma once
#include <glad/glad.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <vector>
#include <string>
#include <chrono>
#include <unordered_map>

namespace NCL {
	namespace CSC8503 {
		class Player;
	}
}

using namespace NCL::CSC8503;
// 简单计时器记录
struct TimerRecord {
    std::string name;         // 阶段名称
    float elapsedTime;        // 经过时间(毫秒)
    float minTime;            // 最小时间
    float maxTime;            // 最大时间
    float avgTime;            // 平均时间
    int sampleCount;          // 样本数量

    TimerRecord() : name(""), elapsedTime(0.0f), minTime(FLT_MAX), maxTime(0.0f), avgTime(0.0f), sampleCount(0) {}
    TimerRecord(const std::string& name) : name(name), elapsedTime(0.0f), minTime(FLT_MAX), maxTime(0.0f), avgTime(0.0f), sampleCount(0) {}

    void Update(float time) {
        elapsedTime = time;
        minTime = std::min(minTime, time);
        maxTime = std::max(maxTime, time);

        // 更新平均值
        avgTime = (avgTime * sampleCount + time) / (sampleCount + 1);
        sampleCount++;
    }
};

// 用于任意文本信息的调试信息记录
struct DebugInfoRecord {
    std::string name;         // 信息名称
    std::string value;        // 当前值(文本形式)
    std::string category;     // 可选类别

    DebugInfoRecord() : name(""), value(""), category("General") {}
    DebugInfoRecord(const std::string& name, const std::string& value, const std::string& category = "General") : name(name), value(value), category(category) {}
};

// 作用域计时器
class ScopedTimer {
public:
    ScopedTimer(class UIManager* uiManager, const std::string& name);
    ~ScopedTimer();

private:
    class UIManager* uiManager;
    std::string name;
    std::chrono::high_resolution_clock::time_point startTime;
};

class UIManager {
public:
    enum class UIState {
        MAIN_MENU,
        TRANSITIONING_TO_SETTINGS,  
        TRANSITIONING_TO_MAIN_MENU, 
        GAME,
        ONLINE_GAME,
        SETTINGS,
        PAUSE_MENU,
        PAUSE_SETTINGS,
        GAME_OVER,
        EXIT
    };

    struct ButtonTransitionData {
        bool isAnimating;
        float startTime;    // 相对于动画开始的延迟
        float progress;     // 当前进度(0.0-1.0)
        ImVec2 initialPos;  // 起始位置
        ImVec2 currentPos;  // 当前位置
        float currentRotation; // 当前旋转角度
        float currentScale;    // 当前缩放
        float currentAlpha;    // 当前透明度
    };

    struct SkillInfo {
        std::string name;           
        std::string key;            
        bool unlocked;             
        bool onCooldown;            
        float cooldownTime;        
        float currentCooldown;      
        int unlockScore;   
        bool visible;

        SkillInfo() : unlocked(false), onCooldown(false), cooldownTime(0.0f),
            currentCooldown(0.0f), unlockScore(0), visible(false) {}

        SkillInfo(const std::string& n, const std::string& k, float cd, int score)
            : name(n), key(k), unlocked(false), onCooldown(false),
            cooldownTime(cd), currentCooldown(0.0f), unlockScore(score), visible(false) {}
    };

    // 技能UI
    std::vector<SkillInfo> m_Skills;         // 技能信息数组
    ImVec2 m_SkillBarSize = ImVec2(300, 30); // 技能条尺寸
    float m_SkillBarSpacing = 5.0f;          // 技能条间距
    bool m_ShowSkillUnlockMessage = false;   // 是否显示技能解锁消息
    int m_UnlockedSkillIndex = -1;           // 最近解锁的技能索引
    float m_UnlockMessageTime = 0.0f;        // 解锁消息显示时间
    float m_UnlockAnimProgress = 0.0f;       // 解锁动画进度
    ImVec2 m_MessageStartPos;                // 消息起始位置
    ImVec2 m_MessageTargetPos;               // 消息目标位置
    ImVec2 m_MessageStartSize;               // 消息起始大小
    bool m_SkillsInitialized = false;        // 技能是否已初始化
    void InitializeSkills();
    void HandleSkillKeyPress(int key);
    void CheckSkillKeys();
    void RenderSkillBars();                                 
    void UpdateSkillCooldowns(float dt);                    
    void CheckSkillUnlocks(int currentScore);               
    void SetSkillCooldown(int skillIndex, bool onCooldown); 
    void ShowSkillUnlockMessage(int skillIndex);            
    void UpdateSkillUnlockAnimation(float dt);              
    void RenderSkillUnlockMessage();                        


    UIManager();
    ~UIManager();

    void Initialize();
    void Render();
    UIState GetState() const;

    void ToggleDebugWindow();
    bool IsDebugWindowVisible() const;
    void RenderDebugWindow();
    bool ShouldExit() const;

    void AddTimerRecord(const std::string& name, float timeMs);
    ScopedTimer CreateTimer(const std::string& name);
    void UpdateFPS(float fps);
    void ResetTimers();
    void AddDebugInfo(const std::string& name, const std::string& value, const std::string& category = "General");
    void RemoveDebugInfo(const std::string& name);
    void ClearDebugCategory(const std::string& category);

    // 设置的获取方法
    float GetMasterVolume() const;
    float GetSFXVolume() const;
    float GetBGMVolume() const;
    float GetFontScale() const;

    // HUD
    void RenderHUD();                  
    void RenderPlayerStats();           
    void RenderInventoryBar();          
    void RenderPlayerPortrait();       
    
    // 玩家资料
    void RenderPlayerProfile();         
    void TogglePlayerProfile();         
    bool IsPlayerProfileVisible() const; 

    // 设置游戏状态回调
    typedef std::function<void(UIState)> StateChangeCallback;
    void SetStateChangeCallback(StateChangeCallback callback) { m_StateChangeCallback = callback; }

    void HandleMouseClick(int button, int action, int mods);
    void SetState(UIState newState);

    // 暂停菜单
    void TogglePauseMenu();
    bool IsPauseMenuVisible() const;
    void RenderPauseMenu();
    
    //鼠标按钮回调
    static UIManager* s_Instance; // 静态成员变量
    static void MouseButtonCallbackProxy(GLFWwindow* window, int button, int action, int mods);
    void ProcessMouseButtonEvent(GLFWwindow* window, int button, int action, int mods);

    void UpdatePlayerHealth(float currentHealth, float maxHealth);
    Player* m_Player = nullptr;  // 存储玩家引用
    void SetPlayer(Player* player) { m_Player = player; }

    float m_PlayerHealth = 1.0f;  
    float m_PlayerMaxHealth = 100.0f; 

    // 连续开火
    void CheckContinuousFiring(GLFWwindow* window);
    void SetWindow(GLFWwindow* window) { m_Window = window; }

    // 记分板
    void ToggleScoreBoard();  
    bool IsScoreBoardVisible() const;  
    void RenderScoreBoard();  
    //void UpdateKillScore(int amount = 1);  
    //int GetKillScore() const;  

    // 得分版和效果目标
    void ShowLevelCompletePopup();
    void RenderLevelCompletePopup();
    void UpdatePopupTimer(float dt);
    bool IsLevelCompletePopupVisible() const;
    int GetCurrentLevelKillTarget() const;
    void SetCurrentLevelKillTarget(int target) { m_CurrentLevelKillTarget = target; }
    bool m_ShowLevelCompletePopup = false;
    float m_PopupTimer = 0.0f;
    ImVec2 m_PopupSize = ImVec2(700, 200);
    int m_CurrentLevelKillTarget = 20; // 完成关卡所需的击杀数

    // 伤害效果
    void UpdateDamageEffect(float dt);      
    void RenderDamageEffect();             
    bool HasActiveDamageEffect() const;
    void TestDamageEffect(float intensity = 0.5f);

    void ToggleDebugTab(); 
    int m_CurrentDebugTab = 0; 

    void UpdateDebugInfo();
    void CheckScoreForVictory();
    void ShowVictoryPopup();
    void RenderVictoryPopup();
    bool IsVictoryPopupVisible() const;

private:
    void RenderMainMenu();
    void RenderSettings();
    void RenderSettingsNoInteraction();
    void RenderTimers();
    void RenderDebugInfo();

    void UpdateAllAudioVolumes();

    void InitializeBGM();
    void ForceBattleBGM();

    UIState m_State;
    ImVec2 m_WindowSize;
    ImVec2 m_ButtonSize;

    bool m_ShowDebugWindow = false;
    ImVec2 m_DebugWindowSize{ 700, 900 };

    // 设置
    float m_FontScale = 1.4f;
    float m_MasterVolume = 0.8f;
    float m_SFXVolume = 0.7f;
    float m_BGMVolume = 0.6f;

    bool m_ShowPauseMenu = false;

    // HUD大小
    ImVec2 m_HealthBarSize = ImVec2(400, 40);
    ImVec2 m_InventorySlotSize = ImVec2(50, 50);
    ImVec2 m_PortraitSize = ImVec2(80, 80);
    ImVec2 m_ProfileWindowSize = ImVec2(800, 600);

    int m_CurrentProfileTab = 0;      
    bool m_ShowPlayerProfile = false;   

    // FPS初始化
    float m_CurrentFPS = 0.0f;

    // 计时器记录
    std::unordered_map<std::string, TimerRecord> m_TimerRecords;

    // 调试信息记录
    std::unordered_map<std::string, DebugInfoRecord> m_DebugInfoRecords;

    // 调试信息的类别(保持顺序)
    std::vector<std::string> m_InfoCategories;

    // 计时器名称的固定顺序，防止闪烁
    std::vector<std::string> m_TimerOrder;

    // 上次排序时间
    float m_LastSortTime = 0.0f;

    bool m_BackgroundLoaded = false;  // 是否已加载背景
    float m_BackgroundOffsetX = 0.0f; // 背景水平偏移(用于循环移动)
    float m_BackgroundOffsetY = 0.0f; // 背景垂直偏移(用于循环移动)
    float m_BackgroundScrollSpeed = 0.05f; // 背景滚动速度

    // 菜单背景元素纹理
    unsigned int m_WaveTexture = 0;
    unsigned int m_SunTexture = 0;
    unsigned int m_LargeCloudTexture = 0;
    unsigned int m_SmallCloudTexture = 0;

    // 元素偏移量
    float m_SunOffsetY = 0.0f;
    float m_WaveOffsetX = 0.0f;
    float m_LargeCloudOffsetX = 0.0f;
    float m_SmallCloudOffsetX = 0.0f;

    // 元素移动速度
    float m_SunBobSpeed = 0.02f;
    float m_WaveScrollSpeed = 0.08f;
    float m_LargeCloudScrollSpeed = 0.03f;
    float m_SmallCloudScrollSpeed = 0.05f;

    void LoadBackground(const char* filename);
    void RenderBackground();

    // 按钮动画
    bool m_IsTransitioning = false;
    UIState m_TargetState;
    float m_TransitionStartTime = 0.0f;
    float m_ButtonAnimDuration = 0.4f;  // 每个按钮的动画持续时间
    float m_ButtonAnimDelay = 0.1f;     // 按钮间的延迟
    std::vector<ButtonTransitionData> m_ButtonAnimData; // 动画数据

    void InitiateTransition(UIState targetState);
    void UpdateTransition();
    void RenderMenuTransition();
    ImVec2 CalculateBezierPoint(float t, const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3);

    StateChangeCallback m_StateChangeCallback;

    void Render3DButton(ImDrawList* drawList, const char* label, float xPos, float yPos, bool& clicked);
    void Render3DSlider(ImDrawList* drawList, const char* label, float xPos, float yPos, float* value, float min, float max, const char* format);

    // 连续开火
    float m_LastGunshotTime = 0.0f;
    float m_GunshotInterval = 0.25f; // 射击间隔
    GLFWwindow* m_Window = nullptr; 
    int m_ActiveGunshotCount = 0; // 当前活跃的枪声数量
    float m_GunshotDuration = 0.3f; // 每个枪声持续时间估计
    std::vector<float> m_GunshotTimes; // 单次开火时间

    // 记分板
    bool m_ShowScoreBoard = false;  
    ImVec2 m_ScoreBoardSize = ImVec2(400, 300);  // 记分板尺寸

    // 伤害效果
    bool m_IsDamageEffect = false;          
    float m_DamageEffectIntensity = 0.0f;   // 伤害效果强度
    float m_DamageEffectDecayRate = 0.15f;   // 效果消退速度
    float m_LastPlayerHealth = 1.0f; 

    void RenderSystemMemoryInfo();
    void RenderBulletCollisionInfo();

    bool m_ShowVictoryPopup = false;
    float m_VictoryPopupTimer = 0.0f;
    ImVec2 m_VictoryPopupSize = ImVec2(800, 500);
    int m_VictoryScoreThreshold = 200; 
    bool m_VictoryAchieved = false; 
};

// 实现作用域计时器
inline ScopedTimer::ScopedTimer(UIManager* uiManager, const std::string& name) : uiManager(uiManager), name(name) {
    startTime = std::chrono::high_resolution_clock::now();
}

inline ScopedTimer::~ScopedTimer() {
    auto endTime = std::chrono::high_resolution_clock::now();
    float timeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    if (uiManager) {
        uiManager->AddTimerRecord(name, timeMs);
    }
}

inline ScopedTimer UIManager::CreateTimer(const std::string& name) {
    return ScopedTimer(this, name);
}
