#include "pch.h"
#include "HeightmapTessellationDemo.h"
#include "HeightmapTessellationMaterial.h"
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
	HeightmapTessellationDemo::HeightmapTessellationDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera),
		mMaterial(game), mRenderStateHelper(game)
	{
	}

	HeightmapTessellationDemo::~HeightmapTessellationDemo()
	{
	}

	bool HeightmapTessellationDemo::AnimationEnabled() const
	{
		return mAnimationEnabled;
	}

	void HeightmapTessellationDemo::SetAnimationEnabled(bool enabled)
	{
		mAnimationEnabled = enabled;
	}

	span<const float> HeightmapTessellationDemo::EdgeFactors() const
	{
		return mMaterial.EdgeFactors();
	}

	span<const float> HeightmapTessellationDemo::InsideFactors() const
	{
		return mMaterial.InsideFactors();
	}

	void HeightmapTessellationDemo::SetUniformFactors(float factor)
	{
		mMaterial.SetUniformFactors(factor);
	}

	float HeightmapTessellationDemo::DisplacementScale() const
	{
		return mMaterial.DisplacementScale();
	}

	void HeightmapTessellationDemo::SetDisplacementScale(float displacementScale)
	{
		mMaterial.SetDisplacementScale(displacementScale);
	}

	void HeightmapTessellationDemo::Initialize()
	{
		mMaterial.Initialize();

		mHeightmap = mGame->Content().Load<Texture2D>(L"Textures\\Heightmap.jpg"s);
		mMaterial.SetHeightmap(mHeightmap);

		const VertexPositionTexture vertices[] =
		{
			VertexPositionTexture(XMFLOAT4(-10.0f, 1.0f, -10.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)), // upper-left
			VertexPositionTexture(XMFLOAT4(10.0f, 1.0f, -10.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)),  // upper-right
			VertexPositionTexture(XMFLOAT4(-10.0f, 1.0f, 10.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)),  // lower-left
			VertexPositionTexture(XMFLOAT4(10.0f, 1.0f, 10.0f, 1.0f), XMFLOAT2(1.0f, 1.0f))    // lower-right
		};
		VertexPositionTexture::CreateVertexBuffer(mGame->Direct3DDevice(), vertices, mVertexBuffer.put());

		auto updateMaterialFunc = [this]() { mUpdateMaterial = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateMaterialFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateMaterialFunc);

		mSpriteBatch = make_unique<SpriteBatch>(mGame->Direct3DDeviceContext());
	}

	void HeightmapTessellationDemo::Update(const Library::GameTime& gameTime)
	{
		if (mAnimationEnabled)
		{
			static XMFLOAT2 texturePosition(0.0f, 0.0f);
			const XMFLOAT2 TextureModulationRate = XMFLOAT2(-0.1f, 0.05f);
			float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();

			texturePosition.x += TextureModulationRate.x * elapsedTime;
			texturePosition.y += TextureModulationRate.y * elapsedTime;
			mMaterial.UpdateTextureMatrix(XMMatrixTranspose(XMMatrixTranslation(texturePosition.x, texturePosition.y, 0.0f)));
		}
	}

	void HeightmapTessellationDemo::Draw(const GameTime&)
	{
		if (mUpdateMaterial)
		{
			mMaterial.UpdateTransforms(XMMatrixTranspose(mCamera->ViewProjectionMatrix()));
			mUpdateMaterial = false;
		}

		mRenderStateHelper.SaveAll();
		mMaterial.Draw(mVertexBuffer.get(), 4);
		
		mSpriteBatch->Begin();
		mSpriteBatch->Draw(mHeightmap->ShaderResourceView().get(), HeightmapDestinationRectangle);
		mSpriteBatch->End();
		mRenderStateHelper.RestoreAll();
	}
}