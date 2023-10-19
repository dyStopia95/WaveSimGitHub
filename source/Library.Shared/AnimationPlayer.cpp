#include "pch.h"
#include "AnimationPlayer.h"
#include "Model.h"
#include "Bone.h"
#include "GameTime.h"
#include "AnimationClip.h"

using namespace std;
using namespace DirectX;

namespace Library
{
	RTTI_DEFINITIONS(AnimationPlayer)

	AnimationPlayer::AnimationPlayer(Game& game, shared_ptr<Model> model, bool interpolationEnabled) :
		GameComponent(game),
		mModel(move(model)), mInterpolationEnabled(interpolationEnabled)
	{
		mFinalTransforms.resize(mModel->Bones().size());
	}

	const shared_ptr<Model>& AnimationPlayer::GetModel() const
	{
		return mModel;
	}

	const shared_ptr<AnimationClip>& AnimationPlayer::CurrentClip() const
	{
		return mCurrentClip;
	}

	float AnimationPlayer::CurrentTime() const
	{
		return mCurrentTime;
	}

	uint32_t AnimationPlayer::CurrentKeyframe() const
	{
		return mCurrentKeyframe;
	}
	
	const vector<XMFLOAT4X4>& AnimationPlayer::BoneTransforms() const
	{
		return mFinalTransforms;
	}

	bool AnimationPlayer::InterpolationEnabled() const
	{
		return mInterpolationEnabled;
	}

	bool AnimationPlayer::IsPlayingClip() const
	{
		return mIsPlayingClip;
	}

	bool AnimationPlayer::IsClipLooped() const
	{
		return mIsClipLooped;
	}

	void AnimationPlayer::SetInterpolationEnabled(bool enabled)
	{
		mInterpolationEnabled = enabled;
	}

	void AnimationPlayer::StartClip(const shared_ptr<AnimationClip>& clip)
	{
		mCurrentClip = clip;
		mCurrentTime = 0.0f;
		mCurrentKeyframe = 0;
		mIsPlayingClip = true;

		XMVECTOR determinant = XMMatrixDeterminant(mModel->RootNode()->TransformMatrix());
		XMMATRIX inverseRootTransform = XMMatrixInverse(&determinant, mModel->RootNode()->TransformMatrix());
		XMStoreFloat4x4(&mInverseRootTransform, inverseRootTransform);
		GetPose(mCurrentTime, *(mModel->RootNode()));
	}

	void AnimationPlayer::PauseClip()
	{
		mIsPlayingClip = false;
	}

	void AnimationPlayer::ResumeClip()
	{
		if (mCurrentClip != nullptr)
		{
			mIsPlayingClip = true;
		}
	}

	void AnimationPlayer::RestartClip()
	{
		if (mCurrentClip != nullptr)
		{
			StartClip(mCurrentClip);
		}
	}

	void AnimationPlayer::Update(const GameTime& gameTime)
	{
		if (mIsPlayingClip)
		{
			assert(mCurrentClip != nullptr);

			mCurrentTime += gameTime.ElapsedGameTimeSeconds().count() * mCurrentClip->TicksPerSecond();
			if (mCurrentTime >= mCurrentClip->Duration())
			{
				if (mIsClipLooped)
				{
					mCurrentTime = 0.0f;
				}
				else
				{
					mIsPlayingClip = false;
					return;
				}
			}

			auto& rootNode = *(mModel->RootNode());
			if (mInterpolationEnabled)
			{
				GetInterpolatedPose(mCurrentTime, rootNode);
			}
			else
			{
				GetPose(mCurrentTime, rootNode);
			}
		}
	}

	void AnimationPlayer::SetCurrentKeyFrame(uint32_t keyframe)
	{
		mCurrentKeyframe = keyframe;
		GetPoseAtKeyframe(mCurrentKeyframe, *(mModel->RootNode()));
	}

	void AnimationPlayer::GetBindPoseBottomUp(SceneNode& sceneNode)
	{
		XMMATRIX toRootTransform = sceneNode.TransformMatrix();

		auto parentNode = sceneNode.GetParent().lock();
		while (parentNode != nullptr)
		{
			toRootTransform = toRootTransform * parentNode->TransformMatrix();
			parentNode = parentNode->GetParent().lock();
		}

		Bone* bone = sceneNode.As<Bone>();
		if (bone != nullptr)
		{
			XMStoreFloat4x4(&(mFinalTransforms[bone->Index()]), bone->OffsetTransformMatrix() * toRootTransform * XMLoadFloat4x4(&mInverseRootTransform));
		}

		for (const auto& childNode : sceneNode.Children())
		{
			GetBindPoseBottomUp(*childNode);
		}
	}

	void AnimationPlayer::GetBindPose()
	{
		GetBindPose(*(mModel->RootNode()));
	}

