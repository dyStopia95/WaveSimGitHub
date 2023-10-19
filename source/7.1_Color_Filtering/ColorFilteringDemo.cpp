#include "pch.h"
#include "ColorFilteringDemo.h"
#include "Camera.h"
#include "Game.h"
#include "GameException.h"
#include "DiffuseLightingDemo.h"
#include "FullScreenQuadMaterial.h"
#include "PixelShader.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace winrt;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	const map<ColorFilters, string> ColorFilteringDemo::ColorFilterNames =
	{
		{ ColorFilters::GrayScale, "GrayScale"s },
		{ ColorFilters::Inverse, "Inverse"s },
		{ ColorFilters::Sepia, "Sepia"s },
		{ ColorFilters::Generic, "Generic (Brightness)"s }
	};

	ColorFilteringDemo::ColorFilteringDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera),
		mRenderTarget(game),
		mFullScreenQuad(game)
	{
	}

	ColorFilteringDemo::~ColorFilteringDemo()
	{
	}

	shared_ptr<DiffuseLightingDemo> ColorFilteringDemo::DiffuseLighting() const
	{
		return mDiffuseLightingDemo;
	}

	ColorFilters ColorFilteringDemo::ActiveColorFilter() const
	{
		return mActiveColorFilter;
	}

	void ColorFilteringDemo::SetActiveColorFilter(ColorFilters colorFilter)
	{
		mActiveColorFilter = colorFilter;
		mFullScreenQuad.Material()->SetShader(mPixelShadersByColorFilter.at(mActiveColorFilter));
	}

	float ColorFilteringDemo::GenericFilterBrightness() const
	{
		return mGenericColorFilterPSConstantBufferData.ColorFilter._11;
	}

	void ColorFilteringDemo::SetGenericFilterBrightness(float brightness)
	{
		XMStoreFloat4x4(&mGenericColorFilterPSConstantBufferData.ColorFilter, XMMatrixScaling(brightness, brightness, brightness));		
		mGame->Direct3DDeviceContext()->UpdateSubresource(mGenericColorFilterPSConstantBuffer.get(), 0, nullptr, &mGenericColorFilterPSConstantBufferData, 0, 0);
	}
	
	void ColorFilteringDemo::Initialize()
	{
		mDiffuseLightingDemo = make_shared<DiffuseLightingDemo>(*mGame, mCamera);
		mDiffuseLightingDemo->Initialize();

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = sizeof(GenericColorFilterPSConstantBuffer);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mGenericColorFilterPSConstantBuffer.put()), "ID3D11Device::CreateBuffer() failed.");

		// Initialize the generic color filter
		mGame->Direct3DDeviceContext()->UpdateSubresource(mGenericColorFilterPSConstantBuffer.get(), 0, nullptr, &mGenericColorFilterPSConstantBufferData, 0, 0);

		mFullScreenQuad.Initialize();
		mFullScreenQuad.Material()->SetTexture(mRenderTarget.OutputTexture().get());
		mFullScreenQuad.Material()->SetUpdateMaterialCallback([&]
		{
			if (mActiveColorFilter == ColorFilters::Generic)
			{
				auto psConstantBuffers = mGenericColorFilterPSConstantBuffer.get();
				mGame->Direct3DDeviceContext()->PSSetConstantBuffers(0, 1, &psConstantBuffers);
			}
		});

		auto& content = mGame->Content();
		mPixelShadersByColorFilter[ColorFilters::GrayScale] = content.Load<PixelShader>(L"Shaders\\GrayScaleColorFilterPS.cso");
		mPixelShadersByColorFilter[ColorFilters::Inverse] = content.Load<PixelShader>(L"Shaders\\InverseColorFilterPS.cso");
		mPixelShadersByColorFilter[ColorFilters::Sepia] = content.Load<PixelShader>(L"Shaders\\SepiaColorFilterPS.cso");
		mPixelShadersByColorFilter[ColorFilters::Generic] = content.Load<PixelShader>(L"Shaders\\GenericColorFilterPS.cso");

		SetActiveColorFilter(ColorFilters::GrayScale);
	}

	void ColorFilteringDemo::Update(const GameTime& gameTime)
	{
		mDiffuseLightingDemo->Update(gameTime);
	}

	void ColorFilteringDemo::Draw(const GameTime& gameTime)
	{
		// Render scene to off-screen render target
		mRenderTarget.Begin();

		mGame->Direct3DDeviceContext()->ClearRenderTargetView(mRenderTarget.RenderTargetView().get(), Colors::CornflowerBlue.f);
		mGame->Direct3DDeviceContext()->ClearDepthStencilView(mRenderTarget.DepthStencilView().get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		mGame->Game::Draw(gameTime);
		mDiffuseLightingDemo->Draw(gameTime);

		mRenderTarget.End();
		
		// Render off-screen texture to a full-screen quad with a color filter shader
		mGame->Direct3DDeviceContext()->ClearRenderTargetView(mGame->RenderTargetView().get(), Colors::CornflowerBlue.f);
		mGame->Direct3DDeviceContext()->ClearDepthStencilView(mGame->DepthStencilView().get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		mFullScreenQuad.Draw(gameTime);
		mFullScreenQuad.Material()->UnbindShaderResources<1>(ShaderStages::PS);
	}
}