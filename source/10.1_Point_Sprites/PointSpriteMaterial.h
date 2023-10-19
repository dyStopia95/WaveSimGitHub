#pragma once

#include <DirectXColors.h>
#include "Material.h"
#include "MatrixHelper.h"
#include "VectorHelper.h"
#include "SamplerStates.h"

namespace Library
{
	class Texture2D;
	class GeometryShader;
}

namespace Rendering
{
	class PointSpriteMaterial : public Library::Material
	{
		RTTI_DECLARATIONS(PointSpriteMaterial, Library::Material)

	public:
		explicit PointSpriteMaterial(Library::Game& game);
		PointSpriteMaterial(const PointSpriteMaterial&) = default;
		PointSpriteMaterial& operator=(const PointSpriteMaterial&) = default;
		PointSpriteMaterial(PointSpriteMaterial&&) = default;
		PointSpriteMaterial& operator=(PointSpriteMaterial&&) = default;
		virtual ~PointSpriteMaterial() = default;

		winrt::com_ptr<ID3D11SamplerState> SamplerState() const;
		void SetSamplerState(winrt::com_ptr<ID3D11SamplerState> samplerState);

		std::shared_ptr<Library::Texture2D> ColorMap() const;
		void SetColorMap(std::shared_ptr<Library::Texture2D> texture);

		virtual std::uint32_t VertexSize() const override;
		virtual void Initialize() override;

		void UpdateCameraData(DirectX::FXMMATRIX viewProjectionMatrix, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& up);
		
	private:
		virtual void EndDraw() override;

		struct GeometryCBufferPerFrame final
		{
			DirectX::XMFLOAT4X4 ViewProjectionMatrix{ Library::MatrixHelper::Identity };
			DirectX::XMFLOAT3 CameraPosition{ Library::Vector3Helper::Zero };
			float Padding{ 0.0f };
			DirectX::XMFLOAT3 CameraUp{ Library::Vector3Helper::Up };
			float Padding2{ 0.0f };
		};

		winrt::com_ptr<ID3D11Buffer> mGeometryCBufferPerFrame;
		GeometryCBufferPerFrame mGeometryCBufferPerFrameData;		
		std::shared_ptr<Library::Texture2D> mColorMap;
		winrt::com_ptr<ID3D11SamplerState> mSamplerState{ Library::SamplerStates::TrilinearWrap };
	};
}