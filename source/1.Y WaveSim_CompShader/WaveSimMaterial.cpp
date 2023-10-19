#include "pch.h"
#include "WaveSimMaterial.h"
#include "VertexDeclarations.h"
#include "Game.h"
#include "Texture1D.h"
//#include "Texture1DArray.h"
#include "VertexShader.h"
#include "PixelShader.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace winrt;
using namespace DirectX;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(WaveSimMaterial)

	WaveSimMaterial::WaveSimMaterial(Library::Game& game, std::shared_ptr<Library::Texture1D> zArray) :
		Material(game), mDisplacementMap{ move(zArray) }
	{
	}

	void WaveSimMaterial::UpdateTransforms(DirectX::CXMMATRIX worldViewProjectionMatrix)
	{
		mGame->Direct3DDeviceContext()->UpdateSubresource(mWVPBuffer.get(), 0, nullptr, worldViewProjectionMatrix.r, 0, 0);
	}

	void WaveSimMaterial::SetSurfaceColor(const DirectX::XMFLOAT4& color)
	{
		SetSurfaceColor(reinterpret_cast<const float*>(&color));
	}

	void WaveSimMaterial::SetSurfaceColor(const float* color)
	{
		mGame->Direct3DDeviceContext()->UpdateSubresource(mPSConstantBuffer.get(), 0, nullptr, color, 0, 0);
	}

	std::uint32_t WaveSimMaterial::VertexSize() const
	{
		return sizeof(Library::VertexXYIndex);
	}

	std::shared_ptr<Library::Texture1D> WaveSimMaterial::GetZArrayRef()
	{
		return mDisplacementMap;
	}

	void WaveSimMaterial::SetZArrayRef(std::shared_ptr<Library::Texture1D> dispMap)
	{
		assert(dispMap != nullptr);
		mDisplacementMap = move(dispMap);
		SetShaderResource(ShaderStages::VS, mDisplacementMap->ShaderResourceView().get());
	}

	/*void WaveSimMaterial::BeginDraw()
	{
		Material::BeginDraw();

	}*/

	void WaveSimMaterial::Initialize()
	{
		Material::Initialize();

		auto& content = mGame->Content();
		auto vertexShader = content.Load<VertexShader>(L"Shaders\\WaveSimVS.cso"s);
		SetShader(vertexShader);

		auto pixelShader = content.Load<PixelShader>(L"Shaders\\BasicPS.cso");
		SetShader(pixelShader);

		auto direct3DDevice = mGame->Direct3DDevice();
		vertexShader->CreateInputLayout<VertexXYIndex>(direct3DDevice);
		SetInputLayout(vertexShader->InputLayout());

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = sizeof(XMFLOAT4X4);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mWVPBuffer.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::VS, mWVPBuffer.get());

		constantBufferDesc.ByteWidth = sizeof(XMFLOAT4);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mPSConstantBuffer.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::PS, mPSConstantBuffer.get());

		/*auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->UpdateSubresource(mWVPBuffer.get(), 0, nullptr, &mWVPBuffData, 0, 0);*/

		AddShaderResource(ShaderStages::VS, mDisplacementMap->ShaderResourceView().get());

	}
}