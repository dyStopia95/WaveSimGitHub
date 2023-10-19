#include "pch.h"
#include "Camera.h"
#include "WaveSim.h"
#include "GameException.h"
#include "Game.h"
#include "Utility.h"
#include "VertexDeclarations.h"
#include "Texture1D.h"
//#include "Texture1DArray.h"
#include <winrt\Windows.Foundation.h>

using namespace std;
using namespace gsl;
using namespace DirectX;
using namespace Library;

namespace Rendering
{
	WaveSim::WaveSim(Library::Game& game, const std::shared_ptr<Library::Camera>& camera, const XMFLOAT4& color) :
		DrawableGameComponent{game,camera},
		mColor(color),
		vertexData(nullptr)
	{

	}

	void WaveSim::Initialize()
	{
		direct3DDevice = GetGame()->Direct3DDevice();

		_nodeArray.SetBulkVariables(_parameters);
		_nodeArray.Initialize();
		length = _nodeArray.GetNodeCount();
		sizeZArray = sizeof(XMFLOAT2) * length;

		D3D11_TEXTURE1D_DESC texDesc{ 0 };
		texDesc.Width = length;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		//texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		texDesc.CPUAccessFlags = 0;
		texDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		//texDesc.Format = DXGI_FORMAT_R32_FLOAT;
		texDesc.ArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		texDesc.MiscFlags = 0;

		//XMFLOAT2* 

		HRESULT hr;
		winrt::com_ptr<ID3D11Texture1D> texture;
		if (FAILED(hr = direct3DDevice->CreateTexture1D(&texDesc, nullptr, texture.put())))
		{
			throw GameException("IDXGIDevice::CreateTexture2D() failed.", hr);
		}

		winrt::com_ptr<ID3D11ShaderResourceView> shaderResourceReview;
		if (FAILED(hr = direct3DDevice->CreateShaderResourceView(texture.get(), nullptr, shaderResourceReview.put())))
		{
			throw GameException("IDXGIDevice::CreateShaderResourceView() failed.", hr);
		}

		mMaterial = make_shared<WaveSimMaterial>(*mGame, Texture1D::CreateTexture1D(direct3DDevice, texDesc));
		//mMaterial->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		mMaterial->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mMaterial->Initialize();
		SetColor(mColor);
		mDisplacementMap = mMaterial->GetZArrayRef().get();

		ID3D11Texture1D* commonTexResource = mDisplacementMap->GetTexResource();

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory(&uavDesc, sizeof(uavDesc));
		uavDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
		uavDesc.Texture1D.MipSlice = 0;

		//HRESULT hr;
		winrt::com_ptr<ID3D11UnorderedAccessView> outputTexture;
		if (FAILED(hr = mGame->Direct3DDevice()->CreateUnorderedAccessView(commonTexResource, &uavDesc, outputTexture.put())))
		{
			throw GameException("IDXGIDevice::CreateUnorderedAccessView() failed.", hr);
		}

		mCompShader = make_shared<WaveSimCompShader>(*mGame, outputTexture);
		mCompShader->SetParams(_parameters);
		mCompShader->Initialize();

		D3D11_RASTERIZER_DESC rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
		rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME; // Set the wireframe fill mode
		rasterizerDesc.CullMode = D3D11_CULL_NONE; // Set the culling mode (you can set to NONE if you want to see both front and back faces)
		rasterizerDesc.FrontCounterClockwise = FALSE;

		ID3D11RasterizerState* pWireframeRasterizerState = nullptr;
		hr = direct3DDevice->CreateRasterizerState(&rasterizerDesc, &pWireframeRasterizerState);
		auto d3dContext = mGame->Direct3DDeviceContext();
		d3dContext->RSSetState(pWireframeRasterizerState);

		if (pWireframeRasterizerState)
		{
			pWireframeRasterizerState->Release();
			pWireframeRasterizerState = nullptr;
		}

		InitializeIndexBuffer();
		InitializeGridTex();
	}

	void WaveSim::SetParameters(SimParams& params)
	{
		_parameters = params;
	}

	void WaveSim::Update(const Library::GameTime& gameTime)
	{
		_nodeArray.Update(gameTime);
		UpdateZValueTexture();
		//UpdateVertexBuffer();
	}

