#pragma once

#include <windows.h>
#include <functional>
#include "Game.h"
#include <iosfwd>

namespace Library
{
	class KeyboardComponent;
	class FpsComponent;
}

namespace Rendering
{
	class BasicTessellationDemo;

	class RenderingGame final : public Library::Game
	{
	public:
		RenderingGame(std::function<void*()> getWindowCallback, std::function<void(SIZE&)> getRenderTargetSizeCallback);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;
		virtual void Shutdown() override;

		void Exit();

	private:
		inline static const DirectX::XMVECTORF32 BackgroundColor{ DirectX::Colors::CornflowerBlue };

		void UpdateTessellationOptions();
		
		std::ostringstream Join(const gsl::span<const float>& values, const std::string& delimiter);

		std::shared_ptr<Library::KeyboardComponent> mKeyboard;
		std::shared_ptr<Library::FpsComponent> mFpsComponent;
		std::shared_ptr<BasicTessellationDemo> mBasicTessellationDemo;
		float mAmbientLightIntensity{ 0.0f };
		float mDirectionalLightIntensity{ 0.0f };
	};
}