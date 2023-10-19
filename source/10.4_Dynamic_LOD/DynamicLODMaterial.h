#pragma once

#include <DirectXColors.h>
#include "Material.h"
#include "MatrixHelper.h"
#include "VectorHelper.h"
#include "SamplerStates.h"

namespace Rendering
{
	class DynamicLODMaterial : public Library::Material
	{
		RTTI_DECLARATIONS(DynamicLODMaterial, Library::Material)

	public:
		explicit DynamicLODMaterial(Library::Game& game);
		DynamicLODMaterial(const DynamicLODMaterial&) = default;
		DynamicLODMaterial& operator=(const DynamicLODMaterial&) = default;
		DynamicLODMaterial(DynamicLODMaterial&&) = default;
		DynamicLODMaterial& operator=(DynamicLODMaterial&&) = default;
		virtual ~DynamicLODMaterial() = default;

		int MaxTessellationFactor() const;
		void SetMaxTessellationFactor(int maxTessellationFactor);

		DirectX::XMFLOAT2 TessellationDistances() const;
		void SetTessellationDistances(DirectX::XMFLOAT2 tessellationDistances);
		void SetTessellationDistances(float minTessellationDistance, float maxTessellationDistance);

		virtual std::uint32_t VertexSize() const override;
		virtual void Initialize() override;

		void UpdateCameraPosition(const DirectX::XMFLOAT3& position);
		void UpdateTransforms(DirectX::FXMMATRIX worldViewProjectionMatrix, DirectX::CXMMATRIX worldMatrix);
		
	private:
		struct HullCBufferPerFrame final
		{
			DirectX::XMFLOAT3 CameraPosition{ Library::Vector3Helper::Zero };
			int MaxTessellationFactor{ 64 };
			DirectX::XMFLOAT2 TessellationDistances{ 2.0f, 20.0f };
			DirectX::XMFLOAT2 Padding{ 0.0f, 0.0f };
		};

		struct HullCBufferPerObject final
		{
			DirectX::XMFLOAT4X4 WorldMatrix{ Library::MatrixHelper::Identity };
		};

		struct DomainCBufferPerObject final
		{
			DirectX::XMFLOAT4X4 WorldViewProjectionMatrix{ Library::MatrixHelper::Identity };
		};

		virtual void BeginDraw() override;
		virtual void EndDraw() override;

		winrt::com_ptr<ID3D11Buffer> mHullCBufferPerFrame;
		winrt::com_ptr<ID3D11Buffer> mHullCBufferPerObject;
		winrt::com_ptr<ID3D11Buffer> mDomainCBufferPerObject;
		HullCBufferPerFrame mHullCBufferPerFrameData;
		HullCBufferPerObject mHullCBufferPerObjectData;
		DomainCBufferPerObject mDomainCBufferPerObjectData;
	};
}