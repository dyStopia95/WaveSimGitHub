#include "pch.h"
#include "BasicTessellationDemo.h"
#include "BasicTessellationMaterial.h"
#include "Camera.h"
#include "VertexDeclarations.h"
#include "Game.h"
#include "GameException.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	BasicTessellationDemo::BasicTessellationDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera),
		mMaterial(game), mRenderStateHelper(game)
	{
	}

	BasicTessellationDemo::~BasicTessellationDemo()
	{
	}

	bool BasicTessellationDemo::UseUniformTessellation() const
	{
		return mUseUniformTessellation;
	}

	void BasicTessellationDemo::SetUseUniformTessellation(bool useUniformTessellation)
	{
		mUseUniformTessellation = useUniformTessellation;
	}

	void BasicTessellationDemo::ToggleUseUniformTessellation()
	{
		mUseUniformTessellation = !mUseUniformTessellation;
	}

	bool BasicTessellationDemo::ShowQuadTopology() const
	{
		return mMaterial.ShowQuadTopology();
	}

	void BasicTessellationDemo::SetShowQuadTopology(bool showQuadTopology)
	{
		mMaterial.SetShowQuadTopology(showQuadTopology);
	}

	void BasicTessellationDemo::ToggleTopology()
	{
		mMaterial.SetShowQuadTopology(!mMaterial.ShowQuadTopology());
	}

	span<const float> BasicTessellationDemo::EdgeFactors() const
	{
		return mMaterial.EdgeFactors();
	}

	void BasicTessellationDemo::SetUniformEdgeFactors(float factor)
	{
		mMaterial.SetUniformEdgeFactors(factor);
	}

	void BasicTessellationDemo::SetEdgeFactor(float factor, uint32_t index)
	{
		mMaterial.SetEdgeFactor(factor, index);
	}

	void BasicTessellationDemo::SetInsideFactor(float factor, std::uint32_t index)
	{
		mMaterial.SetInsideFactor(factor, index);
	}

	span<const float> BasicTessellationDemo::InsideFactors() const
	{
		return mMaterial.InsideFactors();
	}

	void BasicTessellationDemo::Initialize()
	{
		mMaterial.Initialize();

		auto direct3DDevice = mGame->Direct3DDevice();

		// Create vertex buffer for a triangle
		const VertexPosition triVertices[] =
		{
			VertexPosition(XMFLOAT4(-1.0f, -0.5f, 0.0f, 1.0f)),
			VertexPosition(XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)),
			VertexPosition(XMFLOAT4(1.0f, -0.5f, 0.0f, 1.0f))
		};
		VertexPosition::CreateVertexBuffer(direct3DDevice, triVertices, not_null<ID3D11Buffer **>(mTriVertexBuffer.put()));

		// Create vertex buffer for a quad
		const VertexPosition quadVertices[] =
		{
			VertexPosition(XMFLOAT4(-1.0f, 0.5f, 0.0f, 1.0f)),
			VertexPosition(XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f)),
			VertexPosition(XMFLOAT4(-1.0f, -0.5f, 0.0f, 1.0f)),
			VertexPosition(XMFLOAT4(1.0f, -0.5f, 0.0f, 1.0f))
		};
		VertexPosition::CreateVertexBuffer(direct3DDevice, quadVertices, not_null<ID3D11Buffer * *>(mQuadVertexBuffer.put()));

		auto updateMaterialFunc = [this]() { mUpdateMaterial = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateMaterialFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateMaterialFunc);
	}

	void BasicTessellationDemo::Draw(const GameTime&)
	{
		if (mUpdateMaterial)
		{
			mMaterial.UpdateTransforms(XMMatrixTranspose(mCamera->ViewProjectionMatrix()));
			mUpdateMaterial = false;
		}

		mRenderStateHelper.SaveRasterizerState();
		if (mMaterial.ShowQuadTopology())
		{
			mMaterial.Draw(mQuadVertexBuffer.get(), 4);
		}
		else
		{
			mMaterial.Draw(mTriVertexBuffer.get(), 3);
		}

		mRenderStateHelper.RestoreRasterizerState();
	}
}