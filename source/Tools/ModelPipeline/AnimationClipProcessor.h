#pragma once

#include <memory>

struct aiAnimation;

namespace Library
{
	class AnimationClip;
	class Model;
}

namespace ModelPipeline
{
    class AnimationClipProcessor final
    {
    public:
		AnimationClipProcessor() = delete;

		static std::shared_ptr<Library::AnimationClip> LoadAnimationClip(Library::Model& model, aiAnimation& animation);
    };
}