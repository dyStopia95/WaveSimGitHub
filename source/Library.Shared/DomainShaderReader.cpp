#include "pch.h"
#include "DomainShaderReader.h"
#include "Game.h"
#include "GameException.h"
#include "Utility.h"

using namespace std;
using namespace DirectX;
using namespace winrt;

namespace Library
{
	RTTI_DEFINITIONS(DomainShaderReader)

	DomainShaderReader::DomainShaderReader(Game& game) :
		ContentTypeReader(game, DomainShader::TypeIdClass())
	{
	}

	shared_ptr<DomainShader> DomainShaderReader::_Read(const wstring& assetName)
	{
		com_ptr<ID3D11DomainShader> hullShader;
		vector<char> compiledDomainShader;
		Utility::LoadBinaryFile(assetName, compiledDomainShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateDomainShader(&compiledDomainShader[0], compiledDomainShader.size(), nullptr, hullShader.put()), "ID3D11Device::CreatedDomainShader() failed.");
		
		return shared_ptr<DomainShader>(new DomainShader(move(hullShader)));
	}
}