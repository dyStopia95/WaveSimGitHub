#pragma once

#include <DirectXColors.h>
#include "Material.h"
#include "MatrixHelper.h"
#include "SamplerStates.h"

namespace Library
{
	class Texture2D;
}

namespace Rendering
{
	class DisplacementMappingMaterial : public Library::Material
	{
		RTTI_DECLARATIONS(DisplacementMappingMaterial, Library::Material)

	public:
		DisplacementMappingMaterial(Library::Game& game, std::shared_ptr<Library::Texture2D> colorMap, std::shared_ptr<Library::Texture2D> displacementMap);
		DisplacementMappingMaterial(const DisplacementMappingMaterial&) = default;
		DisplacementMappingMaterial& operator=(const DisplacementMappingMaterial&) = default;
		DisplacementMappingMaterial(DisplacementMappingMaterial&&) = default;
		DisplacementMappingMaterial& operator=(DisplacementMappingMaterial&&) = default;
		virtual ~DisplacementMappingMaterial() = default;

		winrt::com_ptr<ID3D11SamplerState> SamplerState() const;
		void SetSamplerState(winrt::com_ptr<ID3D11SamplerState> samplerState);

		std::shared_ptr<Library::Texture2D> ColorMap() const;
		void SetColorMap(std::shared_ptr<Library::Texture2D> texture);

		std::shared_ptr<Library::Texture2D> DisplacementMap() const;
		void SetDisplacementMap(std::shared_ptr<Library::Texture2D> texture);

		const DirectX::XMFLOAT4& AmbientColor() const;
		void SetAmbientColor(const DirectX::XMFLOAT4& color);

		const DirectX::XMFLOAT3& LightDirection() const;
		void SetLightDirection(const DirectX::XMFLOAT3& direction);

		const DirectX::XMFLOAT4& LightColor() const;
		void SetLightColor(const DirectX::XMFLOAT4& color);

		const float DisplacementScale() const;
		void SetDisplacementScale(float displacementScale);

		virtual std::uint32_t VertexSize() const override;
		virtual void Initialize() override;

		void UpdateTransforms(DirectX::FXMMATRIX worldViewProjectionMatrix, DirectX::CXMMATRIX worldMatrix);
		
	private:
		struct VertexCBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection{ Library::MatrixHelper::Identity };
			DirectX::XMFLOAT4X4 World{ Library::MatrixHelper::Identity };
			float DisplacementScale{ 0.1f };
			float Padding[3];
		};

		struct PixelCBufferPerFrame
		{
			DirectX::XMFLOAT4 AmbientColor{ DirectX::Colors::Black };
			DirectX::XMFLOAT3 LightDirection{ 0.0f, 0.0f, 1.0f };
			float Padding;
			DirectX::XMFLOAT4 LightColor{ DirectX::Colors::White };
		};

		virtual void BeginDraw() override;

		void ResetPixelShaderResources();

		winrt::com_ptr<ID3D11Buffer> mVertexCBufferPerObject;
		winrt::com_ptr<ID3D11Buffer> mPixelCBufferPerFrame;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		PixelCBufferPerFrame mPixelCBufferPerFrameData;		
		std::shared_ptr<Library::Texture2D> mColorMap;
		std::shared_ptr<Library::Texture2D> mDisplacementMap;
		winrt::com_ptr<ID3D11SamplerState> mSamplerState{ Library::SamplerStates::TrilinearClamp };
		bool mVertexCBufferPerObjectDataDirty{ true };
		bool mPixelCBufferPerFrameDataDirty{ true };
	};
}