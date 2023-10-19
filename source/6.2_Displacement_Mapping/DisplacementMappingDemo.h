#pragma once

#include <gsl\gsl>
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include "DrawableGameComponent.h"
#include "MatrixHelper.h"
#include "DirectionalLight.h"

namespace Library
{
	class Texture2D;
	class ProxyModel;
}

namespace Rendering
{
	class DisplacementMappingMaterial;

	class DisplacementMappingDemo final : public Library::DrawableGameComponent
	{
	public:
		DisplacementMappingDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		DisplacementMappingDemo(const DisplacementMappingDemo&) = delete;
		DisplacementMappingDemo(DisplacementMappingDemo&&) = default;
		DisplacementMappingDemo& operator=(const DisplacementMappingDemo&) = default;		
		DisplacementMappingDemo& operator=(DisplacementMappingDemo&&) = default;
		~DisplacementMappingDemo();

		bool RealDisplacementMapEnabled() const;
		void SetRealDisplacementMapEnabled(bool enabled);
		void ToggleRealDisplacementMap();

		float AmbientLightIntensity() const;
		void SetAmbientLightIntensity(float intensity);

		float DirectionalLightIntensity() const;
		void SetDirectionalLightIntensity(float intensity);

		const DirectX::XMFLOAT3& LightDirection() const;
		void RotateDirectionalLight(DirectX::XMFLOAT2 amount);

		const float DisplacementScale() const;
		void SetDisplacementScale(float displacementScale);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		std::shared_ptr<DisplacementMappingMaterial> mMaterial;
		DirectX::XMFLOAT4X4 mWorldMatrix{ Library::MatrixHelper::Identity };
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mIndexBuffer;
		std::uint32_t mIndexCount{ 0 };
		Library::DirectionalLight mDirectionalLight;
		std::unique_ptr<Library::ProxyModel> mProxyModel;
		std::shared_ptr<Library::Texture2D> mRealDisplacementMap;
		std::shared_ptr<Library::Texture2D> mDefaultDisplacementMap;
		bool mUpdateMaterial{ true };
		bool mRealDisplacementMapEnabled{ true };
	};
}