#include "pch.h"
#include "GeometryShader.h"

using namespace std;
using namespace DirectX;
using namespace winrt;

namespace Library
{
	RTTI_DEFINITIONS(GeometryShader)

	GeometryShader::GeometryShader(const com_ptr<ID3D11GeometryShader>& vertexShader) :
		mShader(vertexShader)
	{
	}

	com_ptr<ID3D11GeometryShader> GeometryShader::Shader() const
	{
		return mShader;
	}
}