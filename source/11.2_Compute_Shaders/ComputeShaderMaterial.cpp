#include "pch.h"
#include "ComputeShaderMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "ComputeShader.h"
#include "DirectXHelper.h"

using namespace std;
using namespace gsl;
using namespace std::string_literals;
using namespace DirectX;
using namespace winrt;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(ComputeShaderMaterial)

	ComputeShaderMaterial::ComputeShaderMaterial(Game& game, com_ptr<ID3D11UnorderedAccessView> outputTexture) :
		Material(game), mOutputTexture(move(outputTexture))
	{
	}

	com_ptr<ID3D11UnorderedAccessView> ComputeShaderMaterial::OutputTexture() const
	{
		return mOutputTexture;
	}

	void ComputeShaderMaterial::SetOutputTexture(com_ptr<ID3D11UnorderedAccessView> texture)
	{
		mOutputTexture = move(texture);
	}

	XMFLOAT2 ComputeShaderMaterial::TextureSize() const
	{
		return mComputeCBufferPerFrameData.TextureSize;
	}

	void ComputeShaderMaterial::UpdateTextureSize(const XMFLOAT2& textureSize)
	{
		mComputeCBufferPerFrameData.TextureSize = textureSize;
		mGame->Direct3DDeviceContext()->UpdateSubresource(mComputeCBufferPerFrame.get(), 0, nullptr, &mComputeCBufferPerFrameData, 0, 0);
	}

	float ComputeShaderMaterial::BlueColor() const
	{
		return mComputeCBufferPerFrameData.BlueColor;
	}

	void ComputeShaderMaterial::SetBlueColor(float blueColor)
	{
		mComputeCBufferPerFrameData.BlueColor = blueColor;
		mGame->Direct3DDeviceContext()->UpdateSubresource(mComputeCBufferPerFrame.get(), 0, nullptr, &mComputeCBufferPerFrameData, 0, 0);
	}

	void ComputeShaderMaterial::Initialize()
	{
		Material::Initialize();
				
		mComputeShader = mGame->Content().Load<ComputeShader>(L"Shaders\\ComputeShaderDemoCS.cso"s);

		CreateConstantBuffer(mGame->Direct3DDevice(), sizeof(ComputeCBufferPerFrame), mComputeCBufferPerFrame.put());
		mGame->Direct3DDeviceContext()->UpdateSubresource(mComputeCBufferPerFrame.get(), 0, nullptr, &mComputeCBufferPerFrameData, 0, 0);

		//const uint32_t threadsPerGroup = 32;
		mThreadGroupCount.x = static_cast<uint32_t>(mComputeCBufferPerFrameData.TextureSize.x)/* / threadsPerGroup*/;
		mThreadGroupCount.y = static_cast<uint32_t>(mComputeCBufferPerFrameData.TextureSize.y)/* / threadsPerGroup*/;
	}

	void ComputeShaderMaterial::Dispatch()
	{
		assert(mOutputTexture != nullptr);
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();

		direct3DDeviceContext->CSSetShader(mComputeShader->Shader().get(), nullptr, 0);

		auto uaViews = mOutputTexture.get();
		direct3DDeviceContext->CSSetUnorderedAccessViews(0, 1, &uaViews, nullptr);

		auto constantBuffers = mComputeCBufferPerFrame.get();
		direct3DDeviceContext->CSSetConstantBuffers(0, 1, &constantBuffers);

		direct3DDeviceContext->Dispatch(mThreadGroupCount.x, mThreadGroupCount.y, 1);

		static const std::array<ID3D11UnorderedAccessView*, 1> emptyUAViews{ nullptr };
		direct3DDeviceContext->CSSetUnorderedAccessViews(0, 1, emptyUAViews.data(), nullptr);
	}
}