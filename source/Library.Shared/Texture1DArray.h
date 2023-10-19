#pragma once

#include <gsl\gsl>
#include "Texture.h"
#include "Rectangle.h"

namespace Library
{
	class Texture1DArray final : public Texture
	{
		RTTI_DECLARATIONS(Texture1DArray, Texture)

	public:
		Texture1DArray(const winrt::com_ptr<ID3D11ShaderResourceView>& shaderResourceView, std::uint32_t width, std::uint32_t arraySize, ID3D11Texture1D* textureResource);
		Texture1DArray(const Texture1DArray&) = default;
		Texture1DArray& operator=(const Texture1DArray&) = default;
		Texture1DArray(Texture1DArray&&) = default;
		Texture1DArray& operator=(Texture1DArray&&) = default;
		~Texture1DArray() = default;

		static std::shared_ptr<Texture1DArray> CreateTexture1DArray(gsl::not_null<ID3D11Device*> device, const D3D11_TEXTURE1D_DESC& textureDesc);
		static std::shared_ptr<Texture1DArray> CreateTexture1DArray(gsl::not_null<ID3D11Device*> device, std::uint32_t width, std::uint32_t mipLevels = 1, std::uint32_t arraySize = 1, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SAMPLE_DESC sampleDesc = { 1, 0 }, std::uint32_t bindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, std::uint32_t cpuAccessFlags = 0);

		std::uint32_t Width() const;
		std::uint32_t ArraySize() const;

		ID3D11Texture1D* GetTexResource();

	private:
		std::uint32_t mWidth;
		std::uint32_t mArraySize;
		ID3D11Texture1D* texResource;
	};
}