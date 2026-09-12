#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

#include "UIManager.h"
#include "ResourceManager.h"
#include "AudioManager.h"
#include "GameManager.h"
#include "player.h"
#include "Monster.h"

#include "stb_image.h" 

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <imgui_internal.h>


using namespace OpenGL;

UIManager* UIManager::s_Instance = nullptr; // ��̬��Ա������ʼ��

UIManager::UIManager() : m_State(UIState::MAIN_MENU), m_WindowSize(1920, 1080), m_ButtonSize(200, 50) {
    s_Instance = this;

    // Ĭ������
    m_MasterVolume = 0.8f;
    m_SFXVolume = 0.7f;
    m_BGMVolume = 0.6f;
    m_FontScale = 1.4f;

    AudioManager::GetInstance()->Init();

    if (AudioManager::GetInstance()->LoadBGM("../../Assets/Audio/BGM/SoundTrack.mp3")) {
        AudioManager::GetInstance()->SetBGMVolume(m_BGMVolume);
    }

    AudioManager::GetInstance()->LoadBGM("../../Assets/Audio/BGM/maou_bgm_fantasy12_need_boss.mp3");
    AudioManager::GetInstance()->LoadSFX("gunshot", "../../Assets/Audio/SFX/SE.wav");
    AudioManager::GetInstance()->LoadSFX("click", "../../Assets/Audio/SFX/WaterDropBig.mp3");

    AudioManager::GetInstance()->SetSFXVolume(m_SFXVolume);
}

UIManager::~UIManager() {

    AudioManager::Release();
}

UIManager::UIState UIManager::GetState() const {
    return m_State;
}

void UIManager::ToggleDebugWindow() {
    m_ShowDebugWindow = !m_ShowDebugWindow;
}

bool UIManager::IsDebugWindowVisible() const {
    return m_ShowDebugWindow;
}

void UIManager::Initialize() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::GetStyle().FrameRounding = 4.0f;
    ImGui::GetStyle().GrabRounding = 4.0f;

    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    ImGui::GetIO().FontGlobalScale = m_FontScale;

    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.WindowPadding = ImVec2(12, 12);

    LoadBackground("../../Assets/UI/LakeAndMountains.png");

    if (OpenGL::ResourceManager::backgroundShader) {
        OpenGL::ResourceManager::backgroundShader->use();
        OpenGL::ResourceManager::backgroundShader->setVec4("position", glm::vec4(-1.0f, -1.0f, 2.0f, 2.0f));
        OpenGL::ResourceManager::backgroundShader->setVec2("textureOffset", glm::vec2(0.0f, 0.0f));
    }

    m_ButtonAnimDuration = 0.5f;
    m_ButtonAnimDelay = 0.1f;
    m_IsTransitioning = false;

    InitializeBGM();
}

void UIManager::Render() {
    static UIState lastState = m_State;

    if (lastState != m_State) {
        // std::cout << "״̬�� " << static_cast<int>(lastState) << " ��Ϊ " << static_cast<int>(m_State) << std::endl;

        // ��������˵�/���õ���Ϸ��ת��
        if ((m_State == UIState::GAME || m_State == UIState::ONLINE_GAME) && (lastState == UIState::MAIN_MENU || lastState == UIState::SETTINGS || lastState == UIState::TRANSITIONING_TO_SETTINGS || lastState == UIState::TRANSITIONING_TO_MAIN_MENU)) {
            // std::cout << "ֹͣ�˵�BGMǰ״̬: " << (AudioManager::GetInstance()->IsBGMPlaying() ? "������" : "��ֹͣ") << std::endl;
            AudioManager::GetInstance()->StopBGM();
            // std::cout << "ֹͣ�˵�BGM��״̬: " << (AudioManager::GetInstance()->IsBGMPlaying() ? "���ڲ���" : "��ֹͣ") << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            bool loaded = AudioManager::GetInstance()->LoadBGM("../../Assets/Audio/BGM/maou_bgm_fantasy12_need_boss.mp3");
            // std::cout << "ս��BGM���ؽ��: " << (loaded ? "�ɹ�" : "ʧ��") << std::endl;

            if (loaded) {
                float volume = m_BGMVolume * m_MasterVolume;
                // std::cout << "����ս��BGM����: " << volume << std::endl;
                AudioManager::GetInstance()->SetBGMVolume(volume);

                AudioManager::GetInstance()->PlayBGM(true);
                bool isPlaying = AudioManager::GetInstance()->IsBGMPlaying();
                // std::cout << "����ս��BGM���ý��: " << (isPlaying ? "�ɹ�" : "ʧ��") << std::endl;

                // std::cout << "���ź�BGM״̬: " << (AudioManager::GetInstance()->IsBGMPlaying() ? "������" : "δ����") << std::endl;

                if (!AudioManager::GetInstance()->IsBGMPlaying()) {
                    // std::cout << "�����ٴβ���ս��BGM..." << std::endl;
                    AudioManager::GetInstance()->PlayBGM(true);
                    // std::cout << "�ڶ��γ��Ժ�BGM״̬: " << (AudioManager::GetInstance()->IsBGMPlaying() ? "������" : "��δ����") << std::endl;
                }
            }
            else {
                // std::cout << "�޷�����ս��BGM������..." << std::endl;
                AudioManager::GetInstance()->Init();
                bool retryLoaded = AudioManager::GetInstance()->LoadBGM("../../Assets/Audio/BGM/maou_bgm_fantasy12_need_boss.mp3");
                if (retryLoaded) {
                    AudioManager::GetInstance()->SetBGMVolume(m_BGMVolume * m_MasterVolume);
                    AudioManager::GetInstance()->PlayBGM(true);
                    // std::cout << "�����BGM״̬: " << (AudioManager::GetInstance()->IsBGMPlaying() ? "������" : "δ����") << std::endl;
                }
            }
        }

        else if ((lastState == UIState::GAME || lastState == UIState::ONLINE_GAME) && (m_State == UIState::MAIN_MENU || m_State == UIState::SETTINGS || m_State == UIState::TRANSITIONING_TO_SETTINGS || m_State == UIState::TRANSITIONING_TO_MAIN_MENU)) {

            // std::cout << "�л��ز˵�BGM" << std::endl;
            AudioManager::GetInstance()->StopBGM();

            if (AudioManager::GetInstance()->LoadBGM("../../Assets/Audio/BGM/SoundTrack.mp3")) {
                AudioManager::GetInstance()->SetBGMVolume(m_BGMVolume * m_MasterVolume);
                AudioManager::GetInstance()->PlayBGM(true);
                // std::cout << "�˵�BGM����״̬: " << (AudioManager::GetInstance()->IsBGMPlaying() ? "������" : "δ����") << std::endl;
            }
        }

        lastState = m_State;
    }

    if ((m_State == UIState::MAIN_MENU || m_State == UIState::SETTINGS ||
        m_State == UIState::TRANSITIONING_TO_SETTINGS || m_State == UIState::TRANSITIONING_TO_MAIN_MENU) &&
        !AudioManager::GetInstance()->IsBGMPlaying()) {

        // std::cout << "���˵�û��BGM�����Իָ�..." << std::endl;
        if (AudioManager::GetInstance()->LoadBGM("../../Assets/Audio/BGM/SoundTrack.mp3")) {
            AudioManager::GetInstance()->SetBGMVolume(m_BGMVolume * m_MasterVolume);
            AudioManager::GetInstance()->PlayBGM(true);
        }
    }

    if ((m_State == UIState::GAME || m_State == UIState::ONLINE_GAME) && !AudioManager::GetInstance()->IsBGMPlaying()) {

        // std::cout << "��Ϸ״̬û��BGM�����Իָ�ս��BGM..." << std::endl;
        if (AudioManager::GetInstance()->LoadBGM("../../Assets/Audio/BGM/maou_bgm_fantasy12_need_boss.mp3")) {
            AudioManager::GetInstance()->SetBGMVolume(m_BGMVolume * m_MasterVolume);
            AudioManager::GetInstance()->PlayBGM(true);
            // std::cout << "�ָ�ս��BGM״̬: " << (AudioManager::GetInstance()->IsBGMPlaying() ? "������" : "��δ����") << std::endl;
        }
    }

    if (m_State == UIState::MAIN_MENU || m_State == UIState::SETTINGS ||
        m_State == UIState::TRANSITIONING_TO_SETTINGS || m_State == UIState::TRANSITIONING_TO_MAIN_MENU) {
        RenderBackground();
    }

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y));
    ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);

    switch (m_State) {
    case UIState::MAIN_MENU:
        RenderMainMenu();
        break;
    case UIState::TRANSITIONING_TO_SETTINGS:
        RenderMenuTransition();
        UpdateTransition();
        break;
    case UIState::TRANSITIONING_TO_MAIN_MENU:
        RenderMenuTransition();
        UpdateTransition();
        break;
    case UIState::SETTINGS:
        RenderSettings();
        break;
    case UIState::PAUSE_SETTINGS:
        RenderSettings();  // ������ͬ�����ý���
        break;
    case UIState::GAME:
    case UIState::ONLINE_GAME:
        if (m_ShowPauseMenu) {
            RenderPauseMenu();
        }
        break;
    case UIState::EXIT:
        break;
    default:
        // std::cout << "Warning: Unknown UI state: " << static_cast<int>(m_State) << std::endl;
        break;
    }

    ImGui::End();

    glDisable(GL_BLEND);
}

bool UIManager::HasActiveDamageEffect() const
{
    return m_IsDamageEffect && m_DamageEffectIntensity > 0.01f;
}

void UIManager::TestDamageEffect(float intensity)
{
    m_IsDamageEffect = true;
    m_DamageEffectIntensity = intensity;
}

void UIManager::RenderMainMenu() {
    float xPos = m_WindowSize.x * 2 / 5;
    float startY = (m_WindowSize.y - 4 * m_ButtonSize.y - 3 * 20) / 2;

    ImGui::SetCursorPos(ImVec2(xPos, startY - 60));
    ImGui::PushFont(ImGui::GetFont());
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
    ImGui::Text("Menu");
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const char* buttonLabels[] = { "Start Game", "Online Game", "Setting", "Exit" };

    // ��ť3DЧ������
    float depthFactor = 15.0f;      // ����3DЧ����ǿ��
    float shadowOffset = 5.0f;      // ��Ӱƫ����
    float hoverScaleFactor = 1.05f; // ��ͣʱ����������

    for (int i = 0; i < 4; i++) {

        float yPos = startY + i * (m_ButtonSize.y + 20);
        ImVec2 mousePos = ImGui::GetIO().MousePos;
        bool isHovered = mousePos.x >= xPos && mousePos.x <= (xPos + m_ButtonSize.x) &&
            mousePos.y >= yPos && mousePos.y <= (yPos + m_ButtonSize.y);

        // ������ͣ״̬����Ч��ǿ��
        float effectDepth = isHovered ? depthFactor * 1.2f : depthFactor;
        ImVec2 buttonSize = isHovered ? ImVec2(m_ButtonSize.x * hoverScaleFactor, m_ButtonSize.y * hoverScaleFactor) : m_ButtonSize;

        // ���㰴ť���ĸ�������������������3DЧ��
        ImVec2 topLeft = ImVec2(xPos - effectDepth, yPos);
        ImVec2 topRight = ImVec2(xPos + buttonSize.x, yPos - effectDepth);
        ImVec2 bottomRight = ImVec2(xPos + buttonSize.x, yPos + buttonSize.y - effectDepth);
        ImVec2 bottomLeft = ImVec2(xPos - effectDepth, yPos + buttonSize.y);

        // ��ť��Ӱ
        ImVec2 shadowTopLeft = ImVec2(topLeft.x + shadowOffset, topLeft.y + shadowOffset);
        ImVec2 shadowTopRight = ImVec2(topRight.x + shadowOffset, topRight.y + shadowOffset);
        ImVec2 shadowBottomRight = ImVec2(bottomRight.x + shadowOffset, bottomRight.y + shadowOffset);
        ImVec2 shadowBottomLeft = ImVec2(bottomLeft.x + shadowOffset, bottomLeft.y + shadowOffset);

        drawList->AddQuadFilled(
            shadowTopLeft, shadowTopRight, shadowBottomRight, shadowBottomLeft,
            IM_COL32(30, 30, 30, 180) // ��͸����ɫ��Ӱ
        );

        // ��ť������ɫ
        ImU32 colorLeft = isHovered ? IM_COL32(100, 100, 180, 255) : IM_COL32(60, 60, 120, 255); // ���ϰ�
        ImU32 colorRight = isHovered ? IM_COL32(150, 150, 220, 255) : IM_COL32(100, 100, 180, 255); // �Ҳ����

        // ���ư�ť���� - ��벿��
        drawList->AddTriangleFilled(
            topLeft, ImVec2(xPos + buttonSize.x / 2, yPos - effectDepth / 2), bottomLeft,
            colorLeft
        );

        // ���ư�ť���� - �Ұ벿��
        drawList->AddTriangleFilled(
            ImVec2(xPos + buttonSize.x / 2, yPos - effectDepth / 2), topRight, bottomRight,
            colorRight
        );
        drawList->AddTriangleFilled(
            bottomLeft, ImVec2(xPos + buttonSize.x / 2, yPos - effectDepth / 2), bottomRight,
            colorRight
        );

        // ���ư�ť�߿�
        ImU32 borderColor = isHovered ? IM_COL32(220, 220, 255, 255) : IM_COL32(180, 180, 220, 255);
        drawList->AddQuad(
            topLeft, topRight, bottomRight, bottomLeft,
            borderColor, 1.5f
        );

        // ���ư�ť�ı�
        ImVec2 textSize = ImGui::CalcTextSize(buttonLabels[i]);
        float textX = xPos + (buttonSize.x - textSize.x) / 2 - effectDepth / 2;
        float textY = yPos + (buttonSize.y - textSize.y) / 2 - effectDepth / 3;

        drawList->AddText(
            ImVec2(textX + 1, textY + 1),
            IM_COL32(20, 20, 20, 200),
            buttonLabels[i]
        );

        drawList->AddText(
            ImVec2(textX, textY),
            isHovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(240, 240, 240, 255),
            buttonLabels[i]
        );

        if (isHovered && ImGui::IsMouseClicked(0)) {
            AudioManager::GetInstance()->PlaySFX("click");

            switch (i) {
            case 0: // Start Game
                // std::cout << "Start Game button clicked" << std::endl;
                AudioManager::GetInstance()->StopBGM();
                SetState(UIState::GAME);
                if (m_StateChangeCallback) {
                    m_StateChangeCallback(UIState::GAME);
                }
                break;
            case 1: // Online Game
                // std::cout << "Online Game button clicked" << std::endl;
                AudioManager::GetInstance()->StopBGM();
                m_State = UIState::ONLINE_GAME;
                if (m_StateChangeCallback) {
                    m_StateChangeCallback(UIState::ONLINE_GAME);
                }
                break;
            case 2: // Setting
                // std::cout << "Setting button clicked - starting transition" << std::endl;
                InitiateTransition(UIState::SETTINGS);
                break;
            case 3: // Exit
                // std::cout << "Exit button clicked" << std::endl;
                m_State = UIState::EXIT;
                break;
            }
        }
    }
}

