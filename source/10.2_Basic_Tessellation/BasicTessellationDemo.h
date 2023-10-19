#pragma once

#include <gsl\gsl>
#include <winrt\base.h>
#include <d3d11.h>
#include "DrawableGameComponent.h"
#include "BasicTessellationMaterial.h"
#include "RenderStateHelper.h"

namespace Rendering
{
	class BasicTessellationDemo final : public Library::DrawableGameComponent
	{
	public:
		BasicTessellationDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		BasicTessellationDemo(const BasicTessellationDemo&) = delete;
		BasicTessellationDemo(BasicTessellationDemo&&) = default;
		BasicTessellationDemo& operator=(const BasicTessellationDemo&) = default;		
		BasicTessellationDemo& operator=(BasicTessellationDemo&&) = default;
		~BasicTessellationDemo();

		bool UseUniformTessellation() const;
		void SetUseUniformTessellation(bool useUniformTessellation);
		void ToggleUseUniformTessellation();
		bool ShowQuadTopology() const;
		void SetShowQuadTopology(bool showQuadTopology);
		void ToggleTopology();

		gsl::span<const float> EdgeFactors() const;
		void SetUniformEdgeFactors(float factor);
		void SetEdgeFactor(float factor, std::uint32_t index);
		void SetInsideFactor(float factor, std::uint32_t index);

		gsl::span<const float> InsideFactors() const;

		virtual void Initialize() override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		/*void UpdateEdgeFactors();
		void UpdateInsideEdgeFactors();*/

		Library::RenderStateHelper mRenderStateHelper;
		BasicTessellationMaterial mMaterial;
		winrt::com_ptr<ID3D11Buffer> mTriVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mQuadVertexBuffer;
		bool mUpdateMaterial{ true };
		bool mUseUniformTessellation{ true };
	};
}