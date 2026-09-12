#include "Effeksser.h"
#include <iostream>

void EffekseerManager::Initialize() {
    // 直接使用当前 OpenGL 上下文（TutorialGame 运行时已经创建）
    GLFWwindow* currentContext = glfwGetCurrentContext();
    if (!currentContext) {
        std::cerr << "[Effekseer] Error: OpenGL Context is not available!" << std::endl;
        return;
    }

    // 创建 Effekseer 管理器
    manager = Effekseer::Manager::Create(8000);

    // 创建 Effekseer 的 OpenGL 渲染器（使用 OpenGL 3 设备）
    renderer = EffekseerRendererGL::Renderer::Create(8000, EffekseerRendererGL::OpenGLDeviceType::OpenGL3);

 

    if (!manager.Get() || !renderer.Get()) {
        std::cerr << "[Effekseer] Failed to create manager or renderer!" << std::endl;
        return;
    }

    // 绑定渲染器
    manager->SetSpriteRenderer(renderer->CreateSpriteRenderer());
    manager->SetRibbonRenderer(renderer->CreateRibbonRenderer());
    manager->SetRingRenderer(renderer->CreateRingRenderer());
    manager->SetModelRenderer(renderer->CreateModelRenderer());
    manager->SetTrackRenderer(renderer->CreateTrackRenderer());

    // 绑定 OpenGL 上下文
    renderer->SetRestorationOfStatesFlag(true);
}

void EffekseerManager::Shutdown() {
    if (manager.Get()) {
        manager.Reset();  // 使用 Reset() 而不是 Destroy()
    }
    if (renderer.Get()) {
        renderer.Reset();  // 使用 Reset() 而不是 Destroy()
    }
}

Effekseer::EffectRef EffekseerManager::LoadEffect(const std::u16string& filepath, float scale) {
    Effekseer::EffectRef effect = Effekseer::Effect::Create(manager, filepath.c_str(), scale);
    if (!effect.Get()) {
        // std::cerr << "[Effekseer] Failed to load effect: " << std::endl;
    }
    return effect;
}


Effekseer::Handle EffekseerManager::PlayEffect(Effekseer::EffectRef effect, float x, float y, float z) {
    if (!manager.Get()) {
        std::cerr << "[Effekseer] Error: Effekseer Manager is NULL!" << std::endl;
        return -1;
    }
    if (!effect.Get()) {
        std::cerr << "[Effekseer] Error: Effect is NULL!" << std::endl;
        return -1;
    }

    Effekseer::Handle handle = manager->Play(effect, x, y, z);

    return handle;
}


Effekseer::Handle EffekseerManager::PlayEffect(Effekseer::EffectRef effect, float x, float y, float z, float rotX, float rotY, float rotZ) 
{
    Effekseer::Handle handle = PlayEffect(effect, x, y, z);
    if (handle >= 0) {
        SetEffectRotation(handle, rotX, rotY, rotZ);
    }
    return handle;
}

void EffekseerManager::StopEffect(Effekseer::Handle handle) 
{
    if (!manager.Get()) {
        std::cerr << "[Effekseer] Error: Effekseer Manager is NULL!" << std::endl;
        return;
    }

    manager->StopEffect(handle);
}

void EffekseerManager::SetEffectScale(Effekseer::Handle handle, float scaleX, float scaleY, float scaleZ) 
{
    if (manager.Get()) 
    {
        manager->SetScale(handle, scaleX, scaleY, scaleZ);
    }
}

void EffekseerManager::SetEffectRotation(Effekseer::Handle handle, float rotX, float rotY, float rotZ) 
{
    if (!manager.Get()) return;

    manager->SetRotation(handle, rotX, rotY, rotZ);
}

void EffekseerManager::Update() {
    if (manager.Get()) {
        manager->Update();
    }
}


void EffekseerManager::RenderEffFrame(const glm::mat4 view, const glm::mat4 projection)
{
    Effekseer::Matrix44 effekseerView, effekseerProj;
    effekseerView = ConvertToEffekseerMatrix(view);
    effekseerProj = ConvertToEffekseerMatrix(projection);
    renderer->SetCameraMatrix(effekseerView);
    renderer->SetProjectionMatrix(effekseerProj);

    Render();
}

Effekseer::Matrix44 EffekseerManager::ConvertToEffekseerMatrix(const glm::mat4& glmMat)
{
    Effekseer::Matrix44 efkMat;
    const float* p = glm::value_ptr(glmMat);

    for (int i = 0; i < 16; i++)
    {
        efkMat.Values[i / 4][i % 4] = p[i];
    }

    return efkMat;
}


void EffekseerManager::Render() {
    if (!renderer.Get() || !manager.Get()) return;

    // 绑定 OpenGL 上下文（如果在多线程中运行 Effekseer，可能需要确保上下文正确）
    GLFWwindow* currentContext = glfwGetCurrentContext();
    if (!currentContext) {
        std::cerr << "[Effekseer] OpenGL context lost before rendering!" << std::endl;
        return;
    }

    renderer->BeginRendering();
    manager->Draw();
    renderer->EndRendering();
}

//新增特效跟随
void EffekseerManager::SetEffectPosition(Effekseer::Handle handle, float x, float y, float z) {
    if (manager.Get()) {
        manager->SetLocation(handle, x, y, z);
    }
}

bool EffekseerManager::IsEffectPlaying(Effekseer::Handle handle) {
    if (manager.Get()) {
        return manager->Exists(handle);
    }
    return false;
}

