#pragma once

#include <gsl\gsl>
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include <map>
#include "DrawableGameComponent.h"
#include "FullScreenRenderTarget.h"
#include "GaussianBlur.h"

namespace Rendering
{
	class DiffuseLightingDemo;

	class GaussianBlurDemo final : public Library::DrawableGameComponent
	{
	public:
		GaussianBlurDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		GaussianBlurDemo(const GaussianBlurDemo&) = delete;
		GaussianBlurDemo(GaussianBlurDemo&&) = default;
		GaussianBlurDemo& operator=(const GaussianBlurDemo&) = default;		
		GaussianBlurDemo& operator=(GaussianBlurDemo&&) = default;
		~GaussianBlurDemo();

		std::shared_ptr<DiffuseLightingDemo> DiffuseLighting() const;

		float BlurAmount() const;
		void SetBlurAmount(float blurAmount);
		
		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		std::shared_ptr<DiffuseLightingDemo> mDiffuseLightingDemo;		
		Library::FullScreenRenderTarget mRenderTarget;
		Library::GaussianBlur mGaussianBlur;
	};
}