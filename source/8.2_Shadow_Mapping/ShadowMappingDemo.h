#pragma once

#include <gsl\gsl>
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include <DirectXTK\SpriteBatch.h>
#include "DrawableGameComponent.h"
#include "MatrixHelper.h"
#include "PointLight.h"
#include "DepthMap.h"
#include "Rectangle.h"
#include "ShadowMappingMaterial.h"
#include "RenderStateHelper.h"

namespace Library
{
	class ProxyModel;
	class RenderableFrustum;
	class DepthMapMaterial;
}

namespace Rendering
{
	class ShadowMappingDemo final : public Library::DrawableGameComponent
	{
	public:
		ShadowMappingDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		ShadowMappingDemo(const ShadowMappingDemo&) = delete;
		ShadowMappingDemo(ShadowMappingDemo&&) = default;
		ShadowMappingDemo& operator=(const ShadowMappingDemo&) = default;		
		ShadowMappingDemo& operator=(ShadowMappingDemo&&) = default;
		~ShadowMappingDemo();

		ShadowMappingDrawModes DrawMode() const;
		const std::string& DrawModeString() const;
		void SetDrawMode(ShadowMappingDrawModes drawMode);

		float DepthBias() const;
		void SetDepthBias(float bias);

		float SlopeScaledDepthBias() const;
		void SetSlopeScaledDepthBias(float bias);

		float AmbientLightIntensity() const;
		void SetAmbientLightIntensity(float intensity);

		float PointLightIntensity() const;
		void SetPointLightIntensity(float intensity);

		float PointLightRadius() const;
		void SetPointLightRadius(float radius);

		const Library::Camera& Projector() const;
		const DirectX::XMFLOAT3& ProjectorPosition() const;
		const DirectX::XMVECTOR ProjectorPositionVector() const;
		void SetProjectorPosition(const DirectX::XMFLOAT3& position);
		void SetProjectorPosition(DirectX::FXMVECTOR position);
		const DirectX::XMFLOAT3& ProjectorDirection() const;		
		void RotateProjector(const DirectX::XMFLOAT2& amount);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		inline static const std::uint32_t ShadowMapWidth{ 1024 };
		inline static const std::uint32_t ShadowMapHeight{ 1024 };
		static inline const RECT ShadowMapDestinationRectangle{ 0, 512, 256, 768 };
	
		inline static DirectX::XMFLOAT4X4 mProjectedTextureScalingMatrix
		{
			0.5f,  0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f,  0.0f, 1.0f, 0.0f,
			0.5f,  0.5f, 0.0f, 1.0f
		};

		void UpdateTransforms(ShadowMappingMaterial::VertexCBufferPerObject& transforms, DirectX::FXMMATRIX worldViewProjectionMatrix, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX projectiveTextureMatrix);
		void UpdateDepthBiasRasterizerState();

		ShadowMappingMaterial::VertexCBufferPerObject mPlaneTransforms;
		ShadowMappingMaterial::VertexCBufferPerObject mTeapotTransforms;
		DirectX::XMFLOAT4X4 mPlaneWorldMatrix{ Library::MatrixHelper::Identity };
		DirectX::XMFLOAT4X4 mTeapotWorldMatrix{ Library::MatrixHelper::Identity };
		
		Library::PointLight mPointLight;
		Library::DepthMap mShadowMap;
		Library::RenderStateHelper mRenderStateHelper;
		std::shared_ptr<ShadowMappingMaterial> mMaterial;
		std::shared_ptr<Library::DepthMapMaterial> mDepthMapMaterial;
		winrt::com_ptr<ID3D11Buffer> mPlaneVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mTeapotVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mTeapotPositionOnlyVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mTeapotIndexBuffer;
		std::uint32_t mPlaneVertexCount{ 0 };		
		std::uint32_t mTeapotIndexCount{ 0 };
		std::unique_ptr<Library::ProxyModel> mProxyModel;
		std::unique_ptr<Library::Camera> mProjector;
		std::unique_ptr<Library::RenderableFrustum> mRenderableProjectorFrustum;
		std::unique_ptr<DirectX::SpriteBatch> mSpriteBatch;
		bool mUpdateMaterial{ true };
		float mDepthBias{ 0.0f };
		float mSlopeScaledDepthBias{ 2.0f };
		winrt::com_ptr<ID3D11RasterizerState> mDepthBiasState;
	};
}