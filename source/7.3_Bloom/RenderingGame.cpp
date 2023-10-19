#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "FpsComponent.h"
#include "Skybox.h"
#include "BloomDemo.h"
#include "DiffuseLightingDemo.h"
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
			AddImGuiTextField("Ambient Light Intensity (+PgUp/-PgDown): "s, mDiffuseLightingDemo->AmbientLightIntensity(), 2);
			AddImGuiTextField("Directional Light Intensity (+Home/-End): "s, mDiffuseLightingDemo->DirectionalLightIntensity(), 2);
			AddImGuiTextField("Toggle Bloom (B): "s, (mBloomDemo->BloomEnabled() ? "Enabled"s : "Disabled"s));
			AddImGuiTextField("Bloom Draw Mode (Space): "s, mBloomDemo->DrawModeString());

			const BloomSettings& bloomSettings = mBloomDemo->GetBloomSettings();
			AddImGuiTextField("Blur Amount (+Insert/-Delete): "s, bloomSettings.BlurAmount , 2);
			AddImGuiTextField("Bloom Threshold (+O/-P): "s, bloomSettings.BloomThreshold, 2);
			AddImGuiTextField("Bloom Intensity (+U/-I): "s, bloomSettings.BloomIntensity, 2);
			AddImGuiTextField("Bloom Saturation (+T/-Y): "s, bloomSettings.BloomSaturation, 2);
			AddImGuiTextField("Scene Intensity (+Z/-X): "s, bloomSettings.SceneIntensity, 2);
			AddImGuiTextField("Scene Saturation (+C/-V): "s, bloomSettings.SceneSaturation, 2);

			ImGui::End();
		});
		mImGuiComponent->AddRenderBlock(helpTextImGuiRenderBlock);
		mImGuiComponent->Initialize();

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();

		mBloomDemo = make_shared<BloomDemo>(*this, camera);
		mBloomDemo->Initialize();
	
		camera->SetPosition(0.0f, 2.5f, 20.0f);
		mBloomSettings = mBloomDemo->GetBloomSettings();
		mDiffuseLightingDemo = mBloomDemo->DiffuseLighting();
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

		if (mKeyboard->WasKeyPressedThisFrame(Keys::B))
		{
			mBloomDemo->ToggleBloom();
		}

		UpdateAmbientLightIntensity(gameTime);
		UpdateDirectionalLight(gameTime);
		UpdateBloomSettings(gameTime);

		mBloomDemo->Update(gameTime);
		mImGuiComponent->Update(gameTime);

		Game::Update(gameTime);
	}

	void RenderingGame::Draw(const GameTime &gameTime)
	{		
		mBloomDemo->Draw(gameTime);
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
		mDiffuseLightingDemo = nullptr;
		mBloomDemo = nullptr;
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
		float ambientLightIntensity = mDiffuseLightingDemo->AmbientLightIntensity();
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::PageUp, Keys::PageDown, ambientLightIntensity, elapsedTime, [&](const float& ambientLightIntensity)
		{
			mDiffuseLightingDemo->SetAmbientLightIntensity(ambientLightIntensity);
		}, 0.0f, 1.0f);
	}

	void RenderingGame::UpdateDirectionalLight(const GameTime& gameTime)
	{
		const float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();
		float directionaLightIntensity = mDiffuseLightingDemo->DirectionalLightIntensity();
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Home, Keys::End, directionaLightIntensity, elapsedTime, [&](const float& directionaLightIntensity)
		{
			mDiffuseLightingDemo->SetDirectionalLightIntensity(directionaLightIntensity);
		}, 0.0f, 1.0f);

		// Rotate light
		const float RotationRate = XM_2PI * elapsedTime;
		bool updateLightRotation = false;
		auto updateFunc = [&updateLightRotation](const float&) { updateLightRotation = true; };
		XMFLOAT2 rotationAmount = Vector2Helper::Zero;
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Left, Keys::Right, rotationAmount.x, RotationRate, updateFunc);
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Up, Keys::Down, rotationAmount.y, RotationRate, updateFunc);

		if (updateLightRotation)
		{
			mDiffuseLightingDemo->RotateDirectionalLight(rotationAmount);
		}
	}

	void RenderingGame::UpdateBloomSettings(const GameTime& gameTime)
	{
		if (mKeyboard->WasKeyPressedThisFrame(Keys::Space))
		{
			BloomDrawModes activeMode = BloomDrawModes(static_cast<int>(mBloomDemo->DrawMode()) + 1);
			if (activeMode >= BloomDrawModes::End)
			{
				activeMode = BloomDrawModes(0);
			}

			mBloomDemo->SetDrawMode(activeMode);
		}
		
		bool updateBloomSettings = false;
		auto updateFunc = [&updateBloomSettings](const float&) { updateBloomSettings = true; };
		const float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();

		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Insert, Keys::Delete, mBloomSettings.BlurAmount, elapsedTime, updateFunc, 0.0f);
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::O, Keys::P, mBloomSettings.BloomThreshold, elapsedTime, updateFunc, 0.0f, 1.0f);
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::U, Keys::I, mBloomSettings.BloomIntensity, elapsedTime, updateFunc, 0.0f);
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::T, Keys::Y, mBloomSettings.BloomSaturation, elapsedTime, updateFunc, 0.0f);
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Z, Keys::X, mBloomSettings.SceneIntensity, elapsedTime, updateFunc, 0.0f);
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::C, Keys::V, mBloomSettings.SceneSaturation, elapsedTime, updateFunc, 0.0f);

		if (updateBloomSettings)
		{
			mBloomDemo->SetBloomSettings(mBloomSettings);
			updateBloomSettings = false;
		}
	}
}