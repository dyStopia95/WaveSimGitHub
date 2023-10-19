#pragma once

#include <winrt\Windows.Foundation.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>

namespace Library
{
	class Model;
	class Bone;
	class BoneAnimation;
	class OutputStreamHelper;
	class InputStreamHelper;

	struct AnimationClipData final
	{
		AnimationClipData() = default;
		AnimationClipData(const std::string& name, float duration, float ticksPerSecond);
		AnimationClipData(const AnimationClipData&) = default;
		AnimationClipData(AnimationClipData&&) = default;
		AnimationClipData& operator=(const AnimationClipData&) = default;
		AnimationClipData& operator=(AnimationClipData&&) = default;
		~AnimationClipData() = default;

		std::string Name;
		float Duration{ 0.0f };
		float TicksPerSecond{ 0.0f };
		std::vector<std::shared_ptr<BoneAnimation>> BoneAnimations;
		std::map<Bone*, std::shared_ptr<BoneAnimation>> BoneAnimationsByBone;
		std::uint32_t KeyframeCount{ 0 };
	};

    class AnimationClip final
    {
    public:
		AnimationClip(Model& model, InputStreamHelper& streamHelper);
		explicit AnimationClip(AnimationClipData&& animationClipData);
		AnimationClip(const AnimationClip&) = default;
		AnimationClip(AnimationClip&&) = default;
		AnimationClip& operator=(const AnimationClip&) = default;
		AnimationClip& operator=(AnimationClip&&) = default;
		~AnimationClip() = default;

		const std::string& Name() const;
		float Duration() const;
		float TicksPerSecond() const;
		const std::vector<std::shared_ptr<BoneAnimation>>& BoneAnimations() const;
		const std::map<Bone*, std::shared_ptr<BoneAnimation>>& BoneAnimationsByBone() const;
		const std::uint32_t KeyframeCount() const;

		std::uint32_t GetTransform(float time, Bone& bone, DirectX::XMFLOAT4X4& transform) const;
		void GetTransforms(float time, std::vector<DirectX::XMFLOAT4X4>& boneTransforms) const;
		
		void GetTransformAtKeyframe(std::uint32_t keyframe, Bone& bone, DirectX::XMFLOAT4X4& transform) const;
		void GetTransformsAtKeyframe(std::uint32_t keyframe, std::vector<DirectX::XMFLOAT4X4>& boneTransforms) const;

		void GetInteropolatedTransform(float time, Bone& bone, DirectX::XMFLOAT4X4& transform) const;
		void GetInteropolatedTransforms(float time, std::vector<DirectX::XMFLOAT4X4>& boneTransforms) const;

		void Save(OutputStreamHelper& streamHelper);

    private:
		void Load(Model& model, InputStreamHelper& streamHelper);

		AnimationClipData mData;
    };
}
