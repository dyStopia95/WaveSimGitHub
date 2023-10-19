#include "pch.h"
#include "ComputeShader.h"

using namespace std;
using namespace DirectX;
using namespace winrt;

namespace Library
{
	RTTI_DEFINITIONS(ComputeShader)

	ComputeShader::ComputeShader(const com_ptr<ID3D11ComputeShader>& hullShader) :
		mShader(hullShader)
	{
	}

	com_ptr<ID3D11ComputeShader> ComputeShader::Shader() const
	{
		return mShader;
	}
}