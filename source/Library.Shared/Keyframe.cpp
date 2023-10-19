#include "pch.h"
#include "Keyframe.h"
#include "VectorHelper.h"
#include "StreamHelper.h"

using namespace DirectX;

namespace Library
{
	Keyframe::Keyframe(InputStreamHelper& streamHelper)
	{
		Load(streamHelper);
	}

	Keyframe::Keyframe(float time, const XMFLOAT3& translation, const XMFLOAT4& rotationQuaternion, const XMFLOAT3& scale) :
		mTime(time), mTranslation(translation), mRotationQuaternion(rotationQuaternion), mScale(scale)
    {
    }

	float Keyframe::Time() const
	{
		return mTime;
	}

	const XMFLOAT3& Keyframe::Translation() const
	{	
		return mTranslation;
	}

	const XMFLOAT4& Keyframe::RotationQuaternion() const
	{
		return mRotationQuaternion;
	}

	const XMFLOAT3& Keyframe::Scale() const
	{
		return mScale;
	}

	XMVECTOR Keyframe::TranslationVector() const
	{
		return XMLoadFloat3(&mTranslation);
	}

	XMVECTOR Keyframe::RotationQuaternionVector() const
	{
		return XMLoadFloat4(&mRotationQuaternion);
	}

	XMVECTOR Keyframe::ScaleVector() const
	{
		return XMLoadFloat3(&mScale);
	}

	XMMATRIX Keyframe::Transform() const
	{
		static XMVECTOR rotationOrigin = XMLoadFloat4(&Vector4Helper::Zero);

		return XMMatrixAffineTransformation(ScaleVector(), rotationOrigin, RotationQuaternionVector(), TranslationVector());
	}

	void Keyframe::Save(OutputStreamHelper& streamHelper)
	{
		streamHelper << mTime;
		streamHelper << mTranslation.x << mTranslation.y << mTranslation.z;
		streamHelper << mRotationQuaternion.x << mRotationQuaternion.y << mRotationQuaternion.z << mRotationQuaternion.w;
		streamHelper << mScale.x << mScale.y << mScale.z;
	}

	void Keyframe::Load(InputStreamHelper& streamHelper)
	{
		streamHelper >> mTime;
		streamHelper >> mTranslation.x >> mTranslation.y >> mTranslation.z;
		streamHelper >> mRotationQuaternion.x >> mRotationQuaternion.y >> mRotationQuaternion.z >> mRotationQuaternion.w;
		streamHelper >> mScale.x >> mScale.y >> mScale.z;
	}
}
