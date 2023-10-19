#pragma once

#include <gsl\gsl>
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include <random>
#include <chrono>
#include "DrawableGameComponent.h"
#include "FullScreenQuad.h"
#include "Texture2D.h"

using namespace std::chrono_literals;

namespace Rendering
{
	class CpuWriteToTextureDemo final : public Library::DrawableGameComponent
	{
	public:
		CpuWriteToTextureDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		CpuWriteToTextureDemo(const CpuWriteToTextureDemo&) = delete;
		CpuWriteToTextureDemo(CpuWriteToTextureDemo&&) = default;
		CpuWriteToTextureDemo& operator=(const CpuWriteToTextureDemo&) = default;		
		CpuWriteToTextureDemo& operator=(CpuWriteToTextureDemo&&) = default;
		~CpuWriteToTextureDemo();
		
		virtual void Initialize() override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		inline static const std::chrono::seconds _textureUpdateDelay { 1 };

		Library::FullScreenQuad _fullScreenQuad;
		std::shared_ptr<Library::Texture2D>	_colorMap;
		
		std::random_device _device;
		std::default_random_engine _generator{ _device() };
		std::uniform_int_distribution<uint32_t> _colorDistribution{ 0, 255 };
		std::chrono::high_resolution_clock::time_point _lastTextureUpdate;
	};
}