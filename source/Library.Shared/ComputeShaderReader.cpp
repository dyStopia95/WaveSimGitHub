#include "pch.h"
#include "ComputeShaderReader.h"
#include "Game.h"
#include "GameException.h"
#include "Utility.h"

using namespace std;
using namespace DirectX;
using namespace winrt;

namespace Library
{
	RTTI_DEFINITIONS(ComputeShaderReader)

	ComputeShaderReader::ComputeShaderReader(Game& game) :
		ContentTypeReader(game, ComputeShader::TypeIdClass())
	{
	}

	shared_ptr<ComputeShader> ComputeShaderReader::_Read(const wstring& assetName)
	{
		com_ptr<ID3D11ComputeShader> hullShader;
		vector<char> compiledComputeShader;
		Utility::LoadBinaryFile(assetName, compiledComputeShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateComputeShader(&compiledComputeShader[0], compiledComputeShader.size(), nullptr, hullShader.put()), "ID3D11Device::CreatedComputeShader() failed.");
		
		return shared_ptr<ComputeShader>(new ComputeShader(move(hullShader)));
	}
}