#include "pch.h"
#include "ShadowMappingMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VertexDeclarations.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "PixelShaderReader.h"
#include "Texture2D.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace winrt;
using namespace DirectX;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(ShadowMappingMaterial)

	const map<ShadowMappingDrawModes, string> ShadowMappingMaterial::DrawModeDisplayNames
	{
		{ ShadowMappingDrawModes::Basic, "Basic"s },
		{ ShadowMappingDrawModes::ManualPcf, "Manual PCF"s },
		{ ShadowMappingDrawModes::Pcf, "PCF"s }
	};

	const map<ShadowMappingDrawModes, ShadowMappingMaterial::ShadowMappingShaderClasses> ShadowMappingMaterial::DrawModeShaderClassMap
	{
		{ ShadowMappingDrawModes::Basic, ShadowMappingShaderClasses::Basic },
		{ ShadowMappingDrawModes::ManualPcf, ShadowMappingShaderClasses::ManualPcf },
		{ ShadowMappingDrawModes::Pcf, ShadowMappingShaderClasses::Pcf }
	};

	ShadowMappingMaterial::ShadowMappingMaterial(Game& game, shared_ptr<Texture2D> colorMap, com_ptr<ID3D11ShaderResourceView> shadowMap) :
		Material(game),
		mColorMap(move(colorMap)),
		mShadowMap(move(shadowMap))
	{
	}

	ShadowMappingDrawModes ShadowMappingMaterial::DrawMode() const
	{
		return mDrawMode;
	}

	const string& ShadowMappingMaterial::DrawModeString() const
	{
		return DrawModeDisplayNames.at(mDrawMode);
	}

	void ShadowMappingMaterial::SetDrawMode(ShadowMappingDrawModes drawMode)
	{
		mDrawMode = drawMode;
		SetShaderClassInstance(ShaderStages::PS, mShaderClassInstances[DrawModeShaderClassMap.at(mDrawMode)].get());
	}

	shared_ptr<Texture2D> ShadowMappingMaterial::ColorMap() const
	{
		return mColorMap;
	}

	void ShadowMappingMaterial::SetColorMap(shared_ptr<Texture2D> texture)
	{
		assert(texture != nullptr);
		mColorMap = move(texture);
		ResetPixelShaderResources();
	}

	com_ptr<ID3D11ShaderResourceView> ShadowMappingMaterial::ShadowMap() const
	{
		return mShadowMap;
	}

	void ShadowMappingMaterial::SetShadowMap(com_ptr<ID3D11ShaderResourceView> shadowMap)
	{
		assert(shadowMap != nullptr);
		mShadowMap = move(shadowMap);
		ResetPixelShaderResources();
	}

	const XMFLOAT4& ShadowMappingMaterial::AmbientColor() const
	{
		return mPixelCBufferPerFrameData.AmbientColor;
	}

	void ShadowMappingMaterial::SetAmbientColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.AmbientColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const XMFLOAT3& ShadowMappingMaterial::LightPosition() const
	{
		return mPixelCBufferPerFrameData.LightPosition;
	}

	void ShadowMappingMaterial::SetLightPosition(const XMFLOAT3& position)
	{
		mPixelCBufferPerFrameData.LightPosition = position;
		mPixelCBufferPerFrameDataDirty = true;

		mVertexCBufferPerFrameData.LightPosition = position;
		mVertexCBufferPerFrameDataDirty = true;
	}

	const float ShadowMappingMaterial::LightRadius() const
	{
		return mVertexCBufferPerFrameData.LightRadius;
	}

	void ShadowMappingMaterial::SetLightRadius(float radius)
	{
		mVertexCBufferPerFrameData.LightRadius = radius;
		mVertexCBufferPerFrameDataDirty = true;
	}

	const XMFLOAT4& ShadowMappingMaterial::LightColor() const
	{
		return mPixelCBufferPerFrameData.LightColor;
	}

	void ShadowMappingMaterial::SetLightColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.LightColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	uint32_t ShadowMappingMaterial::VertexSize() const
	{
		return sizeof(VertexPositionTextureNormal);
	}

	void ShadowMappingMaterial::Initialize()
	{
		Material::Initialize();
		mAutoUnbindShaderResourcesEnabled = true;

		auto& content = mGame->Content();
		auto vertexShader = content.Load<VertexShader>(L"Shaders\\ShadowMappingVS.cso"s);
		SetShader(vertexShader);

		auto direct3DDevice = mGame->Direct3DDevice();
		vertexShader->CreateInputLayout<VertexPositionTextureNormal>(direct3DDevice);
		SetInputLayout(vertexShader->InputLayout());

		// Create the projected texture mapping shader with class linkage
		auto classLinkage = Shader::CreateClassLinkage(direct3DDevice);
		PixelShaderWithClassLinkageReader pixelShaderContentReader(*mGame, classLinkage);

		auto pixelShader = content.Load<PixelShader>(L"Shaders\\ShadowMappingPS.cso", false, pixelShaderContentReader);
		SetShader(pixelShader);
		
		ThrowIfFailed(classLinkage->CreateClassInstance("BasicShadowMappingShader", 0, 0, 0, 0, mShaderClassInstances[ShadowMappingShaderClasses::Basic].put()));
		ThrowIfFailed(classLinkage->CreateClassInstance("ManualPcfShadowMappingShader", 0, 0, 0, 0, mShaderClassInstances[ShadowMappingShaderClasses::ManualPcf].put()));
		ThrowIfFailed(classLinkage->CreateClassInstance("PcfShadowMappingShader", 0, 0, 0, 0, mShaderClassInstances[ShadowMappingShaderClasses::Pcf].put()));
		SetDrawMode(ShadowMappingDrawModes::Basic);

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = sizeof(VertexCBufferPerFrame);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerFrame.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::VS, mVertexCBufferPerFrame.get());

		constantBufferDesc.ByteWidth = sizeof(VertexCBufferPerObject);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::VS, mVertexCBufferPerObject.get());

		constantBufferDesc.ByteWidth = sizeof(PixelCBufferPerFrame);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerFrame.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::PS, mPixelCBufferPerFrame.get());

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerFrame.get(), 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
		
		ResetPixelShaderResources();

		ID3D11SamplerState* psSamplerStates[] = { SamplerStates::TrilinearWrap.get(), SamplerStates::ShadowMap.get(), SamplerStates::PcfShadowMap.get() };
		AddSamplerStates(ShaderStages::PS, psSamplerStates);
	}

	const XMFLOAT2& ShadowMappingMaterial::ShadowMapSize() const
	{
		return mPixelCBufferPerFrameData.ShadowMapSize;
	}

	void ShadowMappingMaterial::SetShadowMapSize(float width, float height)
	{
		mPixelCBufferPerFrameData.ShadowMapSize.x = width;
		mPixelCBufferPerFrameData.ShadowMapSize.y = height;
		mPixelCBufferPerFrameDataDirty = true;
	}

	void ShadowMappingMaterial::UpdateTransforms(const VertexCBufferPerObject& transforms)
	{
		mVertexCBufferPerObjectData = transforms;
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
	}

	void ShadowMappingMaterial::UpdateTransforms(FXMMATRIX worldViewProjectionMatrix, CXMMATRIX worldMatrix, CXMMATRIX projectiveTextureMatrix)
	{
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, worldViewProjectionMatrix);
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, worldMatrix);
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.ProjectiveTextureMatrix, projectiveTextureMatrix);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
	}

	void ShadowMappingMaterial::BeginDraw()
	{
		Material::BeginDraw();

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();

		if (mVertexCBufferPerFrameDataDirty)
		{
			direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerFrame.get(), 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
			mVertexCBufferPerFrameDataDirty = false;
		}

		if (mPixelCBufferPerFrameDataDirty)
		{
			direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
			mPixelCBufferPerFrameDataDirty = false;
		}
	}

	void ShadowMappingMaterial::ResetPixelShaderResources()
	{
		Material::ClearShaderResources(ShaderStages::PS);
		ID3D11ShaderResourceView* psShaderResources[] = { mColorMap->ShaderResourceView().get(), mShadowMap.get() };
		AddShaderResources(ShaderStages::PS, psShaderResources);
	}
}