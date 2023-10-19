#include "pch.h"
#include "SkinnedModelMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VertexDeclarations.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Texture2D.h"

using namespace std;
using namespace gsl;
using namespace std::string_literals;
using namespace DirectX;
using namespace winrt;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(SkinnedModelMaterial)

	SkinnedModelMaterial::SkinnedModelMaterial(Game& game, shared_ptr<Texture2D> texture) :
		Material(game), mTexture(move(texture))
	{
	}

	com_ptr<ID3D11SamplerState> SkinnedModelMaterial::SamplerState() const
	{
		return mSamplerState;
	}

	void SkinnedModelMaterial::SetSamplerState(com_ptr<ID3D11SamplerState> samplerState)
	{
		assert(samplerState != nullptr);
		mSamplerState = samplerState;
		Material::SetSamplerState(ShaderStages::PS, mSamplerState.get());
	}

	shared_ptr<Texture2D> SkinnedModelMaterial::Texture() const
	{
		return mTexture;
	}

	void SkinnedModelMaterial::SetTexture(shared_ptr<Texture2D> texture)
	{
		assert(texture != nullptr);
		mTexture = move(texture);
		Material::SetShaderResource(ShaderStages::PS, mTexture->ShaderResourceView().get());
	}

	const XMFLOAT4& SkinnedModelMaterial::AmbientColor() const
	{
		return mPixelCBufferPerFrameData.AmbientColor;
	}

	void SkinnedModelMaterial::SetAmbientColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.AmbientColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const XMFLOAT3& SkinnedModelMaterial::LightDirection() const
	{
		return mPixelCBufferPerFrameData.LightDirection;
	}

	void SkinnedModelMaterial::SetLightDirection(const XMFLOAT3& direction)
	{
		mPixelCBufferPerFrameData.LightDirection = direction;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const XMFLOAT4& SkinnedModelMaterial::LightColor() const
	{
		return mPixelCBufferPerFrameData.LightColor;
	}

	void SkinnedModelMaterial::SetLightColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.LightColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	uint32_t SkinnedModelMaterial::VertexSize() const
	{
		return sizeof(VertexSkinnedPositionTextureNormal);
	}

	void SkinnedModelMaterial::Initialize()
	{
		Material::Initialize();
				
		auto& content = mGame->Content();
		auto vertexShader = content.Load<VertexShader>(L"Shaders\\SkinnedModelVS.cso"s);
		SetShader(vertexShader);

		auto pixelShader = content.Load<PixelShader>(L"Shaders\\DiffuseLightingPS.cso");
		SetShader(pixelShader);

		auto direct3DDevice = mGame->Direct3DDevice();
		vertexShader->CreateInputLayout<VertexSkinnedPositionTextureNormal>(direct3DDevice);
		SetInputLayout(vertexShader->InputLayout());

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = sizeof(VertexCBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::VS, mVertexCBufferPerObject.get());

		constantBufferDesc.ByteWidth = sizeof(VertexCBufferSkinning);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferSkinning.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::VS, mVertexCBufferSkinning.get());

		constantBufferDesc.ByteWidth = sizeof(PixelCBufferPerFrame);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerFrame.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::PS, mPixelCBufferPerFrame.get());

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mVertexCBufferSkinning.get(), 0, nullptr, &mVertexCBufferSkinningData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);

		AddShaderResource(ShaderStages::PS, mTexture->ShaderResourceView().get());
		AddSamplerState(ShaderStages::PS, mSamplerState.get());
	}

	void SkinnedModelMaterial::UpdateTransforms(FXMMATRIX worldViewProjectionMatrix, CXMMATRIX worldMatrix)
	{
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, worldViewProjectionMatrix);
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, worldMatrix);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
	}

	void SkinnedModelMaterial::UpdateBoneTransforms(const vector<DirectX::XMFLOAT4X4>& boneTransforms)
	{
		assert(boneTransforms.size() <= VertexCBufferSkinning::MaxBones);

		for (size_t i = 0; i < boneTransforms.size(); i++)
		{
			DirectX::XMMATRIX transform = XMLoadFloat4x4(&boneTransforms[i]);
			transform = XMMatrixTranspose(transform);

			XMStoreFloat4x4(&mVertexCBufferSkinningData.BoneTransforms[i], transform);
		}

		mGame->Direct3DDeviceContext()->UpdateSubresource(mVertexCBufferSkinning.get(), 0, nullptr, &mVertexCBufferSkinningData, 0, 0);
	}

	void SkinnedModelMaterial::BeginDraw()
	{
		Material::BeginDraw();

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();

		if (mPixelCBufferPerFrameDataDirty)
		{
			direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
			mPixelCBufferPerFrameDataDirty = false;
		}
	}
}