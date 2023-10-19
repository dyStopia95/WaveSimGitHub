#pragma once

#include <gsl/gsl>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <string>
#include <cstdint>

namespace Library
{
	class Model;
	class Bone;
	class Keyframe;
	class OutputStreamHelper;
	class InputStreamHelper;

	struct BoneAnimationData final
	{
		std::uint32_t BoneIndex{ 0 };
		std::vector<std::shared_ptr<Keyframe>> Keyframes;
	};

    class BoneAnimation final
    {
    public:
		BoneAnimation(Model& model, InputStreamHelper& streamHelper);
		BoneAnimation(Model& model, const BoneAnimationData& boneAnimationData);
		BoneAnimation(Model& model, BoneAnimationData&& boneAnimationData);
		BoneAnimation(const BoneAnimation&) = default;
		BoneAnimation(BoneAnimation&& rhs) = default;
		BoneAnimation& operator=(const BoneAnimation& rhs) = default;
		BoneAnimation& operator=(BoneAnimation&& rhs) = default;
		~BoneAnimation() = default;

		Bone& GetBone();
		std::vector<std::shared_ptr<Keyframe>>& Keyframes();

		std::uint32_t GetTransform(float time, DirectX::XMFLOAT4X4& transform) const;
		void GetTransformAtKeyframe(std::uint32_t keyframeIndex, DirectX::XMFLOAT4X4& transform) const;
		void GetInteropolatedTransform(float time, DirectX::XMFLOAT4X4& transform) const;

		void Save(OutputStreamHelper& streamHelper);

    private:
		void Load(InputStreamHelper& streamHelper);
		std::uint32_t FindKeyframeIndex(float time) const;

		Model* mModel;
		std::weak_ptr<Bone> mBone;
		std::vector<std::shared_ptr<Keyframe>> mKeyframes;
    };
}
