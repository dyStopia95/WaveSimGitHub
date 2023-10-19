#include "pch.h"
#include "BasicTessellationMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VertexDeclarations.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "DomainShader.h"
#include "HullShader.h"
#include "RasterizerStates.h"

using namespace std;
using namespace gsl;
using namespace std::string_literals;
using namespace DirectX;
using namespace winrt;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(BasicTessellationMaterial)

	BasicTessellationMaterial::BasicTessellationMaterial(Game& game) :
		Material(game)
	{
	}

	bool BasicTessellationMaterial::ShowQuadTopology() const
	{
		return mShowQuadTopology;
	}

	void BasicTessellationMaterial::SetShowQuadTopology(bool showQuadTopology)
	{
		mShowQuadTopology = showQuadTopology;
		UpdateTopology();
	}

	span<const float> BasicTessellationMaterial::EdgeFactors() const
	{
		return mShowQuadTopology ? span<const float>{ mQuadHullCBufferPerFrameData.TessellationEdgeFactors } : span<const float>{ mTriHullCBufferPerFrameData.TessellationEdgeFactors };
	}

	void BasicTessellationMaterial::SetUniformEdgeFactors(float factor)
	{
		UpdateUniformTessellationFactors(factor, mQuadHullCBufferPerFrameData.TessellationEdgeFactors, mQuadHullCBufferPerFrameData.TessellationInsideFactors);
		UpdateUniformTessellationFactors(factor, mTriHullCBufferPerFrameData.TessellationEdgeFactors, mTriHullCBufferPerFrameData.TessellationInsideFactors);
	}

	void BasicTessellationMaterial::SetEdgeFactor(float factor, uint32_t index)
	{
		if (mShowQuadTopology)
		{
			assert(index < std::size(mQuadHullCBufferPerFrameData.TessellationEdgeFactors));
			mQuadHullCBufferPerFrameData.TessellationEdgeFactors[index] = factor;
			mGame->Direct3DDeviceContext()->UpdateSubresource(mQuadHullCBufferPerFrame.get(), 0, nullptr, &mQuadHullCBufferPerFrameData, 0, 0);
		}
		else
		{
			assert(index < std::size(mTriHullCBufferPerFrameData.TessellationEdgeFactors));
			mTriHullCBufferPerFrameData.TessellationEdgeFactors[index] = factor;
			mGame->Direct3DDeviceContext()->UpdateSubresource(mTriHullCBufferPerFrame.get(), 0, nullptr, &mTriHullCBufferPerFrameData, 0, 0);
		}
	}

	void BasicTessellationMaterial::SetInsideFactor(float factor, std::uint32_t index)
	{
		if (mShowQuadTopology)
		{
			assert(index < std::size(mQuadHullCBufferPerFrameData.TessellationInsideFactors));
			mQuadHullCBufferPerFrameData.TessellationInsideFactors[index] = factor;
			mGame->Direct3DDeviceContext()->UpdateSubresource(mQuadHullCBufferPerFrame.get(), 0, nullptr, &mQuadHullCBufferPerFrameData, 0, 0);
		}
		else
		{
			assert(index < std::size(mTriHullCBufferPerFrameData.TessellationInsideFactors));
			mTriHullCBufferPerFrameData.TessellationInsideFactors[index] = factor;
			mGame->Direct3DDeviceContext()->UpdateSubresource(mTriHullCBufferPerFrame.get(), 0, nullptr, &mTriHullCBufferPerFrameData, 0, 0);
		}
	}

	span<const float> BasicTessellationMaterial::InsideFactors() const
	{
		return mShowQuadTopology ? span<const float>{ mQuadHullCBufferPerFrameData.TessellationInsideFactors } : span<const float>{ mTriHullCBufferPerFrameData.TessellationInsideFactors };
	}

	uint32_t BasicTessellationMaterial::VertexSize() const
	{
		return sizeof(VertexPosition);
	}

	void BasicTessellationMaterial::Initialize()
	{
		Material::Initialize();
				
		auto& content = mGame->Content();
		auto vertexShader = content.Load<VertexShader>(L"Shaders\\BasicTessellationDemoVS.cso"s);
		SetShader(vertexShader);

		auto pixelShader = content.Load<PixelShader>(L"Shaders\\BasicTessellationDemoPS.cso");
		SetShader(pixelShader);

		mQuadHullShader = content.Load<HullShader>(L"Shaders\\QuadTessellationHS.cso");
		mTriHullShader = content.Load<HullShader>(L"Shaders\\TriTessellationHS.cso");

		mQuadDomainShader = content.Load<DomainShader>(L"Shaders\\QuadTessellationDS.cso");
		mTriDomainShader = content.Load<DomainShader>(L"Shaders\\TriTessellationDS.cso");

		auto direct3DDevice = mGame->Direct3DDevice();
		vertexShader->CreateInputLayout<VertexPosition>(direct3DDevice);
		SetInputLayout(vertexShader->InputLayout());

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = sizeof(DomainCBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mDomainCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::DS, mDomainCBufferPerObject.get());

		constantBufferDesc.ByteWidth = sizeof(QuadHullCBufferPerFrame);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mQuadHullCBufferPerFrame.put()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(TriHullCBufferPerFrame);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mTriHullCBufferPerFrame.put()), "ID3D11Device::CreateBuffer() failed.");

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->UpdateSubresource(mDomainCBufferPerObject.get(), 0, nullptr, &mDomainCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mQuadHullCBufferPerFrame.get(), 0, nullptr, &mQuadHullCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mTriHullCBufferPerFrame.get(), 0, nullptr, &mTriHullCBufferPerFrameData, 0, 0);

		UpdateTopology();		
	}

	void BasicTessellationMaterial::BeginDraw()
	{
		Material::BeginDraw();
		mGame->Direct3DDeviceContext()->RSSetState(RasterizerStates::Wireframe.get());
	}

	void BasicTessellationMaterial::EndDraw()
	{
		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->HSSetShader(NULL, NULL, 0);
		direct3DDeviceContext->DSSetShader(NULL, NULL, 0);

		Material::EndDraw();
	}

	void BasicTessellationMaterial::UpdateTransforms(FXMMATRIX worldViewProjectionMatrix)
	{
		XMStoreFloat4x4(&mDomainCBufferPerObjectData.WorldViewProjection, worldViewProjectionMatrix);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mDomainCBufferPerObject.get(), 0, nullptr, &mDomainCBufferPerObjectData, 0, 0);
	}

	void BasicTessellationMaterial::UpdateTopology()
	{
		if (mShowQuadTopology)
		{
			SetTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
			SetShader(mQuadDomainShader);
			SetShader(mQuadHullShader);
			SetConstantBuffer(ShaderStages::HS, mQuadHullCBufferPerFrame.get());
		}
		else
		{
			SetTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			SetShader(mTriDomainShader);
			SetShader(mTriHullShader);
			SetConstantBuffer(ShaderStages::HS, mTriHullCBufferPerFrame.get());
		}
	}

	void BasicTessellationMaterial::UpdateUniformTessellationFactors(float source, span<float> edgeFactors, span<float> insideFactors)
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
		direct3DDeviceContext->UpdateSubresource(mQuadHullCBufferPerFrame.get(), 0, nullptr, &mQuadHullCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mTriHullCBufferPerFrame.get(), 0, nullptr, &mTriHullCBufferPerFrameData, 0, 0);
	}
}