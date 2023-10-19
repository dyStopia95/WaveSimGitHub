#include "pch.h"
#include "BloomDemo.h"
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
	BloomDemo::BloomDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera),
		mRenderTarget(game), mBloom(game)
	{
	}

	BloomDemo::~BloomDemo()
	{
	}

	shared_ptr<DiffuseLightingDemo> BloomDemo::DiffuseLighting() const
	{
		return mDiffuseLightingDemo;
	}

	bool BloomDemo::BloomEnabled() const
	{
		return mBloomEnabled;
	}

	void BloomDemo::SetBloomEnabled(bool enabled)
	{
		mBloomEnabled = enabled;
	}

	void BloomDemo::ToggleBloom()
	{
		SetBloomEnabled(!mBloomEnabled);
	}

	BloomDrawModes BloomDemo::DrawMode() const
	{
		return mBloom.DrawMode();
	}

	const string& BloomDemo::DrawModeString() const
	{
		return mBloom.DrawModeString();
	}

	void BloomDemo::SetDrawMode(BloomDrawModes drawMode)
	{
		mBloom.SetDrawMode(drawMode);
	}

	const BloomSettings& BloomDemo::GetBloomSettings() const
	{
		return mBloom.GetBloomSettings();
	}

	void BloomDemo::SetBloomSettings(BloomSettings& settings)
	{
		mBloom.SetBloomSettings(settings);
	}

	void BloomDemo::Initialize()
	{
		mDiffuseLightingDemo = make_shared<DiffuseLightingDemo>(*mGame, mCamera);
		mDiffuseLightingDemo->Initialize();

		mBloom.SetSceneTexture(mRenderTarget.OutputTexture());
		mBloom.Initialize();
	}

	void BloomDemo::Update(const GameTime& gameTime)
	{
		mDiffuseLightingDemo->Update(gameTime);
	}

	void BloomDemo::Draw(const GameTime& gameTime)
	{
		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		if (mBloomEnabled)
		{
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

			mBloom.Draw(gameTime);
		}
		else
		{
			direct3DDeviceContext->ClearRenderTargetView(mGame->RenderTargetView(), reinterpret_cast<const float*>(&Colors::Purple));
			direct3DDeviceContext->ClearDepthStencilView(mGame->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			mGame->Game::Draw(gameTime);
			mDiffuseLightingDemo->Draw(gameTime);
		}
	}
}