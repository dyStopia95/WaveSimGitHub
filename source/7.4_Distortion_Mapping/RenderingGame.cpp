#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "FpsComponent.h"
#include "Skybox.h"
#include "DistortionMappingDemo.h"
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
			AddImGuiTextField("Active Distortion Map (Space): "s, mDistortionMappingDemo->DistortionMapString());
			AddImGuiTextField("Displacement Scale (+Insert/-Delete): "s, mDistortionMappingDemo->DisplacementScale(), 2);

			ImGui::End();
		});
		mImGuiComponent->AddRenderBlock(helpTextImGuiRenderBlock);
		mImGuiComponent->Initialize();

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();

		mDistortionMappingDemo = make_shared<DistortionMappingDemo>(*this, camera);
		mDistortionMappingDemo->Initialize();
	
		camera->SetPosition(0.0f, 2.5f, 20.0f);
		mDiffuseLightingDemo = mDistortionMappingDemo->DiffuseLighting();	
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
	
		UpdateAmbientLightIntensity(gameTime);
		UpdateDirectionalLight(gameTime);
		UpdateDistortionMapping(gameTime);

		mDistortionMappingDemo->Update(gameTime);
		mImGuiComponent->Update(gameTime);

		Game::Update(gameTime);
	}

	void RenderingGame::Draw(const GameTime &gameTime)
	{		
		mDistortionMappingDemo->Draw(gameTime);
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
		mDistortionMappingDemo = nullptr;
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
		float ambientIntensity = mDiffuseLightingDemo->AmbientLightIntensity();
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::PageUp, Keys::PageDown, ambientIntensity, elapsedTime, [&](const float& ambientIntensity)
		{
			mDiffuseLightingDemo->SetAmbientLightIntensity(ambientIntensity);
		}, 0.0f, 1.0f);
	}

	void RenderingGame::UpdateDirectionalLight(const GameTime& gameTime)
	{
		const float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();
		
		// Update light intensity
		float directionalLightIntensity = mDiffuseLightingDemo->DirectionalLightIntensity();
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Home, Keys::End, directionalLightIntensity, elapsedTime, [&](const float& directionalLightIntensity)
		{
			mDiffuseLightingDemo->SetDirectionalLightIntensity(directionalLightIntensity);
		}, 0.0f, 1.0f);

		// Rotate light
		bool updateLightRotation = false;
		auto updateFunc = [&updateLightRotation](const float&) { updateLightRotation = true; };
		XMFLOAT2 rotationAmount = Vector2Helper::Zero;
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Left, Keys::Right, rotationAmount.x, LightRotationRate.x * elapsedTime, updateFunc);
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Up, Keys::Down, rotationAmount.y, LightRotationRate.y * elapsedTime, updateFunc);

		if (updateLightRotation)
		{
			mDiffuseLightingDemo->RotateDirectionalLight(rotationAmount);
		}
	}

	void RenderingGame::UpdateDistortionMapping(const GameTime& gameTime)
	{
		using namespace std::placeholders;
		auto distortionMap = mDistortionMappingDemo->DistortionMap();
		IncrementEnumValue<DistortionMaps>(*mKeyboard, Keys::Space, distortionMap, bind(&DistortionMappingDemo::SetDistortionMap, mDistortionMappingDemo, _1), DistortionMaps::End);

		const float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();
		float displacementScale = mDistortionMappingDemo->DisplacementScale();
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Insert, Keys::Delete, displacementScale, elapsedTime, [&](const float& displacementScale)
		{
			mDistortionMappingDemo->SetDisplacementScale(displacementScale);
		}, 0.0f);
	}
}