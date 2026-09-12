#pragma once


#include "GameTechRenderer.h"
#include "ResourceManager.h"
#include "Transform.h"

using namespace OpenGL;

namespace NCL {
	namespace CSC8503 {
		class Transform;

		class RenderObject
		{
		public:
			RenderObject(Transform* parentTransform, Model* model, AnimationController* animationController);
			RenderObject(Transform* parentTransform, Model* model);
			~RenderObject();

			bool GetIsActive() const {
				return isActive;
			}

			void SetIsActive(bool active) {
				isActive = active;
			}

			Transform* GetTransform() const {
				return transform;
			}

			Model* GetModel() const {
				return model;
			}

			AnimationController* GetAnimationController() const {
				return animationController;
			}

			bool GetIsRepeat() const {
				return isRepeat;
			}

			void SetIsRepeat(bool repeat) {
				isRepeat = repeat;
			}

			Vector4 GetColorFactor() const {
				return colorFactor;
			}

			void SetColorFactor(const Vector4& color) {
				colorFactor = color;
			}

			float GetWhiteTime() const {
				return whiteTime;
			}

			void SetWhiteTime(float time) {
				whiteTime = time;
			}

			int GetAnimationIndex() const {
				return animationIndex;
			}

			void SetAnimationIndex(int index) {
				animationIndex = index;
			}

			int GetOffsetFrame() const {
				return offsetFrame;
			}

			void SetOffsetFrame(int frame) {
				offsetFrame = frame;
			}

			int GetOwnFrame() const {
				return frame;
			}

			int GetAnimationSpeed() const {
				return animationSpeed;
			}

			void SetAnimationSpeed(int speed) {
				animationSpeed = speed;
			}

			int GetWorldID() const {
				return worldID;
			}

			void SetWorldID(int id) {
				worldID = id;
			}


			void UpdateFrame();

			void SetIsTransitioning(bool transitioning) {
				isTransitioning = transitioning;
			} // 添加新的 Setter

			bool GetIsTransitioning() const {
				return isTransitioning;
			}// 添加新的 Getter

	protected:
			int worldID = -1;
			bool isActive = true;

			Transform* transform;
			Model* model;
			AnimationController* animationController;

			bool isRepeat = true;
			Vector4 colorFactor = Vector4(1, 1, 1, 1);
			float whiteTime = 0.0f;
			int animationIndex = 1;
			int offsetFrame = 0;
			int animationSpeed = 1;
			int frame = 0;

			bool isTransitioning = false; 
			bool isTransitioning1 = false;// 是否正在切换动画（防止重复播放受伤动画）
		};
	}
}
