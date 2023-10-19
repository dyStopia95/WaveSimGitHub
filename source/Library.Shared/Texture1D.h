#pragma once

#include <gsl\gsl>
#include "Texture.h"
#include "Rectangle.h"

namespace Library
{
	class Texture1D final : public Texture
	{
		RTTI_DECLARATIONS(Texture1D, Texture)

	public:
		Texture1D(const winrt::com_ptr<ID3D11ShaderResourceView>& shaderResourceView, std::uint32_t width, ID3D11Texture1D* textureResource);
		Texture1D(const Texture1D&) = default;
		Texture1D& operator=(const Texture1D&) = default;
		Texture1D(Texture1D&&) = default;
		Texture1D& operator=(Texture1D&&) = default;
		~Texture1D() = default;

		static std::shared_ptr<Texture1D> CreateTexture1D(gsl::not_null<ID3D11Device*> device, const D3D11_TEXTURE1D_DESC& textureDesc);
		static std::shared_ptr<Texture1D> CreateTexture1D(gsl::not_null<ID3D11Device*> device, std::uint32_t width, std::uint32_t mipLevels = 1, std::uint32_t arraySize = 1, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SAMPLE_DESC sampleDesc = { 1, 0 }, std::uint32_t bindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, std::uint32_t cpuAccessFlags = 0);

		std::uint32_t Width() const;
		ID3D11Texture1D* GetTexResource();

	private:
		std::uint32_t mWidth;
		ID3D11Texture1D* texResource;
	};
}