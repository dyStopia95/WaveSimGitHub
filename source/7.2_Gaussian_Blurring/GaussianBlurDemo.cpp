#include "pch.h"
#include "GaussianBlurDemo.h"
#include "Game.h"
#include "DiffuseLightingDemo.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace winrt;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	GaussianBlurDemo::GaussianBlurDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera),
		mRenderTarget(game), mGaussianBlur(game)
	{
	}

	GaussianBlurDemo::~GaussianBlurDemo()
	{
	}

	shared_ptr<DiffuseLightingDemo> GaussianBlurDemo::DiffuseLighting() const
	{
		return mDiffuseLightingDemo;
	}

	float GaussianBlurDemo::BlurAmount() const
	{
		return mGaussianBlur.BlurAmount();
	}

	void GaussianBlurDemo::SetBlurAmount(float blurAmount)
	{
		mGaussianBlur.SetBlurAmount(blurAmount);
	}
	
	void GaussianBlurDemo::Initialize()
	{
		mDiffuseLightingDemo = make_shared<DiffuseLightingDemo>(*mGame, mCamera);
		mDiffuseLightingDemo->Initialize();

		mGaussianBlur.SetSceneTexture(mRenderTarget.OutputTexture());
		mGaussianBlur.Initialize();
	}

	void GaussianBlurDemo::Update(const GameTime& gameTime)
	{
		mDiffuseLightingDemo->Update(gameTime);
	}

	void GaussianBlurDemo::Draw(const GameTime& gameTime)
	{
		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		// Render scene to off-screen render target
		mRenderTarget.Begin();

		direct3DDeviceContext->ClearRenderTargetView(mRenderTarget.RenderTargetView().get(), Colors::CornflowerBlue.f);
		direct3DDeviceContext->ClearDepthStencilView(mRenderTarget.DepthStencilView().get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		mGame->Game::Draw(gameTime);
		mDiffuseLightingDemo->Draw(gameTime);

		mRenderTarget.End();
		
		// Render off-screen texture to a full-screen quad with gaussian blurring
		direct3DDeviceContext->ClearRenderTargetView(mGame->RenderTargetView(), Colors::CornflowerBlue.f);
		direct3DDeviceContext->ClearDepthStencilView(mGame->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		mGaussianBlur.Draw(gameTime);
	}
}