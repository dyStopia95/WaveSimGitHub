#include "pch.h"
#include "FullScreenQuadDemo.h"
#include "Game.h"
#include "GameException.h"
#include "FullScreenQuadMaterial.h"
#include "PixelShader.h"
#include "Texture2D.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace winrt;
using namespace Library;
using namespace DirectX;

namespace Rendering
{

	FullScreenQuadDemo::FullScreenQuadDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera),
		mFullScreenQuad(game)
	{
	}

	FullScreenQuadDemo::~FullScreenQuadDemo()
	{
	}
	
	void FullScreenQuadDemo::Initialize()
	{
		mFullScreenQuad.Initialize();
		auto fullScreenQuadMaterial = mFullScreenQuad.Material();
		auto colorMap = mGame->Content().Load<Texture2D>(L"Textures\\Cow.png"s);
		fullScreenQuadMaterial->SetTexture(colorMap->ShaderResourceView().get());
		auto pixelShader = mGame->Content().Load<PixelShader>(L"Shaders\\TexturedModelPS.cso");
		fullScreenQuadMaterial->SetShader(pixelShader);
	}

	void FullScreenQuadDemo::Draw(const GameTime& gameTime)
	{	
		mFullScreenQuad.Draw(gameTime);
		mFullScreenQuad.Material()->UnbindShaderResources<1>(ShaderStages::PS);
	}
}