#pragma once

#include "Material.h"
#include "MatrixHelper.h"

namespace Rendering
{
	class BasicTessellationMaterial : public Library::Material
	{
		RTTI_DECLARATIONS(BasicTessellationMaterial, Library::Material)

	public:
		explicit BasicTessellationMaterial(Library::Game& game);
		BasicTessellationMaterial(const BasicTessellationMaterial&) = default;
		BasicTessellationMaterial& operator=(const BasicTessellationMaterial&) = default;
		BasicTessellationMaterial(BasicTessellationMaterial&&) = default;
		BasicTessellationMaterial& operator=(BasicTessellationMaterial&&) = default;
		virtual ~BasicTessellationMaterial() = default;

		virtual std::uint32_t VertexSize() const override;
		virtual void Initialize() override;

		void UpdateTransforms(DirectX::FXMMATRIX worldViewProjectionMatrix);
		void UpdateEdgeFactors();
		void UpdateInsideFactors();
		void UpdateUniformTessellationFactors();
		
	private:
		struct DomainCBufferPerObject final
		{
			DirectX::XMFLOAT4X4 WorldViewProjection{ Library::MatrixHelper::Identity };
		};
		
		struct QuadHullCBufferPerFrame final
		{
			float TessellationEdgeFactors[4];
			float TessellationInsideFactors[2];
			DirectX::XMFLOAT2 Padding;
		};

		struct TriHullCBufferPerFrame final
		{
			float TessellationEdgeFactors[3];
			float TessellationInsideFactor;
		};

		virtual void BeginDraw() override;

		winrt::com_ptr<ID3D11Buffer> mDomainCBufferPerObject;
		winrt::com_ptr<ID3D11Buffer> mQuadHullCBufferPerFrame;
		winrt::com_ptr<ID3D11Buffer> mTriHullCBufferPerFrame;
		DomainCBufferPerObject mDomainCBufferPerObjectData;
		QuadHullCBufferPerFrame mQuadHullCBufferPerFrameData;
		TriHullCBufferPerFrame mTriHullCBufferPerFrameData;
	};
}