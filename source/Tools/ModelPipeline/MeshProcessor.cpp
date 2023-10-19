#include "pch.h"
#include "MeshProcessor.h"
#include "Model.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

using namespace std;
using namespace gsl;
using namespace DirectX;
using namespace Library;

namespace ModelPipeline
{
	shared_ptr<Library::Mesh> MeshProcessor::LoadMesh(Library::Model& model, aiMesh& mesh)
	{
		MeshData meshData;

		meshData.Material = model.Materials().at(mesh.mMaterialIndex);

		// Vertices
		{
			meshData.Vertices.reserve(mesh.mNumVertices);
			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				meshData.Vertices.emplace_back(reinterpret_cast<const float*>(&mesh.mVertices[i]));
			}
		}

		// Normals
		if (mesh.HasNormals())
		{
			meshData.Normals.reserve(mesh.mNumVertices);
			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				meshData.Normals.emplace_back(reinterpret_cast<const float*>(&mesh.mNormals[i]));
			}
		}

		// Tangents and Binormals
		if (mesh.HasTangentsAndBitangents())
		{
			meshData.Tangents.reserve(mesh.mNumVertices);
			meshData.BiNormals.reserve(mesh.mNumVertices);
			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				meshData.Tangents.emplace_back(reinterpret_cast<const float*>(&mesh.mTangents[i]));
				meshData.BiNormals.emplace_back(reinterpret_cast<const float*>(&mesh.mBitangents[i]));
			}
		}

		// Texture Coordinates
		{
			unsigned int uvChannelCount = mesh.GetNumUVChannels();
			meshData.TextureCoordinates.reserve(uvChannelCount);
			for (unsigned int i = 0; i < uvChannelCount; i++)
			{
				vector<XMFLOAT3> textureCoordinates;
				textureCoordinates.reserve(mesh.mNumVertices);
				aiVector3D* aiTextureCoordinates = mesh.mTextureCoords[i];
				for (unsigned int j = 0; j < mesh.mNumVertices; j++)
				{
					textureCoordinates.emplace_back(reinterpret_cast<const float*>(&aiTextureCoordinates[j]));
				}

				meshData.TextureCoordinates.push_back(move(textureCoordinates));
			}
		}

		// Vertex Colors
		{
			unsigned int colorChannelCount = mesh.GetNumColorChannels();
			meshData.VertexColors.reserve(colorChannelCount);
			for (unsigned int i = 0; i < colorChannelCount; i++)
			{
				vector<XMFLOAT4> vertexColors;
				vertexColors.reserve(mesh.mNumVertices);
				aiColor4D* aiVertexColors = mesh.mColors[i];
				for (unsigned int j = 0; j < mesh.mNumVertices; j++)
				{
					vertexColors.emplace_back(reinterpret_cast<const float*>(&aiVertexColors[j]));
				}
				meshData.VertexColors.push_back(move(vertexColors));
			}
		}

		// Faces (note: could pre-reserve if we limit primitive types)
		if (mesh.HasFaces())
		{
			meshData.FaceCount = mesh.mNumFaces;
			for (uint32_t i = 0; i < meshData.FaceCount; i++)
			{
				aiFace* face = &mesh.mFaces[i];

				for (unsigned int j = 0; j < face->mNumIndices; j++)
				{
					meshData.Indices.push_back(face->mIndices[j]);
				}
			}
		}

		// Bones
		if (mesh.HasBones())
		{
			meshData.BoneWeights.resize(mesh.mNumVertices);

			for (unsigned int i = 0; i < mesh.mNumBones; i++)
			{
				aiBone* meshBone = mesh.mBones[i];

				// Look up the bone in the model's hierarchy, or add it if not found.
				uint32_t boneIndex = 0U;
				std::string boneName = meshBone->mName.C_Str();
				auto boneMappingIterator = model.Data().BoneIndexMapping.find(boneName);
				if (boneMappingIterator != model.Data().BoneIndexMapping.end())
				{
					boneIndex = boneMappingIterator->second;
				}
				else
				{
					boneIndex = narrow_cast<uint32_t>(model.Bones().size());
					XMFLOAT4X4 offsetMatrix = XMFLOAT4X4(reinterpret_cast<const float*>(meshBone->mOffsetMatrix[0]));
					XMFLOAT4X4 offset;
					XMStoreFloat4x4(&offset, XMMatrixTranspose(XMLoadFloat4x4(&offsetMatrix)));

					model.Data().Bones.push_back(make_shared<Bone>(boneName, boneIndex, offset));
					model.Data().BoneIndexMapping[boneName] = boneIndex;
				}

				for (unsigned int j = 0; j < meshBone->mNumWeights; j++)
				{
					aiVertexWeight vertexWeight = meshBone->mWeights[j];
					meshData.BoneWeights[vertexWeight.mVertexId].AddWeight(vertexWeight.mWeight, boneIndex);
				}
			}
		}

		return make_shared<Library::Mesh>(model, move(meshData));
	}
}