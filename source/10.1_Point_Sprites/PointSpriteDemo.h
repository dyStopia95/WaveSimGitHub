#pragma once

#include <gsl\gsl>
#include <winrt\base.h>
#include <d3d11.h>
#include "DrawableGameComponent.h"
#include "PointSpriteMaterial.h"

namespace Rendering
{
	class PointSpriteDemo final : public Library::DrawableGameComponent
	{
	public:
		PointSpriteDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		PointSpriteDemo(const PointSpriteDemo&) = delete;
		PointSpriteDemo(PointSpriteDemo&&) = default;
		PointSpriteDemo& operator=(const PointSpriteDemo&) = default;		
		PointSpriteDemo& operator=(PointSpriteDemo&&) = default;
		~PointSpriteDemo();

		virtual void Initialize() override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		void InitializeRandomPoints();

		PointSpriteMaterial mMaterial;
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		std::size_t mVertexCount{ 0 };
		bool mUpdateMaterial{ true };
	};
}