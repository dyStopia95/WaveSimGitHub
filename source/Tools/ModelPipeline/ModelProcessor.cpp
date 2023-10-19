#include "pch.h"
#include "ModelProcessor.h"
#include "ModelMaterialProcessor.h"
#include "MeshProcessor.h"
#include "AnimationClipProcessor.h"
#include "Bone.h"
#include "AnimationClip.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;
using namespace Library;
using namespace DirectX;

namespace ModelPipeline
{
	Library::Model ModelProcessor::LoadModel(const std::string& filename, bool flipUVs)
	{
		Library::Model model;
		ModelData& modelData = model.Data();
		Assimp::Importer importer;

		uint32_t flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipWindingOrder;
		if (flipUVs)
		{
			flags |= aiProcess_FlipUVs;
		}

		const aiScene* scene = importer.ReadFile(filename, flags);
		if (scene == nullptr)
		{
			throw exception(importer.GetErrorString());
		}

		if (scene->HasMaterials())
		{
			for (unsigned int i = 0; i < scene->mNumMaterials; i++)
			{
				shared_ptr<ModelMaterial> modelMaterial = ModelMaterialProcessor::LoadModelMaterial(model, *(scene->mMaterials[i]));
				modelData.Materials.push_back(modelMaterial);
			}
		}

		if (scene->HasMeshes())
		{
			for (unsigned int i = 0; i < scene->mNumMeshes; i++)
			{
				shared_ptr<Mesh> mesh = MeshProcessor::LoadMesh(model, *(scene->mMeshes[i]));
				modelData.Meshes.push_back(mesh);
			}
		}

		if (scene->HasAnimations())
		{
			assert(scene->mRootNode != nullptr);
			modelData.RootNode = BuildSkeleton(modelData, *scene->mRootNode, nullptr);

			modelData.Animations.reserve(scene->mNumAnimations);
			for (unsigned int i = 0; i < scene->mNumAnimations; i++)
			{
				shared_ptr<AnimationClip> animation = AnimationClipProcessor::LoadAnimationClip(model, *(scene->mAnimations[i]));
				modelData.Animations.push_back(animation);
				modelData.AnimationsByName.insert(std::pair<std::string, shared_ptr<AnimationClip>>(animation->Name(), animation));
			}
		}

		return model;
	}

	shared_ptr<SceneNode> ModelProcessor::BuildSkeleton(ModelData& modelData, aiNode& node, const shared_ptr<SceneNode>& parentSceneNode)
	{
		shared_ptr<SceneNode> sceneNode;

		auto boneMapping = modelData.BoneIndexMapping.find(node.mName.C_Str());
		if (boneMapping == modelData.BoneIndexMapping.end())
		{
			sceneNode = make_shared<SceneNode>(node.mName.C_Str());
		}
		else
		{
			sceneNode = modelData.Bones[boneMapping->second];
		}

		XMFLOAT4X4 transform(reinterpret_cast<const float*>(node.mTransformation[0]));
		sceneNode->SetTransform(XMMatrixTranspose(XMLoadFloat4x4(&transform)));
		sceneNode->SetParent(parentSceneNode);

		auto& children = sceneNode->Children();
		children.reserve(node.mNumChildren);
		for (unsigned int i = 0; i < node.mNumChildren; i++)
		{
			shared_ptr<SceneNode> childSceneNode = BuildSkeleton(modelData, *(node.mChildren[i]), sceneNode);
			children.push_back(childSceneNode);
		}

		return sceneNode;
	}
}