#pragma once

#include <gsl\gsl>
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include <map>
#include "DrawableGameComponent.h"
#include "FullScreenRenderTarget.h"
#include "FullScreenQuad.h"

namespace Library
{
	class Texture2D;
}

namespace Rendering
{
	enum class DistortionMaps
	{
		Glass,
		Text,
		NoDistortion,
		End
	};

	class DiffuseLightingDemo;

	class DistortionMappingDemo final : public Library::DrawableGameComponent
	{
	public:
		DistortionMappingDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		DistortionMappingDemo(const DistortionMappingDemo&) = delete;
		DistortionMappingDemo(DistortionMappingDemo&&) = default;
		DistortionMappingDemo& operator=(const DistortionMappingDemo&) = default;		
		DistortionMappingDemo& operator=(DistortionMappingDemo&&) = default;
		~DistortionMappingDemo();

		std::shared_ptr<DiffuseLightingDemo> DiffuseLighting() const;

		float DisplacementScale() const;
		void SetDisplacementScale(float displacementScale);

		DistortionMaps DistortionMap() const;
		const std::string& DistortionMapString() const;
		void SetDistortionMap(DistortionMaps distortionMap);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		static const std::map<DistortionMaps, std::string> DistortionMapNames;

		struct PixelCBufferPerObject
		{
			float DisplacementScale{ 1.0f };
			DirectX::XMFLOAT3 Padding{ 0.0f, 0.0f, 0.0f };
		};

		std::shared_ptr<DiffuseLightingDemo> mDiffuseLightingDemo;		
		Library::FullScreenRenderTarget mRenderTarget;
		Library::FullScreenQuad mFullScreenQuad;		
		winrt::com_ptr<ID3D11Buffer> mPixelCBufferPerObject;
		PixelCBufferPerObject mPixelCBufferPerObjectData;
		std::map<DistortionMaps, std::shared_ptr<Library::Texture2D>> mDistortionMaps;
		DistortionMaps mActiveDistortionMap{ DistortionMaps::Glass };
	};
}