#include "pch.h"
#include "PointSpriteDemo.h"
#include "PointSpriteMaterial.h"
#include "Camera.h"
#include "VertexDeclarations.h"
#include "Game.h"
#include "GameException.h"
#include "Texture2D.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	PointSpriteDemo::PointSpriteDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera),
		mMaterial(game)
	{
	}

	PointSpriteDemo::~PointSpriteDemo()
	{
	}

	void PointSpriteDemo::Initialize()
	{
		mMaterial.Initialize();

		auto colorMap = mGame->Content().Load<Texture2D>(L"Textures\\BookCover.png"s);
		mMaterial.SetColorMap(colorMap);

		InitializeRandomPoints();

		auto updateMaterialFunc = [this]() { mUpdateMaterial = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateMaterialFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateMaterialFunc);
	}

	void PointSpriteDemo::Draw(const GameTime&)
	{
		if (mUpdateMaterial)
		{
			mMaterial.UpdateCameraData(XMMatrixTranspose(mCamera->ViewProjectionMatrix()), mCamera->Position(), mCamera->Up());
			mUpdateMaterial = false;
		}

		mMaterial.Draw(mVertexBuffer.get(), narrow_cast<uint32_t>(mVertexCount));
	}

	void PointSpriteDemo::InitializeRandomPoints()
	{		
		const float maxDistance = 10;
		const float minSize = 0.5f;
		const float maxSize = 5.0f;

		std::random_device randomDevice;
		std::default_random_engine randomGenerator(randomDevice());
		std::uniform_real_distribution<float> distanceDistribution(-maxDistance, maxDistance);
		std::uniform_real_distribution<float> sizeDistribution(minSize, maxSize);

		const size_t maxPoints = 100;
		vector<VertexPositionSize> vertices;
		vertices.reserve(maxPoints);
		for (size_t i = 0; i < maxPoints; i++)
		{
			float x = distanceDistribution(randomGenerator);
			float y = distanceDistribution(randomGenerator);
			float z = distanceDistribution(randomGenerator);

			float size = sizeDistribution(randomGenerator);

			vertices.emplace_back(XMFLOAT4(x, y, z, 1.0f), XMFLOAT2(size, size));
		}

		mVertexCount = vertices.size();
		VertexPositionSize::CreateVertexBuffer(mGame->Direct3DDevice(), vertices, mVertexBuffer.put());
	}
}