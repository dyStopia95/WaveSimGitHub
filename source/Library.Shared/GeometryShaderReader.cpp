#include "pch.h"
#include "GeometryShaderReader.h"
#include "Game.h"
#include "GameException.h"
#include "Utility.h"

using namespace std;
using namespace DirectX;
using namespace winrt;

namespace Library
{
	RTTI_DEFINITIONS(GeometryShaderReader)

	GeometryShaderReader::GeometryShaderReader(Game& game) :
		ContentTypeReader(game, GeometryShader::TypeIdClass())
	{
	}

	shared_ptr<GeometryShader> GeometryShaderReader::_Read(const wstring& assetName)
	{
		com_ptr<ID3D11GeometryShader> hullShader;
		vector<char> compiledGeometryShader;
		Utility::LoadBinaryFile(assetName, compiledGeometryShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateGeometryShader(&compiledGeometryShader[0], compiledGeometryShader.size(), nullptr, hullShader.put()), "ID3D11Device::CreatedGeometryShader() failed.");
		
		return shared_ptr<GeometryShader>(new GeometryShader(move(hullShader)));
	}
}