void UIManager::RenderSettings() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float xPos = m_WindowSize.x * 2 / 5;
    float startY = (m_WindowSize.y - 8 * m_ButtonSize.y - 7 * 20) / 2;
    float currentY = startY;

    float titleX = xPos;
    float titleY = currentY - 60;

    // �ı���Ӱ
    drawList->AddText(
        ImGui::GetFont(),
        ImGui::GetFontSize() * 1.4f,
        ImVec2(titleX + 2, titleY + 2),
        IM_COL32(50, 50, 50, 200),
        "Setting"
    );

    // ����
    drawList->AddText(
        ImGui::GetFont(),
        ImGui::GetFontSize() * 1.4f,
        ImVec2(titleX, titleY),
        IM_COL32(255, 215, 80, 255), // ��ɫ
        "Setting"
    );

    // ʹ��3D�������������
    Render3DSlider(drawList, "Master Audio", xPos, currentY, &m_MasterVolume, 0.0f, 1.0f, "%.2f");
    currentY += m_ButtonSize.y + 20;

    Render3DSlider(drawList, "Sound Effects", xPos, currentY, &m_SFXVolume, 0.0f, 1.0f, "%.2f");
    currentY += m_ButtonSize.y + 20;

    Render3DSlider(drawList, "BGM", xPos, currentY, &m_BGMVolume, 0.0f, 1.0f, "%.2f");
    currentY += m_ButtonSize.y + 20;

    Render3DSlider(drawList, "UI Font Size", xPos, currentY, &m_FontScale, 0.8f, 2.0f, "%.2f");
    currentY += m_ButtonSize.y + 20;
    
    //����
    bool startNetServerClicked = false;
    ImGui::SetCursorPos(ImVec2(xPos, currentY));
    Render3DButton(drawList, "Start Net Server", xPos, currentY, startNetServerClicked);
    if (startNetServerClicked) {
        // std::cout << "Start Net Server button clicked" << std::endl;
    }
    currentY += m_ButtonSize.y + 20;

    // ���ذ�ť��������ͣ�˵������˵�
    bool backClicked = false;
    ImGui::SetCursorPos(ImVec2(xPos, currentY));
    Render3DButton(drawList, "Back", xPos, currentY, backClicked);
    if (backClicked) {
        // std::cout << "Back button clicked - from state: " << static_cast<int>(m_State) << std::endl;

        if (m_State == UIState::PAUSE_SETTINGS) {
            // ������Ǵ���ͣ�˵��������ã����ص���Ϸʱ��ʾ��ͣ�˵�
            m_State = UIState::GAME; 
            m_ShowPauseMenu = true;
            // std::cout << "Returning to game with pause menu" << std::endl;
        }
        else {
            // ����������Ļ����Ϊ�����ص����˵�
            // std::cout << "Returning to main menu from settings" << std::endl;
            InitiateTransition(UIState::MAIN_MENU);
        }
    }
}

void UIManager::RenderDebugWindow() {
    if (!m_ShowDebugWindow) return;

    float padding = 10.0f;
    ImGui::SetNextWindowPos(
        ImVec2(ImGui::GetIO().DisplaySize.x - m_DebugWindowSize.x - padding, padding),
        ImGuiCond_Always
    );
    ImGui::SetNextWindowSize(m_DebugWindowSize, ImGuiCond_Always);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav;

    bool windowOpen = m_ShowDebugWindow;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.85f));

    ImGui::Begin("DebugInfo", &windowOpen, window_flags);
    m_ShowDebugWindow = windowOpen;
    ImGui::PushFont(ImGui::GetFont());
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "FPS: %.1f", m_CurrentFPS);
    ImGui::PopFont();

    // 创建选项卡但不使用BeginTabBar/EndTabBar
    // 相反，根据m_CurrentDebugTab手动处理选项卡选择
    const char* tabs[] = { "Performances", "DebugInfo", "Memory", "Physics" };
    int tabCount = 4; 

    // 选项卡按钮
    for (int i = 0; i < tabCount; i++) {
        if (i > 0) ImGui::SameLine();
        bool isSelected = (m_CurrentDebugTab == i);

        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.7f, 1.0f));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.5f, 1.0f));
        }

        if (ImGui::Button(tabs[i], ImVec2(m_DebugWindowSize.x / tabCount - 5, 0))) {
            m_CurrentDebugTab = i;
        }

        ImGui::PopStyleColor();
    }

    ImGui::Separator();

    // 根据所选选项卡显示内容
    if (m_CurrentDebugTab == 0) {
        RenderTimers();
    }
    else if (m_CurrentDebugTab == 1 && !m_DebugInfoRecords.empty()) {
        RenderDebugInfo();
    }
    else if (m_CurrentDebugTab == 2) {
        RenderSystemMemoryInfo();
    }
    else if (m_CurrentDebugTab == 3) {
        RenderBulletCollisionInfo();
    }

    ImGui::End();
    ImGui::PopStyleColor();
}

void UIManager::RenderTimers() {
    if (!m_TimerRecords.empty()) {
        ImGui::Text("Render Stage Timings:");
        ImGui::Columns(2, "TimerColumns", true);
        ImGui::SetColumnWidth(0, 250); // Increase name column width
        ImGui::Text("Stage"); ImGui::NextColumn();
        ImGui::Text("Time (ms)"); ImGui::NextColumn();
        ImGui::Separator();

        static bool needSorting = true;
        static int lastRecordCount = 0;

        if (m_TimerRecords.size() != lastRecordCount) {
            needSorting = true;
            lastRecordCount = m_TimerRecords.size();
        }

        for (const auto& pair : m_TimerRecords) {
            if (std::find(m_TimerOrder.begin(), m_TimerOrder.end(), pair.first) == m_TimerOrder.end()) {
                m_TimerOrder.push_back(pair.first);
                needSorting = true;
            }
        }

        static float lastSortTime = 0;
        float currentTime = ImGui::GetTime();
        if ((currentTime - lastSortTime > 2.0f && needSorting) || m_TimerOrder.empty()) {
            std::sort(m_TimerOrder.begin(), m_TimerOrder.end(), [this](const std::string& a, const std::string& b) {
                return m_TimerRecords[a].avgTime > m_TimerRecords[b].avgTime;
                });
            lastSortTime = currentTime;
            needSorting = false;
        }

        float totalTime = 0.0f;
        for (const auto& name : m_TimerOrder) {
            auto it = m_TimerRecords.find(name);
            if (it != m_TimerRecords.end()) {
                const TimerRecord& record = it->second;
                totalTime += record.elapsedTime;

                ImGui::Text("%s", record.name.c_str()); ImGui::NextColumn();
                ImGui::Text("%.2f (avg: %.2f)", record.elapsedTime, record.avgTime);
                ImGui::NextColumn();
            }
        }

        ImGui::Separator();
        ImGui::Text("Total"); ImGui::NextColumn();
        ImGui::Text("%.2f", totalTime); ImGui::NextColumn();

        ImGui::Columns(1);
    }
}

