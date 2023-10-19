#pragma once

#include <gsl\gsl>
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include "DrawableGameComponent.h"
#include "MatrixHelper.h"
#include "PointLight.h"
#include "DepthMap.h"
#include "Rectangle.h"
#include "ProjectiveTextureMappingMaterial.h"
#include "RenderStateHelper.h"

namespace Library
{
	class ProxyModel;
	class RenderableFrustum;
	class DepthMapMaterial;
}

namespace Rendering
{
	class ProjectiveTextureMappingDemo final : public Library::DrawableGameComponent
	{
	public:
		ProjectiveTextureMappingDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		ProjectiveTextureMappingDemo(const ProjectiveTextureMappingDemo&) = delete;
		ProjectiveTextureMappingDemo(ProjectiveTextureMappingDemo&&) = default;
		ProjectiveTextureMappingDemo& operator=(const ProjectiveTextureMappingDemo&) = default;		
		ProjectiveTextureMappingDemo& operator=(ProjectiveTextureMappingDemo&&) = default;
		~ProjectiveTextureMappingDemo();

		ProjectiveTextureMappingDrawModes DrawMode() const;
		const std::string& DrawModeString() const;
		void SetDrawMode(ProjectiveTextureMappingDrawModes drawMode);

		float AmbientLightIntensity() const;
		void SetAmbientLightIntensity(float intensity);

		float PointLightIntensity() const;
		void SetPointLightIntensity(float intensity);

		float PointLightRadius() const;
		void SetPointLightRadius(float radius);

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
		inline static const std::uint32_t DepthMapWidth{ 1024 };
		inline static const std::uint32_t DepthMapHeight{ 1024 };
		static inline const RECT DepthMapDestinationRectangle{ 0, 512, 256, 768 };
				
		void DrawWithDepthMap();
		void DrawWithoutDepthMap();		
		void UpdateTransforms(ProjectiveTextureMappingMaterial::VertexCBufferPerObject& transforms, DirectX::FXMMATRIX worldViewProjectionMatrix, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX projectiveTextureMatrix);
		void InitializeProjectedTextureScalingMatrix(uint32_t textureWidth, uint32_t textureHeight);

		ProjectiveTextureMappingMaterial::VertexCBufferPerObject mPlaneTransforms;
		ProjectiveTextureMappingMaterial::VertexCBufferPerObject mTeapotTransforms;
		DirectX::XMFLOAT4X4 mPlaneWorldMatrix{ Library::MatrixHelper::Identity };
		DirectX::XMFLOAT4X4 mTeapotWorldMatrix{ Library::MatrixHelper::Identity };
		DirectX::XMFLOAT4X4 mProjectedTextureScalingMatrix{ Library::MatrixHelper::Zero };
		Library::PointLight mPointLight;
		Library::DepthMap mDepthMap;
		Library::RenderStateHelper mRenderStateHelper;
		std::shared_ptr<ProjectiveTextureMappingMaterial> mMaterial;
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
	};
}