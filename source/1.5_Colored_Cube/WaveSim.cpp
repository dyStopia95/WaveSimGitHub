#include "pch.h"
#include "Camera.h"
#include "WaveSim.h"
#include "GameException.h"
#include "Game.h"
#include "Utility.h"
#include "VertexDeclarations.h"

using namespace std;
using namespace gsl;
using namespace DirectX;
using namespace Library;

namespace Rendering
{
	WaveSim::WaveSim(Library::Game& game, const std::shared_ptr<Library::Camera>& camera, const XMFLOAT4& color) :
		DrawableGameComponent{game,camera},
		mMaterial(*mGame),
		mColor(color),
		vertexData(nullptr)
	{

	}

	void WaveSim::Initialize()
	{
		_nodeArray.SetBulkVariables(_parameters);
		_nodeArray.Initialize();

		mMaterial.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		mMaterial.Initialize();
		SetColor(mColor);

		auto updateMaterialFunc = [this]() { mUpdateMaterial = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateMaterialFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateMaterialFunc);

		InitializeGrid();
	}

	void WaveSim::SetParameters(SimParams& params)
	{
		_parameters = params;
	}

	void WaveSim::Update(const Library::GameTime& gameTime)
	{
		_nodeArray.Update(gameTime);
		UpdateVertexBuffer();
	}

	void WaveSim::UpdateVertexBuffer()
	{
		VertexPosition* vertices = vertexData.get();

		int i = 0;
		auto& nodearray = _nodeArray.GetArray();
		for (auto& node : nodearray)
		{
			vertices[i].Position.y = node._displacement;
			++i;
		}

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		ID3D11DeviceContext3* m_d3dContext = GetGame()->Direct3DDeviceContext();

		m_d3dContext->Map(mVertexBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, vertices, size);
		m_d3dContext->Unmap(mVertexBuffer.get(), 0);

		/*D3D11_BUFFER_DESC vertexBufferDesc{ 0 };
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = size;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData{ 0 };
		vertexSubResourceData.pSysMem = vertices;*/



		//ThrowIfFailed(direct3DDevice->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mVertexBuffer.put()), "ID3D11Device::CreateBuffer() failed");
	}

	void WaveSim::InitializeGrid()
	{
		//ID3D11Device* direct3DDevice = GetGame()->Direct3DDevice();
		direct3DDevice = GetGame()->Direct3DDevice();
		//int length = _nodeArray.GetNodeCount();
		length = _nodeArray.GetNodeCount();

		//int size = sizeof(VertexPosition) * length;
		size = sizeof(VertexPosition) * length;
		
		//std::unique_ptr<VertexPosition> vertexData(new VertexPosition[length]);
		vertexData = make_unique<VertexPosition[]>(length);

		VertexPosition* vertices = vertexData.get();

		int i = 0;
		auto& nodearray = _nodeArray.GetArray();
		for (auto& node : nodearray)
		{
			vertices[i] = VertexPosition{ XMFLOAT4{node._position.x	,node._displacement ,node._position.y ,1} };
			++i;
		}

		D3D11_BUFFER_DESC vertexBufferDesc{ 0 };
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC; //MADE DYNAMIC **
		vertexBufferDesc.ByteWidth = size;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData{ 0 };
		vertexSubResourceData.pSysMem = vertices;

		ThrowIfFailed(direct3DDevice->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mVertexBuffer.put()), "ID3D11Device::CreateBuffer() failed");
	}

	void WaveSim::Draw(const Library::GameTime&)
	{
		const XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		const XMMATRIX wvp = XMMatrixTranspose(worldMatrix * mCamera->ViewProjectionMatrix());
		mMaterial.UpdateTransform(wvp);

		mMaterial.Draw(not_null<ID3D11Buffer*>(mVertexBuffer.get()), _nodeArray.GetNodeCount(), 0);
	}

	const XMFLOAT3& WaveSim::Position() const
	{
		return mPosition;
	}

	void WaveSim::SetPosition(const XMFLOAT3& position)
	{
		mPosition = position;

		XMMATRIX translation = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
		XMStoreFloat4x4(&mWorldMatrix, translation);
		mUpdateMaterial = true;
	}

	void WaveSim::SetPosition(float x, float y, float z)
	{
		SetPosition(XMFLOAT3(x, y, z));
	}

	void WaveSim::SetColor(const XMFLOAT4& color)
	{
		mMaterial.SetSurfaceColor(color);
	}
}