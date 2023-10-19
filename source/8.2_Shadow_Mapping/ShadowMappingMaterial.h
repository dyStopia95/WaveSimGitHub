#pragma once

#include <DirectXColors.h>
#include <map>
#include <array>
#include "Material.h"
#include "VectorHelper.h"
#include "MatrixHelper.h"
#include "SamplerStates.h"

namespace Library
{
	class Texture2D;
}

namespace Rendering
{
	enum class ShadowMappingDrawModes
	{
		Basic = 0,
		ManualPcf,
		Pcf,
		End
	};

	class ShadowMappingMaterial : public Library::Material
	{
		RTTI_DECLARATIONS(ShadowMappingMaterial, Library::Material)
		friend class ShadowMappingDemo;

	public:
		ShadowMappingMaterial(Library::Game& game, std::shared_ptr<Library::Texture2D> colormap, winrt::com_ptr<ID3D11ShaderResourceView> shadowMap);
		ShadowMappingMaterial(const ShadowMappingMaterial&) = default;
		ShadowMappingMaterial& operator=(const ShadowMappingMaterial&) = default;
		ShadowMappingMaterial(ShadowMappingMaterial&&) = default;
		ShadowMappingMaterial& operator=(ShadowMappingMaterial&&) = default;
		virtual ~ShadowMappingMaterial() = default;

		ShadowMappingDrawModes DrawMode() const;
		const std::string& DrawModeString() const;
		void SetDrawMode(ShadowMappingDrawModes drawMode);

		std::shared_ptr<Library::Texture2D> ColorMap() const;
		void SetColorMap(std::shared_ptr<Library::Texture2D> texture);

		winrt::com_ptr<ID3D11ShaderResourceView> ShadowMap() const;
		void SetShadowMap(winrt::com_ptr<ID3D11ShaderResourceView> shadowMap);

		const DirectX::XMFLOAT4& AmbientColor() const;
		void SetAmbientColor(const DirectX::XMFLOAT4& color);

		const DirectX::XMFLOAT3& LightPosition() const;
		void SetLightPosition(const DirectX::XMFLOAT3& position);

		const float LightRadius() const;
		void SetLightRadius(float radius);

		const DirectX::XMFLOAT4& LightColor() const;
		void SetLightColor(const DirectX::XMFLOAT4& color);

		virtual std::uint32_t VertexSize() const override;
		virtual void Initialize() override;

		const DirectX::XMFLOAT2& ShadowMapSize() const;
		void SetShadowMapSize(float width, float height);

		void UpdateTransforms(DirectX::FXMMATRIX worldViewProjectionMatrix, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX projectiveTextureMatrix);
		
	private:
		struct VertexCBufferPerFrame
		{
			DirectX::XMFLOAT3 LightPosition{ Library::Vector3Helper::Zero };
			float LightRadius{ 50.0f };
		};

		struct VertexCBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection{ Library::MatrixHelper::Identity };
			DirectX::XMFLOAT4X4 World{ Library::MatrixHelper::Identity };
			DirectX::XMFLOAT4X4 ProjectiveTextureMatrix{ Library::MatrixHelper::Identity };
		};

		struct PixelCBufferPerFrame
		{
			DirectX::XMFLOAT3 CameraPosition{ Library::Vector3Helper::Zero };
			float Padding;
			DirectX::XMFLOAT4 AmbientColor{ DirectX::Colors::White };
			DirectX::XMFLOAT3 LightPosition{ Library::Vector3Helper::Zero };
			float Padding2;
			DirectX::XMFLOAT4 LightColor{ DirectX::Colors::Black };
			DirectX::XMFLOAT2 ShadowMapSize;
			float Padding3[2];
		};

		enum class ShadowMappingShaderClasses
		{
			Basic = 0,
			ManualPcf,
			Pcf
		};

		static const std::map<ShadowMappingDrawModes, std::string> DrawModeDisplayNames;
		static const std::map<ShadowMappingDrawModes, ShadowMappingShaderClasses> DrawModeShaderClassMap;

		virtual void BeginDraw() override;
		
		void UpdateTransforms(const VertexCBufferPerObject& transforms);
		void ResetPixelShaderResources();

		winrt::com_ptr<ID3D11Buffer> mVertexCBufferPerFrame;
		winrt::com_ptr<ID3D11Buffer> mVertexCBufferPerObject;
		winrt::com_ptr<ID3D11Buffer> mPixelCBufferPerFrame;
		VertexCBufferPerFrame mVertexCBufferPerFrameData;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		PixelCBufferPerFrame mPixelCBufferPerFrameData;
		bool mVertexCBufferPerFrameDataDirty{ true };
		bool mPixelCBufferPerFrameDataDirty{ true };
		std::shared_ptr<Library::Texture2D> mColorMap;
		winrt::com_ptr<ID3D11ShaderResourceView> mShadowMap;
		std::map<ShadowMappingShaderClasses, winrt::com_ptr<ID3D11ClassInstance>> mShaderClassInstances;
		ShadowMappingDrawModes mDrawMode;
	};
}