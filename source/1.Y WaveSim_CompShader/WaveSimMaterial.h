#pragma once

#include <DirectXColors.h>
#include "Material.h"
#include "MatrixHelper.h"
#include "SamplerStates.h"

namespace Library
{
	class Texture1D;
}

namespace Rendering
{
	class WaveSimMaterial : public Library::Material
	{
		RTTI_DECLARATIONS(WaveSimMaterial, Library::Material)

	public:
		WaveSimMaterial(Library::Game& game, std::shared_ptr<Library::Texture1D> zArray);
		WaveSimMaterial(const WaveSimMaterial&) = default;
		WaveSimMaterial& operator=(const WaveSimMaterial&) = default;
		WaveSimMaterial(WaveSimMaterial&&) = default;
		WaveSimMaterial& operator=(WaveSimMaterial&&) = default;
		virtual ~WaveSimMaterial() = default;

		virtual void Initialize() override;
		void UpdateTransforms(DirectX::CXMMATRIX worldViewProjectionMatrix);
		void SetSurfaceColor(const DirectX::XMFLOAT4& color);

		std::shared_ptr<Library::Texture1D> GetZArrayRef();
		void SetZArrayRef(std::shared_ptr<Library::Texture1D> dispMap);

		virtual std::uint32_t VertexSize() const override;

	private:
		winrt::com_ptr<ID3D11Buffer> mPSConstantBuffer;
		winrt::com_ptr<ID3D11Buffer> mWVPBuffer;
		std::shared_ptr<Library::Texture1D> mDisplacementMap;

		//virtual void BeginDraw() override;
		void SetSurfaceColor(const float* color);
	};
}


		/*struct VertexCBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection{ Library::MatrixHelper::Identity };
		};*/
		//VertexCBufferPerObject mWVPBuffData;

//template<size_t nodeCount>

		/*struct ZValuePerNodeCBuffer
		{
			float zDisplacement[nodeCount];
		};*/

		//winrt::com_ptr<ID3D11Buffer> mZValueArrayBuffer;

		//winrt::com_ptr<ID3D11SamplerState> mSamplerState{ Library::SamplerStates::PointClamp };

		//void UpdateZValues(float* zValueArray);
		/*winrt::com_ptr<ID3D11SamplerState> SamplerState() const;
		void SetSamplerState(winrt::com_ptr<ID3D11SamplerState> samplerState);*/