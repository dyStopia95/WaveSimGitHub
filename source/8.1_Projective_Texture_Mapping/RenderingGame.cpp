#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "FpsComponent.h"
#include "Skybox.h"
#include "ProjectiveTextureMappingDemo.h"
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
		
		mProjectiveTextureMappingDemo = make_shared<ProjectiveTextureMappingDemo>(*this, camera);
		mComponents.push_back(mProjectiveTextureMappingDemo);

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
			ImGui::Text("Move Projector (Num-Pad 8/2, 4/6, 3/9)");
			ImGui::Text("Rotate Projector (Arrow Keys)");
			AddImGuiTextField("Toggle Grid (G): "s, (mGrid->Visible() ? "Visible"s : "Not Visible"s));
			AddImGuiTextField("Toggle Skybox (K): "s, (mSkybox->Visible() ? "Visible"s : "Not Visible"s));
			AddImGuiTextField("Ambient Light Intensity (+PgUp/-PgDown): "s, mProjectiveTextureMappingDemo->AmbientLightIntensity(), 2);
			AddImGuiTextField("Point Light Intensity (+Home/-End): "s, mProjectiveTextureMappingDemo->PointLightIntensity(), 2);
			AddImGuiTextField("Point Light Radius (+B/-N): "s, mProjectiveTextureMappingDemo->PointLightRadius(), 2);
			AddImGuiTextField("Draw Mode (Space): "s, mProjectiveTextureMappingDemo->DrawModeString());

			ImGui::End();
		});
		imGui->AddRenderBlock(helpTextImGuiRenderBlock);

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();
	
		camera->SetPosition(0.0f, 5.0f, 20.0f);
		mSkybox->SetVisible(false);
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
	
		UpdateDrawMode();
		UpdateAmbientLightIntensity(gameTime);
		UpdateProjector(gameTime);

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
		mSkybox = nullptr;
		mProjectiveTextureMappingDemo = nullptr;
		RasterizerStates::Shutdown();
		SamplerStates::Shutdown();
		Game::Shutdown();		
	}

	void RenderingGame::Exit()
	{
		PostQuitMessage(0);
	}

	void RenderingGame::UpdateDrawMode()
	{
		using namespace std::placeholders;
		auto drawMode = mProjectiveTextureMappingDemo->DrawMode();		
		IncrementEnumValue<ProjectiveTextureMappingDrawModes>(*mKeyboard, Keys::Space, drawMode, bind(&ProjectiveTextureMappingDemo::SetDrawMode, mProjectiveTextureMappingDemo, _1), ProjectiveTextureMappingDrawModes::End);
	}

	void RenderingGame::UpdateAmbientLightIntensity(const GameTime& gameTime)
	{
		const float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();
		float ambientIntensity = mProjectiveTextureMappingDemo->AmbientLightIntensity();
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::PageUp, Keys::PageDown, ambientIntensity, elapsedTime, [&](const float& ambientIntensity)
		{
			mProjectiveTextureMappingDemo->SetAmbientLightIntensity(ambientIntensity);
		}, 0.0f, 1.0f);
	}

	void RenderingGame::UpdateProjector(const Library::GameTime& gameTime)
	{
		float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();

		// Move projector
		{
			bool updatePosition = false;
			auto updatePositionFunc = [&updatePosition](const float&) { updatePosition = true; };
			const float ProjectorMovementRate = 10.0f * elapsedTime;
			XMFLOAT3 movementAmount = Vector3Helper::Zero;
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::NumPad6, Keys::NumPad4, movementAmount.x, ProjectorMovementRate, updatePositionFunc);
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::NumPad9, Keys::NumPad3, movementAmount.y, ProjectorMovementRate, updatePositionFunc);
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::NumPad2, Keys::NumPad8, movementAmount.z, ProjectorMovementRate, updatePositionFunc);

			if (updatePosition)
			{
				mProjectiveTextureMappingDemo->SetProjectorPosition(mProjectiveTextureMappingDemo->ProjectorPositionVector() + XMLoadFloat3(&movementAmount));
			}
		}

		// Rotate projector
		{
			bool updateRotation = false;
			auto updateRotationFunc = [&updateRotation](const float&) { updateRotation = true; };
			const float RotationRate = XM_2PI * elapsedTime;
			XMFLOAT2 rotationAmount = Vector2Helper::Zero;
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Left, Keys::Right, rotationAmount.x, RotationRate, updateRotationFunc);
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Up, Keys::Down, rotationAmount.y, RotationRate, updateRotationFunc);

			if (updateRotation)
			{
				mProjectiveTextureMappingDemo->RotateProjector(rotationAmount);
			}
		}

		// Update point light intensity
		{
			float intensity = mProjectiveTextureMappingDemo->PointLightIntensity();
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Home, Keys::End, intensity, elapsedTime, [&](const float& intensity)
			{
				mProjectiveTextureMappingDemo->SetPointLightIntensity(intensity);
			}, 0.0f, 1.0f);
		}
		
		// Update point light radius
		{
			const float LightModulationRate = static_cast<float>(numeric_limits<uint8_t>::max()) * elapsedTime;
			float pointLightRadius = mProjectiveTextureMappingDemo->PointLightRadius();
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::B, Keys::N, pointLightRadius, LightModulationRate, [&](const float& pointLightRadius)
			{
				mProjectiveTextureMappingDemo->SetPointLightRadius(pointLightRadius);
			}, 0.0f);
		}
	}
}