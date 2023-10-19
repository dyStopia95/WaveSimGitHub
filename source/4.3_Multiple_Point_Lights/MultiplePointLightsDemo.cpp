#include "pch.h"
#include "MultiplePointLightsDemo.h"
#include "FirstPersonCamera.h"
#include "VertexDeclarations.h"
#include "Game.h"
#include "GameException.h"
#include "Model.h"
#include "Mesh.h"
#include "ProxyModel.h"
#include "MultiplePointLightsMaterial.h"
#include "Texture2D.h"
#include "PointLight.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	MultiplePointLightsDemo::MultiplePointLightsDemo(Game & game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera)
	{
	}

	MultiplePointLightsDemo::~MultiplePointLightsDemo()
	{
	}

	bool MultiplePointLightsDemo::AnimationEnabled() const
	{
		return mAnimationEnabled;
	}

	void MultiplePointLightsDemo::SetAnimationEnabled(bool enabled)
	{
		mAnimationEnabled = enabled;
	}

	void MultiplePointLightsDemo::ToggleAnimation()
	{
		mAnimationEnabled = !mAnimationEnabled;
	}

	float MultiplePointLightsDemo::AmbientLightIntensity() const
	{
		return mMaterial->AmbientColor().x;
	}

	void MultiplePointLightsDemo::SetAmbientLightIntensity(float intensity)
	{
		mMaterial->SetAmbientColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	const array<Library::PointLight, 4>& MultiplePointLightsDemo::PointLights() const
	{
		return mMaterial->PointLights();
	}

	void MultiplePointLightsDemo::SetPointLight(const Library::PointLight& light, size_t index)
	{
		mMaterial->SetPointLight(light, index);
		
		assert(index < mProxyModels.size());
		mProxyModels[index]->SetPosition(light.Position());
	}

	const size_t MultiplePointLightsDemo::SelectedLightIndex() const
	{
		return mSelectedLightIndex;
	}

	const PointLight& MultiplePointLightsDemo::SelectedLight() const
	{
		return mMaterial->PointLights()[mSelectedLightIndex];
	}

	void MultiplePointLightsDemo::UpdateSelectedLight(const PointLight& light)
	{
		SetPointLight(light, mSelectedLightIndex);
	}

	void MultiplePointLightsDemo::SelectLight(size_t index)
	{
		assert(index < mMaterial->PointLights().size());

		const XMFLOAT4 selectedColor{ Colors::Yellow.f };
		const XMFLOAT4 unSelectedColor{ Colors::White.f };

		mProxyModels[mSelectedLightIndex]->SetColor(unSelectedColor);
		mSelectedLightIndex = index;
		mProxyModels[mSelectedLightIndex]->SetColor(selectedColor);
	}

	float MultiplePointLightsDemo::SpecularPower() const
	{
		return mMaterial->SpecularPower();
	}

	void MultiplePointLightsDemo::SetSpecularPower(float power)
	{
		mMaterial->SetSpecularPower(power);
	}

	void MultiplePointLightsDemo::Initialize()
	{
		auto direct3DDevice = mGame->Direct3DDevice();

		const auto model = mGame->Content().Load<Model>(L"Models\\Sphere.obj.bin"s);
		Mesh* mesh = model->Meshes().at(0).get();
		VertexPositionTextureNormal::CreateVertexBuffer(direct3DDevice, *mesh, not_null<ID3D11Buffer**>(mVertexBuffer.put()));
		mesh->CreateIndexBuffer(*direct3DDevice, not_null<ID3D11Buffer**>(mIndexBuffer.put()));
		mIndexCount = narrow<uint32_t>(mesh->Indices().size());

		auto colorMap = mGame->Content().Load<Texture2D>(L"Textures\\EarthComposite.dds"s);
		auto specularMap = mGame->Content().Load<Texture2D>(L"Textures\\EarthSpecularMap.png"s);
		mMaterial = make_shared<MultiplePointLightsMaterial>(*mGame, colorMap, specularMap);
		mMaterial->Initialize();

		for (size_t i = 0; i < mProxyModels.size(); ++i)
		{
			auto& proxyModel = mProxyModels[i];

			proxyModel = make_unique<ProxyModel>(*mGame, mCamera, "Models\\PointLightProxy.obj.bin"s, 0.5f);
			proxyModel->Initialize();
		}
				
		SetPointLight(PointLight(XMFLOAT3(10.0f, 0.0, 0.0f)), 0);
		SetPointLight(PointLight(XMFLOAT3(0.0f, -10.0, 0.0f)), 1);
		SetPointLight(PointLight(XMFLOAT3(-10.0f, 0.0, 0.0f)), 2);
		SetPointLight(PointLight(XMFLOAT3(0.0f, +10.0, 0.0f)), 3);
		SelectLight(0);

		auto firstPersonCamera = mCamera->As<FirstPersonCamera>();
		if (firstPersonCamera != nullptr)
		{
			firstPersonCamera->AddPositionUpdatedCallback([this]() {
				mMaterial->UpdateCameraPosition(mCamera->Position());
			});
		}

		auto updateMaterialFunc = [this]() { mUpdateMaterial = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateMaterialFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateMaterialFunc);
	}

	void MultiplePointLightsDemo::Update(const GameTime& gameTime)
	{
		if (mAnimationEnabled)
		{
			mModelRotationAngle += gameTime.ElapsedGameTimeSeconds().count() * RotationRate;
			XMStoreFloat4x4(&mWorldMatrix, XMMatrixRotationY(mModelRotationAngle));			
			mUpdateMaterial = true;
		}

		for (auto& proxyModel : mProxyModels)
		{
			proxyModel->Update(gameTime);
		}
	}

	void MultiplePointLightsDemo::Draw(const GameTime& gameTime)
	{
		if (mUpdateMaterial)
		{
			const XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
			const XMMATRIX wvp = XMMatrixTranspose(worldMatrix * mCamera->ViewProjectionMatrix());
			mMaterial->UpdateTransforms(wvp, XMMatrixTranspose(worldMatrix));
			mUpdateMaterial = false;
		}

		mMaterial->DrawIndexed(not_null<ID3D11Buffer*>(mVertexBuffer.get()), not_null<ID3D11Buffer*>(mIndexBuffer.get()), mIndexCount);

		for (auto& proxyModel : mProxyModels)
		{
			proxyModel->Draw(gameTime);
		}		
	}
}