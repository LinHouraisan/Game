#include "AudioManager.h"
#include <iostream>
#include <random>
#include <algorithm>

AudioManager* AudioManager::instance = nullptr;

AudioManager::AudioManager() :
    studioSystem(nullptr),
    coreSystem(nullptr),
    bgmSound(nullptr),
    bgmChannel(nullptr),
    bgmVolume(1.0f),
    bgmLoaded(false),
    m_PeakLimitThreshold(0.85f),  // 峰值阈值为85%
    m_UseNormalization(true),     // 启用标准化
    m_UsePeakLimiter(true),
    sfxVolume(1.0f) {

    // 初始化各关卡武器音效暂停时间（毫秒）
    m_WeaponSoundPauseTimes = {
        0,      // 第一关无暂停
        20,     // 第二关微小暂停 (50ms)
        150,    // 第三关小暂停 (150ms)
        220,    // 第四关稍大暂停 (300ms)
        0       // 第五关无暂停
    };
}

void AudioManager::Init() {

    FMOD::Studio::System::create(&studioSystem);
    studioSystem->getCoreSystem(&coreSystem);
    studioSystem->initialize(32, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
    InitializeSFX();
}

void AudioManager::InitializeSFX() {
    // 通用音效
    LoadSFX("boom", "../../Assets/Audio/SFX/boom.mp3");
    LoadSFX("heal", "../../Assets/Audio/SFX/heal.mp3");
    LoadSFX("hurt", "../../Assets/Audio/SFX/hurt.mp3");
    LoadSFX("click", "../../Assets/Audio/SFX/WaterDropBig.mp3");

    // 武器音效
    LoadSFX("gunshot", "../../Assets/Audio/SFX/SE.wav");          // 第一关武器音效
    LoadSFX("buqiang", "../../Assets/Audio/SFX/buqiang.wav");     // 第二关武器音效
    LoadSFX("penzi", "../../Assets/Audio/SFX/penzi.mp3");         // 第三关武器音效
    LoadSFX("heidong", "../../Assets/Audio/SFX/heidong.mp3");     // 第四关武器音效
    LoadSFX("zheshe", "../../Assets/Audio/SFX/zheshe.mp3");       // 第五关武器音效

}

AudioManager* AudioManager::GetInstance() {
    if (!instance) {
        instance = new AudioManager();
    }
    return instance;
}

void AudioManager::Release() {
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

// 供其他模块播放声音的直接接口
void AudioManager::PlaySound(const std::string& name) {
    AudioManager* audioManager = GetInstance();
    if (audioManager) {
        audioManager->PlaySFX(name);
    }
}

bool AudioManager::LoadBGM(const char* filePath) {

    if (bgmSound) {
        bgmSound->release();
        bgmSound = nullptr;
    }

    bgmLoaded = false;

    FMOD_RESULT result = coreSystem->createSound(
        filePath,
        FMOD_DEFAULT,
        nullptr,
        &bgmSound
    );

    if (result != FMOD_OK) {
        // std::cout << "加载BGM文件失败: " << filePath << ", 错误: " << result << std::endl;
        bgmSound = nullptr;
        return false;
    }

    // std::cout << "成功加载BGM: " << filePath << std::endl;
    bgmLoaded = true;
    return true;
}

void AudioManager::PlayBGM(bool loop) {
    if (!bgmLoaded || !bgmSound) {
        // std::cout << "无法播放BGM: 没有BGM加载" << std::endl;
        return;
    }

    // 设置循环模式
    if (loop) {
        bgmSound->setMode(FMOD_LOOP_NORMAL);
    }
    else {
        bgmSound->setMode(FMOD_LOOP_OFF);
    }

    // 播放声音
    FMOD_RESULT result = coreSystem->playSound(
        bgmSound,                       // 要播放的声音
        nullptr,                        // 不分组
        false,                          // 不暂停
        &bgmChannel                     // 输出通道
    );

    if (result != FMOD_OK) {
        // std::cout << "播放BGM失败, 错误: " << result << std::endl;
        return;
    }

    // 设置音量
    if (bgmChannel) {
        bgmChannel->setVolume(bgmVolume);
    }

    // std::cout << "BGM开始播放" << std::endl;
}

void AudioManager::StopBGM() {
    if (bgmChannel) {
        bgmChannel->stop();
        bgmChannel = nullptr;
    }

    if (bgmSound) {
        bgmSound->release();
        bgmSound = nullptr;
    }

    bgmLoaded = false;
}

void AudioManager::PauseBGM() {
    if (bgmChannel) {
        bgmChannel->setPaused(true);
    }
}

void AudioManager::ResumeBGM() {
    if (bgmChannel) {
        bgmChannel->setPaused(false);
    }
}

void AudioManager::SetBGMVolume(float volume) {
    bgmVolume = volume;
    if (bgmChannel) {
        bgmChannel->setVolume(volume);
    }
}

float AudioManager::GetBGMVolume() const {
    return bgmVolume;
}

bool AudioManager::IsBGMPlaying() const {
    if (!bgmChannel) {
        return false;
    }

    bool isPlaying = false;
    bgmChannel->isPlaying(&isPlaying);
    return isPlaying;
}

bool AudioManager::LoadSFX(const std::string& name, const char* filePath) {
    // 原始实现代码
    auto it = sfxSounds.find(name);
    if (it != sfxSounds.end() && it->second) {
        it->second->release();
        sfxSounds.erase(it);
    }

    FMOD::Sound* sound = nullptr;
    FMOD_RESULT result = coreSystem->createSound(
        filePath,
        FMOD_DEFAULT,
        nullptr,
        &sound
    );

    if (result != FMOD_OK) {
        // std::cout << "无法加载声音文件: " << filePath << ", 错误代码: " << result << std::endl;
        return false;
    }

    sfxSounds[name] = sound;
    // std::cout << "成功加载声音: " << name << " 从 " << filePath << std::endl;

    // 如果启用了标准化，则分析音量
    if (m_UseNormalization) {
        AnalyzeSoundVolume(name);
    }

    return true;
}

// 播放音效
void AudioManager::PlaySFX(const std::string& name, bool loop) {
    auto it = sfxSounds.find(name);
    if (it == sfxSounds.end() || !it->second) {
        // std::cout << "无法播放声音: " << name << " - 声音未加载" << std::endl;
        return;
    }

    // 设置循环模式
    if (loop) {
        it->second->setMode(FMOD_LOOP_NORMAL);
    }
    else {
        it->second->setMode(FMOD_LOOP_OFF);
    }

    // 播放声音
    FMOD::Channel* channel = nullptr;
    FMOD_RESULT result = coreSystem->playSound(
        it->second,
        nullptr,
        false,
        &channel
    );

    if (result != FMOD_OK) {
        // std::cout << "播放声音失败: " << name << ", 错误代码: " << result << std::endl;
        return;
    }

    // 存储通道
    sfxChannels[name] = channel;

    if (channel) {
        // 计算最终音量，包含标准化和峰值限制
        float finalVolume = sfxVolume;

        // 如果启用，应用标准化
        if (m_UseNormalization) {
            auto normIt = m_SFXNormalizationLevels.find(name);
            if (normIt != m_SFXNormalizationLevels.end()) {
                finalVolume *= normIt->second;
            }
        }

        // 如果启用，应用峰值限制器
        if (m_UsePeakLimiter) {
            finalVolume = ApplyPeakLimiter(finalVolume);
        }

        // 设置最终音量
        channel->setVolume(finalVolume);

        // std::cout << "声音开始播放: " << name << " 音量为: " << finalVolume << std::endl;
    }
}

// 停止音效
void AudioManager::StopSFX(const std::string& name) {
    auto it = sfxChannels.find(name);
    if (it != sfxChannels.end() && it->second) {
        it->second->stop();
        sfxChannels.erase(it);
    }
}

// 停止所有音效
void AudioManager::StopAllSFX() {
    for (auto& pair : sfxChannels) {
        if (pair.second) {
            pair.second->stop();
        }
    }
    sfxChannels.clear();
}

// 设置所有音效的音量
void AudioManager::SetSFXVolume(float volume) {
    sfxVolume = volume;
    for (auto& pair : sfxChannels) {
        if (pair.second) {
            pair.second->setVolume(volume);
        }
    }
}

// 获取当前音效音量
float AudioManager::GetSFXVolume() const {
    return sfxVolume;
}

// 检查音效是否正在播放
bool AudioManager::IsSFXPlaying(const std::string& name) const {
    auto it = sfxChannels.find(name);
    if (it == sfxChannels.end() || !it->second) {
        return false;
    }

    bool isPlaying = false;
    it->second->isPlaying(&isPlaying);
    return isPlaying;
}

void AudioManager::SetPeakLimitThreshold(float threshold) {
    // 将阈值限制在有效范围内 (0.0-1.0)
    m_PeakLimitThreshold = std::max(0.0f, std::min(threshold, 1.0f));
    // std::cout << "峰值限制阈值设置为: " << m_PeakLimitThreshold << std::endl;
}

float AudioManager::GetPeakLimitThreshold() const {
    return m_PeakLimitThreshold;
}

void AudioManager::EnableNormalization(bool enable) {
    m_UseNormalization = enable;
    // std::cout << "音量标准化 " << (enable ? "已启用" : "已禁用") << std::endl;

    // 如果启用，则对所有当前播放的声音应用标准化
    if (enable) {
        NormalizeAllSounds();
    }
}

void AudioManager::EnablePeakLimiter(bool enable) {
    m_UsePeakLimiter = enable;
    // std::cout << "峰值限制器 " << (enable ? "已启用" : "已禁用") << std::endl;
}

bool AudioManager::IsNormalizationEnabled() const {
    return m_UseNormalization;
}

bool AudioManager::IsPeakLimiterEnabled() const {
    return m_UsePeakLimiter;
}

// 通过检查音频数据分析声音音量（峰值检测）
void AudioManager::AnalyzeSoundVolume(const std::string& name) {
    auto it = sfxSounds.find(name);
    if (it == sfxSounds.end() || !it->second) {
        // std::cout << "无法分析声音: " << name << " - 声音未加载" << std::endl;
        return;
    }

    FMOD::Sound* sound = it->second;

    // 获取声音信息
    FMOD_SOUND_TYPE soundType;
    FMOD_SOUND_FORMAT soundFormat;
    int channels;
    int bits;
    float frequency;

    sound->getFormat(&soundType, &soundFormat, &channels, &bits);
    sound->getDefaults(&frequency, nullptr);

    // 获取声音长度
    unsigned int length = 0;
    sound->getLength(&length, FMOD_TIMEUNIT_PCM);

    // 获取声音数据进行分析
    void* data = nullptr;
    unsigned int dataLength = 0;

    // 锁定声音以访问其数据
    FMOD_RESULT result = sound->lock(0, length, &data, nullptr, &dataLength, nullptr);

    if (result != FMOD_OK) {
        // std::cout << "无法锁定声音数据进行分析: " << name << ", 错误: " << result << std::endl;
        return;
    }

    // 分析数据以找到峰值振幅
    float maxAmplitude = 0.0f;

    // 根据格式处理
    if (soundFormat == FMOD_SOUND_FORMAT_PCM16) {
        // 16位PCM
        short* samples = static_cast<short*>(data);
        unsigned int numSamples = dataLength / sizeof(short);

        for (unsigned int i = 0; i < numSamples; i++) {
            float amplitude = std::abs(static_cast<float>(samples[i]) / 32768.0f); // 标准化到 0.0-1.0
            maxAmplitude = std::max(maxAmplitude, amplitude);
        }
    }
    else if (soundFormat == FMOD_SOUND_FORMAT_PCM8) {
        // 8位PCM
        unsigned char* samples = static_cast<unsigned char*>(data);
        unsigned int numSamples = dataLength;

        for (unsigned int i = 0; i < numSamples; i++) {
            float amplitude = std::abs((static_cast<float>(samples[i]) - 128.0f) / 128.0f); // 标准化到 0.0-1.0
            maxAmplitude = std::max(maxAmplitude, amplitude);
        }
    }
    else if (soundFormat == FMOD_SOUND_FORMAT_PCMFLOAT) {
        // 32位浮点PCM
        float* samples = static_cast<float*>(data);
        unsigned int numSamples = dataLength / sizeof(float);

        for (unsigned int i = 0; i < numSamples; i++) {
            float amplitude = std::abs(samples[i]);
            maxAmplitude = std::max(maxAmplitude, amplitude);
        }
    }

    // 解锁声音
    sound->unlock(data, nullptr, dataLength, 0);

    // 确定标准化因子（目标为最大音量的0.75）
    float targetLevel = 0.75f;
    float normFactor = 1.0f;

    if (maxAmplitude > 0.0f) {
        normFactor = targetLevel / maxAmplitude;
    }

    // 存储此声音的标准化级别
    m_SFXNormalizationLevels[name] = normFactor;

    // std::cout << "声音 '" << name << "' 分析结果: 最大振幅 = " << maxAmplitude << ", 标准化因子 = " << normFactor << std::endl;
}

// 标准化所有当前加载的声音
void AudioManager::NormalizeAllSounds() {
    for (const auto& pair : sfxSounds) {
        AnalyzeSoundVolume(pair.first);
    }

    // std::cout << "所有声音已被分析并设置了标准化级别" << std::endl;
}

// 对音量值应用峰值限制器
float AudioManager::ApplyPeakLimiter(float volume) const {
    if (!m_UsePeakLimiter || volume <= m_PeakLimitThreshold) {
        return volume; // 无需限制
    }

    // 应用软膝限制以减少失真
    float excess = volume - m_PeakLimitThreshold;
    float compressionRatio = 4.0f; // 更高的值 = 更硬的限制
    float compressed = m_PeakLimitThreshold + (excess / compressionRatio);

    return compressed;
}

// 获取直到当前关卡的所有武器音效名称
std::vector<std::string> AudioManager::GetWeaponSoundsUpToLevel(int level) {
    std::vector<std::string> soundNames;

    // 关卡从0开始，确保不超出范围
    int maxLevel = std::min(level, 4);  // 最多5个关卡(0-4)

    // 添加所有直到当前关卡的武器音效
    if (maxLevel >= 0) soundNames.push_back("gunshot");  // 第一关
    if (maxLevel >= 1) soundNames.push_back("buqiang");  // 第二关
    if (maxLevel >= 2) soundNames.push_back("penzi");    // 第三关
    if (maxLevel >= 3) soundNames.push_back("heidong");  // 第四关
    if (maxLevel >= 4) soundNames.push_back("zheshe");   // 第五关

    return soundNames;
}

// 播放指定关卡的武器音效序列
void AudioManager::PlayWeaponSoundsSequence(int level, bool useThreadForPause) {
    // 获取到当前关卡的所有武器音效
    std::vector<std::string> soundsToPlay = GetWeaponSoundsUpToLevel(level);

    if (soundsToPlay.empty()) {
        // std::cout << "没有可播放的武器音效" << std::endl;
        return;
    }

    // 如果不是使用线程播放（适用于连续开火的情况），则只播放最后一个音效
    if (!useThreadForPause) {
        PlaySFX(soundsToPlay.back());
        // std::cout << "播放武器音效: " << soundsToPlay.back() << std::endl;
        return;
    }

    // 使用后台线程播放音效序列，以便不阻塞主线程
    std::thread([this, soundsToPlay, level]() {
        for (size_t i = 0; i < soundsToPlay.size(); i++) {
            // 播放当前音效
            PlaySFX(soundsToPlay[i]);
            // std::cout << "播放武器音效序列 " << i + 1 << "/" << soundsToPlay.size() << ": " << soundsToPlay[i] << std::endl;

            // 如果不是序列中的最后一个音效，则等待指定的暂停时间
            if (i < soundsToPlay.size() - 1 && i < m_WeaponSoundPauseTimes.size()) {
                int pauseTime = m_WeaponSoundPauseTimes[i];
                if (pauseTime > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(pauseTime));
                }
            }
        }
        }).detach();  // 分离线程，让它独立运行
}

void AudioManager::PlayWeaponSoundsMix(int maxLevel, int soundCount) {
    // 获取所有可用声音
    std::vector<std::string> availableSounds = GetWeaponSoundsUpToLevel(maxLevel);

    if (availableSounds.empty()) return;

    // 将数量限制为可用声音数
    soundCount = std::min(soundCount, static_cast<int>(availableSounds.size()));

    // 始终包含当前最高级别的声音
    PlaySFX(availableSounds.back());

    // 准备一个除最高级别外的其他声音列表
    std::vector<std::string> otherSounds(availableSounds.begin(), availableSounds.end() - 1);

    // 创建随机数生成器
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(otherSounds.begin(), otherSounds.end(), g);

    // 播放额外的随机声音
    for (int i = 0; i < std::min(soundCount - 1, static_cast<int>(otherSounds.size())); i++) {
        PlaySFX(otherSounds[i]);
    }

    // std::cout << "播放混合武器声音：来自级别 " << maxLevel << " 的 " << soundCount << " 个声音" << std::endl;
}