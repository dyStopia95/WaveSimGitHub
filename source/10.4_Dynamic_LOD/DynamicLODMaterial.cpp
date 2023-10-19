#include "pch.h"
#include "DynamicLODMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VertexDeclarations.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "HullShader.h"
#include "DomainShader.h"
#include "DirectXHelper.h"
#include "RasterizerStates.h"

using namespace std;
using namespace gsl;
using namespace std::string_literals;
using namespace DirectX;
using namespace winrt;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(DynamicLODMaterial)

	DynamicLODMaterial::DynamicLODMaterial(Game& game) :
		Material(game)
	{
	}

	int DynamicLODMaterial::MaxTessellationFactor() const
	{
		return mHullCBufferPerFrameData.MaxTessellationFactor;
	}

	void DynamicLODMaterial::SetMaxTessellationFactor(int maxTessellationFactor)
	{
		mHullCBufferPerFrameData.MaxTessellationFactor = maxTessellationFactor;
		mGame->Direct3DDeviceContext()->UpdateSubresource(mHullCBufferPerFrame.get(), 0, nullptr, &mHullCBufferPerFrameData, 0, 0);
	}

	XMFLOAT2 DynamicLODMaterial::TessellationDistances() const
	{
		return mHullCBufferPerFrameData.TessellationDistances;
	}

	void DynamicLODMaterial::SetTessellationDistances(XMFLOAT2 tessellationDistances)
	{
		mHullCBufferPerFrameData.TessellationDistances = tessellationDistances;
		mGame->Direct3DDeviceContext()->UpdateSubresource(mHullCBufferPerFrame.get(), 0, nullptr, &mHullCBufferPerFrameData, 0, 0);
	}

	void DynamicLODMaterial::SetTessellationDistances(float minTessellationDistance, float maxTessellationDistance)
	{
		mHullCBufferPerFrameData.TessellationDistances.x = minTessellationDistance;
		mHullCBufferPerFrameData.TessellationDistances.y = maxTessellationDistance;
		mGame->Direct3DDeviceContext()->UpdateSubresource(mHullCBufferPerFrame.get(), 0, nullptr, &mHullCBufferPerFrameData, 0, 0);
	}

	uint32_t DynamicLODMaterial::VertexSize() const
	{
		return sizeof(VertexPosition);
	}

	void DynamicLODMaterial::Initialize()
	{
		Material::Initialize();
				
		auto& content = mGame->Content();
		auto vertexShader = content.Load<VertexShader>(L"Shaders\\DynamicLODDemoVS.cso"s);
		SetShader(vertexShader);

		auto pixelShader = content.Load<PixelShader>(L"Shaders\\DynamicLODDemoPS.cso");
		SetShader(pixelShader);

		auto hullShader = content.Load<HullShader>(L"Shaders\\DynamicLODDemoHS.cso");
		SetShader(hullShader);

		auto domainShader = content.Load<DomainShader>(L"Shaders\\DynamicLODDemoDS.cso");
		SetShader(domainShader);

		auto direct3DDevice = mGame->Direct3DDevice();
		vertexShader->CreateInputLayout<VertexPosition>(direct3DDevice);
		SetInputLayout(vertexShader->InputLayout());

		CreateConstantBuffer(direct3DDevice, sizeof(HullCBufferPerFrame), mHullCBufferPerFrame.put());
		AddConstantBuffer(ShaderStages::HS, mHullCBufferPerFrame.get());

		CreateConstantBuffer(direct3DDevice, sizeof(HullCBufferPerObject), mHullCBufferPerObject.put());
		AddConstantBuffer(ShaderStages::HS, mHullCBufferPerObject.get());

		CreateConstantBuffer(direct3DDevice, sizeof(DomainCBufferPerObject), mDomainCBufferPerObject.put());
		AddConstantBuffer(ShaderStages::DS, mDomainCBufferPerObject.get());

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->UpdateSubresource(mHullCBufferPerFrame.get(), 0, nullptr, &mHullCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mHullCBufferPerObject.get(), 0, nullptr, &mHullCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mDomainCBufferPerObject.get(), 0, nullptr, &mDomainCBufferPerObjectData, 0, 0);

		SetTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}

	void DynamicLODMaterial::UpdateCameraPosition(const XMFLOAT3& position)
	{
		mHullCBufferPerFrameData.CameraPosition = position;
		mGame->Direct3DDeviceContext()->UpdateSubresource(mHullCBufferPerFrame.get(), 0, nullptr, &mHullCBufferPerFrameData, 0, 0);
	}

	void DynamicLODMaterial::UpdateTransforms(FXMMATRIX worldViewProjectionMatrix, CXMMATRIX worldMatrix)
	{
		XMStoreFloat4x4(&mDomainCBufferPerObjectData.WorldViewProjectionMatrix, worldViewProjectionMatrix);		
		mGame->Direct3DDeviceContext()->UpdateSubresource(mDomainCBufferPerObject.get(), 0, nullptr, &mDomainCBufferPerObjectData, 0, 0);

		XMStoreFloat4x4(&mHullCBufferPerObjectData.WorldMatrix, worldMatrix);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mHullCBufferPerObject.get(), 0, nullptr, &mHullCBufferPerObjectData, 0, 0);
	}

	void DynamicLODMaterial::BeginDraw()
	{
		Material::BeginDraw();
		mGame->Direct3DDeviceContext()->RSSetState(RasterizerStates::Wireframe.get());
	}

	void DynamicLODMaterial::EndDraw()
	{
		mGame->Direct3DDeviceContext()->HSSetShader(NULL, NULL, 0);
		mGame->Direct3DDeviceContext()->DSSetShader(NULL, NULL, 0);
		Material::EndDraw();
	}
}