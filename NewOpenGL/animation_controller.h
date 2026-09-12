#pragma once
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include "animator.h"

namespace OpenGL {

    class AnimationController {
    public:
        AnimationController(Animation* animation) : m_CurrentAnimatorIndex(0), m_CurrentSpeed(1.0f) {
            int id = 1; 
            for (const auto& pair : animation->animations) {
                animators[id] = new Animator(pair.second);
                id++;
            }
        }

        ~AnimationController() {
            for (auto& [id, animator] : animators) {
                delete animator;
            }
            animators.clear();
        }

        // 切换动画轨道
        void SwitchAnimation(int trackID) {
            if (animators.find(trackID) == animators.end()) {
                std::cerr << "Error: Animation track " << trackID << " does not exist!\n";
                return;
            }
            m_CurrentAnimatorIndex = trackID;
            animators[m_CurrentAnimatorIndex]->ResetTime();
        }

        // 播放当前选定轨道的动画
        void PlayAnimation() {
            if (m_CurrentAnimatorIndex > 0) {
                animators[m_CurrentAnimatorIndex]->SetLooping(true);
            }
        }

        // 停止当前动画
        void StopAnimation() {
            if (m_CurrentAnimatorIndex > 0) {
                animators[m_CurrentAnimatorIndex]->SetLooping(false);
            }
        }

        // 设置动画速度
        void SetAnimationSpeed(float speed) {
            m_CurrentSpeed = speed;
        }

        // 更新动画
        void Update(float deltaTime) {
            if (m_CurrentAnimatorIndex > 0) {
                animators[m_CurrentAnimatorIndex]->UpdateAnimation(deltaTime * m_CurrentSpeed);
            }
        }

        // 获取当前动画的最终骨骼矩阵
        std::vector<glm::mat4> GetFinalBoneMatrices() {
            if (m_CurrentAnimatorIndex > 0) {
                return animators[m_CurrentAnimatorIndex]->GetFinalBoneMatrices();
            }
            return {};
        }

        // 查询当前动画是否播放完毕
        bool IsAnimationFinished() {
            if (m_CurrentAnimatorIndex > 0) {
                return animators[m_CurrentAnimatorIndex]->IsAnimationFinished();
            }
            return true;
        }

        // 获取所有可用的动画轨道 ID
        std::vector<int> GetAvailableTracks() const {
            std::vector<int> trackIDs;
            for (const auto& [id, _] : animators) {
                trackIDs.push_back(id);
            }
            return trackIDs;
        }

        // 获取动画轨道ID -> 该轨道的动画时长的映射表
        std::map<int, float> GetAnimatorDurations() const {
            std::map<int, float> durations;
            for (const auto& [id, animator] : animators) {
                durations[id] = animator->GetAnimationDuration();
            }
            return durations;
        }

        const std::vector<glm::mat4>& GetPrecomputedMatrices(int trackID) const {
            if (animators.find(trackID) != animators.end()) {
                return animators.at(trackID)->GetBoneMatricesAtFrame();
            }
            static std::vector<glm::mat4> empty(200, glm::mat4(1.0f));
            return empty;
        }


    private:
        std::map<int, Animator*> animators; // 轨道 ID -> Animator 对象
        int m_CurrentAnimatorIndex; // 当前播放的动画轨道
        float m_CurrentSpeed; // 动画播放速度
    };

}
