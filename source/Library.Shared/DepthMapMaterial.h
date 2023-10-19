#pragma once

#include "Material.h"

namespace Library
{
	class DepthMapMaterial : public Material
	{
		RTTI_DECLARATIONS(DepthMapMaterial, Material)

	public:
		explicit DepthMapMaterial(Game& game);
		DepthMapMaterial(const DepthMapMaterial&) = default;
		DepthMapMaterial& operator=(const DepthMapMaterial&) = default;
		DepthMapMaterial(DepthMapMaterial&&) = default;
		DepthMapMaterial& operator=(DepthMapMaterial&&) = default;
		virtual ~DepthMapMaterial() = default;

		virtual std::uint32_t VertexSize() const override;
		virtual void Initialize() override;

		void UpdateTransform(DirectX::CXMMATRIX worldLightViewProjectionMatrix);

	private:
		void SetSurfaceColor(const float* color);

		winrt::com_ptr<ID3D11Buffer> mVSConstantBuffer;
	};
}
