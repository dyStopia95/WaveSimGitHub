#include "pch.h"
#include "Texture1DArray.h"
#include "GameException.h"

using namespace std;
using namespace gsl;
using namespace DirectX;
using namespace winrt;

namespace Library
{
	RTTI_DEFINITIONS(Texture1DArray)


	Texture1DArray::Texture1DArray(const winrt::com_ptr<ID3D11ShaderResourceView>& shaderResourceView, std::uint32_t width, std::uint32_t arraySize, ID3D11Texture1D* textureResource) :
		Texture(shaderResourceView), mWidth{ width }, mArraySize{ arraySize }, texResource { textureResource }
	{
	}

	std::shared_ptr<Texture1DArray> Texture1DArray::CreateTexture1DArray(gsl::not_null<ID3D11Device*> device, const D3D11_TEXTURE1D_DESC& textureDesc)
	{
		HRESULT hr;
		com_ptr<ID3D11Texture1D> texture;
		if (FAILED(hr = device->CreateTexture1D(&textureDesc, nullptr, texture.put())))
		{
			throw GameException("IDXGIDevice::CreateTexture2D() failed.", hr);
		}

		com_ptr<ID3D11ShaderResourceView> shaderResourceReview;
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		ZeroMemory(&SRVDesc, sizeof(SRVDesc));
		SRVDesc.Format = textureDesc.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
		SRVDesc.Texture1DArray.ArraySize = textureDesc.ArraySize;
		SRVDesc.Texture1DArray.MipLevels = textureDesc.MipLevels;

		if (FAILED(hr = device->CreateShaderResourceView(texture.get(), &SRVDesc, shaderResourceReview.put())))
		{
			throw GameException("IDXGIDevice::CreateShaderResourceView() failed.", hr);
		}

		return make_shared<Texture1DArray>(shaderResourceReview, textureDesc.Width, textureDesc.ArraySize, texture.get());
	}

	std::shared_ptr<Texture1DArray> Texture1DArray::CreateTexture1DArray(gsl::not_null<ID3D11Device*> device, std::uint32_t width, std::uint32_t mipLevels, std::uint32_t arraySize, DXGI_FORMAT format, DXGI_SAMPLE_DESC /*sampleDesc*/, std::uint32_t bindFlags, std::uint32_t cpuAccessFlags)
	{
		D3D11_TEXTURE1D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = width;
		textureDesc.MipLevels = mipLevels;
		textureDesc.ArraySize = arraySize;
		textureDesc.Format = format;
		//textureDesc.SampleDesc = sampleDesc;
		textureDesc.BindFlags = bindFlags;
		textureDesc.CPUAccessFlags = cpuAccessFlags;

		return CreateTexture1DArray(device, textureDesc);
	}

	std::uint32_t Texture1DArray::Width() const
	{
		return mWidth;
	}

	std::uint32_t Texture1DArray::ArraySize() const
	{
		return mArraySize;
	}

	ID3D11Texture1D* Texture1DArray::GetTexResource()
	{
		return texResource;
	}

}