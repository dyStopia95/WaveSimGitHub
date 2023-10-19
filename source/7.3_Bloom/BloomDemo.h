#pragma once

#include <gsl\gsl>
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include <map>
#include "DrawableGameComponent.h"
#include "FullScreenRenderTarget.h"
#include "Bloom.h"

namespace Rendering
{
	class DiffuseLightingDemo;

	class BloomDemo final : public Library::DrawableGameComponent
	{
	public:
		BloomDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		BloomDemo(const BloomDemo&) = delete;
		BloomDemo(BloomDemo&&) = default;
		BloomDemo& operator=(const BloomDemo&) = default;		
		BloomDemo& operator=(BloomDemo&&) = default;
		~BloomDemo();

		std::shared_ptr<DiffuseLightingDemo> DiffuseLighting() const;

		bool BloomEnabled() const;
		void SetBloomEnabled(bool enabled);
		void ToggleBloom();

		Library::BloomDrawModes DrawMode() const;
		const std::string& DrawModeString() const;
		void SetDrawMode(Library::BloomDrawModes drawMode);
		
		const Library::BloomSettings& GetBloomSettings() const;
		void SetBloomSettings(Library::BloomSettings& settings);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		std::shared_ptr<DiffuseLightingDemo> mDiffuseLightingDemo;		
		Library::FullScreenRenderTarget mRenderTarget;
		Library::Bloom mBloom;
		bool mBloomEnabled{ true };
	};
}