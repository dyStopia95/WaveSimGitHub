#include "pch.h"
#include "VertexDeclarations.h"
#include "GameException.h"
#include "Mesh.h"

using namespace std;
using namespace gsl;
using namespace DirectX;

namespace Library
{
	void VertexPosition::CreateVertexBuffer(not_null<ID3D11Device*> device, const Mesh& mesh, not_null<ID3D11Buffer**> vertexBuffer)
	{
		const vector<XMFLOAT3>& sourceVertices = mesh.Vertices();

		vector<VertexPosition> vertices;
		vertices.reserve(sourceVertices.size());

		for (size_t i = 0; i < sourceVertices.size(); i++)
		{
			const XMFLOAT3& position = sourceVertices.at(i);
			vertices.emplace_back(XMFLOAT4(position.x, position.y, position.z, 1.0f));
		}

		VertexDeclaration::CreateVertexBuffer(device, vertices, vertexBuffer);
	}

	void VertexPositionColor::CreateVertexBuffer(not_null<ID3D11Device*> device, const Mesh& mesh, not_null<ID3D11Buffer**> vertexBuffer)
	{
		const vector<XMFLOAT3>& sourceVertices = mesh.Vertices();

		vector<VertexPositionColor> vertices;
		vertices.reserve(sourceVertices.size());

		assert(mesh.VertexColors().size() > 0);
		const vector<XMFLOAT4>& vertexColors = mesh.VertexColors().at(0);
		assert(vertexColors.size() == sourceVertices.size());

		for (size_t i = 0; i < sourceVertices.size(); i++)
		{
			const XMFLOAT3& position = sourceVertices.at(i);
			const XMFLOAT4& color = vertexColors.at(i);
			vertices.emplace_back(XMFLOAT4(position.x, position.y, position.z, 1.0f), color);
		}

		VertexDeclaration::CreateVertexBuffer(device, vertices, vertexBuffer);
	}

	void VertexPositionTexture::CreateVertexBuffer(not_null<ID3D11Device*> device, const Mesh& mesh, not_null<ID3D11Buffer**> vertexBuffer)
	{
		const vector<XMFLOAT3>& sourceVertices = mesh.Vertices();
		const vector<XMFLOAT3>& textureCoordinates = mesh.TextureCoordinates().at(0);
		assert(textureCoordinates.size() == sourceVertices.size());

		vector<VertexPositionTexture> vertices;
		vertices.reserve(sourceVertices.size());
		for (size_t i = 0; i < sourceVertices.size(); i++)
		{
			const XMFLOAT3& position = sourceVertices.at(i);
			const XMFLOAT3& uv = textureCoordinates.at(i);
			vertices.emplace_back(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y));
		}

