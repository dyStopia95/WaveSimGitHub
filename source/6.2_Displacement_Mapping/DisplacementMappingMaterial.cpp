#include "pch.h"
#include "DisplacementMappingMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VertexDeclarations.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Texture2D.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace winrt;
using namespace DirectX;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(DisplacementMappingMaterial)

	DisplacementMappingMaterial::DisplacementMappingMaterial(Game& game, shared_ptr<Texture2D> colorMap, shared_ptr<Texture2D> normalMap) :
		Material(game), mColorMap(move(colorMap)), mDisplacementMap(move(normalMap))
	{
	}

	com_ptr<ID3D11SamplerState> DisplacementMappingMaterial::SamplerState() const
	{
		return mSamplerState;
	}

	void DisplacementMappingMaterial::SetSamplerState(com_ptr<ID3D11SamplerState> samplerState)
	{
		assert(samplerState != nullptr);
		mSamplerState = samplerState;
		Material::SetSamplerState(ShaderStages::PS, mSamplerState.get());
	}

	shared_ptr<Texture2D> DisplacementMappingMaterial::ColorMap() const
	{
		return mColorMap;
	}

	void DisplacementMappingMaterial::SetColorMap(shared_ptr<Texture2D> texture)
	{
		assert(texture != nullptr);
		mColorMap = move(texture);
		SetShaderResource(ShaderStages::PS, mColorMap->ShaderResourceView().get());
	}

	shared_ptr<Texture2D> DisplacementMappingMaterial::DisplacementMap() const
	{
		return mDisplacementMap;
	}

	void DisplacementMappingMaterial::SetDisplacementMap(shared_ptr<Texture2D> texture)
	{
		assert(texture != nullptr);
		mDisplacementMap = move(texture);
		SetShaderResource(ShaderStages::VS, mDisplacementMap->ShaderResourceView().get());
	}

	const XMFLOAT4& DisplacementMappingMaterial::AmbientColor() const
	{
		return mPixelCBufferPerFrameData.AmbientColor;
	}

	void DisplacementMappingMaterial::SetAmbientColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.AmbientColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const XMFLOAT3& DisplacementMappingMaterial::LightDirection() const
	{
		return mPixelCBufferPerFrameData.LightDirection;
	}

	void DisplacementMappingMaterial::SetLightDirection(const XMFLOAT3& direction)
	{
		mPixelCBufferPerFrameData.LightDirection = direction;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const XMFLOAT4& DisplacementMappingMaterial::LightColor() const
	{
		return mPixelCBufferPerFrameData.LightColor;
	}

	void DisplacementMappingMaterial::SetLightColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.LightColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const float DisplacementMappingMaterial::DisplacementScale() const
	{
		return mVertexCBufferPerObjectData.DisplacementScale;
	}

	void DisplacementMappingMaterial::SetDisplacementScale(float displacementScale)
	{
		mVertexCBufferPerObjectData.DisplacementScale = displacementScale;
		mVertexCBufferPerObjectDataDirty = true;
	}

	uint32_t DisplacementMappingMaterial::VertexSize() const
	{
		return sizeof(VertexPositionTextureNormal);
	}

	void DisplacementMappingMaterial::Initialize()
	{
		Material::Initialize();

		auto& content = mGame->Content();
		auto vertexShader = content.Load<VertexShader>(L"Shaders\\DisplacementMappingDemoVS.cso"s);
		SetShader(vertexShader);

		auto pixelShader = content.Load<PixelShader>(L"Shaders\\DisplacementMappingDemoPS.cso");
		SetShader(pixelShader);

		auto direct3DDevice = mGame->Direct3DDevice();
		vertexShader->CreateInputLayout<VertexPositionTextureNormal>(direct3DDevice);
		SetInputLayout(vertexShader->InputLayout());

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = sizeof(VertexCBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::VS, mVertexCBufferPerObject.get());

		constantBufferDesc.ByteWidth = sizeof(PixelCBufferPerFrame);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerFrame.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::PS, mPixelCBufferPerFrame.get());

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
	
		AddShaderResource(ShaderStages::VS, mDisplacementMap->ShaderResourceView().get());
		AddShaderResource(ShaderStages::PS, mColorMap->ShaderResourceView().get());
		AddSamplerState(ShaderStages::VS, mSamplerState.get());
		AddSamplerState(ShaderStages::PS, mSamplerState.get());
	}

	void DisplacementMappingMaterial::UpdateTransforms(FXMMATRIX worldViewProjectionMatrix, CXMMATRIX worldMatrix)
	{
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, worldViewProjectionMatrix);
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, worldMatrix);
		mVertexCBufferPerObjectDataDirty = true;
	}

	void DisplacementMappingMaterial::BeginDraw()
	{
		Material::BeginDraw();

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();

		if (mVertexCBufferPerObjectDataDirty)
		{
			mGame->Direct3DDeviceContext()->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
			mVertexCBufferPerObjectDataDirty = false;
		}

		if (mPixelCBufferPerFrameDataDirty)
		{
			direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
			mPixelCBufferPerFrameDataDirty = false;
		}
/*
		const auto vsConstantBuffers = mVertexCBufferPerObject.get();
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, &vsConstantBuffers);

		const auto vsShaderResources = mDisplacementMap->ShaderResourceView().get();
		direct3DDeviceContext->VSSetShaderResources(0, 1, &vsShaderResources);

		const auto psConstantBuffers = mPixelCBufferPerFrame.get();
		direct3DDeviceContext->PSSetConstantBuffers(0, 1, &psConstantBuffers);

		const auto psShaderResources = mColorMap->ShaderResourceView().get();
		direct3DDeviceContext->PSSetShaderResources(0, 1, &psShaderResources);

		const auto textureSamplers = mSamplerState.get();
		direct3DDeviceContext->VSSetSamplers(0, 1, &textureSamplers);
		direct3DDeviceContext->PSSetSamplers(0, 1, &textureSamplers);	*/	
	}
}