#include "pch.h"
#include "HullShaderReader.h"
#include "Game.h"
#include "GameException.h"
#include "Utility.h"

using namespace std;
using namespace DirectX;
using namespace winrt;

namespace Library
{
	RTTI_DEFINITIONS(HullShaderReader)

	HullShaderReader::HullShaderReader(Game& game) :
		ContentTypeReader(game, HullShader::TypeIdClass())
	{
	}

	shared_ptr<HullShader> HullShaderReader::_Read(const wstring& assetName)
	{
		com_ptr<ID3D11HullShader> hullShader;
		vector<char> compiledHullShader;
		Utility::LoadBinaryFile(assetName, compiledHullShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateHullShader(&compiledHullShader[0], compiledHullShader.size(), nullptr, hullShader.put()), "ID3D11Device::CreatedHullShader() failed.");
		
		return shared_ptr<HullShader>(new HullShader(move(hullShader)));
	}
}