void UIManager::RenderDebugInfo() {
    for (const auto& category : m_InfoCategories) {
        bool hasCategoryItems = false;
        for (const auto& pair : m_DebugInfoRecords) {
            if (pair.second.category == category) {
                hasCategoryItems = true;
                break;
            }
        }

        if (!hasCategoryItems) continue;

        if (ImGui::CollapsingHeader(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Columns(2, ("Columns_" + category).c_str(), true);
            ImGui::SetColumnWidth(0, 250); 

            std::vector<std::string> names;
            for (const auto& pair : m_DebugInfoRecords) {
                if (pair.second.category == category) {
                    names.push_back(pair.first);
                }
            }

            std::sort(names.begin(), names.end());

            for (const auto& name : names) {
                const auto& record = m_DebugInfoRecords[name];
                ImGui::Text("%s", record.name.c_str()); ImGui::NextColumn();
                ImGui::Text("%s", record.value.c_str()); ImGui::NextColumn();
            }

            ImGui::Columns(1);
            ImGui::Separator();
        }
    }
}

void UIManager::UpdateFPS(float fps) {
    m_CurrentFPS = fps;
}

void UIManager::AddTimerRecord(const std::string& name, float timeMs) {

    if (m_TimerRecords.find(name) == m_TimerRecords.end()) {
        m_TimerRecords[name] = TimerRecord(name);
    }

    m_TimerRecords[name].Update(timeMs);
}

void UIManager::ResetTimers() {
    m_TimerRecords.clear();
    m_TimerOrder.clear();
}

void UIManager::AddDebugInfo(const std::string& name, const std::string& value, const std::string& category) {
    // Debug info records
    auto it = m_DebugInfoRecords.find(name);
    if (it != m_DebugInfoRecords.end()) {
        it->second.value = value;
    }
    else {
        m_DebugInfoRecords[name] = DebugInfoRecord(name, value, category);

        // Add category if it doesn't exist
        if (std::find(m_InfoCategories.begin(), m_InfoCategories.end(), category) == m_InfoCategories.end()) {
            m_InfoCategories.push_back(category);
        }
    }
}

void UIManager::RemoveDebugInfo(const std::string& name) {
    m_DebugInfoRecords.erase(name);

    // Check if category needs to be removed
    for (auto it = m_InfoCategories.begin(); it != m_InfoCategories.end();) {
        bool categoryUsed = false;
        for (const auto& pair : m_DebugInfoRecords) {
            if (pair.second.category == *it) {
                categoryUsed = true;
                break;
            }
        }

        if (!categoryUsed) {
            it = m_InfoCategories.erase(it);
        }
        else {
            ++it;
        }
    }
}

void UIManager::ClearDebugCategory(const std::string& category) {
    // Remove all debug info in category
    for (auto it = m_DebugInfoRecords.begin(); it != m_DebugInfoRecords.end();) {
        if (it->second.category == category) {
            it = m_DebugInfoRecords.erase(it);
        }
        else {
            ++it;
        }
    }

    // Remove category
    m_InfoCategories.erase(
        std::remove(m_InfoCategories.begin(), m_InfoCategories.end(), category),
        m_InfoCategories.end()
    );
}

bool UIManager::ShouldExit() const {
    return m_State == UIState::EXIT;
}

void UIManager::LoadBackground(const char* filename) {
    // 加载主背景图像纹理
    OpenGL::ResourceManager::backgroundTexture = GameMaterialLoad::LoadTexture(filename);

    // 加载额外的菜单元素
    m_WaveTexture = GameMaterialLoad::LoadTexture("../../Assets/UI/Waves.png");
    m_SunTexture = GameMaterialLoad::LoadTexture("../../Assets/UI/Sun.png");
    m_LargeCloudTexture = GameMaterialLoad::LoadTexture("../../Assets/UI/LargeCloud.png");
    m_SmallCloudTexture = GameMaterialLoad::LoadTexture("../../Assets/UI/SmallCloud.png");

    // 检查主背景是否成功加载
    if (OpenGL::ResourceManager::backgroundTexture) {
        // 纹理参数设置
        glBindTexture(GL_TEXTURE_2D, OpenGL::ResourceManager::backgroundTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        m_BackgroundLoaded = true;
        // std::cout << "背景纹理加载成功: " << filename << std::endl;
    }
    else {
        // std::cout << "背景纹理加载失败: " << filename << std::endl;
    }

    // 为所有元素设置纹理参数
    for (unsigned int* texture : {&m_WaveTexture, &m_SunTexture, &m_LargeCloudTexture, &m_SmallCloudTexture}) {
        if (*texture) {
            glBindTexture(GL_TEXTURE_2D, *texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }

    if (m_WaveTexture && m_SunTexture && m_LargeCloudTexture && m_SmallCloudTexture) {
        // std::cout << "sucessful" << std::endl;
    }
    else {
        // std::cout << "defeat" << std::endl;
    }
}

void UIManager::RenderBackground() {
    if (!m_BackgroundLoaded) return;

    // 保存当前 OpenGL 状态
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);

    float currentTime = ImGui::GetTime();

    // 更新各元素的偏移位置
    m_BackgroundOffsetX += m_BackgroundScrollSpeed * 0.01f;
    if (m_BackgroundOffsetX > 1.0f) m_BackgroundOffsetX -= 1.0f;

    m_WaveOffsetX += m_WaveScrollSpeed * 0.01f;
    if (m_WaveOffsetX > 1.0f) m_WaveOffsetX -= 1.0f;

    m_SunOffsetY = sin(currentTime * m_SunBobSpeed) * 0.05f;

    m_LargeCloudOffsetX += m_LargeCloudScrollSpeed * 0.01f;
    if (m_LargeCloudOffsetX > 1.0f) m_LargeCloudOffsetX -= 1.0f;

    m_SmallCloudOffsetX += m_SmallCloudScrollSpeed * 0.01f;
    if (m_SmallCloudOffsetX > 1.0f) m_SmallCloudOffsetX -= 1.0f;

    // 禁用深度测试以确保背景位于底层
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    OpenGL::ResourceManager::backgroundShader->use();

    // 1. 首先渲染太阳 (最底层)
    if (m_SunTexture) {
        OpenGL::ResourceManager::backgroundShader->setVec2("textureOffset", glm::vec2(0.0f, m_SunOffsetY)); // 垂直浮动
        OpenGL::ResourceManager::backgroundShader->setVec4("position", glm::vec4(-1.0f, 0.0f, 2.0f, 2.0f)); // 全屏渲染

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_SunTexture);
        OpenGL::ResourceManager::backgroundShader->setInt("backgroundTexture", 0);

        glBindVertexArray(OpenGL::ResourceManager::backgroundVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // 2. 渲染主背景 (湖和山) - 第二层
    OpenGL::ResourceManager::backgroundShader->setVec2("textureOffset", glm::vec2(m_BackgroundOffsetX, m_BackgroundOffsetY));
    OpenGL::ResourceManager::backgroundShader->setVec4("position", glm::vec4(-1.0f, -1.0f, 2.0f, 1.5f)); // 全屏

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, OpenGL::ResourceManager::backgroundTexture);
    OpenGL::ResourceManager::backgroundShader->setInt("backgroundTexture", 0);

    glBindVertexArray(OpenGL::ResourceManager::backgroundVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 3. 渲染波浪 (底部长条) - 第三层
    if (m_WaveTexture) {
        OpenGL::ResourceManager::backgroundShader->setVec2("textureOffset", glm::vec2(m_WaveOffsetX, 0.0f));
        // 渲染在屏幕底部，与屏幕等宽
        OpenGL::ResourceManager::backgroundShader->setVec4("position", glm::vec4(-1.0f, -0.8f, 2.0f, 0.6f)); // 底部区域，高度稍微调高

        glBindTexture(GL_TEXTURE_2D, m_WaveTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // 4. 渲染大云 - 第四层 
    if (m_LargeCloudTexture) {
        OpenGL::ResourceManager::backgroundShader->setVec2("textureOffset", glm::vec2(m_LargeCloudOffsetX, 0.0f));
        // 渲染在上半部区域，但横跨整个屏幕宽度
        OpenGL::ResourceManager::backgroundShader->setVec4("position", glm::vec4(-1.0f, 0.2f, 2.0f, 0.5f)); // 上部区域，全屏宽度

        glBindTexture(GL_TEXTURE_2D, m_LargeCloudTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // 5. 渲染小云 - 最上层 
    if (m_SmallCloudTexture) {
        OpenGL::ResourceManager::backgroundShader->setVec2("textureOffset", glm::vec2(m_SmallCloudOffsetX, 0.0f));
        // 渲染在上半部区域，但横跨整个屏幕宽度
        OpenGL::ResourceManager::backgroundShader->setVec4("position", glm::vec4(-1.0f, 0.2f, 2.0f, 0.5f)); // 上部区域，全屏宽度

        glBindTexture(GL_TEXTURE_2D, m_SmallCloudTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // 恢复之前的 OpenGL 状态
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
    glDisable(GL_BLEND);

    // 重置 OpenGL 状态，确保不影响其他渲染
    glUseProgram(0);
    glBindVertexArray(0);
}

void UIManager::InitiateTransition(UIState targetState) {
    m_TargetState = targetState;
    m_TransitionStartTime = ImGui::GetTime();
    m_IsTransitioning = true;

    if (m_State == UIState::GAME || m_State == UIState::ONLINE_GAME) {
        if (targetState == UIState::MAIN_MENU) {
            AudioManager::GetInstance()->StopBGM();
            if (AudioManager::GetInstance()->LoadBGM("../../Assets/Audio/BGM/SoundTrack.mp3")) {
                AudioManager::GetInstance()->SetBGMVolume(m_BGMVolume * m_MasterVolume);
                AudioManager::GetInstance()->PlayBGM(true);
                // std::cout << "�ָ��˵�BGM" << std::endl;
            }
        }
    }

    // Clear button animation data
    m_ButtonAnimData.clear();

    float xPos = m_WindowSize.x * 2 / 5;
    float startY = (m_WindowSize.y - 4 * m_ButtonSize.y - 3 * 20) / 2;

    for (int i = 0; i < 4; i++) {
        ButtonTransitionData data;
        data.isAnimating = true;
        data.startTime = i * m_ButtonAnimDelay; // Staggered delay for animation
        data.progress = 0.0f;
        data.initialPos = ImVec2(xPos, startY + i * (m_ButtonSize.y + 20));
        data.currentPos = data.initialPos;
        data.currentRotation = 0.0f;
        data.currentScale = 1.0f;
        data.currentAlpha = 1.0f;

        m_ButtonAnimData.push_back(data);
    }

    if (m_TargetState == UIState::SETTINGS) {
        m_State = UIState::TRANSITIONING_TO_SETTINGS;
        // std::cout << "Transitioning to Settings" << std::endl;
    }
    else {
        m_State = UIState::TRANSITIONING_TO_MAIN_MENU;
        // std::cout << "Transitioning to Main Menu" << std::endl;
    }

    if (!AudioManager::GetInstance()->IsBGMPlaying()) {
        AudioManager::GetInstance()->PlayBGM(true);
    }
}

void UIManager::UpdateTransition() {
    float currentTime = ImGui::GetTime();
    float elapsedTime = currentTime - m_TransitionStartTime;

    bool allComplete = true;

    for (auto& data : m_ButtonAnimData) {
        // Check if button should start animating
        float buttonElapsedTime = elapsedTime - data.startTime;

        if (buttonElapsedTime < 0) {
            // Button hasn't started animating yet
            allComplete = false;
        }
        else if (buttonElapsedTime <= m_ButtonAnimDuration) {
            // Button is currently animating
            data.progress = buttonElapsedTime / m_ButtonAnimDuration;

            // Update position, rotation, scale and opacity
            if (m_State == UIState::TRANSITIONING_TO_SETTINGS) {
                // Main menu to settings, rotate and fly out
                // Bezier curve control points
                ImVec2 p0 = data.initialPos;
                ImVec2 p1 = ImVec2(p0.x + m_ButtonSize.x * 0.7f, p0.y - m_ButtonSize.y * 0.3f);
                ImVec2 p2 = ImVec2(m_WindowSize.x + m_ButtonSize.x * 0.5f, p0.y - m_ButtonSize.y * 0.7f);
                ImVec2 p3 = ImVec2(m_WindowSize.x + m_ButtonSize.x, p0.y);

                // Calculate position
                data.currentPos = CalculateBezierPoint(data.progress, p0, p1, p2, p3);

                // Rotation (0 to 90 degrees)
                data.currentRotation = data.progress * 90.0f;

                // Scale (shrink to 70% of original)
                data.currentScale = 1.0f - 0.3f * data.progress;

                // Opacity (fade out)
                data.currentAlpha = 1.0f - 0.8f * data.progress;
            }
            else {
                // Settings to main menu, rotate in from right
                // Bezier curve control points
                ImVec2 p0 = ImVec2(m_WindowSize.x + m_ButtonSize.x, data.initialPos.y);
                ImVec2 p1 = ImVec2(m_WindowSize.x + m_ButtonSize.x * 0.5f, data.initialPos.y - m_ButtonSize.y * 0.7f);
                ImVec2 p2 = ImVec2(data.initialPos.x + m_ButtonSize.x * 0.7f, data.initialPos.y - m_ButtonSize.y * 0.3f);
                ImVec2 p3 = data.initialPos;

                // Calculate position
                data.currentPos = CalculateBezierPoint(data.progress, p0, p1, p2, p3);

                // Rotation (from 90 degrees to 0)
                data.currentRotation = 90.0f - data.progress * 90.0f;

                // Scale (from 70% to 100%)
                data.currentScale = 0.7f + 0.3f * data.progress;

                // Opacity (fade in)
                data.currentAlpha = 0.2f + 0.8f * data.progress;
            }

            allComplete = false;
        }
        else {
            // Button animation complete
            data.progress = 1.0f;

            if (m_State == UIState::TRANSITIONING_TO_SETTINGS) {
                // Main menu to settings, buttons off screen
                data.currentPos = ImVec2(m_WindowSize.x + m_ButtonSize.x, data.initialPos.y);
                data.currentAlpha = 0.0f;
            }
            else {
                // Settings to main menu, buttons back to original position
                data.currentPos = data.initialPos;
                data.currentRotation = 0.0f;
                data.currentScale = 1.0f;
                data.currentAlpha = 1.0f;
            }
        }
    }

    // If all buttons have completed animation, wait a bit, then switch state
    if (allComplete && elapsedTime >
        (m_ButtonAnimData.back().startTime + m_ButtonAnimDuration + 0.1f)) {
        m_State = m_TargetState;
        m_IsTransitioning = false;
    }
}

void UIManager::RenderMenuTransition() {
    if (m_State == UIState::TRANSITIONING_TO_SETTINGS) {
        // Main menu to settings transition animation
        // After about one second, start fading in the settings page
        float elapsedTime = ImGui::GetTime() - m_TransitionStartTime;
        float lastButtonStart = m_ButtonAnimData.back().startTime;
        float transitionMidpoint = lastButtonStart + m_ButtonAnimDuration * 0.5f;

        if (elapsedTime > transitionMidpoint) {
            // Fade in settings page
            float settingsAlpha = (elapsedTime - transitionMidpoint) /
                (m_ButtonAnimDuration * 0.5f + 0.1f);
            settingsAlpha = std::min(settingsAlpha, 1.0f);

            // Render settings with transparency
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, settingsAlpha);
            RenderSettingsNoInteraction(); // Render settings UI without interaction
            ImGui::PopStyleVar();
        }
    }
    else {
        // Settings to main menu transition animation
        // Fade out settings page
        float elapsedTime = ImGui::GetTime() - m_TransitionStartTime;
        float fadeOutDuration = m_ButtonAnimData[0].startTime + m_ButtonAnimDuration * 0.3f;

        if (elapsedTime < fadeOutDuration) {
            float settingsAlpha = 1.0f - (elapsedTime / fadeOutDuration);

            // Render settings with transparency
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, settingsAlpha);
            RenderSettingsNoInteraction(); // Render settings UI without interaction
            ImGui::PopStyleVar();
        }
    }

    // Render animating buttons
    for (int i = 0; i < m_ButtonAnimData.size(); i++) {
        const auto& data = m_ButtonAnimData[i];

        // Skip buttons that are off-screen or fully transparent
        if (data.currentAlpha <= 0.01f) continue;

        // Save current drawing state
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->PushClipRect(ImVec2(0, 0), m_WindowSize);

        // Get button label
        const char* buttonLabels[] = { "Start Game", "Online Game", "Setting", "Exit" };
        const char* label = buttonLabels[i];

        // Calculate button center point (for rotation)
        ImVec2 center = ImVec2(
            data.currentPos.x + m_ButtonSize.x * data.currentScale * 0.5f,
            data.currentPos.y + m_ButtonSize.y * data.currentScale * 0.5f
        );

        // Calculate button corner positions
        float halfWidth = m_ButtonSize.x * data.currentScale * 0.5f;
        float halfHeight = m_ButtonSize.y * data.currentScale * 0.5f;

        // Initial corner positions (relative to center)
        ImVec2 corners[4] = {
            ImVec2(-halfWidth, -halfHeight), // Top-left
            ImVec2(halfWidth, -halfHeight),  // Top-right
            ImVec2(halfWidth, halfHeight),   // Bottom-right
            ImVec2(-halfWidth, halfHeight)   // Bottom-left
        };

        // Apply rotation
        float radians = data.currentRotation * 3.14159f / 180.0f;
        float cos_r = cosf(radians);
        float sin_r = sinf(radians);

        for (int j = 0; j < 4; j++) {
            // Rotation transformation
            float x = corners[j].x * cos_r - corners[j].y * sin_r;
            float y = corners[j].x * sin_r + corners[j].y * cos_r;

            // Move relative to center point
            corners[j].x = x + center.x;
            corners[j].y = y + center.y;
        }

        // Render button background
        ImU32 buttonColor = ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.2f, data.currentAlpha));
        drawList->AddQuadFilled(corners[0], corners[1], corners[2], corners[3], buttonColor);

        // Render button border
        ImU32 borderColor = ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, data.currentAlpha));
        drawList->AddQuad(corners[0], corners[1], corners[2], corners[3], borderColor, 1.0f);

        // Calculate text dimensions
        ImVec2 textSize = ImGui::CalcTextSize(label);
        float textX = center.x - textSize.x * 0.5f * cos_r + textSize.y * 0.5f * sin_r;
        float textY = center.y - textSize.x * 0.5f * sin_r - textSize.y * 0.5f * cos_r;

        // Render text (simplified: text doesn't rotate, only shows in button center)
        ImU32 textColor = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, data.currentAlpha));
        drawList->AddText(ImVec2(center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f),
            textColor, label);

        drawList->PopClipRect();
    }
}

void UIManager::RenderSettingsNoInteraction() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float xPos = m_WindowSize.x * 2 / 5;
    float startY = (m_WindowSize.y - 8 * m_ButtonSize.y - 7 * 20) / 2;
    float currentY = startY;

    // ����
    float titleX = xPos;
    float titleY = currentY - 60;

    drawList->AddText(
        ImGui::GetFont(),
        ImGui::GetFontSize() * 1.4f,
        ImVec2(titleX + 2, titleY + 2),
        IM_COL32(50, 50, 50, 200),
        "Setting"
    );

    drawList->AddText(
        ImGui::GetFont(),
        ImGui::GetFontSize() * 1.4f,
        ImVec2(titleX, titleY),
        IM_COL32(255, 215, 80, 255), // ��ɫ
        "Setting"
    );

    // �������þ�̬��ʾ
    // ���������죨�޽�����
    float sliderWidth = m_ButtonSize.x;
    float sliderHeight = 24.0f;
    float labelHeight = 20.0f;
    float depthFactor = 10.0f;

    // ������
    drawList->AddText(
        ImVec2(xPos + 2, currentY + 2),
        IM_COL32(20, 20, 50, 200),
        "Master Audio"
    );
    drawList->AddText(
        ImVec2(xPos, currentY),
        IM_COL32(220, 240, 255, 255),
        "Master Audio"
    );

    ImVec2 topLeft = ImVec2(xPos - depthFactor, currentY + labelHeight);
    ImVec2 topRight = ImVec2(xPos + sliderWidth, currentY + labelHeight - depthFactor);
    ImVec2 bottomRight = ImVec2(xPos + sliderWidth, currentY + labelHeight + sliderHeight - depthFactor);
    ImVec2 bottomLeft = ImVec2(xPos - depthFactor, currentY + labelHeight + sliderHeight);

    drawList->AddQuadFilled(topLeft, topRight, bottomRight, bottomLeft, IM_COL32(80, 80, 150, 200));
    drawList->AddQuad(topLeft, topRight, bottomRight, bottomLeft, IM_COL32(160, 160, 200, 255), 1.5f);

    // ���������ֱ� 
    float handlePos = xPos + sliderWidth * m_MasterVolume;
    drawList->AddCircleFilled(
        ImVec2(handlePos, currentY + labelHeight + sliderHeight / 2 - depthFactor / 2),
        sliderHeight * 0.6f,
        IM_COL32(60, 160, 210, 255)
    );

    currentY += m_ButtonSize.y + 20;

    // ��Ч�������޽�����
    drawList->AddText(
        ImVec2(xPos + 2, currentY + 2),
        IM_COL32(20, 20, 50, 200),
        "Sound Effects"
    );
    drawList->AddText(
        ImVec2(xPos, currentY),
        IM_COL32(220, 240, 255, 255),
        "Sound Effects"
    );

    topLeft = ImVec2(xPos - depthFactor, currentY + labelHeight);
    topRight = ImVec2(xPos + sliderWidth, currentY + labelHeight - depthFactor);
    bottomRight = ImVec2(xPos + sliderWidth, currentY + labelHeight + sliderHeight - depthFactor);
    bottomLeft = ImVec2(xPos - depthFactor, currentY + labelHeight + sliderHeight);

    drawList->AddQuadFilled(topLeft, topRight, bottomRight, bottomLeft, IM_COL32(80, 80, 150, 200));
    drawList->AddQuad(topLeft, topRight, bottomRight, bottomLeft, IM_COL32(160, 160, 200, 255), 1.5f);

    // ��Ч�������ֱ� 
    handlePos = xPos + sliderWidth * m_SFXVolume;
    drawList->AddCircleFilled(
        ImVec2(handlePos, currentY + labelHeight + sliderHeight / 2 - depthFactor / 2),
        sliderHeight * 0.6f,
        IM_COL32(60, 160, 210, 255)
    );

    currentY += m_ButtonSize.y + 20;

    // BGM�������޽�����
    drawList->AddText(
        ImVec2(xPos + 2, currentY + 2),
        IM_COL32(20, 20, 50, 200),
        "BGM"
    );
    drawList->AddText(
        ImVec2(xPos, currentY),
        IM_COL32(220, 240, 255, 255),
        "BGM"
    );

    topLeft = ImVec2(xPos - depthFactor, currentY + labelHeight);
    topRight = ImVec2(xPos + sliderWidth, currentY + labelHeight - depthFactor);
    bottomRight = ImVec2(xPos + sliderWidth, currentY + labelHeight + sliderHeight - depthFactor);
    bottomLeft = ImVec2(xPos - depthFactor, currentY + labelHeight + sliderHeight);

    drawList->AddQuadFilled(topLeft, topRight, bottomRight, bottomLeft, IM_COL32(80, 80, 150, 200));
    drawList->AddQuad(topLeft, topRight, bottomRight, bottomLeft, IM_COL32(160, 160, 200, 255), 1.5f);

    // BGM�������ֱ�
    handlePos = xPos + sliderWidth * m_BGMVolume;
    drawList->AddCircleFilled(
        ImVec2(handlePos, currentY + labelHeight + sliderHeight / 2 - depthFactor / 2),
        sliderHeight * 0.6f,
        IM_COL32(60, 160, 210, 255)
    );

    currentY += m_ButtonSize.y + 20;

    // �������ţ��޽�����
    drawList->AddText(
        ImVec2(xPos + 2, currentY + 2),
        IM_COL32(20, 20, 50, 200),
        "UI Font Size"
    );
    drawList->AddText(
        ImVec2(xPos, currentY),
        IM_COL32(220, 240, 255, 255),
        "UI Font Size"
    );

    topLeft = ImVec2(xPos - depthFactor, currentY + labelHeight);
    topRight = ImVec2(xPos + sliderWidth, currentY + labelHeight - depthFactor);
    bottomRight = ImVec2(xPos + sliderWidth, currentY + labelHeight + sliderHeight - depthFactor);
    bottomLeft = ImVec2(xPos - depthFactor, currentY + labelHeight + sliderHeight);

    drawList->AddQuadFilled(topLeft, topRight, bottomRight, bottomLeft, IM_COL32(80, 80, 150, 200));
    drawList->AddQuad(topLeft, topRight, bottomRight, bottomLeft, IM_COL32(160, 160, 200, 255), 1.5f);

    handlePos = xPos + sliderWidth * ((m_FontScale - 0.8f) / 1.2f); // ���ŵ�0.8-2.0��Χ
    drawList->AddCircleFilled(
        ImVec2(handlePos, currentY + labelHeight + sliderHeight / 2 - depthFactor / 2),
        sliderHeight * 0.6f,
        IM_COL32(60, 160, 210, 255)
    );

    currentY += m_ButtonSize.y + 20;

    // ��������������ť���޽�����
    ImGui::SetCursorPos(ImVec2(xPos, currentY));
    {
        float effectDepth = 15.0f;
        ImVec2 buttonTopLeft = ImVec2(xPos - effectDepth, currentY);
        ImVec2 buttonTopRight = ImVec2(xPos + m_ButtonSize.x, currentY - effectDepth);
        ImVec2 buttonBottomRight = ImVec2(xPos + m_ButtonSize.x, currentY + m_ButtonSize.y - effectDepth);
        ImVec2 buttonBottomLeft = ImVec2(xPos - effectDepth, currentY + m_ButtonSize.y);

        drawList->AddQuadFilled(buttonTopLeft, buttonTopRight, buttonBottomRight, buttonBottomLeft, IM_COL32(30, 95, 140, 200));
        drawList->AddQuad(buttonTopLeft, buttonTopRight, buttonBottomRight, buttonBottomLeft, IM_COL32(80, 170, 220, 255), 1.5f);

        ImVec2 textSize = ImGui::CalcTextSize("Start Net Server");
        float textX = xPos + (m_ButtonSize.x - textSize.x) / 2 - effectDepth / 2;
        float textY = currentY + (m_ButtonSize.y - textSize.y) / 2 - effectDepth / 3;
        drawList->AddText(ImVec2(textX + 1, textY + 1), IM_COL32(20, 20, 50, 200), "Start Net Server");
        drawList->AddText(ImVec2(textX, textY), IM_COL32(220, 240, 255, 255), "Start Net Server");
    }
    currentY += m_ButtonSize.y + 20;

    // ���ذ�ť���޽�����
    ImGui::SetCursorPos(ImVec2(xPos, currentY));
    {
        float effectDepth = 15.0f;
        ImVec2 buttonTopLeft = ImVec2(xPos - effectDepth, currentY);
        ImVec2 buttonTopRight = ImVec2(xPos + m_ButtonSize.x, currentY - effectDepth);
        ImVec2 buttonBottomRight = ImVec2(xPos + m_ButtonSize.x, currentY + m_ButtonSize.y - effectDepth);
        ImVec2 buttonBottomLeft = ImVec2(xPos - effectDepth, currentY + m_ButtonSize.y);

        drawList->AddQuadFilled(buttonTopLeft, buttonTopRight, buttonBottomRight, buttonBottomLeft, IM_COL32(80, 80, 150, 200));
        drawList->AddQuad(buttonTopLeft, buttonTopRight, buttonBottomRight, buttonBottomLeft, IM_COL32(160, 160, 200, 255), 1.5f);

        // �ı�
        ImVec2 textSize = ImGui::CalcTextSize("Back");
        float textX = xPos + (m_ButtonSize.x - textSize.x) / 2 - effectDepth / 2;
        float textY = currentY + (m_ButtonSize.y - textSize.y) / 2 - effectDepth / 3;
        drawList->AddText(ImVec2(textX + 1, textY + 1), IM_COL32(20, 20, 50, 200), "Back");
        drawList->AddText(ImVec2(textX, textY), IM_COL32(220, 240, 255, 255), "Back");
    }
}

// Bezier curve calculation
ImVec2 UIManager::CalculateBezierPoint(float t, const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    ImVec2 point;
    point.x = uuu * p0.x + 3.0f * uu * t * p1.x + 3.0f * u * tt * p2.x + ttt * p3.x;
    point.y = uuu * p0.y + 3.0f * uu * t * p1.y + 3.0f * u * tt * p2.y + ttt * p3.y;

    return point;
}

void UIManager::UpdateAllAudioVolumes() {
    // Update all audio that's affected by volume

    // Update BGM volume (master volume affects it)
    float effectiveBGMVolume = m_BGMVolume * m_MasterVolume;
    AudioManager::GetInstance()->SetBGMVolume(effectiveBGMVolume);

    // Update SFX volume (master volume affects all sound effect volume)
    float effectiveSFXVolume = m_SFXVolume * m_MasterVolume;
    AudioManager::GetInstance()->SetSFXVolume(effectiveSFXVolume);

    // std::cout << "�Ѹ�����Ƶ���� - ������: " << m_MasterVolume << ", BGM: " << m_BGMVolume << " (ʵ��: " << effectiveBGMVolume << "), SFX: " << m_SFXVolume << " (ʵ��: " << effectiveSFXVolume << ")" << std::endl;
}

void UIManager::TogglePlayerProfile() {
    m_ShowPlayerProfile = !m_ShowPlayerProfile;
}

bool UIManager::IsPlayerProfileVisible() const {
    return m_ShowPlayerProfile;
}

void UIManager::RenderHUD() {
    if (m_State != UIState::GAME) return;
    float dt = ImGui::GetIO().DeltaTime; // 获取帧间时间
    UpdateDamageEffect(dt);
    UpdatePopupTimer(dt);
    UpdateSkillCooldowns(dt);
    RenderSkillBars();
    RenderPlayerStats();
    RenderInventoryBar();
    RenderPlayerPortrait();
    if (m_ShowLevelCompletePopup) {
        RenderLevelCompletePopup();
    }
    if (m_ShowVictoryPopup) {
        RenderVictoryPopup();
    }
}

void UIManager::RenderPlayerStats() {
    float padding = 10.0f;

    if (m_Player) {
        UpdatePlayerHealth(m_Player->GetHealth(), m_Player->GetMaxHealth());
    }

    // 血条屏幕左上角
    ImVec2 healthBarPos = ImVec2(padding, padding);
    ImVec2 healthBarEndPos = ImVec2(healthBarPos.x + m_HealthBarSize.x, healthBarPos.y + m_HealthBarSize.y);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // 受伤特效 - 如果需要，创建红色边框
    if (m_IsDamageEffect && m_DamageEffectIntensity > 0.01f) {
        ImVec2 screenSize = ImGui::GetIO().DisplaySize;

        // 计算边框厚度
        float borderThickness = screenSize.x * 0.08f * m_DamageEffectIntensity;

        // 带有渐变的边框 - 从边缘向内淡出
        // 上边框
        drawList->AddRectFilledMultiColor(
            ImVec2(0, 0),
            ImVec2(screenSize.x, borderThickness),
            IM_COL32(255, 0, 0, (int)(200 * m_DamageEffectIntensity)),
            IM_COL32(255, 0, 0, (int)(200 * m_DamageEffectIntensity)),
            IM_COL32(255, 0, 0, 0),
            IM_COL32(255, 0, 0, 0)
        );

        // 下边框
        drawList->AddRectFilledMultiColor(
            ImVec2(0, screenSize.y - borderThickness),
            ImVec2(screenSize.x, screenSize.y),
            IM_COL32(255, 0, 0, 0),
            IM_COL32(255, 0, 0, 0),
            IM_COL32(255, 0, 0, (int)(200 * m_DamageEffectIntensity)),
            IM_COL32(255, 0, 0, (int)(200 * m_DamageEffectIntensity))
        );

        // 左边框
        drawList->AddRectFilledMultiColor(
            ImVec2(0, borderThickness),
            ImVec2(borderThickness, screenSize.y - borderThickness),
            IM_COL32(255, 0, 0, (int)(200 * m_DamageEffectIntensity)),
            IM_COL32(255, 0, 0, 0),
            IM_COL32(255, 0, 0, 0),
            IM_COL32(255, 0, 0, (int)(200 * m_DamageEffectIntensity))
        );

        // 右边框
        drawList->AddRectFilledMultiColor(
            ImVec2(screenSize.x - borderThickness, borderThickness),
            ImVec2(screenSize.x, screenSize.y - borderThickness),
            IM_COL32(255, 0, 0, 0),
            IM_COL32(255, 0, 0, (int)(200 * m_DamageEffectIntensity)),
            IM_COL32(255, 0, 0, (int)(200 * m_DamageEffectIntensity)),
            IM_COL32(255, 0, 0, 0)
        );

        // 添加轻微的红色脉冲效果
        float pulseIntensity = (sin(ImGui::GetTime() * 15.0f) * 0.4f + 0.7f) * m_DamageEffectIntensity;
        drawList->AddRectFilled(ImVec2(0, 0), screenSize, IM_COL32(255, 0, 0, (int)(30 * pulseIntensity)));
    }
    drawList->AddRectFilled(healthBarPos, healthBarEndPos, IM_COL32(60, 60, 60, 200));

    // Draw health bar fill
    ImVec2 healthFillEnd = ImVec2(healthBarPos.x + m_HealthBarSize.x * m_PlayerHealth, healthBarEndPos.y);
    drawList->AddRectFilled(healthBarPos, healthFillEnd, IM_COL32(220, 40, 40, 255));

    // Add border
    drawList->AddRect(healthBarPos, healthBarEndPos, IM_COL32(180, 180, 180, 255));

    // Add text - 显示实际数值而不只是百分比
    char healthText[32];
    float currentHealth = m_PlayerMaxHealth * m_PlayerHealth;
    sprintf_s(healthText, "HP: %.0f/%.0f", currentHealth, m_PlayerMaxHealth);
    drawList->AddText(ImVec2(healthBarPos.x + 5, healthBarPos.y + 2), IM_COL32(255, 255, 255, 255), healthText);
}

void UIManager::UpdatePlayerHealth(float currentHealth, float maxHealth) {
    if (maxHealth > 0) {
        float oldHealth = m_PlayerHealth;
        m_PlayerHealth = currentHealth / maxHealth;  // 转换为比例
        m_PlayerMaxHealth = maxHealth;
        
        // 检测血量减少，触发伤害效果
        if (m_PlayerHealth < oldHealth) {
            float damageTaken = oldHealth - m_PlayerHealth;
            
            // 获取GameTechRenderer实例并激活伤害效果
            GameTechRenderer* renderer = GameTechRenderer::GetInstance();
            if (renderer) {
                float intensity = std::min(damageTaken * 6.0f, 1.0f); // 伤害越大，效果越强
                renderer->ActivateDamageEffect(intensity);
                // std::cout << "Activated damage effect with intensity: " << intensity << std::endl;
            }
        }
    }
    else {
        m_PlayerHealth = 0.0f;
    }

    // 确保比例值在有效范围内
    m_PlayerHealth = std::clamp(m_PlayerHealth, 0.0f, 1.0f);

    // 可以添加调试信息
    AddDebugInfo("Player Health", std::to_string(currentHealth) + "/" + std::to_string(maxHealth), "Game Stats");
}

void UIManager::RenderInventoryBar() {
    const int inventorySlots = 8;
    float padding = 10.0f;
    float slotSpacing = 5.0f;

    // Calculate total inventory bar width
    float totalWidth = inventorySlots * m_InventorySlotSize.x + (inventorySlots - 1) * slotSpacing;

    // Inventory bar position (bottom center)
    float startX = (ImGui::GetIO().DisplaySize.x - totalWidth) / 2;
    float startY = ImGui::GetIO().DisplaySize.y - m_InventorySlotSize.y - padding;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    for (int i = 0; i < inventorySlots; i++) {
        // Calculate current inventory slot position
        ImVec2 slotPos = ImVec2(startX + i * (m_InventorySlotSize.x + slotSpacing), startY);
        ImVec2 slotEndPos = ImVec2(slotPos.x + m_InventorySlotSize.x, slotPos.y + m_InventorySlotSize.y);

        // Draw inventory slot background
        drawList->AddRectFilled(slotPos, slotEndPos, IM_COL32(60, 60, 60, 200));

        // Draw inventory slot border
        drawList->AddRect(slotPos, slotEndPos, IM_COL32(180, 180, 180, 255));

        // Add number label
        char slotText[8];
        sprintf_s(slotText, "%d", i + 1);
        ImVec2 textSize = ImGui::CalcTextSize(slotText);
        drawList->AddText(
            ImVec2(slotPos.x + (m_InventorySlotSize.x - textSize.x) / 2,
                slotEndPos.y - textSize.y - 2),
            IM_COL32(255, 255, 255, 255), slotText);
    }
}

void UIManager::RenderPlayerPortrait() {
    float padding = 10.0f;

    // Portrait position (bottom-right)
    ImVec2 portraitPos = ImVec2(ImGui::GetIO().DisplaySize.x - m_PortraitSize.x - padding,
        ImGui::GetIO().DisplaySize.y - m_PortraitSize.y - padding);
    ImVec2 portraitEndPos = ImVec2(portraitPos.x + m_PortraitSize.x, portraitPos.y + m_PortraitSize.y);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw portrait background
    drawList->AddRectFilled(portraitPos, portraitEndPos, IM_COL32(60, 60, 60, 200));

    // Draw portrait border
    drawList->AddRect(portraitPos, portraitEndPos, IM_COL32(220, 180, 40, 255), 4.0f, 0, 2.0f);

    // Add placeholder text in the center
    const char* portraitText = "Player";
    ImVec2 textSize = ImGui::CalcTextSize(portraitText);
    drawList->AddText(
        ImVec2(portraitPos.x + (m_PortraitSize.x - textSize.x) / 2,
            portraitPos.y + (m_PortraitSize.y - textSize.y) / 2),
        IM_COL32(255, 255, 255, 255), portraitText);
}

void UIManager::RenderPlayerProfile() {
    // Set player profile window position (center)
    ImVec2 windowPos = ImVec2(
        (ImGui::GetIO().DisplaySize.x - m_ProfileWindowSize.x) / 2,
        (ImGui::GetIO().DisplaySize.y - m_ProfileWindowSize.y) / 2
    );

    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(m_ProfileWindowSize);

    // Use semi-transparent dark background
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8f, 0.8f, 0.8f, 0.6f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

    if (ImGui::Begin("Player details", &m_ShowPlayerProfile,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) {

        // Render tab buttons
        ImGui::BeginChild("ProfileTabs", ImVec2(0, 30), true);

        float tabWidth = ImGui::GetContentRegionAvail().x / 9.0f; // 9 tabs

        for (int i = 0; i < 9; i++) {
            if (i > 0) ImGui::SameLine();

            // Color based on current selected tab
            if (m_CurrentProfileTab == i) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.8f, 1.0f));
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.3f, 1.0f));
            }

            char tabName[32];
            if (i == 0) {
                sprintf_s(tabName, "Player");
            }
            else {
                sprintf_s(tabName, "Item %d", i);
            }

            ImGui::SetCursorPosX(i * tabWidth);
            if (ImGui::Button(tabName, ImVec2(tabWidth - 5, 0))) {
                m_CurrentProfileTab = i;
            }

            ImGui::PopStyleColor();
        }

        ImGui::EndChild();

        // Render tab content
        ImGui::BeginChild("ProfileContent", ImVec2(0, 0), true);

        // Display content based on current selected tab
        if (m_CurrentProfileTab == 0) {
            ImGui::Text("Player");
            ImGui::Separator();
            ImGui::Spacing();

            // Use columns layout
            ImGui::Columns(2, "ProfileColumns", false);
            ImGui::SetColumnWidth(0, 150);

            // Left side labels
            ImGui::Text("HP:");
            ImGui::Text("MP:");
            ImGui::Text("Level:");
            ImGui::Text("STR:");
            ImGui::Text("AGI:");
            ImGui::Text("INT:");
            ImGui::Text("STA:");

            ImGui::NextColumn();

            // Right side values (in a real game, would use actual data)
            ImGui::Text("%.0f%%", m_PlayerHealth * 100);
            ImGui::Text("10");
            ImGui::Text("25");
            ImGui::Text("18");
            ImGui::Text("15");
            ImGui::Text("20");

            // Remove column layout
            ImGui::Columns(1);

            // Add space for additional character info
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::BeginChild("AdditionalInfo", ImVec2(0, 0), true);
            ImGui::Text("Player details");
            ImGui::EndChild();
        }
        else {
            // Item tab content
            ImGui::Text("Item %d details", m_CurrentProfileTab);
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::BeginChild("ItemDetails", ImVec2(0, 0), true);
            ImGui::Text("Item");
            ImGui::EndChild();
        }

        ImGui::EndChild();
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

