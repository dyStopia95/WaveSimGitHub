#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "FpsComponent.h"
#include "BasicTessellationDemo.h"
#include "OrthographicCamera.h"
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

		auto camera = make_shared<OrthographicCamera>(*this, 3.0f, 3.0f);
		mComponents.push_back(camera);
		mServices.AddService(Camera::TypeIdClass(), camera.get());

		mBasicTessellationDemo = make_shared<BasicTessellationDemo>(*this, camera);
		mComponents.push_back(mBasicTessellationDemo);

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

				AddImGuiTextField("Topology (T): "s, (mBasicTessellationDemo->ShowQuadTopology() ? "Quadrilateral"s : "Triangle"));
				AddImGuiTextField("Uniform Tessellation (Space): "s, (mBasicTessellationDemo->UseUniformTessellation() ? "True"s : "False"s));
				
				{
					string edgeFactorControls;
					string insideFactorControls;
					if (mBasicTessellationDemo->UseUniformTessellation())
					{
						edgeFactorControls = "+Up/-Down"s;
					}
					else
					{
						if (mBasicTessellationDemo->ShowQuadTopology())
						{
							edgeFactorControls = "+U/-H, +I/-J, +O/-K, +P/-L"s;
							insideFactorControls = "+V/-B, +N/-M";
						}
						else
						{
							edgeFactorControls = "+U/-H, +I/-J, +O/-K"s;
							insideFactorControls = "+V/-B";
						}
					}

					ostringstream edgeFactorsLabel;
					edgeFactorsLabel << "Edge Factors ("s << edgeFactorControls << "): ["s;
					edgeFactorsLabel << Join(mBasicTessellationDemo->EdgeFactors(), ", ").str() << "]"s;
					ImGui::Text(edgeFactorsLabel.str().c_str());

					ostringstream insideFactorsLabel;
					insideFactorsLabel << "Inside Factors (" << insideFactorControls << "): ["s;
					insideFactorsLabel << Join(mBasicTessellationDemo->InsideFactors(), ", ").str() << "]"s;
					ImGui::Text(insideFactorsLabel.str().c_str());
				}

				ImGui::End();
			});
		imGui->AddRenderBlock(helpTextImGuiRenderBlock);

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();

		camera->SetPosition(0.0f, 0.0f, 1.0f);
	}

	void RenderingGame::Update(const GameTime &gameTime)
	{
		if (mKeyboard->WasKeyPressedThisFrame(Keys::Escape))
		{
			Exit();
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
		mFpsComponent = nullptr;
		mBasicTessellationDemo = nullptr;
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
		if (mKeyboard->WasKeyPressedThisFrame(Keys::T))
		{
			mBasicTessellationDemo->ToggleTopology();
		}

		if (mKeyboard->WasKeyPressedThisFrame(Keys::Space))
		{
			mBasicTessellationDemo->ToggleUseUniformTessellation();
		}

		// Update uniform tessellation factors
		const float MinTessellationFactor = 2.0f;
		const float MaxTessellationFactor = 64.0f;
		if (mBasicTessellationDemo->UseUniformTessellation())
		{
			float edgeFactor = mBasicTessellationDemo->EdgeFactors()[0];
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Up, Keys::Down, edgeFactor, 1, [&](const float& edgeFactor)
			{
				mBasicTessellationDemo->SetUniformEdgeFactors(edgeFactor);
			}, MinTessellationFactor, MaxTessellationFactor);
		}
		else
		{
			// Update non-uniform edge factors
			{
				float edgeFactor = mBasicTessellationDemo->EdgeFactors()[0];
				UpdateValueWithKeyboard<float>(*mKeyboard, Keys::U, Keys::H, edgeFactor, 1, [&](const float& edgeFactor)
				{
					mBasicTessellationDemo->SetEdgeFactor(edgeFactor, 0);
				}, MinTessellationFactor, MaxTessellationFactor);
			}
			{
				float edgeFactor = mBasicTessellationDemo->EdgeFactors()[1];
				UpdateValueWithKeyboard<float>(*mKeyboard, Keys::I, Keys::J, edgeFactor, 1, [&](const float& edgeFactor)
				{
					mBasicTessellationDemo->SetEdgeFactor(edgeFactor, 1);
				}, MinTessellationFactor, MaxTessellationFactor);
			}
			{
				float edgeFactor = mBasicTessellationDemo->EdgeFactors()[2];
				UpdateValueWithKeyboard<float>(*mKeyboard, Keys::O, Keys::K, edgeFactor, 1, [&](const float& edgeFactor)
				{
					mBasicTessellationDemo->SetEdgeFactor(edgeFactor, 2);
				}, MinTessellationFactor, MaxTessellationFactor);
			}

			if (mBasicTessellationDemo->ShowQuadTopology())
			{
				float edgeFactor = mBasicTessellationDemo->EdgeFactors()[3];
				UpdateValueWithKeyboard<float>(*mKeyboard, Keys::P, Keys::L, edgeFactor, 1, [&](const float& edgeFactor)
				{
					mBasicTessellationDemo->SetEdgeFactor(edgeFactor, 3);
				}, MinTessellationFactor, MaxTessellationFactor);
			}

			// Update non-uniform inside factors
			{
				float insideFactor = mBasicTessellationDemo->InsideFactors()[0];
				UpdateValueWithKeyboard<float>(*mKeyboard, Keys::V, Keys::B, insideFactor, 1, [&](const float& insideFactor)
					{
						mBasicTessellationDemo->SetInsideFactor(insideFactor, 0);
					}, MinTessellationFactor, MaxTessellationFactor);
			}

			if (mBasicTessellationDemo->ShowQuadTopology())
			{
				// Update non-uniform inside factors
				float insideFactor = mBasicTessellationDemo->InsideFactors()[1];
				UpdateValueWithKeyboard<float>(*mKeyboard, Keys::N, Keys::M, insideFactor, 1, [&](const float& insideFactor)
				{
					mBasicTessellationDemo->SetInsideFactor(insideFactor, 1);
				}, MinTessellationFactor, MaxTessellationFactor);
			}
		}
	}

	ostringstream RenderingGame::Join(const span<const float>& values, const string& delimiter)
	{
		ostringstream out;
		std::copy(values.begin(), values.end() - 1, ostream_iterator<float>(out, delimiter.c_str()));
		out << *(values.end() - 1);

		return out;
	}
}