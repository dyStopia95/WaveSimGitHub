#pragma once

#include <memory>

struct aiNodeAnim;

namespace Library
{
	class BoneAnimation;
	class Model;
}

namespace ModelPipeline
{
    class BoneAnimationProcessor final
    {
    public:
		BoneAnimationProcessor() = delete;

		static std::shared_ptr<Library::BoneAnimation> LoadBoneAnimation(Library::Model& model, aiNodeAnim& nodeAnim);
    };
}