void UIManager::HandleMouseClick(int button, int action, int mods) {
    if (m_State == UIState::GAME) {
        // 如果暂停菜单显示，则点击为菜单操作，否则为游戏操作
        if (m_ShowPauseMenu) {
            if (button == 0 && action == 1) { // 左键, 按下
                AudioManager::GetInstance()->PlaySFX("click");
                //std::cout << "暂停菜单点击" << std::endl;
            }
        }
        else {
            if (button == 0 && action == 1) { // 左键, 按下
                // 获取当前关卡并播放对应武器音效序列
                extern int switchMapCount; // 获取外部变量
                AudioManager::GetInstance()->PlayWeaponSoundsMix(switchMapCount, 3);
                //AudioManager::GetInstance()->PlayWeaponSoundsSequence(switchMapCount);
            }
            else if (button == 1 && action == 1) { // 右键, 按下
                AudioManager::GetInstance()->PlaySFX("click");
            }
        }
    }
    else if (m_State == UIState::ONLINE_GAME) {
        if (m_ShowPauseMenu) {
            if (button == 0 && action == 1) { // 左键, 按下
                AudioManager::GetInstance()->PlaySFX("click");
                //std::cout << "暂停菜单点击" << std::endl;
            }
        }
        else {
            if (button == 0 && action == 1) {
                // 同样使用当前关卡的武器音效序列
                extern int switchMapCount;
                AudioManager::GetInstance()->PlayWeaponSoundsSequence(switchMapCount);
            }
        }
    }
    else {
        if (button == 0 && action == 1) {
            AudioManager::GetInstance()->PlaySFX("click");
            // std::cout << "普通UI点击音效" << std::endl;
        }
    }
}

