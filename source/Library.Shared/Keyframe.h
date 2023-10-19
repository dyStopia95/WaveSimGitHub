#pragma once

#include <DirectXMath.h>

namespace Library
{
	class OutputStreamHelper;
	class InputStreamHelper;

    class Keyframe final
    {
    public:
		Keyframe(InputStreamHelper& streamHelper);
		Keyframe(float time, const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT4& rotationQuaternion, const DirectX::XMFLOAT3& scale);
		Keyframe(const Keyframe&) = default;
		Keyframe(Keyframe&&) = default;
		Keyframe& operator=(const Keyframe&) = default;
		Keyframe& operator=(Keyframe&&) = default;
		~Keyframe() = default;

		float Time() const;
		const DirectX::XMFLOAT3& Translation() const;
		const DirectX::XMFLOAT4& RotationQuaternion() const;
		const DirectX::XMFLOAT3& Scale() const;

		DirectX::XMVECTOR TranslationVector() const;
		DirectX::XMVECTOR RotationQuaternionVector() const;
		DirectX::XMVECTOR ScaleVector() const;
		DirectX::XMMATRIX Transform() const;

		void Save(OutputStreamHelper& streamHelper);

    private:
		void Load(InputStreamHelper& streamHelper);

		float mTime{ 0.0f };
		DirectX::XMFLOAT3 mTranslation;
		DirectX::XMFLOAT4 mRotationQuaternion;
		DirectX::XMFLOAT3 mScale;
    };
}