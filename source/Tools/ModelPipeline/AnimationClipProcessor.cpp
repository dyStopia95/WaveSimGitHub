#include "pch.h"
#include "AnimationClipProcessor.h"
#include "AnimationClip.h"
#include "BoneAnimationProcessor.h"
#include "BoneAnimation.h"
#include <assimp/scene.h>

using namespace std;
using namespace gsl;
using namespace DirectX;
using namespace Library;

namespace ModelPipeline
{
	shared_ptr<AnimationClip> AnimationClipProcessor::LoadAnimationClip(Library::Model& model, aiAnimation& animation)
	{
		AnimationClipData animationClipData(animation.mName.C_Str(), static_cast<float>(animation.mDuration), static_cast<float>(animation.mTicksPerSecond));

		assert(animation.mNumChannels > 0);

		if (animationClipData.TicksPerSecond <= 0.0f)
		{
			animationClipData.TicksPerSecond = 1.0f;
		}

		animationClipData.BoneAnimations.reserve(animation.mNumChannels);
		for (unsigned int i = 0; i < animation.mNumChannels; i++)
		{
			shared_ptr<BoneAnimation> boneAnimation = BoneAnimationProcessor::LoadBoneAnimation(model, *(animation.mChannels[i]));
			animationClipData.BoneAnimations.push_back(boneAnimation);

			assert(animationClipData.BoneAnimationsByBone.find(&(boneAnimation->GetBone())) == animationClipData.BoneAnimationsByBone.end());
			animationClipData.BoneAnimationsByBone[&(boneAnimation->GetBone())] = boneAnimation;
		}

		for (auto& boneAnimation : animationClipData.BoneAnimations)
		{
			if (boneAnimation->Keyframes().size() > animationClipData.KeyframeCount)
			{
				animationClipData.KeyframeCount = narrow_cast<uint32_t>(boneAnimation->Keyframes().size());
			}
		}

		return make_shared<AnimationClip>(move(animationClipData));
	}
}