	void WaveSim::InitializeIndexBuffer()
	{
		//uint16_t rows = static_cast<uint16_t>(_nodeArray.GetRows()) - 1;
		uint16_t rows = static_cast<uint16_t>(_nodeArray.GetRows());
		//uint16_t columns = static_cast<uint16_t>(_nodeArray.GetColumns()) - 1;
		uint16_t columns = static_cast<uint16_t>(_nodeArray.GetColumns());

		indexCount = 3 * 2 * (rows - 1) * (columns - 1);
		std::vector<uint16_t> indices;
		indices.reserve(indexCount);

		for (uint16_t n = 0; n < (rows - 1); ++n)
		{
			for (uint16_t i = 0; i < (columns - 1); ++i)
			{
				//upper triangles
				uint16_t a1 = i + (n + 1) * columns;
				indices.push_back(a1);
				uint16_t a2 = i + (n * columns);
				indices.push_back(a2);
				uint16_t a3 = i + 1 + (n * columns);
				indices.push_back(a3);
				//lower triangles
				uint16_t a4 = i + (n + 1) * columns;
				indices.push_back(a4);
				uint16_t a5 = i + 1 + (n * columns);
				indices.push_back(a5);
				uint16_t a6 = i + 1 + (n + 1) * columns;
				indices.push_back(a6);
			}
		}

		uint16_t* indexVals = new uint16_t[indexCount];
		for (uint16_t i = 0; i < indexCount; ++i)
		{
			indexVals[i] = indices[i];
		}

		D3D11_BUFFER_DESC indexBufferDesc{ 0 };
		indexBufferDesc.ByteWidth = sizeof(uint16_t) * indexCount;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubResourceData{ 0 };
		indexSubResourceData.pSysMem = indexVals;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&indexBufferDesc, &indexSubResourceData, mIndexBuffer.put()), "ID3D11Device::CreateBuffer() failed.");
	}

	void WaveSim::UpdateZValueTexture()
	{
		/*XMFLOAT2* zVals = zValueData.get();
		size_t i = 0;
		auto& nodeArray = _nodeArray.GetArray();
		for (auto& node : nodeArray)
		{
			zVals[i].x = node._displacement;
			++i;
		}*/

		/*D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		ID3D11DeviceContext3* m_d3dContext = GetGame()->Direct3DDeviceContext();

		HRESULT hr = m_d3dContext->Map(texResource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (SUCCEEDED(hr))
		{
			memcpy(mappedResource.pData, zVals, sizeZArray);
			m_d3dContext->Unmap(texResource, 0);
		}*/
	}

	void WaveSim::InitializeGridTex()
	{
		size = sizeof(VertexXYIndex) * length;
		vertexData = make_unique<VertexXYIndex[]>(length);
		zValueData = make_unique<XMFLOAT2[]>(length);
		texResource = mDisplacementMap->GetTexResource();

		//XMFLOAT2* compShaderVertexCopy = new XMFLOAT2[length];

		VertexXYIndex* vertices = vertexData.get();
		XMFLOAT2* zVals = zValueData.get();
		size_t i = 0;
		auto& nodeArray = _nodeArray.GetArray();
		for (auto& node : nodeArray)
		{
			vertices[i] = VertexXYIndex{ XMFLOAT2{node._position.x,node._position.y}, i};
			zVals[i].x = node._displacement;
			/*compShaderVertexCopy[i].x = node._position.x;
			compShaderVertexCopy[i].y = node._position.y;*/
			++i;
		}

		D3D11_BUFFER_DESC vertexBufferDesc{ 0 };
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE; //MADE DYNAMIC **
		vertexBufferDesc.ByteWidth = size;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		//vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData{ 0 };
		vertexSubResourceData.pSysMem = vertices;

		ThrowIfFailed(direct3DDevice->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mVertexBuffer.put()), "ID3D11Device::CreateBuffer() failed");

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		//ID3D11DeviceContext3* m_d3dContext = GetGame()->Direct3DDeviceContext();

		/*HRESULT hr = m_d3dContext->Map(texResource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (SUCCEEDED(hr))
		{
			memcpy(mappedResource.pData, zVals, sizeZArray);
			m_d3dContext->Unmap(texResource, 0);
		}*/

		/*mCompShader->CreateVertexXYArray(compShaderVertexCopy, length);

		delete[] compShaderVertexCopy;*/
	}

	void WaveSim::Draw(const Library::GameTime&)
	{
		const XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		const XMMATRIX wvp = XMMatrixTranspose(worldMatrix * mCamera->ViewProjectionMatrix());
		mMaterial->UpdateTransforms(wvp);

		//mMaterial->Draw(not_null<ID3D11Buffer*>(mVertexBuffer.get()),length, 0);
		mMaterial->DrawIndexed(mVertexBuffer.get(), mIndexBuffer.get(), indexCount, DXGI_FORMAT_R16_UINT,0,0,0,0);
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
		mMaterial->SetSurfaceColor(color);
	}



	void WaveSim::UpdateVertexBuffer()
	{
		/*VertexPosition* vertices = vertexData.get();

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
		m_d3dContext->Unmap(mVertexBuffer.get(), 0);*/
	}

	void WaveSim::InitializeGrid()
	{
		//length = _nodeArray.GetNodeCount();

		//size = sizeof(VertexPosition) * length;
		//
		//vertexData = make_unique<VertexPosition[]>(length);

		//VertexPosition* vertices = vertexData.get();

		//int i = 0;
		//auto& nodearray = _nodeArray.GetArray();
		//for (auto& node : nodearray)
		//{
		//	vertices[i] = VertexPosition{ XMFLOAT4{node._position.x	,node._displacement ,node._position.y ,1} };
		//	++i;
		//}

		//D3D11_BUFFER_DESC vertexBufferDesc{ 0 };
		//vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC; //MADE DYNAMIC **
		//vertexBufferDesc.ByteWidth = size;
		//vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		//vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		//D3D11_SUBRESOURCE_DATA vertexSubResourceData{ 0 };
		//vertexSubResourceData.pSysMem = vertices;

		//ThrowIfFailed(direct3DDevice->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mVertexBuffer.put()), "ID3D11Device::CreateBuffer() failed");
	}
}



