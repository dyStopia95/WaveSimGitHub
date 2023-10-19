#include "pch.h"
#include "AnimationClip.h"
#include "BoneAnimation.h"
#include "Bone.h"
#include "MatrixHelper.h"
#include "StreamHelper.h"

using namespace std;
using namespace gsl;
using namespace DirectX;

namespace Library
{
#pragma region AnimationClipData

	AnimationClipData::AnimationClipData(const string& name, float duration, float ticksPerSecond) :
		Name(name), Duration(duration), TicksPerSecond(ticksPerSecond)
	{
	}

#pragma endregion

	AnimationClip::AnimationClip(Model& model, InputStreamHelper& streamHelper)
	{
		Load(model, streamHelper);
	}

	AnimationClip::AnimationClip(AnimationClipData&& animationClipData) :
		mData(move(animationClipData))
	{		
	}

	const string& AnimationClip::Name() const
	{
		return mData.Name;
	}

	float AnimationClip::Duration() const
	{
		return mData.Duration;
	}

	float AnimationClip::TicksPerSecond() const
	{
		return mData.TicksPerSecond;
	}

	const vector<shared_ptr<BoneAnimation>>& AnimationClip::BoneAnimations() const
	{
		return mData.BoneAnimations;
	}

	const map<Bone*, shared_ptr<BoneAnimation>>& AnimationClip::BoneAnimationsByBone() const
	{
		return mData.BoneAnimationsByBone;
	}

	const uint32_t AnimationClip::KeyframeCount() const
	{
		return mData.KeyframeCount;
	}

	uint32_t AnimationClip::GetTransform(float time, Bone& bone, XMFLOAT4X4& transform) const
	{
		auto foundBoneAnimation = mData.BoneAnimationsByBone.find(&bone);
		if (foundBoneAnimation != mData.BoneAnimationsByBone.end())
		{
			return foundBoneAnimation->second->GetTransform(time, transform);
		}
		else
		{
			transform = MatrixHelper::Identity;
			return numeric_limits<uint32_t>::max();
		}
	}

	void AnimationClip::GetTransforms(float time, vector<XMFLOAT4X4>& boneTransforms) const
	{
		for (auto& boneAnimation : mData.BoneAnimations)
		{
			boneAnimation->GetTransform(time, boneTransforms[boneAnimation->GetBone().Index()]);
		}
	}

	void AnimationClip::GetTransformAtKeyframe(uint32_t keyframe, Bone& bone, XMFLOAT4X4& transform) const
	{
		auto foundBoneAnimation = mData.BoneAnimationsByBone.find(&bone);
		if (foundBoneAnimation != mData.BoneAnimationsByBone.end())
		{
			foundBoneAnimation->second->GetTransformAtKeyframe(keyframe, transform);
		}
		else
		{
			transform = MatrixHelper::Identity;
		}
	}

	void AnimationClip::GetTransformsAtKeyframe(uint32_t keyframe, vector<XMFLOAT4X4>& boneTransforms) const
	{
		for (auto& boneAnimation : mData.BoneAnimations)
		{
			boneAnimation->GetTransformAtKeyframe(keyframe, boneTransforms[boneAnimation->GetBone().Index()]);
		}
	}

	void AnimationClip::GetInteropolatedTransform(float time, Bone& bone, XMFLOAT4X4& transform) const
	{
		auto foundBoneAnimation = mData.BoneAnimationsByBone.find(&bone);
		if (foundBoneAnimation != mData.BoneAnimationsByBone.end())
		{
			foundBoneAnimation->second->GetInteropolatedTransform(time, transform);
		}
		else
		{
			transform = MatrixHelper::Identity;
		}
	}

	void AnimationClip::GetInteropolatedTransforms(float time, vector<XMFLOAT4X4>& boneTransforms) const
	{
		for (auto& boneAnimation : mData.BoneAnimations)
		{
			boneAnimation->GetInteropolatedTransform(time, boneTransforms[boneAnimation->GetBone().Index()]);
		}
	}

	void AnimationClip::Save(OutputStreamHelper& streamHelper)
	{
		streamHelper << mData.Name << mData.Duration << mData.TicksPerSecond;
		
		// Serialize bone animations
		streamHelper << narrow_cast<uint32_t>(mData.BoneAnimations.size());
		for (auto& boneAnimation: mData.BoneAnimations)
		{
			boneAnimation->Save(streamHelper);
		}

		streamHelper << mData.KeyframeCount;
	}

	void AnimationClip::Load(Model& model, InputStreamHelper& streamHelper)
	{
		streamHelper >> mData.Name >> mData.Duration >> mData.TicksPerSecond;

		// Deserialize bone animations
		uint32_t boneAnimationCount;
		streamHelper >> boneAnimationCount;
		mData.BoneAnimations.reserve(boneAnimationCount);
		for (uint32_t i = 0; i < boneAnimationCount; i++)
		{
			shared_ptr<BoneAnimation> boneAnimation = make_shared<BoneAnimation>(model, streamHelper);
			mData.BoneAnimations.push_back(boneAnimation);
			mData.BoneAnimationsByBone[&(boneAnimation->GetBone())] = boneAnimation;
		}

		streamHelper >> mData.KeyframeCount;
	}
}
