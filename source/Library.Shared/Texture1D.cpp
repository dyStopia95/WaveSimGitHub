#include "pch.h"
#include "Texture1D.h"
#include "GameException.h"

using namespace std;
using namespace gsl;
using namespace DirectX;
using namespace winrt;

namespace Library
{
	RTTI_DEFINITIONS(Texture1D)


	Texture1D::Texture1D(const winrt::com_ptr<ID3D11ShaderResourceView>& shaderResourceView, std::uint32_t width, ID3D11Texture1D* textureResource) :
		Texture(shaderResourceView), mWidth{ width }, texResource{ textureResource }
	{
	}

	std::shared_ptr<Texture1D> Texture1D::CreateTexture1D(gsl::not_null<ID3D11Device*> device, const D3D11_TEXTURE1D_DESC& textureDesc)
	{
		HRESULT hr;
		com_ptr<ID3D11Texture1D> texture;
		if (FAILED(hr = device->CreateTexture1D(&textureDesc, nullptr, texture.put())))
		{
			throw GameException("IDXGIDevice::CreateTexture2D() failed.", hr);
		}

		com_ptr<ID3D11ShaderResourceView> shaderResourceReview;
		if (FAILED(hr = device->CreateShaderResourceView(texture.get(), nullptr, shaderResourceReview.put())))
		{
			throw GameException("IDXGIDevice::CreateShaderResourceView() failed.", hr);
		}

		return make_shared<Texture1D>(shaderResourceReview, textureDesc.Width, texture.get());
	}

	std::shared_ptr<Texture1D> Texture1D::CreateTexture1D(gsl::not_null<ID3D11Device*> device, std::uint32_t width, std::uint32_t mipLevels, std::uint32_t arraySize, DXGI_FORMAT format, DXGI_SAMPLE_DESC /*sampleDesc*/, std::uint32_t bindFlags, std::uint32_t cpuAccessFlags)
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

		return CreateTexture1D(device, textureDesc);
	}

	std::uint32_t Texture1D::Width() const
	{
		return mWidth;
	}

	ID3D11Texture1D* Texture1D::GetTexResource()
	{
		return texResource;
	}

}