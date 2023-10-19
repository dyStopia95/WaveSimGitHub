#pragma once

#include <gsl\gsl>
#include <winrt\base.h>
#include <d3d11.h>
#include <DirectXTK\SpriteBatch.h>
#include "DrawableGameComponent.h"
#include "HeightmapTessellationMaterial.h"
#include "RenderStateHelper.h"

namespace Rendering
{
	class HeightmapTessellationDemo final : public Library::DrawableGameComponent
	{
	public:
		HeightmapTessellationDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		HeightmapTessellationDemo(const HeightmapTessellationDemo&) = delete;
		HeightmapTessellationDemo(HeightmapTessellationDemo&&) = default;
		HeightmapTessellationDemo& operator=(const HeightmapTessellationDemo&) = default;		
		HeightmapTessellationDemo& operator=(HeightmapTessellationDemo&&) = default;
		~HeightmapTessellationDemo();

		bool AnimationEnabled() const;
		void SetAnimationEnabled(bool enabled);

		gsl::span<const float> EdgeFactors() const;
		gsl::span<const float> InsideFactors() const;
		void SetUniformFactors(float factor);

		float DisplacementScale() const;
		void SetDisplacementScale(float displacementScale);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		inline static const RECT HeightmapDestinationRectangle = { 0, 512, 256, 768 };

		Library::RenderStateHelper mRenderStateHelper;
		HeightmapTessellationMaterial mMaterial;
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		bool mUpdateMaterial{ true };
		std::unique_ptr<DirectX::SpriteBatch> mSpriteBatch;
		std::shared_ptr<Library::Texture2D> mHeightmap;
		bool mAnimationEnabled{ false };
	};
}