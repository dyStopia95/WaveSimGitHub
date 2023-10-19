#include "pch.h"
#include "DomainShader.h"

using namespace std;
using namespace DirectX;
using namespace winrt;

namespace Library
{
	RTTI_DEFINITIONS(DomainShader)

	DomainShader::DomainShader(const com_ptr<ID3D11DomainShader>& hullShader) :
		mShader(hullShader)
	{
	}

	com_ptr<ID3D11DomainShader> DomainShader::Shader() const
	{
		return mShader;
	}
}