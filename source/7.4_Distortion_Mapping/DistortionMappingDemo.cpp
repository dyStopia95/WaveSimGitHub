#include "pch.h"
#include "DistortionMappingDemo.h"
#include "Game.h"
#include "DiffuseLightingDemo.h"
#include "FullScreenQuadMaterial.h"
#include "PixelShader.h"
#include "Texture2D.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace winrt;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	const map<DistortionMaps, string> DistortionMappingDemo::DistortionMapNames
	{
		{ DistortionMaps::Glass, "Glass"s },
		{ DistortionMaps::Text, "Text"s },
		{ DistortionMaps::NoDistortion, "No Distortion"s }
	};

	DistortionMappingDemo::DistortionMappingDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera),
		mRenderTarget(game), mFullScreenQuad(game)
	{
	}

	DistortionMappingDemo::~DistortionMappingDemo()
	{
	}

	shared_ptr<DiffuseLightingDemo> DistortionMappingDemo::DiffuseLighting() const
	{
		return mDiffuseLightingDemo;
	}

	float DistortionMappingDemo::DisplacementScale() const
	{
		return mPixelCBufferPerObjectData.DisplacementScale;
	}

	void DistortionMappingDemo::SetDisplacementScale(float displacementScale)
	{
		mPixelCBufferPerObjectData.DisplacementScale = displacementScale;
		mGame->Direct3DDeviceContext()->UpdateSubresource(mPixelCBufferPerObject.get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);
	}

	DistortionMaps DistortionMappingDemo::DistortionMap() const
	{
		return mActiveDistortionMap;
	}

	const string& DistortionMappingDemo::DistortionMapString() const
	{
		return DistortionMapNames.at(mActiveDistortionMap);
	}

	void DistortionMappingDemo::SetDistortionMap(DistortionMaps distortionMap)
	{
		mActiveDistortionMap = distortionMap;

		ID3D11ShaderResourceView* shaderResourceViews[] = { mRenderTarget.OutputTexture().get(), mDistortionMaps.at(mActiveDistortionMap)->ShaderResourceView().get() };
		mFullScreenQuad.Material()->SetTextures(shaderResourceViews);
	}

	void DistortionMappingDemo::Initialize()
	{
		mDiffuseLightingDemo = make_shared<DiffuseLightingDemo>(*mGame, mCamera);
		mDiffuseLightingDemo->Initialize();

		mFullScreenQuad.Initialize();
		auto fullScreenQuadMaterial = mFullScreenQuad.Material();
		auto pixelShader = mGame->Content().Load<PixelShader>(L"Shaders\\DistortionMappingPS.cso");
		fullScreenQuadMaterial->SetShader(pixelShader);

		auto& content = mGame->Content();
		mDistortionMaps[DistortionMaps::Glass] = content.Load<Texture2D>(L"Textures\\DistortionMapGlass.png"s);
		mDistortionMaps[DistortionMaps::Text] = content.Load<Texture2D>(L"Textures\\DistortionMapText.png"s);
		mDistortionMaps[DistortionMaps::NoDistortion] = content.Load<Texture2D>(L"Textures\\DistortionMapNone.png"s);

		ID3D11ShaderResourceView* shaderResourceViews[] = { mRenderTarget.OutputTexture().get(), mDistortionMaps.at(mActiveDistortionMap)->ShaderResourceView().get() };
		fullScreenQuadMaterial->SetTextures(shaderResourceViews);

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = sizeof(PixelCBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");
		fullScreenQuadMaterial->AddConstantBuffer(ShaderStages::PS, mPixelCBufferPerObject.get());
		mGame->Direct3DDeviceContext()->UpdateSubresource(mPixelCBufferPerObject.get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);
	}

	void DistortionMappingDemo::Update(const GameTime& gameTime)
	{
		mDiffuseLightingDemo->Update(gameTime);
	}

	void DistortionMappingDemo::Draw(const GameTime& gameTime)
	{
		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		
		// Render scene to off-screen render target
		mRenderTarget.Begin();

		direct3DDeviceContext->ClearRenderTargetView(mRenderTarget.RenderTargetView().get(), Colors::CornflowerBlue.f);
		direct3DDeviceContext->ClearDepthStencilView(mRenderTarget.DepthStencilView().get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		mGame->Game::Draw(gameTime);
		mDiffuseLightingDemo->Draw(gameTime);

		mRenderTarget.End();

		// Render off-screen texture to a full-screen quad with gaussian blurring
		direct3DDeviceContext->ClearRenderTargetView(mGame->RenderTargetView(), Colors::CornflowerBlue.f);
		direct3DDeviceContext->ClearDepthStencilView(mGame->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		mFullScreenQuad.Draw(gameTime);
		mFullScreenQuad.Material()->UnbindShaderResources<1>(ShaderStages::PS);
	}
}