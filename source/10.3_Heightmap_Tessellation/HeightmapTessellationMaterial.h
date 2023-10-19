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
	class HeightmapTessellationMaterial : public Library::Material
	{
		RTTI_DECLARATIONS(HeightmapTessellationMaterial, Library::Material)

	public:
		explicit HeightmapTessellationMaterial(Library::Game& game);
		HeightmapTessellationMaterial(const HeightmapTessellationMaterial&) = default;
		HeightmapTessellationMaterial& operator=(const HeightmapTessellationMaterial&) = default;
		HeightmapTessellationMaterial(HeightmapTessellationMaterial&&) = default;
		HeightmapTessellationMaterial& operator=(HeightmapTessellationMaterial&&) = default;
		virtual ~HeightmapTessellationMaterial() = default;

		winrt::com_ptr<ID3D11SamplerState> SamplerState() const;
		void SetSamplerState(winrt::com_ptr<ID3D11SamplerState> samplerState);

		std::shared_ptr<Library::Texture2D> Heightmap() const;
		void SetHeightmap(std::shared_ptr<Library::Texture2D> heightmap);

		gsl::span<const float> EdgeFactors() const;
		gsl::span<const float> InsideFactors() const;
		void SetUniformFactors(float factor);

		float DisplacementScale() const;
		void SetDisplacementScale(float displacementScale);

		virtual std::uint32_t VertexSize() const override;
		virtual void Initialize() override;

		void UpdateTransforms(DirectX::FXMMATRIX viewProjectionMatrix);
		void UpdateTextureMatrix(DirectX::FXMMATRIX textureMatrix);
		
	private:
		struct VertexCBufferPerObject final
		{
			DirectX::XMFLOAT4X4 TextureMatrix{ Library::MatrixHelper::Identity };
		};		

		struct DomainCBufferPerObject final
		{
			DirectX::XMFLOAT4X4 WorldViewProjection{ Library::MatrixHelper::Identity };
			float DisplacementScale{ 1.0f };
			DirectX::XMFLOAT3 Padding;
		};
		
		struct HullCBufferPerFrame final
		{
			float TessellationEdgeFactors[4]{ 10.0f, 10.0f, 10.0f, 10.0f };
			float TessellationInsideFactors[2]{ 10.0f, 10.0f };
			DirectX::XMFLOAT2 Padding;
		};

		virtual void BeginDraw() override;
		virtual void EndDraw() override;

		void UpdateUniformTessellationFactors(float source, gsl::span<float> edgeFactors, gsl::span<float> insideFactors);

		winrt::com_ptr<ID3D11Buffer> mDomainCBufferPerObject;
		winrt::com_ptr<ID3D11Buffer> mHullCBufferPerFrame;
		winrt::com_ptr<ID3D11Buffer> mVertexCBufferPerObject;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		DomainCBufferPerObject mDomainCBufferPerObjectData;
		HullCBufferPerFrame mHullCBufferPerFrameData;

		std::shared_ptr<Library::Texture2D> mHeightmap;
		winrt::com_ptr<ID3D11SamplerState> mSamplerState{ Library::SamplerStates::TrilinearWrap };
	};
}