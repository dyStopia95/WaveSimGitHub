#include "pch.h"
#include "SceneNode.h"
#include "MatrixHelper.h"
#include "StreamHelper.h"

using namespace std;
using namespace DirectX;

namespace Library
{
	RTTI_DEFINITIONS(SceneNode)

	SceneNode::SceneNode(const string& name) :
		SceneNode(name, MatrixHelper::Identity)
	{
	}

	SceneNode::SceneNode(const string& name, const XMFLOAT4X4& transform) :
		mName(name), mTransform(transform)
    {
    }

	const string& SceneNode::Name() const
	{
		return mName;
	}

	weak_ptr<SceneNode> SceneNode::GetParent() const
	{
		return mParent;
	}

	vector<shared_ptr<SceneNode>>& SceneNode::Children()
	{
		return mChildren;
	}

	const XMFLOAT4X4& SceneNode::Transform() const
	{
		return mTransform;
	}

	XMMATRIX SceneNode::TransformMatrix() const
	{
		return XMLoadFloat4x4(&mTransform);
	}

	void SceneNode::SetParent(shared_ptr<SceneNode> parent)
	{
		mParent = move(parent);
	}

	void SceneNode::SetTransform(const XMFLOAT4X4& transform)
	{
		mTransform = transform;
	}

	void SceneNode::SetTransform(CXMMATRIX transform)
	{
		XMFLOAT4X4 t;
		XMStoreFloat4x4(&t, transform);

		SetTransform(t);
	}

	void SceneNode::Save(OutputStreamHelper& streamHelper)
	{
		streamHelper << mName << mTransform;
	}

	void SceneNode::Load(InputStreamHelper& streamHelper)
	{
		streamHelper >> mName >> mTransform;
	}
}
