#include "pch.h"
#include "Model.h"
#include "Mesh.h"
#include "StreamHelper.h"
#include "GameException.h"
#include "ModelMaterial.h"
#include "AnimationClip.h"

using namespace std;
using namespace gsl;
using namespace DirectX;

namespace Library
{
	RTTI_DEFINITIONS(Model)

	Model::Model(const string& filename)
	{
		Load(filename);
	}

	Model::Model(ifstream& file)
	{
		Load(file);
	}

	Model::Model(ModelData&& modelData) :
		mData(move(modelData))
	{
	}

	bool Model::HasMeshes() const
	{
		return (mData.Meshes.size() > 0);
	}

	bool Model::HasMaterials() const
	{
		return (mData.Materials.size() > 0);
	}

	bool Model::HasAnimations() const
	{
		return (mData.Animations.size() > 0);
	}

	const vector<shared_ptr<Mesh>>& Model::Meshes() const
	{
		return mData.Meshes;
	}

	const vector<shared_ptr<ModelMaterial>>& Model::Materials() const
	{
		return mData.Materials;
	}

	const vector<shared_ptr<AnimationClip>>& Model::Animations() const
	{
		return mData.Animations;
	}

	const map<string, shared_ptr<AnimationClip>>& Model::AnimationsByName() const
	{
		return mData.AnimationsByName;
	}

	const vector<shared_ptr<Bone>> Model::Bones() const
	{
		return mData.Bones;
	}

	const map<string, uint32_t> Model::BoneIndexMapping() const
	{
		return mData.BoneIndexMapping;
	}

	shared_ptr<SceneNode> Model::RootNode() const
	{
		return mData.RootNode;
	}

	ModelData& Model::Data()
	{
		return mData;
	}

	void Model::Save(const string& filename) const
	{
		ofstream file(filename.c_str(), ios::binary);
		if (!file.good())
		{
			throw exception("Could not open file.");
		}

		Save(file);
	}

	void Model::Save(ofstream& file) const
	{
		OutputStreamHelper streamHelper(file);

		// Serialize materials
		streamHelper << narrow_cast<uint32_t>(mData.Materials.size());
		for (const auto& material : mData.Materials)
		{
			material->Save(streamHelper);
		}

		// Serialize meshes
		streamHelper << narrow_cast<uint32_t>(mData.Meshes.size());
		for (auto& mesh : mData.Meshes)
		{
			mesh->Save(streamHelper);
		}

		// Serialize bones
		streamHelper << narrow_cast<uint32_t>(mData.Bones.size());
		for (auto& bone : mData.Bones)
		{
			bone->Save(streamHelper);
		}

		// Serialize skeleton hierachy
		bool hasSkeleton = mData.RootNode != nullptr;
		streamHelper << hasSkeleton;
		if (hasSkeleton)
		{
			SaveSkeleton(streamHelper, mData.RootNode);
		}

		// Serialize animations
		streamHelper << narrow_cast<uint32_t>(mData.Animations.size());
		for (auto& animation : mData.Animations)
		{
			animation->Save(streamHelper);
		}
	}

	void Model::Load(const string& filename)
	{
		ifstream file(filename.c_str(), ios::binary);
		if (!file.good())
		{
			throw GameException("Could not open file.");
		}

		Load(file);
	}

	void Model::Load(ifstream& file)
	{
		InputStreamHelper streamHelper(file);

		// Desrialize materials
		uint32_t materialCount;
		streamHelper >> materialCount;
		mData.Materials.reserve(materialCount);
		for (uint32_t i = 0; i < materialCount; i++)
		{
			mData.Materials.emplace_back(make_shared<ModelMaterial>(*this, streamHelper));
		}

		// Desrialize meshes
		uint32_t meshCount;
		streamHelper >> meshCount;
		mData.Meshes.reserve(meshCount);
		for (uint32_t i = 0; i < meshCount; i++)
		{
			mData.Meshes.emplace_back(make_shared<Mesh>(*this, streamHelper));
		}

		// Deserialize bones
		uint32_t boneCount;
		streamHelper >> boneCount;
		mData.Bones.reserve(boneCount);
		for (uint32_t index = 0; index < boneCount; index++)
		{
			auto bone = mData.Bones.emplace_back(make_shared<Bone>(streamHelper));
			mData.BoneIndexMapping[bone->Name()] = index;
		}

		// Deserialize skeleton hierachy
		bool hasSkeleton;
		streamHelper >> hasSkeleton;
		if (hasSkeleton)
		{
			mData.RootNode = LoadSkeleton(streamHelper, nullptr);
		}

		// Deserialize animations
		uint32_t animationCount;
		streamHelper >> animationCount;
		mData.Animations.reserve(animationCount);
		for (uint32_t i = 0; i < animationCount; i++)
		{
			auto animation = mData.Animations.emplace_back(make_shared<AnimationClip>(*this, streamHelper));
			mData.AnimationsByName[animation->Name()] = animation;
		}
	}

	void Model::SaveSkeleton(OutputStreamHelper& streamHelper, const shared_ptr<SceneNode>& sceneNode) const
	{
		streamHelper << sceneNode->Name();

		bool isBone = sceneNode->Is(Bone::TypeIdClass());
		streamHelper << isBone;
		if (isBone == false)
		{
			streamHelper << sceneNode->Transform();
		}

		streamHelper << narrow_cast<uint32_t>(sceneNode->Children().size());
		for (auto& childNode : sceneNode->Children())
		{
			SaveSkeleton(streamHelper, childNode);
		}
	}

	shared_ptr<SceneNode> Model::LoadSkeleton(InputStreamHelper& streamHelper, shared_ptr<SceneNode> parentSceneNode)
	{
		shared_ptr<SceneNode> sceneNode;

		// Deserialize bone's name for lookup
		string name;
		streamHelper >> name;

		bool isBone;
		streamHelper >> isBone;

		if (isBone)
		{
			uint32_t boneIndex = mData.BoneIndexMapping.at(name);
			sceneNode = mData.Bones[boneIndex];
		}
		else
		{
			XMFLOAT4X4 transform;
			streamHelper >> transform;
			sceneNode = make_shared<SceneNode>(name, transform);
		}

		sceneNode->SetParent(move(parentSceneNode));

		// Deserialize children
		uint32_t childCount;
		streamHelper >> childCount;
		sceneNode->Children().reserve(childCount);
		for (uint32_t i = 0; i < childCount; i++)
		{
			auto childSceneNode = LoadSkeleton(streamHelper, sceneNode);
			sceneNode->Children().push_back(childSceneNode);
		}

		return sceneNode;
	}
}