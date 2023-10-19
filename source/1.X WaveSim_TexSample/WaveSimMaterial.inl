//#pragma once
//#include "WaveSimMaterial.h"
//#include "VertexDeclarations.h"
//#include "Game.h"
//#include "Texture1D.h"
//
//namespace Rendering
//{
//	WaveSimMaterial::WaveSimMaterial(Library::Game& game) :
//		Material(game)
//	{
//
//	}
//
//	void WaveSimMaterial::UpdateTransform(DirectX::CXMMATRIX worldViewProjectionMatrix)
//	{
//		mGame->Direct3DDeviceContext()->UpdateSubresource(mWVPBuffer.get(), 0, nullptr, worldViewProjectionMatrix.r, 0, 0);
//	}
//
//	std::uint32_t WaveSimMaterial::VertexSize() const
//	{
//		return sizeof(Library::VertexXYIndex);
//	}
//
//	void WaveSimMaterial::SetSurfaceColor(const DirectX::XMFLOAT4& color)
//	{
//		SetSurfaceColor(reinterpret_cast<const float*>(&color));
//	}
//
//	void WaveSimMaterial::SetSurfaceColor(const float* color)
//	{
//		mGame->Direct3DDeviceContext()->UpdateSubresource(mPSConstantBuffer.get(), 0, nullptr, color, 0, 0);
//	}
//
//	/*template<size_t nodeCount>
//	WaveSimMaterial<nodeCount>::WaveSimMaterial(Library::Game& game) :
//		Material(game)
//	{
//
//	}
//
//	template<size_t nodeCount>
//	inline void WaveSimMaterial<nodeCount>::UpdateTransform(DirectX::CXMMATRIX worldViewProjectionMatrix)
//	{
//		mGame->Direct3DDeviceContext()->UpdateSubresource(mWVPBuffer.get(), 0, nullptr, worldViewProjectionMatrix.r, 0, 0);
//	}
//
//	template<size_t nodeCount>
//	inline std::uint32_t WaveSimMaterial<nodeCount>::VertexSize() const
//	{
//		return sizeof(Library::VertexXYIndex);
//	}
//
//	template<size_t nodeCount>
//	inline void WaveSimMaterial<nodeCount>::SetSurfaceColor(const DirectX::XMFLOAT4& color)
//	{
//		SetSurfaceColor(reinterpret_cast<const float*>(&color));
//	}
//
//	template<size_t nodeCount>
//	inline void WaveSimMaterial<nodeCount>::SetSurfaceColor(const float* color)
//	{
//		mGame->Direct3DDeviceContext()->UpdateSubresource(mPSConstantBuffer.get(), 0, nullptr, color, 0, 0);
//	}*/
//
//}