#pragma once
#include <DirectXColors.h>
#include "Material.h"
#include "MatrixHelper.h"
#include "VectorHelper.h"
#include <DirectXMath.h>

namespace Library
{
	class Texture1D;
	class ComputeShader;
}

namespace Rendering
{
	struct SimParams;

	class WaveSimCompShader : public Library::Material
	{
		RTTI_DECLARATIONS(WaveSimCompShader, Library::Material)

	public:
		explicit WaveSimCompShader(Library::Game& game, winrt::com_ptr<ID3D11UnorderedAccessView> outputTexture);
		WaveSimCompShader(const WaveSimCompShader&) = default;
		WaveSimCompShader& operator=(const WaveSimCompShader&) = default;
		WaveSimCompShader(WaveSimCompShader&&) = default;
		WaveSimCompShader& operator=(WaveSimCompShader&&) = default;
		virtual ~WaveSimCompShader() = default;

		void SetParams(const SimParams& parameters);
		void CreateVertexXYArray(DirectX::XMFLOAT2* xyArray, UINT length);

		winrt::com_ptr<ID3D11UnorderedAccessView> OutputTexture() const;
		void SetOutputTexture(winrt::com_ptr<ID3D11UnorderedAccessView> texture);

		virtual void Initialize() override;
		void Dispatch();

	private:

		struct SimParamBuffer final
		{
			float C2{ 0 };
			float dampingFactor{ 0.0f };
			float springConstant{ 0.0f };
			float deltaT{ 0.f };
			float spacing2{ 0.f };
			int rows{ 0 };
			int columns{ 0 };
			int nodeCount{ 0 };
		};

		std::shared_ptr<Library::ComputeShader> mComputeShader;
		std::shared_ptr<Library::Texture1D> mVertexXYArray;
		winrt::com_ptr<ID3D11Buffer> mSimParamsCB;
		SimParamBuffer mSimParamsCBData;
		ID3D11DeviceContext* d3dContextPtr{ nullptr };
		winrt::com_ptr<ID3D11UnorderedAccessView> mOutputTexture;
		int mNodeCount{ 0 };
	};


}
