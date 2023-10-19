#pragma once

#include <gsl\gsl>
#include <winrt\base.h>
#include <d3d11.h>
#include "DrawableGameComponent.h"
#include "FullScreenQuad.h"

namespace Rendering
{
	class ComputeShaderMaterial;

	class ComputeShaderDemo final : public Library::DrawableGameComponent
	{
	public:
		ComputeShaderDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		ComputeShaderDemo(const ComputeShaderDemo&) = delete;
		ComputeShaderDemo(ComputeShaderDemo&&) = default;
		ComputeShaderDemo& operator=(const ComputeShaderDemo&) = default;		
		ComputeShaderDemo& operator=(ComputeShaderDemo&&) = default;
		~ComputeShaderDemo();

		bool AnimationEnabled() const;
		void SetAnimationEnabled(bool enabled);

		float BlueColor() const;
		void SetBlueColor(float blueColor);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		std::unique_ptr<ComputeShaderMaterial> mMaterial;		
		Library::FullScreenQuad mFullScreenQuad;
		winrt::com_ptr<ID3D11ShaderResourceView> mColorTexture;
		bool mAnimationEnabled{ true };
	};
}