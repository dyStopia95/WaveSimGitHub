#include "pch.h"
#include "WaveSimCompShader.h"
#include "Game.h"
#include "GameException.h"
#include "ComputeShader.h"
#include "DirectXHelper.h"
#include "Texture1D.h"
#include "NodeArray.h"

using namespace std;
using namespace gsl;
using namespace std::string_literals;
using namespace DirectX;
using namespace winrt;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(WaveSimCompShader)

	WaveSimCompShader::WaveSimCompShader(Library::Game& game, winrt::com_ptr<ID3D11UnorderedAccessView> outputTexture) :
		Material(game), mOutputTexture(move(outputTexture))
	{
	}

	void WaveSimCompShader::SetParams(const SimParams& parameters)
	{
		mSimParamsCBData.C2 = parameters.c * parameters.c;
		mSimParamsCBData.dampingFactor = parameters.dmpFactor;
		mSimParamsCBData.springConstant = parameters.k;
		mSimParamsCBData.rows = parameters.rows;
		mSimParamsCBData.columns = parameters.columns;
		mNodeCount = parameters.rows * parameters.columns;
		mSimParamsCBData.nodeCount = mNodeCount;
		mSimParamsCBData.spacing2 = parameters.spacing * parameters.spacing;
		mSimParamsCBData.deltaT = parameters.deltaT;
	}

	void WaveSimCompShader::CreateVertexXYArray(DirectX::XMFLOAT2* xyArray, UINT length)
	{
		D3D11_TEXTURE1D_DESC texDesc{ 0 };
		texDesc.Width = length;
		texDesc.Usage = D3D11_USAGE_DYNAMIC;
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		texDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		//texDesc.Format = DXGI_FORMAT_R32_FLOAT;
		texDesc.ArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		texDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA indexSubResourceData{ 0 };
		indexSubResourceData.pSysMem = xyArray;

		HRESULT hr;
		com_ptr<ID3D11Texture1D> texture;
		if (FAILED(hr = mGame->Direct3DDevice()->CreateTexture1D(&texDesc, &indexSubResourceData, texture.put())))
		{
			throw GameException("IDXGIDevice::CreateTexture2D() failed.", hr);
		}

		com_ptr<ID3D11ShaderResourceView> SRV;
		if (FAILED(hr = mGame->Direct3DDevice()->CreateShaderResourceView(texture.get(), nullptr, SRV.put())))
		{
			throw GameException("IDXGIDevice::CreateShaderResourceView() failed.", hr);
		}

		mVertexXYArray = make_shared<Texture1D>(SRV, length, texture.get());
	}

	winrt::com_ptr<ID3D11UnorderedAccessView> WaveSimCompShader::OutputTexture() const
	{
		return mOutputTexture;
	}

	void WaveSimCompShader::SetOutputTexture(winrt::com_ptr<ID3D11UnorderedAccessView> texture)
	{
		mOutputTexture = move(texture);
	}

	void WaveSimCompShader::Initialize()
	{
		Material::Initialize();
		mComputeShader = mGame->Content().Load<ComputeShader>(L"Shaders\\WaveSimCS.cso"s);
		CreateConstantBuffer(mGame->Direct3DDevice(), sizeof(SimParamBuffer), mSimParamsCB.put());
		mGame->Direct3DDeviceContext()->UpdateSubresource(mSimParamsCB.get(), 0, nullptr, &mSimParamsCBData, 0, 0);
	}

	void WaveSimCompShader::Dispatch()
	{
		assert(mOutputTexture != nullptr);
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();

		direct3DDeviceContext->CSSetShader(mComputeShader->Shader().get(), nullptr, 0);

		auto uaViews = mOutputTexture.get();
		direct3DDeviceContext->CSSetUnorderedAccessViews(0, 1, &uaViews, nullptr);

		auto constantBuffers = mSimParamsCB.get();
		direct3DDeviceContext->CSSetConstantBuffers(0, 1, &constantBuffers);

		/*auto vertexTex = mVertexXYArray->ShaderResourceView().get();
		direct3DDeviceContext->CSSetShaderResources(0, 1, &vertexTex);*/

		direct3DDeviceContext->Dispatch(mNodeCount, 1, 1);

		static const std::array<ID3D11UnorderedAccessView*, 1> emptyUAViews{ nullptr };
		direct3DDeviceContext->CSSetUnorderedAccessViews(0, 1, emptyUAViews.data(), nullptr);
	}

}
