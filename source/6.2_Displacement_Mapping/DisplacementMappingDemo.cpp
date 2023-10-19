#include "pch.h"
#include "DisplacementMappingDemo.h"
#include "Camera.h"
#include "VertexDeclarations.h"
#include "Game.h"
#include "GameException.h"
#include "Model.h"
#include "Mesh.h"
#include "ProxyModel.h"
#include "DisplacementMappingMaterial.h"
#include "Texture2D.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	DisplacementMappingDemo::DisplacementMappingDemo(Game & game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera)
	{
	}

	DisplacementMappingDemo::~DisplacementMappingDemo()
	{
	}

	bool DisplacementMappingDemo::RealDisplacementMapEnabled() const
	{
		return mRealDisplacementMapEnabled;
	}

	void DisplacementMappingDemo::SetRealDisplacementMapEnabled(bool enabled)
	{
		mRealDisplacementMapEnabled = enabled;
		mMaterial->SetDisplacementMap(mRealDisplacementMapEnabled ? mRealDisplacementMap : mDefaultDisplacementMap);
	}

	void DisplacementMappingDemo::ToggleRealDisplacementMap()
	{
		SetRealDisplacementMapEnabled(!mRealDisplacementMapEnabled);
	}

	float DisplacementMappingDemo::AmbientLightIntensity() const
	{
		return mMaterial->AmbientColor().x;
	}

	void DisplacementMappingDemo::SetAmbientLightIntensity(float intensity)
	{
		mMaterial->SetAmbientColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	float DisplacementMappingDemo::DirectionalLightIntensity() const
	{
		return mMaterial->LightColor().x;
	}

	void DisplacementMappingDemo::SetDirectionalLightIntensity(float intensity)
	{
		mMaterial->SetLightColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	const XMFLOAT3& DisplacementMappingDemo::LightDirection() const
	{
		return mDirectionalLight.Direction();
	}

	void DisplacementMappingDemo::RotateDirectionalLight(XMFLOAT2 amount)
	{
		XMMATRIX lightRotationMatrix = XMMatrixRotationY(amount.x) * XMMatrixRotationAxis(mDirectionalLight.RightVector(), amount.y);
		mDirectionalLight.ApplyRotation(lightRotationMatrix);
		mProxyModel->ApplyRotation(lightRotationMatrix);
		mMaterial->SetLightDirection(mDirectionalLight.DirectionToLight());
	}

	const float DisplacementMappingDemo::DisplacementScale() const
	{
		return mMaterial->DisplacementScale();
	}

	void DisplacementMappingDemo::SetDisplacementScale(float displacementScale)
	{
		mMaterial->SetDisplacementScale(displacementScale);
	}

	void DisplacementMappingDemo::Initialize()
	{
		auto direct3DDevice = mGame->Direct3DDevice();
		const auto model = mGame->Content().Load<Model>(L"Models\\Plane_HiRes.obj.bin"s);
		Mesh* mesh = model->Meshes().at(0).get();
		VertexPositionTextureNormal::CreateVertexBuffer(direct3DDevice, *mesh, not_null<ID3D11Buffer**>(mVertexBuffer.put()));
		mesh->CreateIndexBuffer(*direct3DDevice, not_null<ID3D11Buffer**>(mIndexBuffer.put()));
		mIndexCount = narrow<uint32_t>(mesh->Indices().size());

		auto colorMap = mGame->Content().Load<Texture2D>(L"Textures\\Blocks_COLOR_RGB.png"s);
		mRealDisplacementMap = mGame->Content().Load<Texture2D>(L"Textures\\Blocks_DISP.png"s);
		mDefaultDisplacementMap = mGame->Content().Load<Texture2D>(L"Textures\\DefaultDisplacementMap.png"s);
		mMaterial = make_shared<DisplacementMappingMaterial>(*mGame, colorMap, mRealDisplacementMap);
		mMaterial->Initialize();

		mProxyModel = make_unique<ProxyModel>(*mGame, mCamera, "Models\\DirectionalLightProxy.obj.bin"s, 0.5f);
		mProxyModel->Initialize();
		mProxyModel->SetPosition(10.0f, 0.0, 0.0f);
		mProxyModel->ApplyRotation(XMMatrixRotationY(XM_PIDIV2));

		mMaterial->SetLightDirection(mDirectionalLight.DirectionToLight());

		auto updateMaterialFunc = [this]() { mUpdateMaterial = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateMaterialFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateMaterialFunc);

		XMStoreFloat4x4(&mWorldMatrix, XMMatrixScaling(5.0f, 5.0f, 5.0f) * XMMatrixRotationX(XM_PIDIV2) * XMMatrixTranslation(0.0f, 2.5f, 0.0f));
	}

	void DisplacementMappingDemo::Update(const GameTime& gameTime)
	{
		mProxyModel->Update(gameTime);
	}

	void DisplacementMappingDemo::Draw(const GameTime& gameTime)
	{
		if (mUpdateMaterial)
		{
			const XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
			const XMMATRIX wvp = XMMatrixTranspose(worldMatrix * mCamera->ViewProjectionMatrix());
			mMaterial->UpdateTransforms(wvp, XMMatrixTranspose(worldMatrix));
			mUpdateMaterial = false;
		}

		mMaterial->DrawIndexed(not_null<ID3D11Buffer*>(mVertexBuffer.get()), not_null<ID3D11Buffer*>(mIndexBuffer.get()), mIndexCount);
		mProxyModel->Draw(gameTime);
	}
}