void UIManager::InitializeBGM() {

    AudioManager::GetInstance()->StopBGM();

    if (m_State == UIState::GAME || m_State == UIState::ONLINE_GAME) {
        bool loaded = AudioManager::GetInstance()->LoadBGM("../../Assets/Audio/BGM/maou_bgm_fantasy12_need_boss.mp3");
        if (loaded) {
            float volume = m_BGMVolume * m_MasterVolume;
            AudioManager::GetInstance()->SetBGMVolume(volume);
            AudioManager::GetInstance()->PlayBGM(true);
            // std::cout << "ս��BGM�ѳ�ʼ��" << std::endl;
        }
    }
    else {
        bool loaded = AudioManager::GetInstance()->LoadBGM("../../Assets/Audio/BGM/SoundTrack.mp3");
        if (loaded) {
            float volume = m_BGMVolume * m_MasterVolume;
            AudioManager::GetInstance()->SetBGMVolume(volume);
            AudioManager::GetInstance()->PlayBGM(true);
            // std::cout << "�˵�BGM�ѳ�ʼ��" << std::endl;
        }
    }
}

void UIManager::SetState(UIState newState) {
    UIState oldState = m_State;
    m_State = newState;

    // std::cout << "�ֶ�����״̬�� " << static_cast<int>(oldState) << " �� " << static_cast<int>(newState) << std::endl;

    if ((newState == UIState::GAME || newState == UIState::ONLINE_GAME) &&
        (oldState == UIState::MAIN_MENU || oldState == UIState::SETTINGS)) {

        // Ϊȷ��BGM��ȷ���ţ��ӳ�һС��ʱ���ٳ��Բ���
        // std::cout << "�ӳ�100ms��ǿ�Ʋ���ս��BGM" << std::endl;
        std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        this->ForceBattleBGM();
        }).detach();
    }
}

void UIManager::ForceBattleBGM() {

    AudioManager::GetInstance()->StopBGM();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    bool loaded1 = AudioManager::GetInstance()->LoadBGM("../../Assets/Audio/BGM/maou_bgm_fantasy12_need_boss.mp3");
    if (loaded1) {
        AudioManager::GetInstance()->SetBGMVolume(m_BGMVolume * m_MasterVolume);
        AudioManager::GetInstance()->PlayBGM(true);
        bool isPlaying = AudioManager::GetInstance()->IsBGMPlaying();
        // std::cout << "����1��� - ����: " << (loaded1 ? "�ɹ�" : "ʧ��")<< ", ����: " << (isPlaying ? "�ɹ�" : "ʧ��") << ", ״̬: " << (AudioManager::GetInstance()->IsBGMPlaying() ? "������" : "δ����") << std::endl;

        if (AudioManager::GetInstance()->IsBGMPlaying()) {
            return; 
        }
    }

    AudioManager::GetInstance()->Init();
    bool loaded2 = AudioManager::GetInstance()->LoadBGM("../../Assets/Audio/BGM/maou_bgm_fantasy12_need_boss.mp3");
    if (loaded2) {
        AudioManager::GetInstance()->SetBGMVolume(m_BGMVolume * m_MasterVolume);
        AudioManager::GetInstance()->PlayBGM(true);
        bool isPlaying = AudioManager::GetInstance()->IsBGMPlaying();
        // std::cout << "����2��� - ����: " << (loaded2 ? "�ɹ�" : "ʧ��")<< ", ����: " << (isPlaying ? "�ɹ�" : "ʧ��") << ", ״̬: " << (AudioManager::GetInstance()->IsBGMPlaying() ? "������" : "δ����") << std::endl;
    }
}

void UIManager::Render3DButton(ImDrawList* drawList, const char* label, float xPos, float yPos, bool& clicked) {
    // �������Ƿ���ͣ�ڰ�ť����
    ImVec2 mousePos = ImGui::GetIO().MousePos;
    bool isHovered = mousePos.x >= xPos && mousePos.x <= (xPos + m_ButtonSize.x) &&
        mousePos.y >= yPos && mousePos.y <= (yPos + m_ButtonSize.y);

    // 3DЧ������
    float depthFactor = 15.0f; // ����3DЧ����ǿ��
    float shadowOffset = 5.0f; // ��Ӱƫ����
    float hoverScaleFactor = 1.05f; // ��ͣʱ����������

    // ������ͣ״̬����Ч��ǿ��
    float effectDepth = isHovered ? depthFactor * 1.2f : depthFactor;
    ImVec2 buttonSize = isHovered ?
        ImVec2(m_ButtonSize.x * hoverScaleFactor, m_ButtonSize.y * hoverScaleFactor) :
        m_ButtonSize;

    // ���㰴ť���ĸ��������������Σ�α3DЧ����
    ImVec2 topLeft = ImVec2(xPos - effectDepth, yPos);
    ImVec2 topRight = ImVec2(xPos + buttonSize.x, yPos - effectDepth);
    ImVec2 bottomRight = ImVec2(xPos + buttonSize.x, yPos + buttonSize.y - effectDepth);
    ImVec2 bottomLeft = ImVec2(xPos - effectDepth, yPos + buttonSize.y);

    // ��ť��Ӱ
    ImVec2 shadowTopLeft = ImVec2(topLeft.x + shadowOffset, topLeft.y + shadowOffset);
    ImVec2 shadowTopRight = ImVec2(topRight.x + shadowOffset, topRight.y + shadowOffset);
    ImVec2 shadowBottomRight = ImVec2(bottomRight.x + shadowOffset, bottomRight.y + shadowOffset);
    ImVec2 shadowBottomLeft = ImVec2(bottomLeft.x + shadowOffset, bottomLeft.y + shadowOffset);

    drawList->AddQuadFilled(
        shadowTopLeft, shadowTopRight, shadowBottomRight, shadowBottomLeft,
        IM_COL32(30, 30, 30, 180) // ��͸����ɫ��Ӱ
    );

    // ��ť������ɫ
    ImU32 colorLeft = isHovered ? IM_COL32(100, 100, 180, 255) : IM_COL32(60, 60, 120, 255); // ���ϰ�
    ImU32 colorRight = isHovered ? IM_COL32(150, 150, 220, 255) : IM_COL32(100, 100, 180, 255); // �Ҳ����

    // ���ư�ť���� - ��벿��
    drawList->AddTriangleFilled(
        topLeft, ImVec2(xPos + buttonSize.x / 2, yPos - effectDepth / 2), bottomLeft,
        colorLeft
    );

    // ���ư�ť���� - �Ұ벿��
    drawList->AddTriangleFilled(
        ImVec2(xPos + buttonSize.x / 2, yPos - effectDepth / 2), topRight, bottomRight,
        colorRight
    );
    drawList->AddTriangleFilled(
        bottomLeft, ImVec2(xPos + buttonSize.x / 2, yPos - effectDepth / 2), bottomRight,
        colorRight
    );

    // ��ť�߿�
    ImU32 borderColor = isHovered ? IM_COL32(220, 220, 255, 255) : IM_COL32(180, 180, 220, 255);
    drawList->AddQuad(
        topLeft, topRight, bottomRight, bottomLeft,
        borderColor, 1.5f
    );

    ImVec2 textSize = ImGui::CalcTextSize(label);
    float textX = xPos + (buttonSize.x - textSize.x) / 2 - effectDepth / 2;
    float textY = yPos + (buttonSize.y - textSize.y) / 2 - effectDepth / 3;

    // Ϊ�ı����ϸ΢��Ӱ��ǿ�ɶ���
    drawList->AddText(
        ImVec2(textX + 1, textY + 1),
        IM_COL32(20, 20, 20, 200),
        label
    );

    drawList->AddText(
        ImVec2(textX, textY),
        isHovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(240, 240, 240, 255),
        label
    );

    clicked = isHovered && ImGui::IsMouseClicked(0);
    if (clicked) {
        AudioManager::GetInstance()->PlaySFX("click");
    }
}

void UIManager::Render3DSlider(ImDrawList* drawList, const char* label, float xPos, float yPos, float* value, float min, float max, const char* format) {
    float sliderWidth = m_ButtonSize.x;
    float sliderHeight = 24.0f;
    float labelHeight = 20.0f;
    float totalHeight = labelHeight + sliderHeight;

    // �������Ƿ���ͣ�ڻ�����
    ImVec2 mousePos = ImGui::GetIO().MousePos;
    bool isHovered = mousePos.x >= xPos && mousePos.x <= (xPos + sliderWidth) &&
        mousePos.y >= yPos + labelHeight && mousePos.y <= (yPos + totalHeight);

    // 3DЧ������
    float depthFactor = 10.0f; 
    float shadowOffset = 4.0f;
    float hoverScaleFactor = 1.03f;

    // ������ͣ״̬����Ч��ǿ��
    float effectDepth = isHovered ? depthFactor * 1.2f : depthFactor;

    // ImGui::SetCursorPos(ImVec2(xPos, yPos));
    // ImGui::Text("%s", label);

    // 3D��ǩ�ı�
    ImVec2 labelTextSize = ImGui::CalcTextSize(label);
    float labelTextX = xPos;
    float labelTextY = yPos;

    // �ı���Ӱ������3DЧ����
    drawList->AddText(
        ImVec2(labelTextX + 2, labelTextY + 2),
        IM_COL32(20, 20, 50, 200),
        label
    );

    drawList->AddText(
        ImVec2(labelTextX, labelTextY),
        IM_COL32(220, 240, 255, 255),
        label
    );

    // ����������
    ImVec2 topLeft = ImVec2(xPos - effectDepth, yPos + labelHeight);
    ImVec2 topRight = ImVec2(xPos + sliderWidth, yPos + labelHeight - effectDepth);
    ImVec2 bottomRight = ImVec2(xPos + sliderWidth, yPos + totalHeight - effectDepth);
    ImVec2 bottomLeft = ImVec2(xPos - effectDepth, yPos + totalHeight);

    // ��Ӱ
    ImVec2 shadowTopLeft = ImVec2(topLeft.x + shadowOffset, topLeft.y + shadowOffset);
    ImVec2 shadowTopRight = ImVec2(topRight.x + shadowOffset, topRight.y + shadowOffset);
    ImVec2 shadowBottomRight = ImVec2(bottomRight.x + shadowOffset, bottomRight.y + shadowOffset);
    ImVec2 shadowBottomLeft = ImVec2(bottomLeft.x + shadowOffset, bottomLeft.y + shadowOffset);

    drawList->AddQuadFilled(
        shadowTopLeft, shadowTopRight, shadowBottomRight, shadowBottomLeft,
        IM_COL32(30, 30, 30, 180) // ��͸����Ӱ
    );

    // ��ɫ���� 
    ImU32 colorLeft = isHovered ? IM_COL32(30, 95, 140, 255) : IM_COL32(20, 60, 100, 255);    // ���
    ImU32 colorRight = isHovered ? IM_COL32(60, 150, 190, 255) : IM_COL32(40, 120, 160, 255); // ǳ����

    // ���ƻ��챳�� - ��벿��
    drawList->AddTriangleFilled(
        topLeft, ImVec2(xPos + sliderWidth / 2, yPos + labelHeight - effectDepth / 2), bottomLeft,
        colorLeft
    );

    // ���ƻ��챳�� - �Ұ벿��
    drawList->AddTriangleFilled(
        ImVec2(xPos + sliderWidth / 2, yPos + labelHeight - effectDepth / 2), topRight, bottomRight,
        colorRight
    );
    drawList->AddTriangleFilled(
        bottomLeft, ImVec2(xPos + sliderWidth / 2, yPos + labelHeight - effectDepth / 2), bottomRight,
        colorRight
    );

    ImU32 borderColor = isHovered ? IM_COL32(120, 210, 255, 255) : IM_COL32(80, 170, 220, 255);
    drawList->AddQuad(
        topLeft, topRight, bottomRight, bottomLeft,
        borderColor, 1.5f
    );

    // ���콻��
    bool valueChanged = false;
    if (isHovered && ImGui::IsMouseDown(0)) {
        float relX = (mousePos.x - xPos) / sliderWidth;
        relX = std::max(0.0f, std::min(relX, 1.0f));
        *value = min + (max - min) * relX;
        valueChanged = true;
    }

    // �����ֱ�λ��
    float handlePos = xPos + sliderWidth * (*value - min) / (max - min);

    // ���ƻ���ֵ
    char valueText[32];
    snprintf(valueText, sizeof(valueText), format, *value);
    ImVec2 textSize = ImGui::CalcTextSize(valueText);
    float textX = xPos + sliderWidth - textSize.x - 5;
    float textY = yPos + labelHeight + (sliderHeight - textSize.y) / 2 - effectDepth / 3;

    drawList->AddText(
        ImVec2(textX + 1, textY + 1),
        IM_COL32(20, 20, 20, 200),
        valueText
    );

    drawList->AddText(
        ImVec2(textX, textY),
        IM_COL32(220, 240, 255, 255),
        valueText
    );

    float handleRadius = sliderHeight * 0.6f;
    ImVec2 handleCenter = ImVec2(handlePos, yPos + labelHeight + sliderHeight / 2 - effectDepth / 2);

    drawList->AddCircleFilled(
        ImVec2(handleCenter.x + 2, handleCenter.y + 2),
        handleRadius,
        IM_COL32(30, 30, 30, 180)
    );

    drawList->AddCircleFilled(
        handleCenter,
        handleRadius,
        isHovered ? IM_COL32(90, 200, 240, 255) : IM_COL32(60, 160, 210, 255)
    );

    drawList->AddCircle(
        handleCenter,
        handleRadius,
        IM_COL32(180, 230, 255, 255),
        0, 2.0f
    );

    return;
}

