#include "pch.h"
#include "ComputeShaderDemo.h"
#include "ComputeShaderMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "Texture2D.h"
#include "FullScreenQuadMaterial.h"
#include "PixelShader.h"
#include "ComputeShaderMaterial.h"
#include "SamplerStates.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace winrt;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	ComputeShaderDemo::ComputeShaderDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera),
		mFullScreenQuad(game)
	{
	}

	ComputeShaderDemo::~ComputeShaderDemo()
	{
	}

	bool ComputeShaderDemo::AnimationEnabled() const
	{
		return mAnimationEnabled;
	}

	void ComputeShaderDemo::SetAnimationEnabled(bool enabled)
	{
		mAnimationEnabled = enabled;
	}

	float ComputeShaderDemo::BlueColor() const
	{
		return mMaterial->BlueColor();
	}

	void ComputeShaderDemo::SetBlueColor(float blueColor) 
	{
		return mMaterial->SetBlueColor(blueColor);
	}

	void ComputeShaderDemo::Initialize()
	{
		mFullScreenQuad.Initialize();
		auto fullScreenQuadMaterial = mFullScreenQuad.Material();
		auto pixelShader = mGame->Content().Load<PixelShader>(L"Shaders\\TexturedModelPS.cso");
		fullScreenQuadMaterial->SetShader(pixelShader);

		const auto renderTargetSize = mGame->RenderTargetSize();
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = renderTargetSize.cx;
		textureDesc.Height = renderTargetSize.cy;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

		HRESULT hr;
		com_ptr<ID3D11Texture2D> texture;
		if (FAILED(hr = mGame->Direct3DDevice()->CreateTexture2D(&textureDesc, nullptr, texture.put())))
		{
			throw GameException("IDXGIDevice::CreateTexture2D() failed.", hr);
		}
		
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory(&uavDesc, sizeof(uavDesc));
		uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		
		winrt::com_ptr<ID3D11UnorderedAccessView> outputTexture;		
		if (FAILED(hr = mGame->Direct3DDevice()->CreateUnorderedAccessView(texture.get(), &uavDesc, outputTexture.put())))
		{
			throw GameException("IDXGIDevice::CreateUnorderedAccessView() failed.", hr);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
		shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		if (FAILED(hr = mGame->Direct3DDevice()->CreateShaderResourceView(texture.get(), &shaderResourceViewDesc, mColorTexture.put())))
		{
			throw GameException("IDXGIDevice::CreateShaderResourceView() failed.", hr);
		}

		fullScreenQuadMaterial->SetTexture(mColorTexture.get());
		fullScreenQuadMaterial->SetSamplerState(SamplerStates::TrilinearClamp);

		mMaterial = make_unique<ComputeShaderMaterial>(*mGame, outputTexture);
		mMaterial->Initialize();
	}

	void ComputeShaderDemo::Update(const Library::GameTime& gameTime)
	{
		if (mAnimationEnabled)
		{
			mMaterial->SetBlueColor(0.5f * sinf(gameTime.TotalGameTimeSeconds().count()) + 0.5f);
		}
	}

	void ComputeShaderDemo::Draw(const GameTime& gameTime)
	{
		mMaterial->Dispatch();

		mFullScreenQuad.Draw(gameTime);
		mFullScreenQuad.Material()->UnbindShaderResources<1>(ShaderStages::PS);
	}
}