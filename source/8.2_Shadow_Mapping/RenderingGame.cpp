#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "FpsComponent.h"
#include "Skybox.h"
#include "ShadowMappingDemo.h"
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

		mCamera = make_shared<FirstPersonCamera>(*this);
		mComponents.push_back(mCamera);
		mServices.AddService(Camera::TypeIdClass(), mCamera.get());

		mGrid = make_shared<Grid>(*this, mCamera);
		mComponents.push_back(mGrid);

		mSkybox = make_shared<Skybox>(*this, mCamera, L"Textures\\Maskonaive2_1024.dds", 500.0f);
		mComponents.push_back(mSkybox);
		
		mShadowMappingDemo = make_shared<ShadowMappingDemo>(*this, mCamera);
		mComponents.push_back(mShadowMappingDemo);

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
			AddImGuiTextField("Ambient Light Intensity (+PgUp/-PgDown): "s, mShadowMappingDemo->AmbientLightIntensity(), 2);
			AddImGuiTextField("Point Light Intensity (+Home/-End): "s, mShadowMappingDemo->PointLightIntensity(), 2);
			AddImGuiTextField("Point Light Radius (+B/-N): "s, mShadowMappingDemo->PointLightRadius());
			AddImGuiTextField("Draw Mode (Space): "s, mShadowMappingDemo->DrawModeString());
			AddImGuiTextField("Depth Bias (+Z/-X): "s, mShadowMappingDemo->DepthBias());
			AddImGuiTextField("Slope-Scaled Depth Bias (+C/-V): "s, mShadowMappingDemo->SlopeScaledDepthBias());
			AddImGuiTextField("Camera Movement Rate (+U/-I): "s, mCamera->MovementRate());
			AddImGuiTextField("Camera Mouse Sensitivity (+O/-P): "s, mCamera->MouseSensitivity());

			ImGui::End();
		});
		imGui->AddRenderBlock(helpTextImGuiRenderBlock);

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();
	
		mCamera->SetPosition(0.0f, 5.0f, 20.0f);
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
		UpdateAmbientLightIntensity();
		UpdateProjector();
		UpdateDepthBias();
		UpdateCamera();

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
		mShadowMappingDemo = nullptr;
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
		auto drawMode = mShadowMappingDemo->DrawMode();		
		IncrementEnumValue<ShadowMappingDrawModes>(*mKeyboard, Keys::Space, drawMode, bind(&ShadowMappingDemo::SetDrawMode, mShadowMappingDemo, _1), ShadowMappingDrawModes::End);
	}

	void RenderingGame::UpdateAmbientLightIntensity()
	{
		const float IntensityRateOfChange = 0.017f;
		float ambientIntensity = mShadowMappingDemo->AmbientLightIntensity();
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::PageUp, Keys::PageDown, ambientIntensity, IntensityRateOfChange, [&](const float& ambientIntensity)
		{
			mShadowMappingDemo->SetAmbientLightIntensity(ambientIntensity);
		}, 0.0f, 1.0f);
	}

	void RenderingGame::UpdateProjector()
	{
		// Move and rotate projector
		{
			bool updatePosition = false;
			auto updatePositionFunc = [&updatePosition](const float&) { updatePosition = true; };
			const float MovementRate = 0.2f;
			XMFLOAT3 movementAmount = Vector3Helper::Zero;
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::NumPad6, Keys::NumPad4, movementAmount.x, MovementRate, updatePositionFunc);
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::NumPad9, Keys::NumPad3, movementAmount.y, MovementRate, updatePositionFunc);
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::NumPad8, Keys::NumPad2, movementAmount.z, MovementRate, updatePositionFunc);

			const float RotationRate = 0.1f;
			XMFLOAT2 rotationAmount = Vector2Helper::Zero;
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Left, Keys::Right, rotationAmount.x, RotationRate, updatePositionFunc);
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Up, Keys::Down, rotationAmount.y, RotationRate, updatePositionFunc);

			if (updatePosition)
			{
				const Camera& projector = mShadowMappingDemo->Projector();
				XMVECTOR rotationVector = XMLoadFloat2(&rotationAmount);
				
				mShadowMappingDemo->RotateProjector(rotationAmount);

				XMVECTOR position = XMLoadFloat3(&projector.Position());
				XMVECTOR movement = XMLoadFloat3(&movementAmount);

				XMVECTOR right = XMLoadFloat3(&projector.Right());
				XMVECTOR horizontalStrafe = right * XMVectorGetX(movement);
				position += horizontalStrafe;

				XMVECTOR up = XMLoadFloat3(&projector.Up());
				XMVECTOR verticalStrafe = up * XMVectorGetY(movement);
				position += verticalStrafe;

				XMVECTOR forward = XMLoadFloat3(&projector.Direction()) * XMVectorGetZ(movement);
				position += forward;

				mShadowMappingDemo->SetProjectorPosition(position);
			}
		}

		const float MovementRateRateOfChange = 1.0f;
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::U, Keys::I, mCamera->MovementRate(), MovementRateRateOfChange, nullptr, 0.0f);

		// Update point light intensity
		{
			float intensity = mShadowMappingDemo->PointLightIntensity();
			const float IntensityRateOfChange = 0.017f;
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Home, Keys::End, intensity, IntensityRateOfChange, [&](const float& intensity)
			{
				mShadowMappingDemo->SetPointLightIntensity(intensity);
			}, 0.0f, 1.0f);
		}
		
		// Update point light radius
		{
			const float RadiusRateOfChange = 4.25f;
			float pointLightRadius = mShadowMappingDemo->PointLightRadius();
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::B, Keys::N, pointLightRadius, RadiusRateOfChange, [&](const float& pointLightRadius)
			{
				mShadowMappingDemo->SetPointLightRadius(pointLightRadius);
			}, 0.0f);
		}
	}

	void RenderingGame::UpdateDepthBias()
	{
		const float DepthBiasModulationRate = 0.01f;

		// Update depth bias
		{
			
			float depthBias = mShadowMappingDemo->DepthBias();
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Z, Keys::X, depthBias, DepthBiasModulationRate, [&](const float& depthBias)
			{
				mShadowMappingDemo->SetDepthBias(depthBias);
			}, 0.0f);
		}

		// Update slope-scaled depth bias
		{
			float depthBias = mShadowMappingDemo->SlopeScaledDepthBias();
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::C, Keys::V, depthBias, DepthBiasModulationRate, [&](const float& depthBias)
			{
				mShadowMappingDemo->SetSlopeScaledDepthBias(depthBias);
			}, 0.0f);
		}
	}

	void RenderingGame::UpdateCamera()
	{
		const float MovementRateRateOfChange = 1.0f;
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::U, Keys::I, mCamera->MovementRate(), MovementRateRateOfChange, nullptr, 0.0f);

		const float MouseSensitivityRateOfChange = 0.001f;
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::O, Keys::P, mCamera->MouseSensitivity(), MouseSensitivityRateOfChange, nullptr, 0.0f);
	}
}