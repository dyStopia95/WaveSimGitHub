#pragma once

#include <gsl\gsl>
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include <map>
#include "DrawableGameComponent.h"
#include "FullScreenRenderTarget.h"
#include "FullScreenQuad.h"
#include "MatrixHelper.h"

namespace Library
{
	class PixelShader;
}

namespace Rendering
{
	class DiffuseLightingDemo;

	enum class ColorFilters
	{
		GrayScale = 0,
		Inverse,
		Sepia,
		Generic,
		End
	};

	class ColorFilteringDemo final : public Library::DrawableGameComponent
	{
	public:
		ColorFilteringDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		ColorFilteringDemo(const ColorFilteringDemo&) = delete;
		ColorFilteringDemo(ColorFilteringDemo&&) = default;
		ColorFilteringDemo& operator=(const ColorFilteringDemo&) = default;		
		ColorFilteringDemo& operator=(ColorFilteringDemo&&) = default;
		~ColorFilteringDemo();

		std::shared_ptr<DiffuseLightingDemo> DiffuseLighting() const;

		ColorFilters ActiveColorFilter() const;
		void SetActiveColorFilter(ColorFilters colorFilter);

		float GenericFilterBrightness() const;
		void SetGenericFilterBrightness(float brightness);
		
		static const std::map<ColorFilters, std::string> ColorFilterNames;

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		struct GenericColorFilterPSConstantBuffer
		{
			DirectX::XMFLOAT4X4 ColorFilter{ Library::MatrixHelper::Identity };
		};

		std::shared_ptr<DiffuseLightingDemo> mDiffuseLightingDemo;		
		Library::FullScreenRenderTarget mRenderTarget;
		Library::FullScreenQuad mFullScreenQuad;
		std::map<ColorFilters, std::shared_ptr<Library::PixelShader>> mPixelShadersByColorFilter;
		ColorFilters mActiveColorFilter;	
		winrt::com_ptr<ID3D11Buffer> mGenericColorFilterPSConstantBuffer;
		GenericColorFilterPSConstantBuffer mGenericColorFilterPSConstantBufferData;
	};
}