#include "pch.h"
#include "DynamicLODDemo.h"
#include "DynamicLODMaterial.h"
#include "Camera.h"
#include "VertexDeclarations.h"
#include "Game.h"
#include "GameException.h"
#include "Model.h"
#include "Mesh.h"
#include "FirstPersonCamera.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	DynamicLODDemo::DynamicLODDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera),
		mMaterial(game)
	{
	}

	DynamicLODDemo::~DynamicLODDemo()
	{
	}

	int DynamicLODDemo::MaxTessellationFactor() const
	{
		return mMaterial.MaxTessellationFactor();
	}

	void DynamicLODDemo::SetMaxTessellationFactor(int maxTessellationFactor)
	{
		mMaterial.SetMaxTessellationFactor(maxTessellationFactor);
	}

	XMFLOAT2 DynamicLODDemo::TessellationDistances() const
	{
		return mMaterial.TessellationDistances();
	}

	void DynamicLODDemo::SetTessellationDistances(DirectX::XMFLOAT2 tessellationDistances)
	{
		mMaterial.SetTessellationDistances(tessellationDistances);
	}

	void DynamicLODDemo::SetTessellationDistances(float minTessellationDistance, float maxTessellationDistance)
	{
		mMaterial.SetTessellationDistances(minTessellationDistance, maxTessellationDistance);
	}

	void DynamicLODDemo::Initialize()
	{
		mMaterial.Initialize();

		auto direct3DDevice = mGame->Direct3DDevice();
		const auto model = mGame->Content().Load<Model>(L"Models\\Icosahedron.obj.bin"s);
		Mesh* mesh = model->Meshes().at(0).get();
		VertexPosition::CreateVertexBuffer(direct3DDevice, *mesh, mVertexBuffer.put());
		mesh->CreateIndexBuffer(*direct3DDevice, mIndexBuffer.put());
		mIndexCount = narrow<uint32_t>(mesh->Indices().size());

		auto updateMaterialFunc = [this]() { mUpdateMaterial = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateMaterialFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateMaterialFunc);

		auto firstPersonCamera = mCamera->As<FirstPersonCamera>();
		if (firstPersonCamera != nullptr)
		{
			firstPersonCamera->AddPositionUpdatedCallback([this]() {
				mMaterial.UpdateCameraPosition(mCamera->Position());
			});
		}
	}

	void DynamicLODDemo::Draw(const GameTime&)
	{
		if (mUpdateMaterial)
		{
			const XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
			const XMMATRIX wvp = XMMatrixTranspose(worldMatrix * mCamera->ViewProjectionMatrix());
			mMaterial.UpdateTransforms(wvp, XMMatrixTranspose(worldMatrix));
			mUpdateMaterial = false;
		}

		mMaterial.DrawIndexed(mVertexBuffer.get(), mIndexBuffer.get(), narrow_cast<uint32_t>(mIndexCount));
	}
}