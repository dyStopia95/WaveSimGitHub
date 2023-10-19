#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "FpsComponent.h"
#include "Skybox.h"
#include "GaussianBlurDemo.h"
#include "DiffuseLightingDemo.h"
#include "Grid.h"
#include "FirstPersonCamera.h"
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

			{
				stringstream gridVisibleLabel;
				gridVisibleLabel << "Toggle Grid (G): " << (mGrid->Visible() ? "Visible" : "Not Visible");
				ImGui::Text(gridVisibleLabel.str().c_str());
			}
			{
				stringstream skyboxVisibleLabel;
				skyboxVisibleLabel << "Toggle Skybox (K): " << (mSkybox->Visible() ? "Visible" : "Not Visible");
				ImGui::Text(skyboxVisibleLabel.str().c_str());
			}
			{
				stringstream ambientLightIntensityLabel;
				ambientLightIntensityLabel << setprecision(2) << "Ambient Light Intensity (+PgUp/-PgDown): " << mDiffuseLightingDemo->AmbientLightIntensity();
				ImGui::Text(ambientLightIntensityLabel.str().c_str());
			}
			{
				stringstream directionalLightIntensityLabel;
				directionalLightIntensityLabel << setprecision(2) << "Directional Light Intensity (+Home/-End): " << mDiffuseLightingDemo->DirectionalLightIntensity();
				ImGui::Text(directionalLightIntensityLabel.str().c_str());
			}
			{
				stringstream blurAmountLabel;
				blurAmountLabel << setprecision(2) << "Blur Amount (+Insert/-Delete): " << mGaussianBlurDemo->BlurAmount();
				ImGui::Text(blurAmountLabel.str().c_str());
			}

			ImGui::End();
		});
		mImGuiComponent->AddRenderBlock(helpTextImGuiRenderBlock);
		mImGuiComponent->Initialize();

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();

		mGaussianBlurDemo = make_shared<GaussianBlurDemo>(*this, camera);
		mGaussianBlurDemo->Initialize();
	
		camera->SetPosition(0.0f, 2.5f, 20.0f);
		mDiffuseLightingDemo = mGaussianBlurDemo->DiffuseLighting();
		mAmbientLightIntensity = mDiffuseLightingDemo->AmbientLightIntensity();
		mDirectionalLightIntensity = mDiffuseLightingDemo->DirectionalLightIntensity();
		mBlurAmount = mGaussianBlurDemo->BlurAmount();
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
		UpdateBlurAmount(gameTime);

		mGaussianBlurDemo->Update(gameTime);
		mImGuiComponent->Update(gameTime);

		Game::Update(gameTime);
	}

	void RenderingGame::Draw(const GameTime &gameTime)
	{		
		mGaussianBlurDemo->Draw(gameTime);
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
		mGaussianBlurDemo = nullptr;
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
		if (mKeyboard->IsKeyDown(Keys::PageUp) && mAmbientLightIntensity < 1.0f)
		{
			mAmbientLightIntensity += gameTime.ElapsedGameTimeSeconds().count();
			mAmbientLightIntensity = min(mAmbientLightIntensity, 1.0f);
			mDiffuseLightingDemo->SetAmbientLightIntensity(mAmbientLightIntensity);
		}
		else if (mKeyboard->IsKeyDown(Keys::PageDown) && mAmbientLightIntensity > 0.0f)
		{
			mAmbientLightIntensity -= gameTime.ElapsedGameTimeSeconds().count();
			mAmbientLightIntensity = max(mAmbientLightIntensity, 0.0f);
			mDiffuseLightingDemo->SetAmbientLightIntensity(mAmbientLightIntensity);
		}
	}

	void RenderingGame::UpdateDirectionalLight(const GameTime& gameTime)
	{
		float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();

		// Update light intensity
		if (mKeyboard->IsKeyDown(Keys::Home) && mDirectionalLightIntensity < 1.0f)
		{
			mDirectionalLightIntensity += elapsedTime;
			mDirectionalLightIntensity = min(mDirectionalLightIntensity, 1.0f);
			mDiffuseLightingDemo->SetDirectionalLightIntensity(mDirectionalLightIntensity);
		}
		else if (mKeyboard->IsKeyDown(Keys::End) && mDirectionalLightIntensity > 0.0f)
		{
			mDirectionalLightIntensity -= elapsedTime;
			mDirectionalLightIntensity = max(mDirectionalLightIntensity, 0.0f);
			mDiffuseLightingDemo->SetDirectionalLightIntensity(mDirectionalLightIntensity);
		}

		// Rotate light
		XMFLOAT2 rotationAmount = Vector2Helper::Zero;
		if (mKeyboard->IsKeyDown(Keys::Left))
		{
			rotationAmount.x += LightRotationRate.x * elapsedTime;
		}
		if (mKeyboard->IsKeyDown(Keys::Right))
		{
			rotationAmount.x -= LightRotationRate.x * elapsedTime;
		}
		if (mKeyboard->IsKeyDown(Keys::Up))
		{
			rotationAmount.y += LightRotationRate.y * elapsedTime;
		}
		if (mKeyboard->IsKeyDown(Keys::Down))
		{
			rotationAmount.y -= LightRotationRate.y * elapsedTime;
		}

		if (rotationAmount.x != 0.0f || rotationAmount.y != 0.0f)
		{
			mDiffuseLightingDemo->RotateDirectionalLight(rotationAmount);
		}
	}

	void RenderingGame::UpdateBlurAmount(const Library::GameTime & gameTime)
	{
		if (mKeyboard->IsKeyDown(Keys::Insert))
		{
			mBlurAmount += gameTime.ElapsedGameTimeSeconds().count();
			mGaussianBlurDemo->SetBlurAmount(mBlurAmount);
		}
		else if (mKeyboard->IsKeyDown(Keys::Delete) && mBlurAmount > 0.0f)
		{
			mBlurAmount -= gameTime.ElapsedGameTimeSeconds().count();
			mBlurAmount = max(mBlurAmount, 0.0f);
			mGaussianBlurDemo->SetBlurAmount(mBlurAmount);
		}
	}
}