#include "pch.h"
#include "CpuWriteToTextureDemo.h"
#include "Game.h"
#include "GameException.h"
#include "FullScreenQuadMaterial.h"
#include "PixelShader.h"
#include "Texture2D.h"
#include "DirectXTK/SimpleMath.h"
#include "ColorHelper.h"

using namespace std;
using namespace std::string_literals;
using namespace std::chrono;
using namespace gsl;
using namespace winrt;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	CpuWriteToTextureDemo::CpuWriteToTextureDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera),
		_fullScreenQuad(game)
	{
	}

	CpuWriteToTextureDemo::~CpuWriteToTextureDemo()
	{
	}

	void CpuWriteToTextureDemo::Initialize()
	{
		const auto renderTargetSize = mGame->RenderTargetSize();
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = renderTargetSize.cx;
		textureDesc.Height = renderTargetSize.cy;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DYNAMIC;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		_colorMap = Texture2D::CreateTexture2D(mGame->Direct3DDevice(), textureDesc);

		_fullScreenQuad.Initialize();
		auto fullScreenQuadMaterial = _fullScreenQuad.Material();
		fullScreenQuadMaterial->SetTexture(_colorMap->ShaderResourceView().get());

		auto pixelShader = mGame->Content().Load<PixelShader>(L"Shaders\\TexturedModelPS.cso");
		fullScreenQuadMaterial->SetShader(pixelShader);
	}

	void CpuWriteToTextureDemo::Draw(const GameTime& gameTime)
	{
		if (high_resolution_clock::now() > _lastTextureUpdate + _textureUpdateDelay)
		{
			com_ptr<ID3D11Resource> colorMapResource;
			_colorMap->ShaderResourceView()->GetResource(colorMapResource.put());
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			ThrowIfFailed(mGame->Direct3DDeviceContext()->Map(colorMapResource.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

			for (size_t column = 0; column < mappedResource.DepthPitch; ++column)
			{
				auto position = reinterpret_cast<uint8_t*>(mappedResource.pData) + column;
				*position = static_cast<uint8_t>(_colorDistribution(_generator));
			}

			mGame->Direct3DDeviceContext()->Unmap(colorMapResource.get(), 0);
			
			_lastTextureUpdate = high_resolution_clock::now();
		}

		_fullScreenQuad.Draw(gameTime);
		_fullScreenQuad.Material()->UnbindShaderResources<1>(ShaderStages::PS);
	}
}