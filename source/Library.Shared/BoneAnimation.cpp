#include "pch.h"
#include "BoneAnimation.h"
#include "Model.h"
#include "Bone.h"
#include "Keyframe.h"
#include "StreamHelper.h"
#include "VectorHelper.h"

using namespace std;
using namespace gsl;
using namespace DirectX;

namespace Library
{
	BoneAnimation::BoneAnimation(Model& model, InputStreamHelper& streamHelper) :
		mModel(&model)
	{
		Load(streamHelper);
	}

	BoneAnimation::BoneAnimation(Model& model, const BoneAnimationData& boneAnimationData) :
		mModel(&model), mBone(model.Bones().at(boneAnimationData.BoneIndex)), mKeyframes(boneAnimationData.Keyframes)
	{
	}

	BoneAnimation::BoneAnimation(Model& model, BoneAnimationData&& boneAnimationData) :
		mModel(&model), mBone(model.Bones().at(boneAnimationData.BoneIndex)), mKeyframes(move(boneAnimationData.Keyframes))
	{
		boneAnimationData.BoneIndex = 0U;
	}

	Bone& BoneAnimation::GetBone()
	{
		auto bone = mBone.lock();
		assert(bone != nullptr);
		
		return *bone;
	}
	
	vector<shared_ptr<Keyframe>>& BoneAnimation::Keyframes()
	{
		return mKeyframes;
	}

	uint32_t BoneAnimation::GetTransform(float time, XMFLOAT4X4& transform) const
	{
		uint32_t keyframeIndex = FindKeyframeIndex(time);
		const shared_ptr<Keyframe>& keyframe = mKeyframes[keyframeIndex];
		XMStoreFloat4x4(&transform, keyframe->Transform());

		return keyframeIndex;
	}

	void BoneAnimation::GetTransformAtKeyframe(uint32_t keyframeIndex, XMFLOAT4X4& transform) const
	{
		// Clamp the keyframe
		if (keyframeIndex >= mKeyframes.size())
		{
			keyframeIndex = narrow_cast<uint32_t>(mKeyframes.size() - 1);
		}
		
		const shared_ptr<Keyframe>& keyframe = mKeyframes[keyframeIndex];
		XMStoreFloat4x4(&transform, keyframe->Transform());
	}

	void BoneAnimation::GetInteropolatedTransform(float time, XMFLOAT4X4& transform) const
	{
		const shared_ptr<Keyframe>& firstKeyframe = mKeyframes.front();
		const shared_ptr<Keyframe>& lastKeyframe = mKeyframes.back();

		if (time <= firstKeyframe->Time())
		{
			// Specified time is before the start time of the animation, so return the first keyframe
			XMStoreFloat4x4(&transform, firstKeyframe->Transform());
		}
		else if (time >= lastKeyframe->Time())
		{
			// Specified time is after the end time of the animation, so return the last keyframe
			XMStoreFloat4x4(&transform, lastKeyframe->Transform());
		}
		else
		{
			// Interpolate the transform between keyframes
			const uint32_t keyframeIndex = FindKeyframeIndex(time);
			const shared_ptr<Keyframe>& keyframeOne = mKeyframes[keyframeIndex];
			const shared_ptr<Keyframe>& keyframeTwo = mKeyframes[keyframeIndex + 1];

			const XMVECTOR translationOne = keyframeOne->TranslationVector();
			const XMVECTOR rotationQuaternionOne = keyframeOne->RotationQuaternionVector();
			const XMVECTOR scaleOne = keyframeOne->ScaleVector();
			
			const XMVECTOR translationTwo = keyframeTwo->TranslationVector();
			const XMVECTOR rotationQuaternionTwo = keyframeTwo->RotationQuaternionVector();
			const XMVECTOR scaleTwo = keyframeTwo->ScaleVector();

			float lerpValue = ((time - keyframeOne->Time()) / (keyframeTwo->Time() - keyframeOne->Time()));
			const XMVECTOR translation = XMVectorLerp(translationOne, translationTwo, lerpValue);
			const XMVECTOR rotationQuaternion = XMQuaternionSlerp(rotationQuaternionOne, rotationQuaternionTwo, lerpValue);
			const XMVECTOR scale = XMVectorLerp(scaleOne, scaleTwo, lerpValue);

			const XMVECTOR rotationOrigin = XMLoadFloat4(&Vector4Helper::Zero);
			XMStoreFloat4x4(&transform, XMMatrixAffineTransformation(scale, rotationOrigin, rotationQuaternion, translation));
		}
	}

	void BoneAnimation::Save(OutputStreamHelper& streamHelper)
	{
		Bone& bone = GetBone();
		streamHelper << bone.Name();

		streamHelper << narrow_cast<uint32_t>(mKeyframes.size());
		for (auto& keyframe: mKeyframes)
		{
			keyframe->Save(streamHelper);
		}
	}

	void BoneAnimation::Load(InputStreamHelper& streamHelper)
	{
		// Deserialize the referenced bone
		string name;
		streamHelper >> name;
		uint32_t boneIndex = mModel->BoneIndexMapping().at(name);		
		mBone = mModel->Bones().at(boneIndex);

		// Deserialize the keyframes
		uint32_t keyframeCount;
		streamHelper >> keyframeCount;
		mKeyframes.reserve(keyframeCount);
		for (uint32_t i = 0; i < keyframeCount; i++)
		{
			mKeyframes.push_back(make_shared<Keyframe>(streamHelper));
		}
	}

	uint32_t BoneAnimation::FindKeyframeIndex(float time) const
	{
		const shared_ptr<Keyframe>& firstKeyframe = mKeyframes.front();
		if (time <= firstKeyframe->Time())
		{
			return 0;
		}

		const shared_ptr<Keyframe>& lastKeyframe = mKeyframes.back();
		if (time >= lastKeyframe->Time())
		{
			return narrow_cast<uint32_t>(mKeyframes.size() - 1);
		}

		uint32_t keyframeIndex = 1;

		for (; keyframeIndex < mKeyframes.size() - 1 && time >= mKeyframes[keyframeIndex]->Time(); keyframeIndex++);

		return keyframeIndex - 1;
	}
}
