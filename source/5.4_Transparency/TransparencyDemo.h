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
}

namespace Rendering
{
	class TransparencyMaterial;

	class TransparencyDemo final : public Library::DrawableGameComponent
	{
	public:
		TransparencyDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		TransparencyDemo(const TransparencyDemo&) = delete;
		TransparencyDemo(TransparencyDemo&&) = default;
		TransparencyDemo& operator=(const TransparencyDemo&) = default;		
		TransparencyDemo& operator=(TransparencyDemo&&) = default;
		~TransparencyDemo();

		float AmbientLightIntensity() const;
		void SetAmbientLightIntensity(float intensity);
		
		float DirectionalLightIntensity() const;
		void SetDirectionalLightIntensity(float intensity);

		const DirectX::XMFLOAT3& LightDirection() const;
		void RotateDirectionalLight(DirectX::XMFLOAT2 amount);

		float SpecularIntensity() const;
		void SetSpecularIntensity(float intensity);

		float SpecularPower() const;
		void SetSpecularPower(float power);
		
		float FogStart() const;
		void SetFogStart(float fogStart);

		float FogRange() const;
		void SetFogRange(float fogRange);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		inline static const float RotationRate{ DirectX::XM_PI };

		std::shared_ptr<TransparencyMaterial> mMaterial;
		DirectX::XMFLOAT4X4 mWorldMatrix{ Library::MatrixHelper::Identity };
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		std::uint32_t mVertexCount{ 0 };
		Library::DirectionalLight mDirectionalLight;
		std::unique_ptr<Library::ProxyModel> mProxyModel;
		bool mUpdateMaterial{ true };
	};
}