void UIManager::TogglePauseMenu() {
    if (m_State == UIState::GAME || m_State == UIState::ONLINE_GAME) {
        m_ShowPauseMenu = !m_ShowPauseMenu;

        // ͬ������GameManager����ͣ״̬
        GameManager::GetInstance()->SetPaused(m_ShowPauseMenu);

        if (m_ShowPauseMenu) {
            // std::cout << "Pause menu opened - Game paused" << std::endl;
        }
        else {
            // std::cout << "Pause menu closed - Game resumed" << std::endl;
        }
    }
}

bool UIManager::IsPauseMenuVisible() const {
    return m_ShowPauseMenu;
}

void UIManager::RenderPauseMenu() {
    if (!m_ShowPauseMenu) return;

    // ��͸��ȫ�����ǲ�
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(
        ImVec2(0, 0),
        ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y),
        IM_COL32(0, 0, 0, 180)  // Semi-transparent black
    );

    float xPos = m_WindowSize.x * 2 / 5;
    float startY = (m_WindowSize.y - 4 * m_ButtonSize.y - 3 * 20) / 2;

    // ����
    ImGui::SetCursorPos(ImVec2(xPos, startY - 60));
    ImGui::PushFont(ImGui::GetFont());
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
    ImGui::Text("Pause Menu");
    ImGui::PopStyleColor();
    ImGui::PopFont();

    const char* buttonLabels[] = { "Continue Game", "Settings", "Main Menu", "Exit" };

    // 3DЧ������
    float depthFactor = 15.0f;
    float shadowOffset = 5.0f;
    float hoverScaleFactor = 1.05f;

    for (int i = 0; i < 4; i++) {
        bool clicked = false;
        float yPos = startY + i * (m_ButtonSize.y + 20);
        ImGui::SetCursorPos(ImVec2(xPos, yPos));
        Render3DButton(drawList, buttonLabels[i], xPos, yPos, clicked);

        if (clicked) {
            AudioManager::GetInstance()->PlaySFX("click");

            switch (i) {
            case 0: // ������Ϸ
                // std::cout << "Continue Game button clicked" << std::endl;
                m_ShowPauseMenu = false;
                GameManager::GetInstance()->SetPaused(false);   // ȷ��ȡ����ͣ
                break;
            case 1: // ����
                // std::cout << "Pause Settings button clicked" << std::endl;
                m_State = UIState::PAUSE_SETTINGS;
                m_ShowPauseMenu = false;
                break;
            case 2: // ���˵�
                // std::cout << "Main Menu button clicked from pause menu" << std::endl;
                // ͨ������״̬���Ļص���������Ϸ״̬
                m_ShowPauseMenu = false;
                AudioManager::GetInstance()->StopBGM();
                SetState(UIState::MAIN_MENU);
                if (m_StateChangeCallback) {
                    m_StateChangeCallback(UIState::MAIN_MENU);
                }
                break;
            case 3: // �˳�
                // std::cout << "Exit button clicked from pause menu" << std::endl;
                m_State = UIState::EXIT;
                break;
            }
        }
    }
}

void UIManager::MouseButtonCallbackProxy(GLFWwindow* window, int button, int action, int mods) {
    if (s_Instance) {
        s_Instance->ProcessMouseButtonEvent(window, button, action, mods);
    }
}

void UIManager::ProcessMouseButtonEvent(GLFWwindow* window, int button, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();

    // �������¼�
    HandleMouseClick(button, action, mods);

    // ����UI״̬�����Ƿ��¼����ݸ�ImGui
    if (m_State != UIState::GAME && m_State != UIState::ONLINE_GAME) {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    }
    else if (m_ShowPauseMenu) {
        // ��ͣ�˵���ʾʱ������ImGui��������¼�
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    }
}

void UIManager::CheckContinuousFiring(GLFWwindow* window) {
    // 只在游戏中且非暂停状态下检查连续射击
    if ((m_State == UIState::GAME || m_State == UIState::ONLINE_GAME) &&
        !m_ShowPauseMenu && window != nullptr) {

        // 更新已播放的枪声状态
        float currentTime = glfwGetTime();

        // 清理过期的枪声记录
        while (!m_GunshotTimes.empty() &&
            currentTime - m_GunshotTimes.front() > m_GunshotDuration) {
            m_GunshotTimes.erase(m_GunshotTimes.begin());
        }

        // 当前活跃的枪声数量
        int activeGunshotCount = m_GunshotTimes.size();

        // 直接检查鼠标按钮状态
        int leftMouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

        if (leftMouseState == GLFW_PRESS && currentTime - m_LastGunshotTime >= m_GunshotInterval && activeGunshotCount < 3) { // 限制最多枪声

            extern int switchMapCount;
            AudioManager::GetInstance()->PlayWeaponSoundsMix(switchMapCount, 3); // 同时播放3个声音，避免过多

            // 更新状态
            m_LastGunshotTime = currentTime;
            m_GunshotTimes.push_back(currentTime);
        }
    }
}

// 设置菜单音量
float  UIManager::GetMasterVolume() const {
    return m_MasterVolume; 
}

float  UIManager::GetSFXVolume() const {
    return m_SFXVolume; 
}

float  UIManager::GetBGMVolume() const {
    return m_BGMVolume;
}

float  UIManager::GetFontScale() const {
    return m_FontScale;
}

void UIManager::ToggleScoreBoard() {
    m_ShowScoreBoard = !m_ShowScoreBoard;
}

bool UIManager::IsScoreBoardVisible() const {
    return m_ShowScoreBoard;
}

void UIManager::RenderScoreBoard() {
    if (!m_ShowScoreBoard) return;
    ImVec2 windowPos = ImVec2((ImGui::GetIO().DisplaySize.x - m_ScoreBoardSize.x) / 2, (ImGui::GetIO().DisplaySize.y - m_ScoreBoardSize.y) / 2);

    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(m_ScoreBoardSize);

    // 半透明深色背景和金色边框
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8f, 0.7f, 0.0f, 0.6f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;

    if (ImGui::Begin("ScoreBoard", &m_ShowScoreBoard, windowFlags)) {

        ImDrawList* drawList = ImGui::GetWindowDrawList();  // 带阴影的标题
        ImVec2 titlePos = ImGui::GetCursorScreenPos();
        float originalScale = ImGui::GetFont()->Scale;
        ImGui::GetFont()->Scale *= 2.0f; // 使用大字体显示标题
        ImGui::PushFont(ImGui::GetFont());
        std::string titleText = "Score";
        ImVec2 textSize = ImGui::CalcTextSize(titleText.c_str());

        titlePos.x = windowPos.x + (m_ScoreBoardSize.x - textSize.x) / 2;    // 居中显示标题    
        drawList->AddText(ImVec2(titlePos.x + 2, titlePos.y + 2), IM_COL32(0, 0, 0, 200), titleText.c_str());     // 阴影效果
        drawList->AddText(ImVec2(titlePos.x, titlePos.y), IM_COL32(220, 180, 40, 255), titleText.c_str());
        ImGui::GetFont()->Scale = originalScale;  // 恢复原始字体大小
        ImGui::PopFont();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + textSize.y + 20);
        ImGui::SetCursorPosX((m_ScoreBoardSize.x - 150) / 2);  // 水平居中显示分数
        ImGui::GetFont()->Scale *= 3.0f;
        ImGui::PushFont(ImGui::GetFont());

        // 从Monster类直接获取击杀数
        std::string scoreText = std::to_string(NCL::CSC8503::Monster::GetKillCount());
        textSize = ImGui::CalcTextSize(scoreText.c_str());
        ImVec2 scorePos = ImGui::GetCursorScreenPos();
        scorePos.x = windowPos.x + (m_ScoreBoardSize.x - textSize.x) / 2;

        // 阴影效果
        drawList->AddText(ImVec2(scorePos.x + 3, scorePos.y + 3), IM_COL32(0, 0, 0, 200), scoreText.c_str());

        // 金色
        drawList->AddText(ImVec2(scorePos.x, scorePos.y), IM_COL32(255, 215, 0, 255), scoreText.c_str());

        ImGui::GetFont()->Scale = originalScale;
        ImGui::PopFont();

        // 在分数后添加空间
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + textSize.y + 40);
        std::string noteText = "F11 Close";   // 底部提示
        textSize = ImGui::CalcTextSize(noteText.c_str());
        ImVec2 notePos = ImGui::GetCursorScreenPos();
        notePos.x = windowPos.x + (m_ScoreBoardSize.x - textSize.x) / 2;

        drawList->AddText(notePos, IM_COL32(180, 180, 180, 200), noteText.c_str());
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

bool UIManager::IsLevelCompletePopupVisible() const {
    return m_ShowLevelCompletePopup;
}

int UIManager::GetCurrentLevelKillTarget() const {
    return m_CurrentLevelKillTarget;;
}

void UIManager::ShowLevelCompletePopup() {
    m_ShowLevelCompletePopup = true;
    m_PopupTimer = 5.0f; // 显示弹窗5秒
    AudioManager::GetInstance()->PlaySFX("click");
}

void UIManager::UpdatePopupTimer(float dt) {
    if (m_ShowLevelCompletePopup && m_PopupTimer > 0.0f) {
        m_PopupTimer -= dt;
        if (m_PopupTimer <= 0.0f) {
            m_PopupTimer = 0.0f;
            m_ShowLevelCompletePopup = false; //计时结束后关闭弹窗
        }
    }

    if (m_ShowVictoryPopup && m_VictoryPopupTimer > 0.0f) {
        m_VictoryPopupTimer -= dt;
        if (m_VictoryPopupTimer <= 0.0f) {
            m_VictoryPopupTimer = 0.0f;
            m_ShowVictoryPopup = false; //计时结束后关闭弹窗
        }
    }

    if ((m_State == UIState::GAME || m_State == UIState::ONLINE_GAME) &&
        !m_ShowPauseMenu && !m_ShowVictoryPopup) {
        CheckScoreForVictory();
    }
}

void UIManager::RenderLevelCompletePopup() {
    if (!m_ShowLevelCompletePopup) return;

    // 移动弹窗到屏幕上方，避免遮挡玩家
    ImVec2 windowPos = ImVec2(
        (ImGui::GetIO().DisplaySize.x - m_PopupSize.x) / 2,
        ImGui::GetIO().DisplaySize.y * 0.15f // 放在屏幕上方15%的位置
    );

    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(m_PopupSize);

    // 半透明背景和金色边框
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.2f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.9f, 0.7f, 0.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoInputs; // 防止任何交互

    ImGui::Begin("Level Complete", nullptr, windowFlags);

    // 获取绘图列表
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    float titleScale = 2.5f; // 增大字体比例
    ImGui::PushFont(ImGui::GetFont());
    ImGui::GetFont()->Scale *= titleScale;

    std::string titleText = "LEVEL COMPLETE!";
    ImVec2 titleSize = ImGui::CalcTextSize(titleText.c_str());

    // 居中标题
    ImGui::SetCursorPosX((m_PopupSize.x - titleSize.x) / 2);

    // 绘制阴影效果
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 0.7f));
    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 3, ImGui::GetCursorPosY() + 3));
    ImGui::Text("%s", titleText.c_str());
    ImGui::PopStyleColor();

    ImGui::SetCursorPos(ImVec2((m_PopupSize.x - titleSize.x) / 2, ImGui::GetCursorPosY() - titleSize.y));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
    ImGui::Text("%s", titleText.c_str());
    ImGui::PopStyleColor();

    // 恢复字体大小
    ImGui::GetFont()->Scale /= titleScale;
    ImGui::PopFont();

    float msgScale = 1.5f;      // 使用大一点的消息文本
    ImGui::PushFont(ImGui::GetFont());
    ImGui::GetFont()->Scale *= msgScale;

    // 添加一些垂直空间
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 30);

    std::string congratsText = "Press E to interact arrows or coins.";
    ImVec2 congratsSize = ImGui::CalcTextSize(congratsText.c_str());
    ImGui::SetCursorPosX((m_PopupSize.x - congratsSize.x) / 2);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::Text("%s", congratsText.c_str());
    ImGui::PopStyleColor();

    // 添加垂直空间
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);

    std::string instructionText = "GO TO NEXT LEVEL";
    ImVec2 instrSize = ImGui::CalcTextSize(instructionText.c_str());
    ImGui::SetCursorPosX((m_PopupSize.x - instrSize.x) / 2);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::Text("%s", instructionText.c_str());
    ImGui::PopStyleColor();

    // 恢复字体大小
    ImGui::GetFont()->Scale /= msgScale;
    ImGui::PopFont();

    // 添加计时器显示
    float timerProgress = m_PopupTimer / 5.0f;

    // 在弹窗底部添加倒计时条
    ImVec2 barStart = ImVec2(50, m_PopupSize.y - 40);
    ImVec2 barEnd = ImVec2(m_PopupSize.x - 50, m_PopupSize.y - 30);

    // 倒计时条背景
    drawList->AddRectFilled(
        ImVec2(windowPos.x + barStart.x, windowPos.y + barStart.y),
        ImVec2(windowPos.x + barEnd.x, windowPos.y + barEnd.y),
        IM_COL32(60, 60, 60, 180)
    );

    // 倒计时条填充
    drawList->AddRectFilled(
        ImVec2(windowPos.x + barStart.x, windowPos.y + barStart.y),
        ImVec2(windowPos.x + barStart.x + (barEnd.x - barStart.x) * timerProgress, windowPos.y + barEnd.y),
        IM_COL32(255, 215, 0, 220)
    );

    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

