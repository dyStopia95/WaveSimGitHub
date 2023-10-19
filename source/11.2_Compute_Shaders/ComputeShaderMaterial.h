#pragma once

#include <DirectXColors.h>
#include "Material.h"
#include "MatrixHelper.h"
#include "VectorHelper.h"

namespace Library
{
	class Texture2D;
	class ComputeShader;
}

namespace Rendering
{
	class ComputeShaderMaterial : public Library::Material
	{
		RTTI_DECLARATIONS(ComputeShaderMaterial, Library::Material)

	public:
		explicit ComputeShaderMaterial(Library::Game& game, winrt::com_ptr<ID3D11UnorderedAccessView> outputTexture);
		ComputeShaderMaterial(const ComputeShaderMaterial&) = default;
		ComputeShaderMaterial& operator=(const ComputeShaderMaterial&) = default;
		ComputeShaderMaterial(ComputeShaderMaterial&&) = default;
		ComputeShaderMaterial& operator=(ComputeShaderMaterial&&) = default;
		virtual ~ComputeShaderMaterial() = default;

		winrt::com_ptr<ID3D11UnorderedAccessView> OutputTexture() const;
		void SetOutputTexture(winrt::com_ptr<ID3D11UnorderedAccessView> texture);

		DirectX::XMFLOAT2 TextureSize() const;
		void UpdateTextureSize(const DirectX::XMFLOAT2& textureSize);

		float BlueColor() const;
		void SetBlueColor(float blueColor);

		virtual void Initialize() override;
		void Dispatch();
	
	private:
		struct ComputeCBufferPerFrame final
		{
			DirectX::XMFLOAT2 TextureSize{ 1024, 768 };
			float BlueColor { 1.0f };
			float Padding{ 0.0f };
		};

		std::shared_ptr<Library::ComputeShader> mComputeShader;
		winrt::com_ptr<ID3D11Buffer> mComputeCBufferPerFrame;
		ComputeCBufferPerFrame mComputeCBufferPerFrameData;
		winrt::com_ptr<ID3D11UnorderedAccessView> mOutputTexture;
		DirectX::XMUINT2 mThreadGroupCount{ 0, 0 };
	};
}