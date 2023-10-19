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
	class Texture2D;
	class TexturedModelMaterial;
}

namespace Rendering
{
	class DistortionMaskingDemo final : public Library::DrawableGameComponent
	{
	public:
		DistortionMaskingDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		DistortionMaskingDemo(const DistortionMaskingDemo&) = delete;
		DistortionMaskingDemo(DistortionMaskingDemo&&) = default;
		DistortionMaskingDemo& operator=(const DistortionMaskingDemo&) = default;		
		DistortionMaskingDemo& operator=(DistortionMaskingDemo&&) = default;
		~DistortionMaskingDemo();

		bool DrawCutoutModeEnabled() const;
		void SetDrawCutoutModeEnabled(bool enabled);
		void ToggledDrawCutoutModeEnabled();

		float DisplacementScale() const;
		void SetDisplacementScale(float displacementScale);

		virtual void Initialize() override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		struct PixelCBufferPerObject
		{
			float DisplacementScale{ 1.0f };
			DirectX::XMFLOAT3 Padding;
		};

		enum class DistortionShaderClass
		{
			Cutout = 0,
			Composite,
			NoDistortion
		};

		inline static const DirectX::XMVECTORF32 CutoutZeroColor{ DirectX::Colors::Black };

		DirectX::XMFLOAT4X4 mWorldMatrix{ Library::MatrixHelper::Identity };
		std::shared_ptr<Library::TexturedModelMaterial> mTexturedModelMaterial;
		Library::FullScreenRenderTarget mSceneRenderTarget;
		Library::FullScreenRenderTarget mCutoutRenderTarget;
		Library::FullScreenQuad mFullScreenQuad;		
		winrt::com_ptr<ID3D11Buffer> mPixelCBufferPerObject;
		PixelCBufferPerObject mPixelCBufferPerObjectData;
		std::shared_ptr<Library::Texture2D> mDistortionMap;
		std::map<DistortionShaderClass, winrt::com_ptr<ID3D11ClassInstance>> mShaderClassInstances;
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mIndexBuffer;
		std::uint32_t mIndexCount{ 0 };
		bool mDrawCutoutModeEnabled{ false };		
		bool mUpdateMaterial{ true };
	};
}