void UIManager::UpdateDamageEffect(float dt) {
    float currentHealth = m_PlayerHealth;

    // 检测玩家是否受伤（生命值减少）
    if (currentHealth < m_LastPlayerHealth) {
        // 计算伤害量并增加效果强度
        float damageTaken = m_LastPlayerHealth - currentHealth;
        m_DamageEffectIntensity = std::min(m_DamageEffectIntensity + damageTaken * 3.5f, 1.0f);
        m_IsDamageEffect = true;
        // std::cout << "Player took damage: " << damageTaken << ", Effect intensity: " << m_DamageEffectIntensity << std::endl;
    }

    // 随时间衰减效果
    if (m_DamageEffectIntensity > 0.0f) {
        float oldIntensity = m_DamageEffectIntensity;
        m_DamageEffectIntensity -= m_DamageEffectDecayRate * dt;

        if (m_DamageEffectIntensity <= 0.0f) {
            m_DamageEffectIntensity = 0.0f;
            m_IsDamageEffect = false;
            // std::cout << "Damage effect ended" << std::endl;
        }
        else if (int(oldIntensity * 100) != int(m_DamageEffectIntensity * 100)) {
            // std::cout << "Damage effect intensity: " << m_DamageEffectIntensity << std::endl;
        }
    }

    m_LastPlayerHealth = currentHealth;
}

void UIManager::RenderDamageEffect() {
    if (!m_IsDamageEffect || m_DamageEffectIntensity <= 0.01f) {
        return;
    }

    auto damageEffectTimer = CreateTimer("UI - Damage Effect");

    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    ImVec2 screenSize = ImGui::GetIO().DisplaySize; 

    float effectIntensity = m_DamageEffectIntensity * 1.5f;  //效果可见度
    effectIntensity = std::min(effectIntensity, 1.0f); 

    // 调整内外半径使效果更加明显
    float innerRadius = screenSize.x * 0.1f; // 缩小内圆半径（增大红色区域）
    float outerRadius = screenSize.x * 0.9f; // 增大外圆半径（扩大影响区域）

    // 屏幕中心
    ImVec2 center(screenSize.x * 0.5f, screenSize.y * 0.5f);

    // 绘制环形渐变
    const int segments = 60;

    // 绘制填充的渐变多边形
    for (int i = 0; i < segments; i++) {
        float angle1 = 2.0f * IM_PI * (float)i / (float)segments;
        float angle2 = 2.0f * IM_PI * (float)(i + 1) / (float)segments;

        // 外部点（屏幕边缘）
        ImVec2 outer1(
            center.x + cos(angle1) * outerRadius,
            center.y + sin(angle1) * outerRadius
        );
        ImVec2 outer2(
            center.x + cos(angle2) * outerRadius,
            center.y + sin(angle2) * outerRadius
        );

        // 内部点（渐变开始处）
        ImVec2 inner1(
            center.x + cos(angle1) * innerRadius,
            center.y + sin(angle1) * innerRadius
        );
        ImVec2 inner2(
            center.x + cos(angle2) * innerRadius,
            center.y + sin(angle2) * innerRadius
        );

        // 计算角落点（使红色延伸到屏幕边缘）
        ImVec2 corner;
        if (abs(outer1.x - center.x) > abs(outer1.y - center.y)) {
            // 水平方向更远
            corner.x = (outer1.x > center.x) ? screenSize.x : 0;
            corner.y = outer1.y + (corner.x - outer1.x) * (outer1.y - center.y) / (outer1.x - center.x);
        }
        else {
            // 垂直方向更远
            corner.y = (outer1.y > center.y) ? screenSize.y : 0;
            corner.x = outer1.x + (corner.y - outer1.y) * (outer1.x - center.x) / (outer1.y - center.y);
        }

        ImVec2 corner2;
        if (abs(outer2.x - center.x) > abs(outer2.y - center.y)) {
            corner2.x = (outer2.x > center.x) ? screenSize.x : 0;
            corner2.y = outer2.y + (corner2.x - outer2.x) * (outer2.y - center.y) / (outer2.x - center.x);
        }
        else {
            corner2.y = (outer2.y > center.y) ? screenSize.y : 0;
            corner2.x = outer2.x + (corner2.y - outer2.y) * (outer2.x - center.x) / (outer2.y - center.y);
        }

        // 根据伤害强度计算红色透明度 - 增强透明度以便更容易看到
        ImU32 innerColor = IM_COL32(255, 0, 0, 0);  // 内部完全透明
        ImU32 outerColor = IM_COL32(255, 0, 0, (int)(220 * effectIntensity)); // 外部红色（增强）
        ImU32 cornerColor = IM_COL32(255, 0, 0, (int)(255 * effectIntensity)); // 角落最大红色

        // 绘制三角形扇形
        drawList->AddTriangleFilled(inner1, outer1, inner2, innerColor);
        drawList->AddTriangleFilled(inner2, outer1, outer2, outerColor);
        drawList->AddTriangleFilled(outer1, corner, corner2, cornerColor);
        drawList->AddTriangleFilled(outer1, corner2, outer2, cornerColor);
    }
}

void UIManager::InitializeSkills() {
    if (m_SkillsInitialized) return;

    // 清空并重新初化
    m_Skills.clear();

    // 四个技能: 名称、按键、冷却时间、解锁所需分数
    m_Skills.push_back(SkillInfo("Dash", "Q", 1.0f, 0));       // 初始解锁
    m_Skills.push_back(SkillInfo("Shockwave", "Y", 1.0f, 5));     // 需要5分解锁
    m_Skills.push_back(SkillInfo("BlackHole", "J", 10.0f, 10));     // 需要10分解锁
    m_Skills.push_back(SkillInfo("Turret", "K", 10.0f, 15));     // 需要15分解锁

    m_Skills[0].unlocked = true;  // 默认解锁第一个技能
    m_Skills[0].visible = true;
    m_SkillsInitialized = true;
}

void UIManager::HandleSkillKeyPress(int key) {
    // 只有在游戏状态下处理
    if (m_State != UIState::GAME && m_State != UIState::ONLINE_GAME) return;

    int skillIndex = -1;

    // 技能按键映射
    if (key == GLFW_KEY_Q) skillIndex = 0;       // 冲刺技能
    else if (key == GLFW_KEY_Y) skillIndex = 1;  // 震荡波技能
    else if (key == GLFW_KEY_J) skillIndex = 2;  // 黑洞技能
    else if (key == GLFW_KEY_K) skillIndex = 3;  // 炮台技能

    if (skillIndex >= 0 && skillIndex < m_Skills.size()) {
        // 检查技能是否可用
        if (m_Skills[skillIndex].unlocked && m_Skills[skillIndex].visible && !m_Skills[skillIndex].onCooldown) {
            // 触发技能冷却
            m_Skills[skillIndex].onCooldown = true;
            m_Skills[skillIndex].currentCooldown = m_Skills[skillIndex].cooldownTime;
            AudioManager::GetInstance()->PlaySFX("click");
        }
    }
}

void UIManager::CheckSkillKeys() {
    if (!m_Window) return;

    // 检查 Q键（冲刺）
    static bool qKeyPressed = false;
    bool qKeyDown = glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS;
    if (qKeyDown && !qKeyPressed) {
        HandleSkillKeyPress(GLFW_KEY_Q);
    }
    qKeyPressed = qKeyDown;

    // 检查 Y键（震荡波）
    static bool yKeyPressed = false;
    bool yKeyDown = glfwGetKey(m_Window, GLFW_KEY_Y) == GLFW_PRESS;
    if (yKeyDown && !yKeyPressed) {
        HandleSkillKeyPress(GLFW_KEY_Y);
    }
    yKeyPressed = yKeyDown;

    // 检查 J键（黑洞）
    static bool jKeyPressed = false;
    bool jKeyDown = glfwGetKey(m_Window, GLFW_KEY_J) == GLFW_PRESS;
    if (jKeyDown && !jKeyPressed) {
        HandleSkillKeyPress(GLFW_KEY_J);
    }
    jKeyPressed = jKeyDown;

    // 检查 K键（炮台）
    static bool kKeyPressed = false;
    bool kKeyDown = glfwGetKey(m_Window, GLFW_KEY_K) == GLFW_PRESS;
    if (kKeyDown && !kKeyPressed) {
        HandleSkillKeyPress(GLFW_KEY_K);
    }
    kKeyPressed = kKeyDown;
}


void UIManager::UpdateSkillCooldowns(float dt) {
    // 更新每个技能的冷却时间
    for (auto& skill : m_Skills) {
        if (skill.onCooldown) {
            skill.currentCooldown -= dt;
            if (skill.currentCooldown <= 0.0f) {
                skill.currentCooldown = 0.0f;
                skill.onCooldown = false;
            }
        }
    }

    // 更新技能解锁动画
    if (m_ShowSkillUnlockMessage) {
        UpdateSkillUnlockAnimation(dt);
    }

    // 检查按键事件
    CheckSkillKeys();
}

void UIManager::SetSkillCooldown(int skillIndex, bool onCooldown) {
    if (skillIndex >= 0 && skillIndex < m_Skills.size()) {
        m_Skills[skillIndex].onCooldown = onCooldown;
        if (onCooldown && m_Skills[skillIndex].currentCooldown <= 0.0f) {
            m_Skills[skillIndex].currentCooldown = m_Skills[skillIndex].cooldownTime;
        }
    }
}

void UIManager::CheckSkillUnlocks(int currentScore) {
    // 检查每个未解锁的技能是否可以解锁
    for (int i = 0; i < m_Skills.size(); i++) {
        if (!m_Skills[i].unlocked && currentScore >= m_Skills[i].unlockScore) {
           
            m_Skills[i].unlocked = true;   // 解锁技能
            ShowSkillUnlockMessage(i);      

            // 只解锁一个技能然后退出，避免同时显示多个解锁消息
            break;
        }
    }
}

void UIManager::ShowSkillUnlockMessage(int skillIndex) {
    if (skillIndex >= 0 && skillIndex < m_Skills.size()) {
        m_ShowSkillUnlockMessage = true;
        m_UnlockedSkillIndex = skillIndex;
        m_UnlockMessageTime = 0.0f;
        m_UnlockAnimProgress = 0.0f;

        // 计算消息起始位置 (屏幕中央)
        m_MessageStartPos = ImVec2((ImGui::GetIO().DisplaySize.x - 400) / 2,(ImGui::GetIO().DisplaySize.y - 200) / 2);

        // 计算目标位置 (技能条位置)
        float startY = 10.0f + m_HealthBarSize.y + 10.0f; // 血条下方空间
        m_MessageTargetPos = ImVec2(10.0f,startY + (m_SkillBarSize.y + m_SkillBarSpacing) * skillIndex);
        m_MessageStartSize = ImVec2(400, 200); // 起始消息大小
        m_Skills[skillIndex].unlocked = true;    // 将技能设为解锁状态但不可见
        // m_Skills[skillIndex].visible = false; // 动画结束后才设置为可见
    }
}

void UIManager::UpdateSkillUnlockAnimation(float dt) {
    if (!m_ShowSkillUnlockMessage) return;

    m_UnlockMessageTime += dt;

    const float TOTAL_ANIM_TIME = 3.0f;   // 动画持续时间

    if (m_UnlockMessageTime <= TOTAL_ANIM_TIME) {
        // 使用缓动函数使动画更流畅
        float t = m_UnlockMessageTime / TOTAL_ANIM_TIME;

        // 使用三次贝塞尔缓动: 开始较慢，中间加速，结束减速
        m_UnlockAnimProgress = t * t * (3.0f - 2.0f * t);
    }
    else {
        // 动画结束，显示解锁的技能
        if (m_UnlockedSkillIndex >= 0 && m_UnlockedSkillIndex < m_Skills.size()) {
            m_Skills[m_UnlockedSkillIndex].visible = true;
        }
 
        m_ShowSkillUnlockMessage = false;
        m_UnlockAnimProgress = 1.0f;
    }
}

void UIManager::RenderSkillUnlockMessage() {
    if (!m_ShowSkillUnlockMessage || m_UnlockedSkillIndex < 0) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // 计算当前位置和大小 (使用插值)
    ImVec2 currPos = ImVec2(
        m_MessageStartPos.x + (m_MessageTargetPos.x - m_MessageStartPos.x) * m_UnlockAnimProgress,
        m_MessageStartPos.y + (m_MessageTargetPos.y - m_MessageStartPos.y) * m_UnlockAnimProgress
    );

    ImVec2 currSize = ImVec2(
        m_MessageStartSize.x + (m_SkillBarSize.x - m_MessageStartSize.x) * m_UnlockAnimProgress,
        m_MessageStartSize.y + (m_SkillBarSize.y - m_MessageStartSize.y) * m_UnlockAnimProgress
    );

    // 计算透明度 (开始显示，结束时消失)
    float alpha = 1.0f;
    if (m_UnlockAnimProgress > 0.7f) {
        alpha = (1.0f - m_UnlockAnimProgress) / 0.3f;
    }

    // 绘制消息背景
    drawList->AddRectFilled(
        currPos,
        ImVec2(currPos.x + currSize.x, currPos.y + currSize.y),
        IM_COL32(40, 40, 80, (int)(200 * alpha))
    );

    // 绘制消息边框
    drawList->AddRect(
        currPos,
        ImVec2(currPos.x + currSize.x, currPos.y + currSize.y),
        IM_COL32(220, 180, 40, (int)(255 * alpha)),
        4.0f, 0, 2.0f
    );

    // 消息内容
    std::string skillName = m_Skills[m_UnlockedSkillIndex].name;
    std::string message = "NewSkill: " + skillName + " (" + m_Skills[m_UnlockedSkillIndex].key + ")";

    // 根据动画进度调整文字大小和位置
    float fontSize = 24.0f + (12.0f - 24.0f) * m_UnlockAnimProgress;

    // 文字绘制在消息中央
    ImVec2 textSize = ImGui::CalcTextSize(message.c_str());
    textSize.x *= fontSize / 14.0f; // 估算文字大小变化
    textSize.y *= fontSize / 14.0f;

    ImVec2 textPos = ImVec2(
        currPos.x + (currSize.x - textSize.x) * 0.5f,
        currPos.y + (currSize.y - textSize.y) * 0.5f
    );

    // 绘制文字阴影和文字
    drawList->AddText(
        ImGui::GetFont(),
        fontSize,
        ImVec2(textPos.x + 2, textPos.y + 2),
        IM_COL32(0, 0, 0, (int)(150 * alpha)),
        message.c_str()
    );

    drawList->AddText(
        ImGui::GetFont(),
        fontSize,
        textPos,
        IM_COL32(255, 220, 100, (int)(255 * alpha)),
        message.c_str()
    );
}

