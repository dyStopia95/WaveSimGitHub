#pragma once

#include <gsl\gsl>
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include "DrawableGameComponent.h"
#include "FullScreenQuad.h"


namespace Rendering
{
	class FullScreenQuadDemo final : public Library::DrawableGameComponent
	{
	public:
		FullScreenQuadDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		FullScreenQuadDemo(const FullScreenQuadDemo&) = delete;
		FullScreenQuadDemo(FullScreenQuadDemo&&) = default;
		FullScreenQuadDemo& operator=(const FullScreenQuadDemo&) = default;		
		FullScreenQuadDemo& operator=(FullScreenQuadDemo&&) = default;
		~FullScreenQuadDemo();
		
		virtual void Initialize() override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		Library::FullScreenQuad mFullScreenQuad;
	};
}