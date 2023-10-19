#include "pch.h"
#include "HeightmapTessellationMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VertexDeclarations.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "DomainShader.h"
#include "HullShader.h"
#include "RasterizerStates.h"
#include "Texture2D.h"

using namespace std;
using namespace gsl;
using namespace std::string_literals;
using namespace DirectX;
using namespace winrt;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(HeightmapTessellationMaterial)

	HeightmapTessellationMaterial::HeightmapTessellationMaterial(Game& game) :
		Material(game)
	{
	}

	com_ptr<ID3D11SamplerState> HeightmapTessellationMaterial::SamplerState() const
	{
		return com_ptr<ID3D11SamplerState>();
	}

	void HeightmapTessellationMaterial::SetSamplerState(com_ptr<ID3D11SamplerState> samplerState)
	{
		assert(samplerState != nullptr);
		mSamplerState = samplerState;
		Material::SetSamplerState(ShaderStages::DS, mSamplerState.get());
	}

	shared_ptr<Texture2D> HeightmapTessellationMaterial::Heightmap() const
	{
		return mHeightmap;
	}

	void HeightmapTessellationMaterial::SetHeightmap(shared_ptr<Texture2D> heightmap)
	{
		assert(heightmap != nullptr);
		mHeightmap = move(heightmap);
		Material::SetShaderResource(ShaderStages::DS, mHeightmap->ShaderResourceView().get());
	}

	span<const float> HeightmapTessellationMaterial::EdgeFactors() const
	{
		return span<const float>{ mHullCBufferPerFrameData.TessellationEdgeFactors };
	}

	gsl::span<const float> HeightmapTessellationMaterial::InsideFactors() const
	{
		return span<const float>{ mHullCBufferPerFrameData.TessellationInsideFactors };
	}

	void HeightmapTessellationMaterial::SetUniformFactors(float factor)
	{
		UpdateUniformTessellationFactors(factor, mHullCBufferPerFrameData.TessellationEdgeFactors, mHullCBufferPerFrameData.TessellationInsideFactors);
	}

	float HeightmapTessellationMaterial::DisplacementScale() const
	{
		return mDomainCBufferPerObjectData.DisplacementScale;
	}

	void HeightmapTessellationMaterial::SetDisplacementScale(float displacementScale)
	{
		mDomainCBufferPerObjectData.DisplacementScale = displacementScale;
		mGame->Direct3DDeviceContext()->UpdateSubresource(mDomainCBufferPerObject.get(), 0, nullptr, &mDomainCBufferPerObjectData, 0, 0);
	}

	uint32_t HeightmapTessellationMaterial::VertexSize() const
	{
		return sizeof(VertexPositionTexture);
	}

	void HeightmapTessellationMaterial::Initialize()
	{
		Material::Initialize();
				
		auto& content = mGame->Content();
		auto vertexShader = content.Load<VertexShader>(L"Shaders\\HeightmapTessellationDemoVS.cso"s);
		SetShader(vertexShader);

		auto pixelShader = content.Load<PixelShader>(L"Shaders\\HeightmapTessellationDemoPS.cso");
		SetShader(pixelShader);

		auto hullShader = content.Load<HullShader>(L"Shaders\\HeightmapTessellationDemoHS.cso");
		SetShader(hullShader);

		auto domainShader = content.Load<DomainShader>(L"Shaders\\HeightmapTessellationDemoDS.cso");
		SetShader(domainShader);

		auto direct3DDevice = mGame->Direct3DDevice();
		vertexShader->CreateInputLayout<VertexPositionTexture>(direct3DDevice);
		SetInputLayout(vertexShader->InputLayout());

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = sizeof(VertexCBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::VS, mVertexCBufferPerObject.get());

		constantBufferDesc.ByteWidth = sizeof(DomainCBufferPerObject);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mDomainCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::DS, mDomainCBufferPerObject.get());

		constantBufferDesc.ByteWidth = sizeof(HullCBufferPerFrame);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mHullCBufferPerFrame.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::HS, mHullCBufferPerFrame.get());

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mDomainCBufferPerObject.get(), 0, nullptr, &mDomainCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mHullCBufferPerFrame.get(), 0, nullptr, &mHullCBufferPerFrameData, 0, 0);		
		
		SetTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);		
		AddSamplerState(ShaderStages::DS, mSamplerState.get());
	}

	void HeightmapTessellationMaterial::BeginDraw()
	{
		Material::BeginDraw();
		mGame->Direct3DDeviceContext()->RSSetState(RasterizerStates::Wireframe.get());
	}

	void HeightmapTessellationMaterial::EndDraw()
	{
		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->HSSetShader(NULL, NULL, 0);
		direct3DDeviceContext->DSSetShader(NULL, NULL, 0);

		Material::EndDraw();
	}

	void HeightmapTessellationMaterial::UpdateTransforms(FXMMATRIX worldViewProjectionMatrix)
	{
		XMStoreFloat4x4(&mDomainCBufferPerObjectData.WorldViewProjection, worldViewProjectionMatrix);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mDomainCBufferPerObject.get(), 0, nullptr, &mDomainCBufferPerObjectData, 0, 0);
	}

	void HeightmapTessellationMaterial::UpdateTextureMatrix(DirectX::FXMMATRIX textureMatrix)
	{
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.TextureMatrix, textureMatrix);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
	}

	void HeightmapTessellationMaterial::UpdateUniformTessellationFactors(float source, span<float> edgeFactors, span<float> insideFactors)
	{
		for (auto& factor : edgeFactors)
		{
			factor = source;
		}

		for (auto& factor : insideFactors)
		{
			factor = source;
		}

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->UpdateSubresource(mHullCBufferPerFrame.get(), 0, nullptr, &mHullCBufferPerFrameData, 0, 0);
	}
}