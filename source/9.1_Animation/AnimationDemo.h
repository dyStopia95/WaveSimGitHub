#pragma once

#include <gsl\gsl>
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include "DrawableGameComponent.h"
#include "MatrixHelper.h"
#include "DirectionalLight.h"

namespace Library
{
	class ProxyModel;
	class AnimationPlayer;
	class Model;
	class AnimationClip;
}

namespace Rendering
{
	class SkinnedModelMaterial;

	class AnimationDemo final : public Library::DrawableGameComponent
	{
	public:
		AnimationDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		AnimationDemo(const AnimationDemo&) = delete;
		AnimationDemo(AnimationDemo&&) = default;
		AnimationDemo& operator=(const AnimationDemo&) = default;		
		AnimationDemo& operator=(AnimationDemo&&) = default;
		~AnimationDemo();

		bool ManualAdvanceEnabled() const;
		void SetManualAdvanceEnabled(bool enabled);
		void ToggleManualAdvance();
		
		const std::unique_ptr<Library::AnimationPlayer>& AnimationPlayer() const;
		void TogglePause();		
		void RestartClip();

		void IncrementCurrentKeyframe();
		void DecrementCurrentKeyframe();		

		bool InterpolationEnabled() const;
		void SetInterpolationEnabled(bool enabled);
		void ToggleInterpolation();

		float AmbientLightIntensity() const;
		void SetAmbientLightIntensity(float intensity);

		float DirectionalLightIntensity() const;
		void SetDirectionalLightIntensity(float intensity);

		const DirectX::XMFLOAT3& LightDirection() const;
		void RotateDirectionalLight(DirectX::XMFLOAT2 amount);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		inline static const float RotationRate{ DirectX::XM_PI };

		std::shared_ptr<SkinnedModelMaterial> mMaterial;
		DirectX::XMFLOAT4X4 mWorldMatrix{ Library::MatrixHelper::Identity };
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mIndexBuffer;
		std::uint32_t mIndexCount{ 0 };
		Library::DirectionalLight mDirectionalLight;
		std::unique_ptr<Library::ProxyModel> mProxyModel;
		bool mUpdateMaterial{ true };
		bool mUpdateMaterialBoneTransforms{ true };
		std::unique_ptr<Library::AnimationPlayer> mAnimationPlayer;
		bool mManualAdvanceEnabled{ false };
		std::shared_ptr<Library::Model> mSkinnedModel;
	};
}