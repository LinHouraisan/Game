#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include "animation.h"
#include "bone.h"
#include <fstream>
namespace OpenGL {

    class Animator
    {
    public:
        Animator(Animation* animation)
            : m_CurrentAnimation(animation), m_CurrentTime(0.0f), m_DeltaTime(0.0f), m_Looping(true)
        {
            m_FinalBoneMatrices.reserve(100);
            for (int i = 0; i < 100; i++)
                m_FinalBoneMatrices.push_back(glm::mat4(1.0f));

            if (m_CurrentAnimation)
                m_AnimationDuration = m_CurrentAnimation->GetDuration(); // 存储动画时长
            else
                m_AnimationDuration = 0.0f;

            animatorName = animation->thisName;
            PrecomputeBoneMatrices(); 
        }

        // 预计算所有关键帧对应的骨骼变换矩阵
        void PrecomputeBoneMatrices()
        {
            if (!m_CurrentAnimation)
                return;

            // 清空之前的预计算数据
            precomputedBoneMatrices.clear();

            // 这里以每秒60帧为采样频率
            float dtSeconds = 1.0f / 60.0f;
            // 注意：m_CurrentTime 的单位是动画的 tick，
            // 因为在 UpdateAnimation 中是这么计算的：
            // m_CurrentTime += GetTicksPerSecond() * dt
            // 所以采样时刻应为：
            float dtTicks = m_CurrentAnimation->GetTicksPerSecond() * dtSeconds;

            // 根据动画总时长计算采样帧数（多加1确保覆盖整个动画）
            int numSamples = static_cast<int>(std::ceil(m_AnimationDuration / dtTicks)) + 1;
            precomputedBoneMatrices.reserve(numSamples);

            // 保存当前的 m_CurrentTime，以免影响正常播放
            float originalTime = m_CurrentTime;

            // 遍历整个动画时长，逐帧计算骨骼矩阵
            for (int i = 0; i < numSamples-1; ++i)
            {
                m_CurrentTime = i * dtTicks;
                // 防止超过动画总时长
                if (m_CurrentTime > m_AnimationDuration)
                    m_CurrentTime = m_AnimationDuration;

                // 计算当前时刻的骨骼变换矩阵（从根节点开始）
                CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));

                // 将计算好的骨骼矩阵集合存入预计算数据中
                precomputedBoneMatrices.push_back(m_FinalBoneMatrices);
            }

