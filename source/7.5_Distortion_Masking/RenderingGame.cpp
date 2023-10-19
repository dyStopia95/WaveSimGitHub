#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "FpsComponent.h"
#include "Skybox.h"
#include "DistortionMaskingDemo.h"
#include "Grid.h"
#include "FirstPersonCamera.h"
#include "SamplerStates.h"
#include "RasterizerStates.h"
#include "VectorHelper.h"
#include "ImGuiComponent.h"
#include "imgui_impl_dx11.h"
#include "Utility.h"
#include "UtilityWin32.h"

using namespace std;
using namespace DirectX;
using namespace Library;

namespace Rendering
{
	RenderingGame::RenderingGame(function<void*()> getWindowCallback, function<void(SIZE&)> getRenderTargetSizeCallback) :
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

		mSkybox = make_shared<Skybox>(*this, camera, L"Textures\\Maskonaive2_1024.dds", 500.0f);
		mComponents.push_back(mSkybox);
		
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
			
			ImGui::Text("Camera (WASD + Left-Click-Mouse-Look)");			
			ImGui::Text("Rotate Directional Light (Arrow Keys)");
			AddImGuiTextField("Toggle Grid (G): "s, (mGrid->Visible() ? "Visible"s : "Not Visible"s));
			AddImGuiTextField("Toggle Skybox (K): "s, (mSkybox->Visible() ? "Visible"s : "Not Visible"s));
			AddImGuiTextField("Draw Cutout Mode (Space): "s, (mDistortionMaskingDemo->DrawCutoutModeEnabled() ? "Enabled"s : "Disabled"s));
			AddImGuiTextField("Displacement Scale (+Insert/-Delete): "s, mDistortionMaskingDemo->DisplacementScale(), 2);

			ImGui::End();
		});
		mImGuiComponent->AddRenderBlock(helpTextImGuiRenderBlock);
		mImGuiComponent->Initialize();

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();

		mDistortionMaskingDemo = make_shared<DistortionMaskingDemo>(*this, camera);
		mDistortionMaskingDemo->Initialize();
	
		camera->SetPosition(0.0f, 2.5f, 20.0f);
	}

	void RenderingGame::Update(const GameTime &gameTime)
	{
		if (mKeyboard->WasKeyPressedThisFrame(Keys::Escape) || mGamePad->WasButtonPressedThisFrame(GamePadButtons::Back))
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

		if (mKeyboard->WasKeyPressedThisFrame(Keys::K))
		{
			mSkybox->SetVisible(!mSkybox->Visible());
		}

		if (mKeyboard->WasKeyPressedThisFrame(Keys::Space))
		{
			mDistortionMaskingDemo->ToggledDrawCutoutModeEnabled();
		}
	
		UpdateDistortionMapping(gameTime);

		mDistortionMaskingDemo->Update(gameTime);
		mImGuiComponent->Update(gameTime);

		Game::Update(gameTime);
	}

	void RenderingGame::Draw(const GameTime &gameTime)
	{		
		mDistortionMaskingDemo->Draw(gameTime);
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
		mGrid = nullptr;
		mFpsComponent = nullptr;
		mSkybox = nullptr;
		mImGuiComponent->Shutdown();
		mImGuiComponent = nullptr;
		mDistortionMaskingDemo = nullptr;
		RasterizerStates::Shutdown();
		SamplerStates::Shutdown();
		Game::Shutdown();		
	}

	void RenderingGame::Exit()
	{
		PostQuitMessage(0);
	}

	void RenderingGame::UpdateDistortionMapping(const GameTime& gameTime)
	{
		const float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();
		float displacementScale = mDistortionMaskingDemo->DisplacementScale();
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Insert, Keys::Delete, displacementScale, elapsedTime, [&](const float& displacementScale)
		{
			mDistortionMaskingDemo->SetDisplacementScale(displacementScale);
		}, 0.0f);
	}
}