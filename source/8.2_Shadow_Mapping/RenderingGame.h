#pragma once

#include <windows.h>
#include <functional>
#include "Game.h"

namespace Library
{
	class KeyboardComponent;
	class MouseComponent;
	class GamePadComponent;
	class ImGuiComponent;
	class FpsComponent;
	class Grid;
	class Skybox;
	class FirstPersonCamera;
}

namespace Rendering
{
	class ShadowMappingDemo;
	
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

		void UpdateDrawMode();
		void UpdateAmbientLightIntensity();
		void UpdateProjector();
		void UpdateDepthBias();
		void UpdateCamera();

		std::shared_ptr<Library::FirstPersonCamera> mCamera;
		std::shared_ptr<Library::KeyboardComponent> mKeyboard;
		std::shared_ptr<Library::MouseComponent> mMouse;
		std::shared_ptr<Library::GamePadComponent> mGamePad;
		std::shared_ptr<Library::FpsComponent> mFpsComponent;
		std::shared_ptr<Library::Grid> mGrid;
		std::shared_ptr<Library::Skybox> mSkybox;
		std::shared_ptr<ShadowMappingDemo> mShadowMappingDemo;
	};
}