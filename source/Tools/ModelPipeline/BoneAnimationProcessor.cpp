#include "pch.h"
#include "BoneAnimationProcessor.h"
#include "Model.h"
#include "BoneAnimation.h"
#include "Keyframe.h"
#include <assimp/scene.h>

using namespace std;
using namespace DirectX;
using namespace Library;

namespace ModelPipeline
{
	shared_ptr<BoneAnimation> BoneAnimationProcessor::LoadBoneAnimation(Model& model, aiNodeAnim& nodeAnim)
	{
		assert(nodeAnim.mNumPositionKeys == nodeAnim.mNumRotationKeys);
		assert(nodeAnim.mNumPositionKeys == nodeAnim.mNumScalingKeys);

		BoneAnimationData boneAnimationData;
		boneAnimationData.BoneIndex = model.BoneIndexMapping().at(nodeAnim.mNodeName.C_Str());

		for (unsigned int i = 0; i < nodeAnim.mNumPositionKeys; i++)
		{
			aiVectorKey positionKey = nodeAnim.mPositionKeys[i];
			aiQuatKey rotationKey = nodeAnim.mRotationKeys[i];
			aiVectorKey scaleKey = nodeAnim.mScalingKeys[i];

			assert(positionKey.mTime == rotationKey.mTime);
			assert(positionKey.mTime == scaleKey.mTime);

			boneAnimationData.Keyframes.push_back(make_shared<Keyframe>(static_cast<float>(positionKey.mTime), XMFLOAT3(positionKey.mValue.x, positionKey.mValue.y, positionKey.mValue.z), XMFLOAT4(rotationKey.mValue.x, rotationKey.mValue.y, rotationKey.mValue.z, rotationKey.mValue.w), XMFLOAT3(scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z)));
		}

		return make_shared<BoneAnimation>(model, move(boneAnimationData));
	}
}