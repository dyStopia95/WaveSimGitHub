#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "FpsComponent.h"
#include "FirstPersonCamera.h"
#include "Grid.h"
#include "DynamicLODDemo.h"
#include "SamplerStates.h"
#include "RasterizerStates.h"
#include "VectorHelper.h"
#include "ImGuiComponent.h"
#include "imgui_impl_dx11.h"
#include "UtilityWin32.h"

using namespace std;
using namespace gsl;
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

		mMouse = make_shared<MouseComponent>(*this, MouseModes::Absolute);
		mComponents.push_back(mMouse);
		mServices.AddService(MouseComponent::TypeIdClass(), mMouse.get());

		mGamePad = make_shared<GamePadComponent>(*this);
		mComponents.push_back(mGamePad);
		mServices.AddService(GamePadComponent::TypeIdClass(), mGamePad.get());

		auto camera = make_shared<FirstPersonCamera>(*this);
		mComponents.push_back(camera);
		mServices.AddService(Camera::TypeIdClass(), camera.get());

		mGrid = make_shared<Grid>(*this, camera);
		mComponents.push_back(mGrid);

		mDynamicLODDemo = make_shared<DynamicLODDemo>(*this, camera);
		mComponents.push_back(mDynamicLODDemo);

		auto imGui = make_shared<ImGuiComponent>(*this);
		mComponents.push_back(imGui);
		mServices.AddService(ImGuiComponent::TypeIdClass(), imGui.get());
		auto imGuiWndProcHandler = make_shared<UtilityWin32::WndProcHandler>(ImGui_ImplWin32_WndProcHandler);
		UtilityWin32::AddWndProcHandler(imGuiWndProcHandler);

		auto helpTextImGuiRenderBlock = make_shared<ImGuiComponent::RenderBlock>([this]()
			{
				ImGui::Begin("Controls");
				ImGui::SetNextWindowPos(ImVec2(10, 10));

				{
					ostringstream fpsLabel;
					fpsLabel << setprecision(3) << "Frame Rate: " << mFpsComponent->FrameRate() << "    Total Elapsed Time: " << mGameTime.TotalGameTimeSeconds().count();
					ImGui::Text(fpsLabel.str().c_str());
				}

				ImGui::Text("Camera (WASD + Left-Click-Mouse-Look)");
				AddImGuiTextField("Toggle Grid (G): "s, (mGrid->Visible() ? "Visible"s : "Not Visible"s));
				AddImGuiTextField("Max Tessellation Factor (+PageUp/-PageDown): "s, mDynamicLODDemo->MaxTessellationFactor());				

				const auto tessellationDistances = mDynamicLODDemo->TessellationDistances();
				AddImGuiTextField("Min Tessellation Distance(+Insert/-Delete): "s, tessellationDistances.x);
				AddImGuiTextField("Max Tessellation Distance(+Home/-End): "s, tessellationDistances.y);

				ImGui::End();
			});
		imGui->AddRenderBlock(helpTextImGuiRenderBlock);

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();

		camera->SetPosition(0.0f, 2.5f, 20.0f);
	}

	void RenderingGame::Update(const GameTime &gameTime)
	{
		if (mKeyboard->WasKeyPressedThisFrame(Keys::Escape))
		{
			Exit();
		}

		if (mMouse->WasButtonPressedThisFrame(MouseButtons::Left))
		{
			mMouse->SetMode(MouseModes::Relative);
		}

		if (mMouse->WasButtonReleasedThisFrame(MouseButtons::Left))
		{
			mMouse->SetMode(MouseModes::Absolute);
		}

		if (mKeyboard->WasKeyPressedThisFrame(Keys::G))
		{
			mGrid->SetVisible(!mGrid->Visible());
		}
		
		UpdateTessellationOptions();

		Game::Update(gameTime);
	}

	void RenderingGame::Draw(const GameTime &gameTime)
	{
		mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView.get(), BackgroundColor.f);
		mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		Game::Draw(gameTime);

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
		mGrid = nullptr;
		mFpsComponent = nullptr;
		mDynamicLODDemo = nullptr;
		RasterizerStates::Shutdown();
		SamplerStates::Shutdown();
		Game::Shutdown();		
	}

	void RenderingGame::Exit()
	{
		PostQuitMessage(0);
	}

	void RenderingGame::UpdateTessellationOptions()
	{
		{
			const int MinTessellationFactor = 1;
			const int MaxTessellationFactor = 64;
			int maxTessellationFactor = mDynamicLODDemo->MaxTessellationFactor();
			UpdateValueWithKeyboard<int>(*mKeyboard, Keys::PageUp, Keys::PageDown, maxTessellationFactor, 1, [&](const int& maxTessellationFactor)
			{
				mDynamicLODDemo->SetMaxTessellationFactor(maxTessellationFactor);
			}, MinTessellationFactor, MaxTessellationFactor);
		}

		{
			const int MinTessellatioDistance = 1;
			auto tessellationDistances = mDynamicLODDemo->TessellationDistances();
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Insert, Keys::Delete, tessellationDistances.x, 0.1f, [&, tessellationDistances](const float& minTessellationDistance)
			{
				mDynamicLODDemo->SetTessellationDistances(minTessellationDistance, tessellationDistances.y);
			}, MinTessellatioDistance, tessellationDistances.y);

			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Home, Keys::End, tessellationDistances.y, 0.1f, [&, tessellationDistances](const float& maxTessellationDistance)
			{
				mDynamicLODDemo->SetTessellationDistances(tessellationDistances.x, maxTessellationDistance);
			}, tessellationDistances.x);
		}
	}
}