            // 插帧或者填充，保持120帧
            AlignPrecomputedBoneMatrices();
            // 恢复原来的时间状态
            m_CurrentTime = originalTime;

        }

        void AlignPrecomputedBoneMatrices()
        {
            constexpr int targetFrames = 120;
            int currentFrames = static_cast<int>(precomputedBoneMatrices.size());

            // 处理超出 120 帧的情况：均匀采样 120 帧
            if (currentFrames > targetFrames)
            {
                std::vector<std::vector<glm::mat4>> alignedMatrices;
                alignedMatrices.reserve(targetFrames);

                float step = static_cast<float>(currentFrames - 1) / (targetFrames - 1);
                for (int i = 0; i < targetFrames; ++i)
                {
                    int index = static_cast<int>(std::round(i * step));
                    alignedMatrices.push_back(precomputedBoneMatrices[index]);
                }

                precomputedBoneMatrices = std::move(alignedMatrices);
            }
            // 处理不足 120 帧的情况：均匀填充
            else if (currentFrames < targetFrames)
            {
                std::vector<std::vector<glm::mat4>> alignedMatrices = precomputedBoneMatrices;
                int missingFrames = targetFrames - currentFrames;

                for (int i = 0; i < missingFrames; ++i)
                {
                    int insertIndex = (i * currentFrames) / missingFrames; // 均匀选择填充点
                    alignedMatrices.push_back(precomputedBoneMatrices[insertIndex]);
                }

                precomputedBoneMatrices = std::move(alignedMatrices);
            }
        }

        const std::vector<glm::mat4>& GetBoneMatricesAtFrame() const
        {
            if (!m_CurrentAnimation || precomputedBoneMatrices.empty())
            {
                static std::vector<glm::mat4> defaultMatrices(100, glm::mat4(1.0f));
                return defaultMatrices;
            }

            // 取当前帧数据
            const std::vector<glm::mat4>& currentMatrices = precomputedBoneMatrices[m_CurrentFrameIndex];

            // 更新当前帧索引，确保循环播放
            m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % precomputedBoneMatrices.size();

            return currentMatrices;
        }

        void UpdateAnimation(float dt)
        {
            m_DeltaTime = dt;
            if (m_CurrentAnimation)
            {
                m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
                if (m_Looping)
                    m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
                else
                    m_CurrentTime = std::min(m_CurrentTime, m_CurrentAnimation->GetDuration());
                CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
            }
        }

        // 设置当前动画（切换动画时调用），并重置播放时间
        void PlayAnimation(Animation* pAnimation, bool reset = true)
        {
            m_CurrentAnimation = pAnimation;
            if (reset)
                ResetTime();
        }

        void ResetTime() { m_CurrentTime = 0.0f; }

        void SetLooping(bool loop) { m_Looping = loop; }

        // 查询非循环动画是否播放完毕
        bool IsAnimationFinished() const {
            return (!m_Looping && m_CurrentAnimation && m_CurrentTime >= m_CurrentAnimation->GetDuration());
        }

        std::vector<glm::mat4> GetFinalBoneMatrices()
        {
            return m_FinalBoneMatrices;
        }


        void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
        {
            std::string nodeName = node->name;
            glm::mat4 nodeTransform = node->transformation;

            Bone* bone = m_CurrentAnimation->FindBone(nodeName);
            if (bone)
            {
                bone->Update(m_CurrentTime);    // 从关键帧插值得到骨骼局部矩阵
                nodeTransform = bone->GetLocalTransform();
            }

            glm::mat4 globalTransformation = parentTransform * nodeTransform;

            auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
            if (boneInfoMap.find(nodeName) != boneInfoMap.end())
            {
                int index = boneInfoMap[nodeName].id;
                glm::mat4 offset = boneInfoMap[nodeName].offset;

                // ********** 在这里插入全局逆矩阵 **********
                glm::mat4 globalInverse = m_CurrentAnimation->GetGlobalInverseTransform();
                m_FinalBoneMatrices[index] = globalInverse * globalTransformation * offset;
            }

            for (int i = 0; i < node->childrenCount; i++)
            {
                CalculateBoneTransform(&node->children[i], globalTransformation);
            }
        }

        float GetAnimationDuration() const {
            return m_AnimationDuration;
        }

        void WriteBoneMatricesToFile(const std::vector<std::vector<glm::mat4>>& precomputedBoneMatrices, const std::string& filename) {
            std::ofstream file(filename);
            if (!file) {
                std::cerr << "Failed to open file for writing: " << filename << std::endl;
                return;
            }

            for (const auto& boneFrame : precomputedBoneMatrices) {
                for (const auto& mat : boneFrame) {
                    const float* ptr = glm::value_ptr(mat);
                    for (int i = 0; i < 16; ++i) {
                        file << ptr[i] << " ";
                    }
                    file << "\n"; // 每个矩阵换行
                }
                file << "\n"; // 额外换行区分帧
            }

            file.close();
            // std::cout << "Bone matrices successfully written to " << filename << std::endl;
        }

    private:
        std::vector<std::vector<glm::mat4>> precomputedBoneMatrices;

        std::string animatorName;
        float m_TicksPerSecond;
        mutable int m_CurrentFrameIndex = 0;
        std::vector<glm::mat4> m_FinalBoneMatrices;
        Animation* m_CurrentAnimation;
        float m_CurrentTime;
        float m_DeltaTime;
        bool m_Looping;
        float m_AnimationDuration; 
    };
}