		VertexDeclaration::CreateVertexBuffer(device, vertices, vertexBuffer);
	}

	void VertexPositionNormal::CreateVertexBuffer(not_null<ID3D11Device*> device, const Mesh& mesh, not_null<ID3D11Buffer**> vertexBuffer)
	{
		const vector<XMFLOAT3>& sourceVertices = mesh.Vertices();
		const vector<XMFLOAT3>& sourceNormals = mesh.Normals();
		assert(sourceNormals.size() == sourceVertices.size());

		vector<VertexPositionNormal> vertices;
		vertices.reserve(sourceVertices.size());
		for (size_t i = 0; i < sourceVertices.size(); i++)
		{
			const XMFLOAT3& position = sourceVertices.at(i);
			const XMFLOAT3& normal = sourceNormals.at(i);
			vertices.emplace_back(XMFLOAT4(position.x, position.y, position.z, 1.0f), normal);
		}

		VertexDeclaration::CreateVertexBuffer(device, vertices, vertexBuffer);
	}

	void VertexPositionTextureNormal::CreateVertexBuffer(not_null<ID3D11Device*> device, const Mesh& mesh, not_null<ID3D11Buffer**> vertexBuffer)
	{
		const vector<XMFLOAT3>& sourceVertices = mesh.Vertices();
		const auto& sourceUVs = mesh.TextureCoordinates().at(0);
		assert(sourceUVs.size() == sourceVertices.size());
		const auto& sourceNormals = mesh.Normals();
		assert(sourceNormals.size() == sourceVertices.size());

		vector<VertexPositionTextureNormal> vertices;
		vertices.reserve(sourceVertices.size());
		for (size_t i = 0; i < sourceVertices.size(); i++)
		{
			const XMFLOAT3& position = sourceVertices.at(i);
			const XMFLOAT3& uv = sourceUVs.at(i);
			const XMFLOAT3& normal = sourceNormals.at(i);
			vertices.emplace_back(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y), normal);
		}

		VertexDeclaration::CreateVertexBuffer(device, vertices, vertexBuffer);
	}

	void VertexPositionTextureNormalTangent::CreateVertexBuffer(not_null<ID3D11Device*> device, const Mesh& mesh, not_null<ID3D11Buffer**> vertexBuffer)
	{
		const vector<XMFLOAT3>& sourceVertices = mesh.Vertices();
		const auto& sourceUVs = mesh.TextureCoordinates().at(0);
		assert(sourceUVs.size() == sourceVertices.size());
		const auto& sourceNormals = mesh.Normals();
		assert(sourceNormals.size() == sourceVertices.size());
		const auto& sourceTangents = mesh.Tangents();
		assert(sourceTangents.size() == sourceVertices.size());

		vector<VertexPositionTextureNormalTangent> vertices;
		vertices.reserve(sourceVertices.size());
		for (size_t i = 0; i < sourceVertices.size(); i++)
		{
			const XMFLOAT3& position = sourceVertices.at(i);
			const XMFLOAT3& uv = sourceUVs.at(i);
			const XMFLOAT3& normal = sourceNormals.at(i);
			const XMFLOAT3& tangent = sourceTangents.at(i);
			vertices.emplace_back(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y), normal, tangent);
		}

		VertexDeclaration::CreateVertexBuffer(device, vertices, vertexBuffer);
	}

	void VertexSkinnedPositionTextureNormal::CreateVertexBuffer(not_null<ID3D11Device*> device, const Mesh& mesh, not_null<ID3D11Buffer**> vertexBuffer)
	{
		const vector<XMFLOAT3>& sourceVertices = mesh.Vertices();
		const auto& sourceUVs = mesh.TextureCoordinates().at(0);
		assert(sourceUVs.size() == sourceVertices.size());
		const auto& sourceNormals = mesh.Normals();
		assert(sourceNormals.size() == sourceVertices.size());
		const auto& boneWeights = mesh.BoneWeights();
		assert(boneWeights.size() == sourceVertices.size());

		vector<VertexSkinnedPositionTextureNormal> vertices;
		vertices.reserve(sourceVertices.size());
		for (size_t i = 0; i < sourceVertices.size(); i++)
		{
			const XMFLOAT3& position = sourceVertices.at(i);
			const XMFLOAT3& uv = sourceUVs.at(i);
			const XMFLOAT3& normal = sourceNormals.at(i);
			const BoneVertexWeights& vertexWeights = boneWeights.at(i);

			float weights[BoneVertexWeights::MaxBoneWeightsPerVertex];
			uint32_t indices[BoneVertexWeights::MaxBoneWeightsPerVertex];
			ZeroMemory(weights, sizeof(float) * size(weights));
			ZeroMemory(indices, sizeof(uint32_t) * size(indices));
			for (size_t j = 0; j < vertexWeights.Weights().size(); j++)
			{
				const BoneVertexWeights::VertexWeight& vertexWeight = vertexWeights.Weights().at(j);
				weights[j] = vertexWeight.Weight;
				indices[j] = vertexWeight.BoneIndex;
			}

			vertices.emplace_back(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y), normal, XMUINT4(indices), XMFLOAT4(weights));
		}

		VertexDeclaration::CreateVertexBuffer(device, vertices, vertexBuffer);
	}

	/*void VertexXYIndex::CreateVertexBuffer(gsl::not_null<ID3D11Device*> device, const Library::Mesh& mesh, gsl::not_null<ID3D11Buffer**> vertexBuffer)
	{
		const vector<XMFLOAT3>& sourceVertices = mesh.Vertices();

		vector<VertexXYIndex> vertices;
		vertices.reserve(sourceVertices.size());

		for (size_t i = 0; i < sourceVertices.size(); i++)
		{
			const XMFLOAT3& position = sourceVertices.at(i);
			vertices.emplace_back(XMFLOAT4(position.x, position.y, position.z, 1.0f));
		}

		VertexDeclaration::CreateVertexBuffer(device, vertices, vertexBuffer);
	}*/
}