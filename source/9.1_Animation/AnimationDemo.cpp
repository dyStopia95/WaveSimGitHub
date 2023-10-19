#include "pch.h"
#include "AnimationDemo.h"
#include "Camera.h"
#include "VertexDeclarations.h"
#include "Game.h"
#include "GameException.h"
#include "Model.h"
#include "Mesh.h"
#include "ProxyModel.h"
#include "SkinnedModelMaterial.h"
#include "Texture2D.h"
#include "AnimationPlayer.h"
#include "AnimationClip.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	AnimationDemo::AnimationDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera)
	{
	}

	AnimationDemo::~AnimationDemo()
	{
	}

	bool AnimationDemo::ManualAdvanceEnabled() const
	{
		return mManualAdvanceEnabled;
	}

	void AnimationDemo::SetManualAdvanceEnabled(bool enabled)
	{
		mManualAdvanceEnabled = enabled;
	}

	void AnimationDemo::ToggleManualAdvance()
	{
		mManualAdvanceEnabled = !mManualAdvanceEnabled;
	}

	const unique_ptr<Library::AnimationPlayer>& AnimationDemo::AnimationPlayer() const
	{
		return mAnimationPlayer;
	}

	void AnimationDemo::TogglePause()
	{
		if (mAnimationPlayer->IsPlayingClip())
		{
			mAnimationPlayer->PauseClip();
		}
		else
		{
			mAnimationPlayer->ResumeClip();
		}
	}

	void AnimationDemo::RestartClip()
	{
		mAnimationPlayer->RestartClip();
		mUpdateMaterialBoneTransforms = true;
	}

	void AnimationDemo::IncrementCurrentKeyframe()
	{
		auto currentKeyFrame = mAnimationPlayer->CurrentKeyframe();
		currentKeyFrame++;
		if (currentKeyFrame >= mAnimationPlayer->CurrentClip()->KeyframeCount())
		{
			currentKeyFrame = 0;
		}

		mAnimationPlayer->SetCurrentKeyFrame(currentKeyFrame);
		mUpdateMaterialBoneTransforms = true;
	}

	void AnimationDemo::DecrementCurrentKeyframe()
	{
		auto currentKeyFrame = mAnimationPlayer->CurrentKeyframe();
		currentKeyFrame = (currentKeyFrame == 0 ? mAnimationPlayer->CurrentClip()->KeyframeCount() - 1 : --currentKeyFrame);
		mAnimationPlayer->SetCurrentKeyFrame(currentKeyFrame);
		mUpdateMaterialBoneTransforms = true;
	}

	bool AnimationDemo::InterpolationEnabled() const
	{
		return mAnimationPlayer->InterpolationEnabled();
	}

	void AnimationDemo::SetInterpolationEnabled(bool enabled)
	{
		mAnimationPlayer->SetInterpolationEnabled(enabled);
	}

	void AnimationDemo::ToggleInterpolation()
	{
		mAnimationPlayer->SetInterpolationEnabled(!mAnimationPlayer->InterpolationEnabled());
	}

	float AnimationDemo::AmbientLightIntensity() const
	{
		return mMaterial->AmbientColor().x;
	}

	void AnimationDemo::SetAmbientLightIntensity(float intensity)
	{
		mMaterial->SetAmbientColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	float AnimationDemo::DirectionalLightIntensity() const
	{
		return mMaterial->LightColor().x;
	}

	void AnimationDemo::SetDirectionalLightIntensity(float intensity)
	{
		mMaterial->SetLightColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	const XMFLOAT3& AnimationDemo::LightDirection() const
	{
		return mDirectionalLight.Direction();
	}

	void AnimationDemo::RotateDirectionalLight(XMFLOAT2 amount)
	{
		XMMATRIX lightRotationMatrix = XMMatrixRotationY(amount.x) * XMMatrixRotationAxis(mDirectionalLight.RightVector(), amount.y);
		mDirectionalLight.ApplyRotation(lightRotationMatrix);
		mProxyModel->ApplyRotation(lightRotationMatrix);
		mMaterial->SetLightDirection(mDirectionalLight.DirectionToLight());
	} 

	void AnimationDemo::Initialize()
	{
		auto direct3DDevice = mGame->Direct3DDevice();
		mSkinnedModel = mGame->Content().Load<Model>(L"Models\\RunningSoldier.dae.bin"s);
		Mesh* mesh = mSkinnedModel->Meshes().at(0).get();
		VertexSkinnedPositionTextureNormal::CreateVertexBuffer(direct3DDevice, *mesh, not_null<ID3D11Buffer**>(mVertexBuffer.put()));
		mesh->CreateIndexBuffer(*direct3DDevice, not_null<ID3D11Buffer**>(mIndexBuffer.put()));
		mIndexCount = narrow<uint32_t>(mesh->Indices().size());
		
		auto texture = mGame->Content().Load<Texture2D>(L"Textures\\Soldier.png"s);
		mMaterial = make_shared<SkinnedModelMaterial>(*mGame, texture);
		mMaterial->Initialize();

		mAnimationPlayer = make_unique<Library::AnimationPlayer>(*mGame, mSkinnedModel, false);
		mAnimationPlayer->StartClip(mSkinnedModel->Animations().at(0));
		mMaterial->UpdateBoneTransforms(mAnimationPlayer->BoneTransforms());
		mAnimationPlayer->CurrentClip();

		mProxyModel = make_unique<ProxyModel>(*mGame, mCamera, "Models\\DirectionalLightProxy.obj.bin"s, 0.5f);
		mProxyModel->Initialize();
		mProxyModel->SetPosition(10.0f, 0.0, 0.0f);
		mProxyModel->ApplyRotation(XMMatrixRotationY(XM_PIDIV2));

		mMaterial->SetLightDirection(mDirectionalLight.DirectionToLight());

		auto updateMaterialFunc = [this]() { mUpdateMaterial = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateMaterialFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateMaterialFunc);

		XMStoreFloat4x4(&mWorldMatrix, XMMatrixScaling(0.05f, 0.05f, 0.05f));
	}

	void AnimationDemo::Update(const GameTime& gameTime)
	{
		if (mManualAdvanceEnabled == false)
		{
			mAnimationPlayer->Update(gameTime);
			mUpdateMaterialBoneTransforms = true;
		}

		mProxyModel->Update(gameTime);
	}

	void AnimationDemo::Draw(const GameTime& gameTime)
	{
		if (mUpdateMaterial)
		{
			const XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
			const XMMATRIX wvp = XMMatrixTranspose(worldMatrix * mCamera->ViewProjectionMatrix());
			mMaterial->UpdateTransforms(wvp, XMMatrixTranspose(worldMatrix));
			mUpdateMaterial = false;
		}

		if (mUpdateMaterialBoneTransforms)
		{
			mMaterial->UpdateBoneTransforms(mAnimationPlayer->BoneTransforms());
			mUpdateMaterialBoneTransforms = false;
		}

		mMaterial->DrawIndexed(not_null<ID3D11Buffer*>(mVertexBuffer.get()), not_null<ID3D11Buffer*>(mIndexBuffer.get()), mIndexCount);
		mProxyModel->Draw(gameTime);
	}
}