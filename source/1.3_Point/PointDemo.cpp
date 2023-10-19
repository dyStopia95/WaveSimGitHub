#include "pch.h"
#include "PointDemo.h"
#include "Utility.h"
#include "Game.h"
#include "GameException.h"

using namespace std;
using namespace std::string_literals;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	RTTI_DEFINITIONS(PointDemo)

	PointDemo::PointDemo(Game& game) :
		DrawableGameComponent(game)
	{
	}

	void PointDemo::Initialize()
	{
		// Load a compiled vertex shader
		vector<char> compiledVertexShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\PointDemoVS.cso"s, compiledVertexShader);		
		ThrowIfFailed(mGame->Direct3DDevice()->CreateVertexShader(compiledVertexShader.data(), compiledVertexShader.size(), nullptr, mVertexShader.put()), "ID3D11Device::CreatedVertexShader() failed.");

		// Load a compiled pixel shader
		vector<char> compiledPixelShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\PointDemoPS.cso"s, compiledPixelShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreatePixelShader(compiledPixelShader.data(), compiledPixelShader.size(), nullptr, mPixelShader.put()), "ID3D11Device::CreatedPixelShader() failed.");
	}

	void PointDemo::Draw(const GameTime&)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		direct3DDeviceContext->VSSetShader(mVertexShader.get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.get(), nullptr, 0);

		direct3DDeviceContext->Draw(1, 0);
	}
}