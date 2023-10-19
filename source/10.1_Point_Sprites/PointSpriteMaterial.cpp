#include "pch.h"
#include "PointSpriteMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VertexDeclarations.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "GeometryShader.h"
#include "Texture2D.h"

using namespace std;
using namespace gsl;
using namespace std::string_literals;
using namespace DirectX;
using namespace winrt;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(PointSpriteMaterial)

	PointSpriteMaterial::PointSpriteMaterial(Game& game) :
		Material(game)
	{
	}

	com_ptr<ID3D11SamplerState> PointSpriteMaterial::SamplerState() const
	{
		return mSamplerState;
	}

	void PointSpriteMaterial::SetSamplerState(com_ptr<ID3D11SamplerState> samplerState)
	{
		assert(samplerState != nullptr);
		mSamplerState = samplerState;
		Material::SetSamplerState(ShaderStages::PS, mSamplerState.get());
	}

	shared_ptr<Texture2D> PointSpriteMaterial::ColorMap() const
	{
		return mColorMap;
	}

	void PointSpriteMaterial::SetColorMap(shared_ptr<Texture2D> texture)
	{
		assert(texture != nullptr);
		mColorMap = move(texture);
		SetShaderResource(ShaderStages::PS, mColorMap->ShaderResourceView().get());
	}

	uint32_t PointSpriteMaterial::VertexSize() const
	{
		return sizeof(VertexPositionSize);
	}

	void PointSpriteMaterial::Initialize()
	{
		Material::Initialize();
				
		auto& content = mGame->Content();
		auto vertexShader = content.Load<VertexShader>(L"Shaders\\PointSpriteDemoVS.cso"s);
		SetShader(vertexShader);

		auto pixelShader = content.Load<PixelShader>(L"Shaders\\PointSpriteDemoPS.cso");
		SetShader(pixelShader);

		auto geometryShader = content.Load<GeometryShader>(L"Shaders\\PointSpriteDemoGS.cso");
		SetShader(geometryShader);

		auto direct3DDevice = mGame->Direct3DDevice();
		vertexShader->CreateInputLayout<VertexPositionSize>(direct3DDevice);
		SetInputLayout(vertexShader->InputLayout());

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = sizeof(GeometryCBufferPerFrame);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mGeometryCBufferPerFrame.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::GS, mGeometryCBufferPerFrame.get());

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->UpdateSubresource(mGeometryCBufferPerFrame.get(), 0, nullptr, &mGeometryCBufferPerFrameData, 0, 0);

		SetTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	}

	void PointSpriteMaterial::UpdateCameraData(FXMMATRIX viewProjectionMatrix, const XMFLOAT3& position, const XMFLOAT3& up)
	{
		XMStoreFloat4x4(&mGeometryCBufferPerFrameData.ViewProjectionMatrix, viewProjectionMatrix);
		mGeometryCBufferPerFrameData.CameraPosition = position;
		mGeometryCBufferPerFrameData.CameraUp = up;
		mGame->Direct3DDeviceContext()->UpdateSubresource(mGeometryCBufferPerFrame.get(), 0, nullptr, &mGeometryCBufferPerFrameData, 0, 0);
	}

	void PointSpriteMaterial::EndDraw()
	{
		mGame->Direct3DDeviceContext()->GSSetShader(NULL, NULL, 0);
		Material::EndDraw();
	}
}