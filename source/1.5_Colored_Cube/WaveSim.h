#pragma once

#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include "DrawableGameComponent.h"
#include "VectorHelper.h"
#include "MatrixHelper.h"
#include "NodeArray.h"
#include "VertexDeclarations.h"
#include "BasicMaterial.h"

namespace Rendering
{
	

	class WaveSim : public Library::DrawableGameComponent
	{
		NodeArray _nodeArray;
		Library::BasicMaterial mMaterial;
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		DirectX::XMFLOAT3 mPosition{ Library::Vector3Helper::Zero };
		DirectX::XMFLOAT4X4 mWorldMatrix{ Library::MatrixHelper::Identity };
		bool mUpdateMaterial{ true };
		DirectX::XMFLOAT4 mColor;
		SimParams _parameters;

		int length;
		int size;
		ID3D11Device* direct3DDevice;
		std::unique_ptr<Library::VertexPosition[]> vertexData;


		void InitializeGrid();
		void UpdateVertexBuffer();

	public:
		WaveSim
		(
			Library::Game& game,
			const std::shared_ptr<Library::Camera>& camera,
			const DirectX::XMFLOAT4& color = DefaultColor
		);

		virtual void Initialize() override;
		void SetParameters(SimParams& params);
		virtual void Update(const Library::GameTime& gameTime) override;

		virtual void Draw(const Library::GameTime& gameTime) override;

		const DirectX::XMFLOAT3& Position() const;
		void SetPosition(const DirectX::XMFLOAT3& position);
		void SetPosition(float x, float y, float z);

		void SetColor(const DirectX::XMFLOAT4& color);

		inline static const DirectX::XMFLOAT4 DefaultColor{ 0.961f, 0.871f, 0.702f, 1.0f };
	};
}