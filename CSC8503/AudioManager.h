#pragma once
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include <string>
#include <vector>
#include <future>
#include <thread>
#include <chrono>

class AudioManager {
private:
    static AudioManager* instance;

    FMOD::Studio::System* studioSystem;
    FMOD::System* coreSystem;

    // BGM相关变量
    FMOD::Sound* bgmSound;
    FMOD::Channel* bgmChannel;
    float bgmVolume;
    bool bgmLoaded;

    // 音效相关变量
    std::unordered_map<std::string, FMOD::Sound*> sfxSounds;
    std::unordered_map<std::string, FMOD::Channel*> sfxChannels;
    float sfxVolume;

    // 内部音效初始化方法
    void InitializeSFX();

    float m_PeakLimitThreshold;  // 最大音量峰值阈值 (0.0-1.0)
    std::unordered_map<std::string, float> m_SFXNormalizationLevels;  // 存储每个声音的标准化级别
    bool m_UseNormalization;     // 音量标准化
    bool m_UsePeakLimiter;       // 峰值限制器

    // 关卡武器音效暂停时间（毫秒）
    std::vector<int> m_WeaponSoundPauseTimes;

public:

    AudioManager();
    static AudioManager* GetInstance();
    static void Release();

    void Init();

    // BGM相关方法
    bool LoadBGM(const char* filePath);
    void PlayBGM(bool loop = true);
    void StopBGM();
    void PauseBGM();
    void ResumeBGM();
    void SetBGMVolume(float volume);
    float GetBGMVolume() const;
    bool IsBGMPlaying() const;

    // 音效相关方法
    bool LoadSFX(const std::string& name, const char* filePath);
    void PlaySFX(const std::string& name, bool loop = false);
    void StopSFX(const std::string& name);
    void StopAllSFX();
    void SetSFXVolume(float volume);
    float GetSFXVolume() const;
    bool IsSFXPlaying(const std::string& name) const;

    // 接口方法
    static void PlaySound(const std::string& name);

    // 音量标准化和峰值限制方法
    void SetPeakLimitThreshold(float threshold);
    float GetPeakLimitThreshold() const;
    void EnableNormalization(bool enable);
    void EnablePeakLimiter(bool enable);
    bool IsNormalizationEnabled() const;
    bool IsPeakLimiterEnabled() const;
    void NormalizeAllSounds();
    void AnalyzeSoundVolume(const std::string& name);
    float ApplyPeakLimiter(float volume) const;

    // 关卡层级播放武器音效
    std::vector<std::string> GetWeaponSoundsUpToLevel(int level);
    void PlayWeaponSoundsSequence(int level, bool useThreadForPause = true);

    void PlayWeaponSoundsMix(int maxLevel, int soundCount = 3);
};