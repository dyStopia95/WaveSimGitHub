#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "FpsComponent.h"
#include "AnimationDemo.h"
#include "Grid.h"
#include "FirstPersonCamera.h"
#include "SamplerStates.h"
#include "RasterizerStates.h"
#include "VectorHelper.h"
#include "ImGuiComponent.h"
#include "imgui_impl_dx11.h"
#include "UtilityWin32.h"
#include "AnimationClip.h"
#include "AnimationPlayer.h"

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

		mAnimationDemo = make_shared<AnimationDemo>(*this, camera);
		mComponents.push_back(mAnimationDemo);

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
				stringstream fpsLabel;
				fpsLabel << setprecision(3) << "Frame Rate: " << mFpsComponent->FrameRate() << "    Total Elapsed Time: " << mGameTime.TotalGameTimeSeconds().count();
				ImGui::Text(fpsLabel.str().c_str());
			}
			
			ImGui::Text("Camera (WASD + Left-Click-Mouse-Look)");			
			ImGui::Text("Rotate Directional Light (Arrow Keys)");
			AddImGuiTextField("Toggle Grid (G): "s, (mGrid->Visible() ? "Visible"s : "Not Visible"s));
			AddImGuiTextField("Ambient Light Intensity (+PgUp/-PgDown): "s, mAnimationDemo->AmbientLightIntensity(), 2);
			AddImGuiTextField("Directional Light Intensity (+Home/-End): "s, mAnimationDemo->DirectionalLightIntensity(), 2);
			
			auto& currentClip = mAnimationDemo->AnimationPlayer()->CurrentClip();
			AddImGuiTextField("Animation Name: "s, currentClip->Name());
			AddImGuiTextField("Animation Duration: "s, currentClip->Duration(), 2);
			AddImGuiTextField("Animation Ticks Per Second: "s, currentClip->TicksPerSecond());
			AddImGuiTextField("Animation # of Keyframes: "s, currentClip->KeyframeCount());
			ImGui::Text("Restart Animation (R)");
			
			if (mAnimationDemo->ManualAdvanceEnabled())
			{
				AddImGuiTextField("Current Keyframe (-4/+6)/(Single-Step -1/+3): "s, mAnimationDemo->AnimationPlayer()->CurrentKeyframe());
			}
			else
			{
				if (!mAnimationDemo->InterpolationEnabled())
				{
					AddImGuiTextField("Current Keyframe: "s, mAnimationDemo->AnimationPlayer()->CurrentKeyframe());
				}

				AddImGuiTextField("Current Animation Time: "s, mAnimationDemo->AnimationPlayer()->CurrentTime());
				AddImGuiTextField("Animation Status (P): "s, (mAnimationDemo->AnimationPlayer()->IsPlayingClip() ? "Playing"s : "Paused"s));
			}

			AddImGuiTextField("Toggle Frame Advance Mode (Space): "s, (mAnimationDemo->ManualAdvanceEnabled() ? "Manual" : "Auto"));
			AddImGuiTextField("Keyframe Interpolation (I): "s, (mAnimationDemo->InterpolationEnabled() ? "Enabled" : "Disabled"));

			ImGui::End();
		});
		imGui->AddRenderBlock(helpTextImGuiRenderBlock);

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();
		
		camera->SetPosition(0.0f, 0.0f, 1.0f);
		mAmbientLightIntensity = mAnimationDemo->AmbientLightIntensity();
		mDirectionalLightIntensity = mAnimationDemo->DirectionalLightIntensity();
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

		UpdateAmbientLightIntensity(gameTime);
		UpdateDirectionalLight(gameTime);
		UpdateAnimationOptions();

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
		mAnimationDemo = nullptr;
		RasterizerStates::Shutdown();
		SamplerStates::Shutdown();
		Game::Shutdown();		
	}

	void RenderingGame::Exit()
	{
		PostQuitMessage(0);
	}

	void RenderingGame::UpdateAmbientLightIntensity(const GameTime& gameTime)
	{
		const float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();
		float ambientIntensity = mAnimationDemo->AmbientLightIntensity();
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::PageUp, Keys::PageDown, ambientIntensity, elapsedTime, [&](const float& ambientIntensity)
		{
			mAnimationDemo->SetAmbientLightIntensity(ambientIntensity);
		}, 0.0f, 1.0f);
	}

	void RenderingGame::UpdateDirectionalLight(const GameTime& gameTime)
	{
		float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();

		// Rotate directional light
		{
			bool updateRotation = false;
			auto updateRotationFunc = [&updateRotation](const float&) { updateRotation = true; };
			const float RotationRate = XM_2PI * elapsedTime;
			XMFLOAT2 rotationAmount = Vector2Helper::Zero;
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Left, Keys::Right, rotationAmount.x, RotationRate, updateRotationFunc);
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Up, Keys::Down, rotationAmount.y, RotationRate, updateRotationFunc);

			if (updateRotation)
			{
				mAnimationDemo->RotateDirectionalLight(rotationAmount);
			}
		}

		// Update light intensity
		{
			float intensity = mAnimationDemo->DirectionalLightIntensity();
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Home, Keys::End, intensity, elapsedTime, [&](const float& intensity)
			{
				mAnimationDemo->SetDirectionalLightIntensity(intensity);
			}, 0.0f, 1.0f);
		}
	}

	void RenderingGame::UpdateAnimationOptions()
	{
		if (mKeyboard->WasKeyPressedThisFrame(Keys::Space))
		{
			mAnimationDemo->ToggleManualAdvance();
		}

		if (mKeyboard->WasKeyPressedThisFrame(Keys::R))
		{
			mAnimationDemo->RestartClip();
		}

		if (mKeyboard->WasKeyPressedThisFrame(Keys::I))
		{
			mAnimationDemo->ToggleInterpolation();
		}

		if (mAnimationDemo->ManualAdvanceEnabled())
		{
			if (mKeyboard->WasKeyPressedThisFrame(Keys::NumPad1) || mKeyboard->IsKeyDown(Keys::NumPad4))
			{
				mAnimationDemo->DecrementCurrentKeyframe();
			}
			else if (mKeyboard->WasKeyPressedThisFrame(Keys::NumPad3) || mKeyboard->IsKeyDown(Keys::NumPad6))
			{
				mAnimationDemo->IncrementCurrentKeyframe();
			}
		}
		else if (mKeyboard->WasKeyPressedThisFrame(Keys::P))
		{
			mAnimationDemo->TogglePause();
		}
	}
}