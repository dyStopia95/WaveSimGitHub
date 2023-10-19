#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "FpsComponent.h"
#include "FirstPersonCamera.h"
#include "Grid.h"
#include "SamplerStates.h"
#include "RasterizerStates.h"
#include "VectorHelper.h"
#include "ImGuiComponent.h"
#include "imgui_impl_dx11.h"
#include "UtilityWin32.h"
#include "HeightmapTessellationDemo.h"

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
		mGrid->SetVisible(false);
		mComponents.push_back(mGrid);

		mHeightmapTessellationDemo = make_shared<HeightmapTessellationDemo>(*this, camera);
		mComponents.push_back(mHeightmapTessellationDemo);

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

				{
					ostringstream edgeFactorsLabel;
					edgeFactorsLabel << "Uniform Tessellation Factor (+Up/-Down): "s << mHeightmapTessellationDemo->EdgeFactors()[0];
					ImGui::Text(edgeFactorsLabel.str().c_str());
				}

				{
					ostringstream displacementScaleLabel;
					displacementScaleLabel << "Displacement Scale (+PageUp/-PageDown): "s << mHeightmapTessellationDemo->DisplacementScale();
					ImGui::Text(displacementScaleLabel.str().c_str());
				}

				AddImGuiTextField("Toggle Animation (Space): "s, (mHeightmapTessellationDemo->AnimationEnabled() ? "True"s : "False"s));

				ImGui::End();
			});
		imGui->AddRenderBlock(helpTextImGuiRenderBlock);

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();
		camera->SetPosition(0.0f, 5.0f, 35.0f);
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

		if (mKeyboard->WasKeyPressedThisFrame(Keys::Space))
		{
			mHeightmapTessellationDemo->SetAnimationEnabled(!mHeightmapTessellationDemo->AnimationEnabled());
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
		mHeightmapTessellationDemo = nullptr;
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
		// Update uniform tessellation factors
		const float MinTessellationFactor = 1.0f;
		const float MaxTessellationFactor = 64.0f;
		float edgeFactor = mHeightmapTessellationDemo->EdgeFactors()[0];
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Up, Keys::Down, edgeFactor, 1, [&](const float& edgeFactor)
		{
			mHeightmapTessellationDemo->SetUniformFactors(edgeFactor);
		}, MinTessellationFactor, MaxTessellationFactor);

		float displacementScale = mHeightmapTessellationDemo->DisplacementScale();
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::PageUp, Keys::PageDown, displacementScale, 0.05f, [&](const float& displacementScale)
			{
				mHeightmapTessellationDemo->SetDisplacementScale(displacementScale);
			});
	}
}