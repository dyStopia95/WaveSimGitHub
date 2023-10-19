#include "pch.h"
#include "DistortionMaskingDemo.h"
#include "Game.h"
#include "FullScreenQuad.h"
#include "FullScreenQuadMaterial.h"
#include "PixelShader.h"
#include "PixelShaderReader.h"
#include "Texture2D.h"
#include "Utility.h"
#include "TexturedModelMaterial.h"
#include "Model.h"
#include "Mesh.h"
#include "VertexDeclarations.h"
#include "Camera.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace winrt;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	DistortionMaskingDemo::DistortionMaskingDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera),
		mSceneRenderTarget(game), mCutoutRenderTarget(game),
		mFullScreenQuad(game)
	{
	}

	DistortionMaskingDemo::~DistortionMaskingDemo()
	{
	}

	bool DistortionMaskingDemo::DrawCutoutModeEnabled() const
	{
		return mDrawCutoutModeEnabled;
	}

	void DistortionMaskingDemo::SetDrawCutoutModeEnabled(bool enabled)
	{
		mDrawCutoutModeEnabled = enabled;
	}

	void DistortionMaskingDemo::ToggledDrawCutoutModeEnabled()
	{
		mDrawCutoutModeEnabled = !mDrawCutoutModeEnabled;
	}

	float DistortionMaskingDemo::DisplacementScale() const
	{
		return mPixelCBufferPerObjectData.DisplacementScale;
	}

	void DistortionMaskingDemo::SetDisplacementScale(float displacementScale)
	{
		mPixelCBufferPerObjectData.DisplacementScale = displacementScale;
		mGame->Direct3DDeviceContext()->UpdateSubresource(mPixelCBufferPerObject.get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);
	}

	void DistortionMaskingDemo::Initialize()
	{
		auto direct3DDevice = mGame->Direct3DDevice();
		auto& content = mGame->Content();
		const auto model = content.Load<Model>(L"Models\\nickzucc_blogspot_male_fleshbust_L2_MID.OBJ.bin"s);
		Mesh* mesh = model->Meshes().at(0).get();
		VertexPositionTexture::CreateVertexBuffer(direct3DDevice, *mesh, not_null<ID3D11Buffer**>(mVertexBuffer.put()));
		mesh->CreateIndexBuffer(*direct3DDevice, not_null<ID3D11Buffer**>(mIndexBuffer.put()));
		mIndexCount = narrow<uint32_t>(mesh->Indices().size());

		mFullScreenQuad.Initialize();
		auto fullScreenQuadMaterial = mFullScreenQuad.Material();

		// Create the distortion masking shader with class
		auto classLinkage = Shader::CreateClassLinkage(direct3DDevice);
		PixelShaderWithClassLinkageReader pixelShaderContentReader(*mGame, classLinkage);
		auto pixelShader = content.Load<PixelShader>(L"Shaders\\DistortionMaskingPS.cso", false, pixelShaderContentReader);
		fullScreenQuadMaterial->SetShader(pixelShader);
		
		ThrowIfFailed(classLinkage->CreateClassInstance("CutoutDistortionShader", 0, 0, 0, 0, mShaderClassInstances[DistortionShaderClass::Cutout].put()));
		ThrowIfFailed(classLinkage->CreateClassInstance("CompositeDistortionShader", 0, 0, 0, 0, mShaderClassInstances[DistortionShaderClass::Composite].put()));
		ThrowIfFailed(classLinkage->CreateClassInstance("NoDistortionShader", 0, 0, 0, 0, mShaderClassInstances[DistortionShaderClass::NoDistortion].put()));

		mTexturedModelMaterial = make_shared<TexturedModelMaterial>(*mGame);
		mTexturedModelMaterial->Initialize();
		mTexturedModelMaterial->SetShader(pixelShader);
		mTexturedModelMaterial->SetShaderClassInstance(ShaderStages::PS, mShaderClassInstances.at(DistortionShaderClass::Cutout).get());

		mDistortionMap = content.Load<Texture2D>(L"Textures\\DistortionMapGlass.png"s);
		ID3D11ShaderResourceView* psShaderResources[] = { nullptr, mDistortionMap->ShaderResourceView().get() };
		mTexturedModelMaterial->AddShaderResources(ShaderStages::PS, psShaderResources);

		auto updateMaterialFunc = [this]() { mUpdateMaterial = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateMaterialFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateMaterialFunc);

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = sizeof(PixelCBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");
		mGame->Direct3DDeviceContext()->UpdateSubresource(mPixelCBufferPerObject.get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);

		XMStoreFloat4x4(&mWorldMatrix, XMMatrixScaling(0.1f, 0.1f, 0.1f));

		fullScreenQuadMaterial->SetAutoUnbindShaderResourcesEnabled(true);
	}

	void DistortionMaskingDemo::Draw(const GameTime& gameTime)
	{
		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		
		// Draw un-distorted background objects
		mSceneRenderTarget.Begin();

		direct3DDeviceContext->ClearRenderTargetView(mSceneRenderTarget.RenderTargetView().get(), Colors::Purple.f);
		direct3DDeviceContext->ClearDepthStencilView(mSceneRenderTarget.DepthStencilView().get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		mGame->Game::Draw(gameTime);

		mSceneRenderTarget.End();

		// Create distortion map "cutout" using 3D model
		mCutoutRenderTarget.Begin();

		direct3DDeviceContext->ClearRenderTargetView(mCutoutRenderTarget.RenderTargetView().get(), CutoutZeroColor.f);
		direct3DDeviceContext->ClearDepthStencilView(mCutoutRenderTarget.DepthStencilView().get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
				
		if (mUpdateMaterial)
		{
			const XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
			const XMMATRIX wvp = XMMatrixTranspose(worldMatrix * mCamera->ViewProjectionMatrix());
			mTexturedModelMaterial->UpdateTransform(wvp);
			mUpdateMaterial = false;
		}

		mTexturedModelMaterial->DrawIndexed(not_null<ID3D11Buffer*>(mVertexBuffer.get()), not_null<ID3D11Buffer*>(mIndexBuffer.get()), mIndexCount);

		mCutoutRenderTarget.End();

		auto fullScreenQuadMaterial = mFullScreenQuad.Material();

		// Final Draw
		direct3DDeviceContext->ClearRenderTargetView(mGame->RenderTargetView(), Colors::Purple.f);
		direct3DDeviceContext->ClearDepthStencilView(mGame->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		if (mDrawCutoutModeEnabled)
		{
			fullScreenQuadMaterial->SetShaderClassInstance(ShaderStages::PS, mShaderClassInstances.at(DistortionShaderClass::NoDistortion).get());
			fullScreenQuadMaterial->SetTexture(mCutoutRenderTarget.OutputTexture().get());
			mFullScreenQuad.Draw(gameTime);
		}
		else
		{
			fullScreenQuadMaterial->SetShaderClassInstance(ShaderStages::PS, mShaderClassInstances.at(DistortionShaderClass::Composite).get());
			ID3D11ShaderResourceView* shaderResourceViews[] = { mSceneRenderTarget.OutputTexture().get(), mCutoutRenderTarget.OutputTexture().get() };
			fullScreenQuadMaterial->SetTextures(shaderResourceViews);
			
			fullScreenQuadMaterial->SetUpdateMaterialCallback([&]
			{
				auto psConstantBuffers = mPixelCBufferPerObject.get();
				mGame->Direct3DDeviceContext()->PSSetConstantBuffers(0, 1, &psConstantBuffers);
			});

			mFullScreenQuad.Draw(gameTime);
		}
	}
}