/*auto updateMaterialFunc = [this]() { mUpdateMaterial = true; };
mCamera->AddViewMatrixUpdatedCallback(updateMaterialFunc);
mCamera->AddProjectionMatrixUpdatedCallback(updateMaterialFunc);*/

//InitializeGrid();


//m_d3dContext->Map(mVertexBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
//memcpy(mappedResource.pData, zVals, sizeZArray);
//m_d3dContext->Unmap(mVertexBuffer.get(), 0);

//D3D11_MAPPED_SUBRESOURCE mappedResource;
//HRESULT hr = g_pImmediateContext->Map(pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
//if (SUCCEEDED(hr))
//{
//	// Copy the new data to the mapped resource
//	memcpy(mappedResource.pData, newData, dataSize);

//	// Unmap the resource to apply the changes
//	g_pImmediateContext->Unmap(pTexture, 0);
//}

/*float* initZ = new float[length] {0};

D3D11_SUBRESOURCE_DATA initData{};
ZeroMemory(&initData, sizeof(initData));
initData.pSysMem = initZ;
initData.SysMemPitch = sizeof(float);*/


		//int size = sizeof(VertexPosition) * length;
		//std::unique_ptr<VertexPosition> vertexData(new VertexPosition[length]);

//ID3D11Device* direct3DDevice = GetGame()->Direct3DDevice();
//int length = _nodeArray.GetNodeCount();

/*D3D11_BUFFER_DESC vertexBufferDesc{ 0 };
vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
vertexBufferDesc.ByteWidth = size;
vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

D3D11_SUBRESOURCE_DATA vertexSubResourceData{ 0 };
vertexSubResourceData.pSysMem = vertices;*/


/*D3D11_TEXTURE2D_DESC blah{};
		blah.Width = 16;
		blah.Height = 16;
		blah.MipLevels = 1;
		blah.ArraySize = 1;
		blah.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		blah.SampleDesc.Quality = 0;
		blah.SampleDesc.Count = 1;
		blah.Usage = D3D11_USAGE_DEFAULT;
		blah.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		blah.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		blah.MiscFlags = 0;

		winrt::com_ptr<ID3D11Texture2D> ptr;
		direct3DDevice->CreateTexture2D(&blah, nullptr, ptr.put());*/
		/*winrt::com_ptr<ID3D11Texture1D> texture;
		HRESULT hr;
		if (FAILED(hr = direct3DDevice->CreateTexture1D(&texDesc, &initData, texture.put())))
		{
			throw GameException("IDXGIDevice::CreateTexture2D() failed.", hr);
		}*/


//ThrowIfFailed(direct3DDevice->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mVertexBuffer.put()), "ID3D11Device::CreateBuffer() failed");