#pragma once

#include <gsl\gsl>
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include <array>
#include "DrawableGameComponent.h"
#include "MatrixHelper.h"
#include "PointLight.h"

namespace Library
{
	class PointLight;
	class ProxyModel;
}

namespace Rendering
{
	class MultiplePointLightsMaterial;

	class MultiplePointLightsDemo final : public Library::DrawableGameComponent
	{
	public:
		MultiplePointLightsDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		MultiplePointLightsDemo(const MultiplePointLightsDemo&) = delete;
		MultiplePointLightsDemo(MultiplePointLightsDemo&&) = default;
		MultiplePointLightsDemo& operator=(const MultiplePointLightsDemo&) = default;		
		MultiplePointLightsDemo& operator=(MultiplePointLightsDemo&&) = default;
		~MultiplePointLightsDemo();

		bool AnimationEnabled() const;
		void SetAnimationEnabled(bool enabled);
		void ToggleAnimation();

		float AmbientLightIntensity() const;
		void SetAmbientLightIntensity(float intensity);

		const std::array<Library::PointLight, 4>& PointLights() const;
		void SetPointLight(const Library::PointLight& light, size_t index);
				
		const size_t SelectedLightIndex() const;
		const Library::PointLight& SelectedLight() const;
		void UpdateSelectedLight(const Library::PointLight& light);
		void SelectLight(size_t index);

		float SpecularPower() const;
		void SetSpecularPower(float power);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		inline static const float RotationRate{ DirectX::XM_PI };

		std::shared_ptr<MultiplePointLightsMaterial> mMaterial;
		DirectX::XMFLOAT4X4 mWorldMatrix{ Library::MatrixHelper::Identity };
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mIndexBuffer;
		std::uint32_t mIndexCount{ 0 };
		std::array<std::unique_ptr<Library::ProxyModel>, 4> mProxyModels;
		size_t mSelectedLightIndex{ 0 };
		float mModelRotationAngle{ 0.0f };
		bool mAnimationEnabled{ true };
		bool mUpdateMaterial{ true };
	};
}