void UIManager::RenderSkillBars() {
    if (!m_SkillsInitialized) {
        InitializeSkills();
    }

    if (m_State != UIState::GAME && m_State != UIState::ONLINE_GAME) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float padding = 10.0f;

    // 从记分板获取当前分数
    int currentScore = NCL::CSC8503::Monster::GetKillCount();

    CheckSkillUnlocks(currentScore);

    // 计算技能条的起始位置 (血条下方)
    float startY = padding + m_HealthBarSize.y + padding;

    // 渲染每个已解锁且可见的技能条
    for (int i = 0; i < m_Skills.size(); i++) {
        if (!m_Skills[i].unlocked || !m_Skills[i].visible) continue;

        float yPos = startY + i * (m_SkillBarSize.y + m_SkillBarSpacing);
        ImVec2 barPos = ImVec2(padding, yPos);
        ImVec2 barEndPos = ImVec2(barPos.x + m_SkillBarSize.x, barPos.y + m_SkillBarSize.y);

        // 绘制技能
        ImU32 bgColor = m_Skills[i].onCooldown ? IM_COL32(60, 60, 80, 200) : IM_COL32(40, 60, 90, 200);
        drawList->AddRectFilled(barPos, barEndPos, bgColor);
        drawList->AddRect(barPos, barEndPos, IM_COL32(180, 180, 220, 255));
        std::string text = m_Skills[i].name + " [" + m_Skills[i].key + "]";
        ImVec2 textPos = ImVec2(barPos.x + 5, barPos.y + (m_SkillBarSize.y - ImGui::GetTextLineHeight()) / 2);
        drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 150), text.c_str());
        ImU32 textColor = m_Skills[i].onCooldown ? IM_COL32(200, 200, 200, 200) : IM_COL32(255, 255, 255, 255);
        drawList->AddText(textPos, textColor, text.c_str());

        // 如果在冷却中，绘制增强版冷却效果
        if (m_Skills[i].onCooldown && m_Skills[i].cooldownTime > 0) {
            // 计算冷却进度
            float cdProgress = 1.0f - (m_Skills[i].currentCooldown / m_Skills[i].cooldownTime);

            // 1. 绘制冷却进度条背景
            ImVec2 cdBarPos = ImVec2(barPos.x, barPos.y + m_SkillBarSize.y + 2);
            ImVec2 cdBarEndPos = ImVec2(barEndPos.x, cdBarPos.y + 5);
            drawList->AddRectFilled(cdBarPos, cdBarEndPos, IM_COL32(40, 40, 40, 180));

            // 2. 绘制冷却进度条
            ImVec2 cdFillEnd = ImVec2(cdBarPos.x + m_SkillBarSize.x * cdProgress, cdBarEndPos.y);

            // 创建渐变色进度条 (从红色到绿色)
            ImU32 startColor = IM_COL32(230, 50, 50, 255);  // 红色
            ImU32 endColor = IM_COL32(50, 230, 50, 255);    // 绿色

            // 根据冷却进度计算颜色
            ImU32 progressColor;
            if (cdProgress < 0.5f) {
                // 从红色渐变到黄色
                float t = cdProgress * 2.0f;
                progressColor = IM_COL32(230,50 + (int)(180 * t),50,255);
            }
            else {
                // 从黄色渐变到绿色
                float t = (cdProgress - 0.5f) * 2.0f;
                progressColor = IM_COL32(230 - (int)(180 * t),230,50 + (int)(180 * t), 255);
            }

            drawList->AddRectFilled(cdBarPos, cdFillEnd, progressColor);

            // 3. 绘制冷却时间文本
            char cdText[16];
            sprintf_s(cdText, "%.1fs", m_Skills[i].currentCooldown);

            ImVec2 cdTextSize = ImGui::CalcTextSize(cdText);
            ImVec2 cdTextPos = ImVec2(
                barPos.x + (m_SkillBarSize.x - cdTextSize.x) / 2,
                barPos.y + (m_SkillBarSize.y - cdTextSize.y) / 2
            );

            // 如果冷却时间超过50%，显示黑色字体，否则显示白色字体
            ImU32 cdTextColor = cdProgress > 0.5f ? IM_COL32(30, 30, 30, 255) : IM_COL32(255, 255, 255, 255);

            // 冷却时间文字阴影
            drawList->AddText(ImVec2(cdTextPos.x + 1, cdTextPos.y + 1), IM_COL32(0, 0, 0, 150), cdText);

            // 冷却时间文字
            drawList->AddText(cdTextPos, cdTextColor, cdText);

            // 4. 添加闪烁效果 (接近冷却结束时)
            if (cdProgress > 0.9f) {
                float time = ImGui::GetTime();
                float alpha = (sin(time * 15.0f) * 0.5f + 0.5f) * 255;

                drawList->AddRect(barPos,barEndPos, IM_COL32(255, 255, 100, (int)alpha),0, 0, 2.0f);
            }
        }
    }

    // 渲染技能解锁消息 (如果有)
    RenderSkillUnlockMessage();
}

void UIManager::ToggleDebugTab() {
    // 只在调试窗口可见时切换
    if (!m_ShowDebugWindow) return;

    // 循环切换选项卡
    m_CurrentDebugTab = (m_CurrentDebugTab + 1) % 4;

    // std::cout << "切换到调试选项卡: " << (m_CurrentDebugTab == 0 ? "performances" : m_CurrentDebugTab == 1 ? "DebuggingInfo" : m_CurrentDebugTab == 2 ? "Memory" : "Physics") << std::endl;
}

void UIManager::RenderBulletCollisionInfo() {
    using namespace NCL::CSC8503;

    // 获取当前帧的碰撞信息
    auto collisions = BulletWorldManager::GetCollisionDetails();

    ImGui::Text("Current collision log: %d", (int)collisions.size());

    if (collisions.empty()) {
        ImGui::Text("no current collisions");
        return;
    }

    // 如果有碰撞，显示碰撞详情
    if (ImGui::BeginTable("CollisionTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("collision object");
        ImGui::TableSetupColumn("collision point");
        ImGui::TableSetupColumn("penetration depth");
        ImGui::TableHeadersRow();

        for (int i = 0; i < collisions.size() && i < 10; i++) { // 最多显示10对
            const auto& pair = collisions[i];

            ImGui::TableNextRow();

            // 获取对象指针，尝试获取名称
            void* userPtrA = pair.objectA->getUserPointer();
            void* userPtrB = pair.objectB->getUserPointer();

            ImGui::TableSetColumnIndex(0);
            // 显示对象指针作为标识
            ImGui::Text("object %p <-> %p", pair.objectA, pair.objectB);

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", (int)pair.points.size());

            ImGui::TableSetColumnIndex(2);
            // 显示最小穿透深度（负值表示更深的穿透）
            float minDepth = 0;
            for (const auto& pt : pair.points) {
                minDepth = std::min(minDepth, pt.distance);
            }
            ImGui::Text("%.3f", minDepth);

            // 如果展开这一行，显示更多详情
            if (ImGui::TreeNode(("detail##" + std::to_string(i)).c_str())) {
                for (int j = 0; j < pair.points.size(); j++) {
                    const auto& pt = pair.points[j];
                    ImGui::Text("point %d: position(%.1f, %.1f, %.1f) surface line(%.1f, %.1f, %.1f)",
                        j,
                        pt.position.x(), pt.position.y(), pt.position.z(),
                        pt.normal.x(), pt.normal.y(), pt.normal.z());
                }
                ImGui::TreePop();
            }
        }

        ImGui::EndTable();
    }

}

void UIManager::UpdateDebugInfo() {
    // 此方法每帧调用一次，用于更新调试信息
#ifdef _WIN32
    // 更新内存使用信息
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        SIZE_T physicalMemUsed = pmc.WorkingSetSize / 1024; // 转换为KB
        SIZE_T virtualMemUsed = pmc.PrivateUsage / 1024;    // 转换为KB

        AddDebugInfo("physical memory (KB)", std::to_string((int)physicalMemUsed), "system resource");
        AddDebugInfo("physical memory (MB)", std::to_string(physicalMemUsed / 1024.0f), "system resource");
        AddDebugInfo("virtual memory (KB)", std::to_string((int)virtualMemUsed), "system resource");
        AddDebugInfo("virtual memory (MB)", std::to_string(virtualMemUsed / 1024.0f), "system resource");
    }
#endif

    // 更新碰撞信息
    using namespace NCL::CSC8503;
    auto collisions = BulletWorldManager::GetCollisionDetails();
    AddDebugInfo("logarithmic collision ", std::to_string(collisions.size()), "physical system");

    // 清除旧的碰撞详情
    for (int i = 0; i < 10; i++) {
        RemoveDebugInfo("collision pair #" + std::to_string(i));
    }

    // 添加最新的碰撞详情（最多显示10个）
    int count = std::min((int)collisions.size(), 10);
    for (int i = 0; i < count; i++) {
        const auto& pair = collisions[i];

        // 尝试获取对象信息
        void* userPtrA = pair.objectA->getUserPointer();
        void* userPtrB = pair.objectB->getUserPointer();

        // 使用指针地址标识碰撞对象
        std::stringstream ss;
        ss << std::hex << userPtrA << " <-> " << userPtrB
            << " (" << pair.points.size() << "point)";

        AddDebugInfo("collision pair #" + std::to_string(i), ss.str(), "physical system");
    }
}

void UIManager::RenderSystemMemoryInfo() {
    // 使用Windows API获取进程内存使用情况
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        SIZE_T physicalMemUsed = pmc.WorkingSetSize / 1024; // 转换为KB
        SIZE_T virtualMemUsed = pmc.PrivateUsage / 1024;    // 转换为KB

        ImGui::Text("physical memory: %d KB (%.2f MB)",
            (int)physicalMemUsed, physicalMemUsed / 1024.0f);
        ImGui::Text("virtual memory: %d KB (%.2f MB)",
            (int)virtualMemUsed, virtualMemUsed / 1024.0f);

        // 绘制内存使用历史图表
        static float memoryHistory[100] = { 0 };
        static int memoryHistoryOffset = 0;

        // 更新历史数据
        memoryHistory[memoryHistoryOffset] = (float)physicalMemUsed;
        memoryHistoryOffset = (memoryHistoryOffset + 1) % 100;

        ImGui::PlotLines("Memory Usage History (KB)", memoryHistory, 100, memoryHistoryOffset,
            NULL, 0.0f, FLT_MAX, ImVec2(0, 80));
    }
    else {
        ImGui::Text("Cannot get memory information");
    }
#else
    ImGui::Text("在非Windows平台无法获取内存信息");
#endif
}

bool UIManager::IsVictoryPopupVisible() const {
    return m_ShowVictoryPopup;
}

void UIManager::CheckScoreForVictory() {
    // 如果已经触发过胜利，不再检查
    if (m_VictoryAchieved) return;

    // 从Monster类获取当前击杀数/分数
    int currentScore = NCL::CSC8503::Monster::GetKillCount();

    // 检查是否达到胜利条件
    if (currentScore >= m_VictoryScoreThreshold) {
        ShowVictoryPopup();
        m_VictoryAchieved = true; // 标记已经达成胜利，防止重复显示
    }
}

void UIManager::ShowVictoryPopup() {
    m_ShowVictoryPopup = true;
    m_VictoryPopupTimer = 10.0f; // 显示弹窗10秒

    // 播放胜利音效
    AudioManager::GetInstance()->PlaySFX("click"); // 可以替换为专门的胜利音效

    // 可以在这里添加其他胜利效果，比如暂停游戏等
    // GameManager::GetInstance()->SetPaused(true);
}

void UIManager::RenderVictoryPopup() {
    if (!m_ShowVictoryPopup) return;

    // 居中显示弹窗
    ImVec2 windowPos = ImVec2(
        (ImGui::GetIO().DisplaySize.x - m_VictoryPopupSize.x) / 2,
        (ImGui::GetIO().DisplaySize.y - m_VictoryPopupSize.y) / 2
    );

    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(m_VictoryPopupSize);

    // 半透明金色背景和边框
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.15f, 0.05f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.84f, 0.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 15.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(25, 25));

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoInputs; // 防止任何交互

    ImGui::Begin("Victory Popup", nullptr, windowFlags);

    // 获取绘图列表
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // 使用更大的字体显示胜利标题
    float titleScale = 3.5f;
    ImGui::PushFont(ImGui::GetFont());
    ImGui::GetFont()->Scale *= titleScale;

    std::string titleText = " Win! ";
    ImVec2 titleSize = ImGui::CalcTextSize(titleText.c_str());

    // 居中标题
    ImGui::SetCursorPosX((m_VictoryPopupSize.x - titleSize.x) / 2);

    // 标题阴影
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 0.7f));
    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 4, ImGui::GetCursorPosY() + 4));
    ImGui::Text("%s", titleText.c_str());
    ImGui::PopStyleColor();

    // 标题文本
    ImGui::SetCursorPos(ImVec2((m_VictoryPopupSize.x - titleSize.x) / 2, ImGui::GetCursorPosY() - titleSize.y));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.84f, 0.0f, 1.0f));
    ImGui::Text("%s", titleText.c_str());
    ImGui::PopStyleColor();

    // 恢复字体大小
    ImGui::GetFont()->Scale /= titleScale;
    ImGui::PopFont();

    // 添加一些垂直空间
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50);

    // 副标题
    float subtitleScale = 2.0f;
    ImGui::PushFont(ImGui::GetFont());
    ImGui::GetFont()->Scale *= subtitleScale;

    std::string subtitleText = "Score 400 !";
    ImVec2 subtitleSize = ImGui::CalcTextSize(subtitleText.c_str());
    ImGui::SetCursorPosX((m_VictoryPopupSize.x - subtitleSize.x) / 2);

    // 副标题阴影
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 0.7f));
    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 2, ImGui::GetCursorPosY() + 2));
    ImGui::Text("%s", subtitleText.c_str());
    ImGui::PopStyleColor();

    // 副标题文本
    ImGui::SetCursorPos(ImVec2((m_VictoryPopupSize.x - subtitleSize.x) / 2, ImGui::GetCursorPosY() - subtitleSize.y));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::Text("%s", subtitleText.c_str());
    ImGui::PopStyleColor();

    ImGui::GetFont()->Scale /= subtitleScale;
    ImGui::PopFont();

    // 添加更多垂直空间
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 80);

    // 详细信息
    float infoScale = 1.5f;
    ImGui::PushFont(ImGui::GetFont());
    ImGui::GetFont()->Scale *= infoScale;

    std::string infoText = "Your are Win !!!!";
    ImVec2 infoSize = ImGui::CalcTextSize(infoText.c_str());
    ImGui::SetCursorPosX((m_VictoryPopupSize.x - infoSize.x) / 2);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::Text("%s", infoText.c_str());
    ImGui::PopStyleColor();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 30);

    std::string continueText = "Continue game or Exit";
    ImVec2 continueSize = ImGui::CalcTextSize(continueText.c_str());
    ImGui::SetCursorPosX((m_VictoryPopupSize.x - continueSize.x) / 2);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.8f, 1.0f));
    ImGui::Text("%s", continueText.c_str());
    ImGui::PopStyleColor();

    ImGui::GetFont()->Scale /= infoScale;
    ImGui::PopFont();

    // 添加倒计时条
    ImGui::SetCursorPosY(m_VictoryPopupSize.y - 60);
    float timerProgress = m_VictoryPopupTimer / 10.0f; // 基于10秒的计时器

    // 倒计时条背景
    ImVec2 barStart = ImVec2(50, m_VictoryPopupSize.y - 50);
    ImVec2 barEnd = ImVec2(m_VictoryPopupSize.x - 50, m_VictoryPopupSize.y - 40);

    drawList->AddRectFilled(
        ImVec2(windowPos.x + barStart.x, windowPos.y + barStart.y),
        ImVec2(windowPos.x + barEnd.x, windowPos.y + barEnd.y),
        IM_COL32(60, 60, 60, 180)
    );

    // 倒计时条填充
    drawList->AddRectFilled(
        ImVec2(windowPos.x + barStart.x, windowPos.y + barStart.y),
        ImVec2(windowPos.x + barStart.x + (barEnd.x - barStart.x) * timerProgress, windowPos.y + barEnd.y),
        IM_COL32(255, 215, 0, 220)
    );

    // 显示剩余时间
    char timerText[32];
    sprintf_s(timerText, "close: %.1f", m_VictoryPopupTimer);
    ImVec2 timerTextSize = ImGui::CalcTextSize(timerText);
    ImGui::SetCursorPos(ImVec2((m_VictoryPopupSize.x - timerTextSize.x) / 2, m_VictoryPopupSize.y - 35));
    ImGui::Text("%s", timerText);

    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}