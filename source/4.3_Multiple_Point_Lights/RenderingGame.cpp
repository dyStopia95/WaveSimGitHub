#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "FpsComponent.h"
#include "MultiplePointLightsDemo.h"
#include "Grid.h"
#include "FirstPersonCamera.h"
#include "SamplerStates.h"
#include "RasterizerStates.h"
#include "VectorHelper.h"
#include "ImGuiComponent.h"
#include "imgui_impl_dx11.h"
#include "UtilityWin32.h"
#include <limits>

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
		SamplerStates::Initialize(Direct3DDevice()); 
		RasterizerStates::Initialize(Direct3DDevice());

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

		mMultiplePointLightsDemo = make_shared<MultiplePointLightsDemo>(*this, camera);
		mComponents.push_back(mMultiplePointLightsDemo);

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
			ImGui::Text("Move Point Light (Num-Pad 8/2, 4/6, 3/9)");

			{
				stringstream gridVisibleLabel;
				gridVisibleLabel << "Toggle Grid (G): " << (mGrid->Visible() ? "Visible" : "Not Visible");
				ImGui::Text(gridVisibleLabel.str().c_str());
			}
			{
				stringstream animationEnabledLabel;
				animationEnabledLabel << "Toggle Animation (Space): " << (mMultiplePointLightsDemo->AnimationEnabled() ? "Enabled" : "Disabled");
				ImGui::Text(animationEnabledLabel.str().c_str());
			}
			{
				stringstream ambientLightIntensityLabel;
				ambientLightIntensityLabel << setprecision(2) << "Ambient Light Intensity (+PgUp/-PgDown): " << mMultiplePointLightsDemo->AmbientLightIntensity();
				ImGui::Text(ambientLightIntensityLabel.str().c_str());
			}
			{
				stringstream selectedLightLabel;
				selectedLightLabel << "Selected Point Light (+Tab/-Shift-Tab): " << mMultiplePointLightsDemo->SelectedLightIndex();
				ImGui::Text(selectedLightLabel.str().c_str());
			}
			{
				stringstream lightIntensityLabel;
				lightIntensityLabel << setprecision(2) << "Point Light Intensity (+Home/-End): " << mMultiplePointLightsDemo->SelectedLight().Color().x;
				ImGui::Text(lightIntensityLabel.str().c_str());
			}
			{
				stringstream pointLightRadiusLabel;
				pointLightRadiusLabel << "Point Light Radius (+B/-N): " << mMultiplePointLightsDemo->SelectedLight().Radius();
				ImGui::Text(pointLightRadiusLabel.str().c_str());
			}
			{
				stringstream specularPowerLabel;
				specularPowerLabel << "Specular Power (+O/-P): " << mMultiplePointLightsDemo->SpecularPower();
				ImGui::Text(specularPowerLabel.str().c_str());
			}			

			ImGui::End();
		});
		imGui->AddRenderBlock(helpTextImGuiRenderBlock);

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();
		
		camera->SetPosition(0.0f, 2.5f, 32.0f);		
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

		if (mKeyboard->WasKeyPressedThisFrame(Keys::Space))
		{
			mMultiplePointLightsDemo->ToggleAnimation();
		}

		if (mKeyboard->WasKeyPressedThisFrame(Keys::G))
		{
			mGrid->SetVisible(!mGrid->Visible());
		}

		UpdateAmbientLightIntensity(gameTime);
		UpdateSelectedPointLight();
		UpdatePointLight(gameTime);
		UpdateSpecularPower(gameTime);

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
		mMultiplePointLightsDemo = nullptr;
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
		auto lightIntensity = mMultiplePointLightsDemo->AmbientLightIntensity();
		if (mKeyboard->IsKeyDown(Keys::PageUp) && lightIntensity < 1.0f)
		{
			lightIntensity += gameTime.ElapsedGameTimeSeconds().count();
			lightIntensity = min(lightIntensity, 1.0f);
			mMultiplePointLightsDemo->SetAmbientLightIntensity(lightIntensity);
		}
		else if (mKeyboard->IsKeyDown(Keys::PageDown) && lightIntensity > 0.0f)
		{
			lightIntensity -= gameTime.ElapsedGameTimeSeconds().count();
			lightIntensity = max(lightIntensity, 0.0f);
			mMultiplePointLightsDemo->SetAmbientLightIntensity(lightIntensity);
		}
	}

	void RenderingGame::UpdatePointLight(const GameTime& gameTime)
	{
		float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();
		PointLight selectedLight = mMultiplePointLightsDemo->SelectedLight();
		bool updateSelectedLight = false;

		// Update light intensity
		auto lightIntensity = selectedLight.Color().x;
		if (mKeyboard->IsKeyDown(Keys::Home) && lightIntensity < 1.0f)
		{		
			lightIntensity += elapsedTime;
			lightIntensity = min(lightIntensity, 1.0f);
			selectedLight.SetColor(lightIntensity, lightIntensity, lightIntensity, 1.0f);
			updateSelectedLight = true;
		}
		else if (mKeyboard->IsKeyDown(Keys::End) && lightIntensity > 0.0f)
		{
			lightIntensity -= elapsedTime;
			lightIntensity = max(lightIntensity, 0.0f);
			selectedLight.SetColor(lightIntensity, lightIntensity, lightIntensity, 1.0f);
			updateSelectedLight = true;
		}

		// Move light
		XMFLOAT3 movementAmount = Vector3Helper::Zero;
		if (mKeyboard->IsKeyDown(Keys::NumPad4))
		{
			movementAmount.x = -1.0f;
		}

		if (mKeyboard->IsKeyDown(Keys::NumPad6))
		{
			movementAmount.x = 1.0f;
		}

		if (mKeyboard->IsKeyDown(Keys::NumPad9))
		{
			movementAmount.y = 1.0f;
		}

		if (mKeyboard->IsKeyDown(Keys::NumPad3))
		{
			movementAmount.y = -1.0f;
		}

		if (mKeyboard->IsKeyDown(Keys::NumPad8))
		{
			movementAmount.z = -1.0f;
		}

		if (mKeyboard->IsKeyDown(Keys::NumPad2))
		{
			movementAmount.z = 1.0f;
		}

		const float LightMovementRate = 10.0f;
		const float LightModulationRate = static_cast<float>(numeric_limits<uint8_t>::max());
		if (movementAmount.x != 0.0f || movementAmount.y != 0.0f || movementAmount.z != 0.0f)
		{
			XMVECTOR movement = XMLoadFloat3(&movementAmount) * LightMovementRate * elapsedTime;
			selectedLight.SetPosition(selectedLight.PositionVector() + movement);
			updateSelectedLight = true;
		}

		// Update the light's radius
		if (mKeyboard->IsKeyDown(Keys::B))
		{
			float radius = selectedLight.Radius() + LightModulationRate * elapsedTime;
			selectedLight.SetRadius(radius);
			updateSelectedLight = true;
		}
		else if (mKeyboard->IsKeyDown(Keys::N))
		{
			float radius = selectedLight.Radius() - LightModulationRate * elapsedTime;
			radius = max(radius, 0.0f);
			selectedLight.SetRadius(radius);
			updateSelectedLight = true;
		}

		if (updateSelectedLight)
		{
			mMultiplePointLightsDemo->UpdateSelectedLight(selectedLight);
		}
	}

	void RenderingGame::UpdateSpecularPower(const Library::GameTime & gameTime)
	{
		float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();

		auto specularPower = mMultiplePointLightsDemo->SpecularPower();
		const auto ModulationRate = numeric_limits<uint8_t>::max();
		if (mKeyboard->IsKeyDown(Keys::O) && specularPower < numeric_limits<uint8_t>::max())
		{
			specularPower += ModulationRate * elapsedTime;
			specularPower = min(specularPower, static_cast<float>(numeric_limits<uint8_t>::max()));
			mMultiplePointLightsDemo->SetSpecularPower(specularPower);
		}
		else if (mKeyboard->IsKeyDown(Keys::P) && specularPower > 1.0f)
		{
			specularPower -= ModulationRate * elapsedTime;
			specularPower = max(specularPower, 1.0f);
			mMultiplePointLightsDemo->SetSpecularPower(specularPower);
		}
	}

	void RenderingGame::UpdateSelectedPointLight()
	{
		if (mKeyboard->WasKeyPressedThisFrame(Keys::Tab))
		{
			size_t lightIndex = mMultiplePointLightsDemo->SelectedLightIndex();
			if (mKeyboard->IsKeyDown(Keys::LeftShift) || mKeyboard->IsKeyDown(Keys::RightShift))
			{
				lightIndex = (lightIndex == 0 ? mMultiplePointLightsDemo->PointLights().size() - 1 : --lightIndex);
			}
			else
			{
				lightIndex = (lightIndex == mMultiplePointLightsDemo->PointLights().size() - 1 ? 0 : ++lightIndex);
			}

			mMultiplePointLightsDemo->SelectLight(lightIndex);
		}
	}
}