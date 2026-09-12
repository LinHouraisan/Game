#pragma once
#include <GLFW/glfw3.h>
#include <Effekseer/Effekseer.h>
#include <EffekseerRendererGL/EffekseerRendererGL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class EffekseerManager {
public:
    // 获取单例
    static EffekseerManager* GetInstance() {
        static EffekseerManager instance;
        return &instance;
    }

    // 初始化（OpenGL 上下文必须已创建）
    void Initialize();

    // 释放资源
    void Shutdown();

    // 加载特效，指定特效缩放
    Effekseer::EffectRef LoadEffect(const std::u16string& filepath, float scale);

    // 播放特效
    Effekseer::Handle PlayEffect(Effekseer::EffectRef effect, float x, float y, float z);
    Effekseer::Handle PlayEffect(Effekseer::EffectRef effect, float x, float y, float z, float rotX, float rotY, float rotZ);

    // 停止播放特效
    void StopEffect(Effekseer::Handle handle);

    void SetEffectScale(Effekseer::Handle handle, float scaleX, float scaleY, float scaleZ);
    void SetEffectRotation(Effekseer::Handle handle, float rotX, float rotY, float rotZ);

    EffekseerRendererGL::RendererRef GetRenderer() { return renderer; }

    void RenderEffFrame(const glm::mat4 view, const glm::mat4 projection);


    void Update();

    //新增特效跟随
    void SetEffectPosition(Effekseer::Handle handle, float x, float y, float z);
    bool IsEffectPlaying(Effekseer::Handle handle);

private:
    // 构造 & 析构
    EffekseerManager() = default;
    ~EffekseerManager() = default;

    // 禁止拷贝构造 & 赋值
    EffekseerManager(const EffekseerManager&) = delete;
    EffekseerManager& operator=(const EffekseerManager&) = delete;

    void Render();

    Effekseer::Matrix44 ConvertToEffekseerMatrix(const glm::mat4& glmMat);


    // Effekseer 组件
    Effekseer::ManagerRef manager;
    EffekseerRendererGL::RendererRef renderer;
};
