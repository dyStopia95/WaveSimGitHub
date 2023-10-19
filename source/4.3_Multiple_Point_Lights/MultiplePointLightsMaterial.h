#pragma once

#include "Material.h"
#include "VectorHelper.h"
#include "MatrixHelper.h"
#include "SamplerStates.h"
#include "PointLight.h"
#include <array>
#include <DirectXColors.h>

namespace Library
{
	class Texture2D;
}

namespace Rendering
{
	class MultiplePointLightsMaterial : public Library::Material
	{
		RTTI_DECLARATIONS(MultiplePointLightsMaterial, Library::Material)
		inline static const std::uint32_t LightCount{ 4 };

	public:
		MultiplePointLightsMaterial(Library::Game& game, std::shared_ptr<Library::Texture2D> colormap, std::shared_ptr<Library::Texture2D> specularMap);
		MultiplePointLightsMaterial(const MultiplePointLightsMaterial&) = default;
		MultiplePointLightsMaterial& operator=(const MultiplePointLightsMaterial&) = default;
		MultiplePointLightsMaterial(MultiplePointLightsMaterial&&) = default;
		MultiplePointLightsMaterial& operator=(MultiplePointLightsMaterial&&) = default;
		virtual ~MultiplePointLightsMaterial() = default;

		winrt::com_ptr<ID3D11SamplerState> SamplerState() const;
		void SetSamplerState(winrt::com_ptr<ID3D11SamplerState> samplerState);

		std::shared_ptr<Library::Texture2D> ColorMap() const;
		void SetColorMap(std::shared_ptr<Library::Texture2D> texture);

		std::shared_ptr<Library::Texture2D> SpecularMap() const;
		void SetSpecularMap(std::shared_ptr<Library::Texture2D> texture);

		const DirectX::XMFLOAT4& AmbientColor() const;
		void SetAmbientColor(const DirectX::XMFLOAT4& color);

		const std::array<Library::PointLight, 4>& PointLights() const;
		void SetPointLight(const Library::PointLight& light, size_t index);

		const float SpecularPower() const;
		void SetSpecularPower(float power);

		virtual std::uint32_t VertexSize() const override;
		virtual void Initialize() override;

		void UpdateCameraPosition(const DirectX::XMFLOAT3& position);
		void UpdateTransforms(DirectX::FXMMATRIX worldViewProjectionMatrix, DirectX::CXMMATRIX worldMatrix);
		
	private:
		struct VertexCBufferPerFrame
		{
			struct LightData
			{
				DirectX::XMFLOAT3 Position{ Library::Vector3Helper::Zero };
				float Radius{ 50.0f };
			};

			LightData Lights[4];
		};

		struct VertexCBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection{ Library::MatrixHelper::Identity };
			DirectX::XMFLOAT4X4 World{ Library::MatrixHelper::Identity };
		};

		struct PixelCBufferPerFrame
		{
			struct LightData
			{
				DirectX::XMFLOAT3 Position{ Library::Vector3Helper::Zero };
				float Padding;
				DirectX::XMFLOAT3 Color{ DirectX::Colors::Black };
				float Padding2;
			};

			LightData Lights[4];
			DirectX::XMFLOAT3 CameraPosition{ Library::Vector3Helper::Zero };
			float Padding;
			DirectX::XMFLOAT4 AmbientColor{ DirectX::Colors::Black };
		};

		struct PixelCBufferPerObject
		{			
			float SpecularPower{ 128.0f };
			DirectX::XMFLOAT3 Padding;
		};

		virtual void BeginDraw() override;

		void ResetPixelShaderResources();

		winrt::com_ptr<ID3D11Buffer> mVertexCBufferPerFrame;
		winrt::com_ptr<ID3D11Buffer> mVertexCBufferPerObject;
		winrt::com_ptr<ID3D11Buffer> mPixelCBufferPerFrame;
		winrt::com_ptr<ID3D11Buffer> mPixelCBufferPerObject;
		VertexCBufferPerFrame mVertexCBufferPerFrameData;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		PixelCBufferPerFrame mPixelCBufferPerFrameData;
		PixelCBufferPerObject mPixelCBufferPerObjectData;
		bool mVertexCBufferPerFrameDataDirty{ true };
		bool mPixelCBufferPerFrameDataDirty{ true };
		bool mPixelCBufferPerObjectDataDirty{ true };
		std::array<Library::PointLight, 4> mLights;
		std::shared_ptr<Library::Texture2D> mColorMap;
		std::shared_ptr<Library::Texture2D> mSpecularMap;
		winrt::com_ptr<ID3D11SamplerState> mSamplerState{ Library::SamplerStates::TrilinearClamp };
	};
}