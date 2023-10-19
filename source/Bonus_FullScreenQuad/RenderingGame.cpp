#include "pch.h"
#include "RenderingGame.h"
#include "PerspectiveCamera.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "FpsComponent.h"
#include "FullScreenQuadDemo.h"
#include "SamplerStates.h"
#include "RasterizerStates.h"
#include "VectorHelper.h"
#include "ImGuiComponent.h"
#include "imgui_impl_dx11.h"
#include "UtilityWin32.h"

using namespace std;
using namespace DirectX;
using namespace Library;

namespace Rendering
{
	RenderingGame::RenderingGame(std::function<void*()> getWindowCallback, std::function<void(SIZE&)> getRenderTargetSizeCallback) :
		Game(getWindowCallback, getRenderTargetSizeCallback)
	{
	}

	void RenderingGame::Initialize()
	{
		auto direct3DDevice = Direct3DDevice();
		SamplerStates::Initialize(direct3DDevice); 
		RasterizerStates::Initialize(direct3DDevice);

		mKeyboard = make_shared<KeyboardComponent>(*this);
		mComponents.push_back(mKeyboard);
		mServices.AddService(KeyboardComponent::TypeIdClass(), mKeyboard.get());
	
		auto camera = make_shared<PerspectiveCamera>(*this);
		mComponents.push_back(camera);
		mServices.AddService(Camera::TypeIdClass(), camera.get());

		mImGuiComponent = make_shared<ImGuiComponent>(*this);
		mServices.AddService(ImGuiComponent::TypeIdClass(), mImGuiComponent.get());
		auto imGuiWndProcHandler = make_shared<UtilityWin32::WndProcHandler>(ImGui_ImplWin32_WndProcHandler);
		UtilityWin32::AddWndProcHandler(imGuiWndProcHandler);

		auto helpTextImGuiRenderBlock = make_shared<ImGuiComponent::RenderBlock>([this]()
		{
			ImGui::Begin("Controls");
			ImGui::SetNextWindowPos(ImVec2(10, 10));
			
			{
				stringstream fpsLabel;
				fpsLabel << setprecision(3) << "Frame Rate: " << mFpsComponent->FrameRate() << "    Total Elapsed Time: " << mGameTime.TotalGameTimeSeconds().count();
				ImGui::Text(fpsLabel.str().c_str());
			}
		
			ImGui::End();
		});
		mImGuiComponent->AddRenderBlock(helpTextImGuiRenderBlock);
		mImGuiComponent->Initialize();

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		auto demo = make_shared<FullScreenQuadDemo>(*this, camera);
		mComponents.push_back(demo);

		Game::Initialize();
	}

	void RenderingGame::Update(const GameTime &gameTime)
	{
		if (mKeyboard->WasKeyPressedThisFrame(Keys::Escape))
		{
			Exit();
		}

		mImGuiComponent->Update(gameTime);

		Game::Update(gameTime);
	}

	void RenderingGame::Draw(const GameTime &gameTime)
	{		
		mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView.get(), BackgroundColor.f);
		mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		Game::Draw(gameTime);

		mImGuiComponent->Draw(gameTime);

		HRESULT hr = mSwapChain->Present(1, 0);

		// If the device was removed either by a disconnection or a driver upgrade, we must recreate all device resources.
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			HandleDeviceLost();
		}
		else
		{
			ThrowIfFailed(hr, "IDXGISwapChain::Present() failed.");
		}
	}

	void RenderingGame::Shutdown()
	{
		mFpsComponent = nullptr;
		mImGuiComponent->Shutdown();
		mImGuiComponent = nullptr;
		mFullScreenQuadDemo = nullptr;
		RasterizerStates::Shutdown();
		SamplerStates::Shutdown();
		Game::Shutdown();		
	}

	void RenderingGame::Exit()
	{
		PostQuitMessage(0);
	}
}