	void AnimationPlayer::GetBindPose(SceneNode& sceneNode)
	{
		XMMATRIX toParentTransform = sceneNode.TransformMatrix();
		auto parentNode = sceneNode.GetParent().lock();
		XMMATRIX toRootTransform = (parentNode != nullptr ? toParentTransform * XMLoadFloat4x4(&(mToRootTransforms.at(parentNode.get()))) : toParentTransform);
		XMStoreFloat4x4(&(mToRootTransforms[&sceneNode]), toRootTransform);		

		Bone* bone = sceneNode.As<Bone>();
		if (bone != nullptr)
		{
			XMStoreFloat4x4(&(mFinalTransforms[bone->Index()]), bone->OffsetTransformMatrix() * toRootTransform * XMLoadFloat4x4(&mInverseRootTransform));
		}

		for (const auto& childNode : sceneNode.Children())
		{
			GetBindPose(*childNode);
		}
	}

	void AnimationPlayer::GetPose(float time, SceneNode& sceneNode)
	{
		XMFLOAT4X4 toParentTransform;
		Bone* bone = sceneNode.As<Bone>();
		if (bone != nullptr)
		{
			auto keyframe = mCurrentClip->GetTransform(time, *bone, toParentTransform);
			if (keyframe != numeric_limits<uint32_t>::max())
			{
				mCurrentKeyframe = keyframe;
			}
		}
		else
		{
			toParentTransform = sceneNode.Transform();
		}

		auto parentNode = sceneNode.GetParent().lock();
		XMMATRIX toRootTransform = (parentNode != nullptr ? XMLoadFloat4x4(&toParentTransform) * XMLoadFloat4x4(&(mToRootTransforms.at(parentNode.get()))) : XMLoadFloat4x4(&toParentTransform));
		XMStoreFloat4x4(&(mToRootTransforms[&sceneNode]), toRootTransform);

		if (bone != nullptr)
		{
			XMStoreFloat4x4(&(mFinalTransforms[bone->Index()]), bone->OffsetTransformMatrix() * toRootTransform * XMLoadFloat4x4(&mInverseRootTransform));
		}

		for (const auto& childNode : sceneNode.Children())
		{
			GetPose(time, *childNode);
		}
	}

	void AnimationPlayer::GetPoseAtKeyframe(uint32_t keyframe, SceneNode& sceneNode)
	{
		XMFLOAT4X4 toParentTransform;
		Bone* bone = sceneNode.As<Bone>();
		if (bone != nullptr)
		{
			mCurrentClip->GetTransformAtKeyframe(keyframe, *bone, toParentTransform);
		}
		else
		{
			toParentTransform = sceneNode.Transform();
		}

		auto parentNode = sceneNode.GetParent().lock();
		XMMATRIX toRootTransform = (parentNode != nullptr ? XMLoadFloat4x4(&toParentTransform) * XMLoadFloat4x4(&(mToRootTransforms.at(parentNode.get()))) : XMLoadFloat4x4(&toParentTransform));
		XMStoreFloat4x4(&(mToRootTransforms[&sceneNode]), toRootTransform);

		if (bone != nullptr)
		{
			XMStoreFloat4x4(&(mFinalTransforms[bone->Index()]), bone->OffsetTransformMatrix() * toRootTransform * XMLoadFloat4x4(&mInverseRootTransform));
		}

		for (const auto& childNode : sceneNode.Children())
		{
			GetPoseAtKeyframe(keyframe, *childNode);
		}
	}

	void AnimationPlayer::GetInterpolatedPose(float time, SceneNode& sceneNode)
	{
		XMFLOAT4X4 toParentTransform;
		Bone* bone = sceneNode.As<Bone>();
		if (bone != nullptr)
		{
			mCurrentClip->GetInteropolatedTransform(time, *bone, toParentTransform);
		}
		else
		{
			toParentTransform = sceneNode.Transform();
		}

		auto parentNode = sceneNode.GetParent().lock();
		XMMATRIX toRootTransform = (parentNode != nullptr ? XMLoadFloat4x4(&toParentTransform) * XMLoadFloat4x4(&(mToRootTransforms.at(parentNode.get()))) : XMLoadFloat4x4(&toParentTransform));
		XMStoreFloat4x4(&(mToRootTransforms[&sceneNode]), toRootTransform);

		if (bone != nullptr)
		{
			XMStoreFloat4x4(&(mFinalTransforms[bone->Index()]), bone->OffsetTransformMatrix() * toRootTransform * XMLoadFloat4x4(&mInverseRootTransform));
		}

		for (const auto& childNode : sceneNode.Children())
		{
			GetInterpolatedPose(time, *childNode);
		}
	}
}
