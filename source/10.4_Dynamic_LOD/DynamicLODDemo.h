#pragma once

#include <gsl\gsl>
#include <winrt\base.h>
#include <d3d11.h>
#include "DrawableGameComponent.h"
#include "DynamicLODMaterial.h"
#include "MatrixHelper.h"

namespace Rendering
{
	class DynamicLODDemo final : public Library::DrawableGameComponent
	{
	public:
		DynamicLODDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		DynamicLODDemo(const DynamicLODDemo&) = delete;
		DynamicLODDemo(DynamicLODDemo&&) = default;
		DynamicLODDemo& operator=(const DynamicLODDemo&) = default;		
		DynamicLODDemo& operator=(DynamicLODDemo&&) = default;
		~DynamicLODDemo();

		int MaxTessellationFactor() const;
		void SetMaxTessellationFactor(int maxTessellationFactor);

		DirectX::XMFLOAT2 TessellationDistances() const;
		void SetTessellationDistances(DirectX::XMFLOAT2 tessellationDistances);
		void SetTessellationDistances(float minTessellationDistance, float maxTessellationDistance);

		virtual void Initialize() override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		DynamicLODMaterial mMaterial;
		DirectX::XMFLOAT4X4 mWorldMatrix{ Library::MatrixHelper::Identity };
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mIndexBuffer;
		std::size_t mIndexCount{ 0 };
		bool mUpdateMaterial{ true };
	};
}