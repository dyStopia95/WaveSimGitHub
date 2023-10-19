#include "pch.h"
#include "ProjectiveTextureMappingMaterial.h"
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
	RTTI_DEFINITIONS(ProjectiveTextureMappingMaterial)

	const map<ProjectiveTextureMappingDrawModes, string> ProjectiveTextureMappingMaterial::DrawModeDisplayNames
	{
		{ ProjectiveTextureMappingDrawModes::Basic, "Basic"s },
		{ ProjectiveTextureMappingDrawModes::NoReverseProjection, "No Reverse Projection"s },
		{ ProjectiveTextureMappingDrawModes::WithDepthMap, "With Depth Map"s }
	};

	const map<ProjectiveTextureMappingDrawModes, ProjectiveTextureMappingMaterial::ProjectiveTextureMappingShaderClasses> ProjectiveTextureMappingMaterial::DrawModeShaderClassMap
	{
		{ ProjectiveTextureMappingDrawModes::Basic, ProjectiveTextureMappingShaderClasses::Basic },
		{ ProjectiveTextureMappingDrawModes::NoReverseProjection, ProjectiveTextureMappingShaderClasses::NoReverse },
		{ ProjectiveTextureMappingDrawModes::WithDepthMap, ProjectiveTextureMappingShaderClasses::DepthMap }
	};

	ProjectiveTextureMappingMaterial::ProjectiveTextureMappingMaterial(Game& game, shared_ptr<Texture2D> colorMap, shared_ptr<Texture2D> projectedMap, com_ptr<ID3D11ShaderResourceView> depthMap) :
		Material(game),
		mColorMap(move(colorMap)),
		mProjectedMap(move(projectedMap)),
		mDepthMap(move(depthMap))
	{
	}

	ProjectiveTextureMappingDrawModes ProjectiveTextureMappingMaterial::DrawMode() const
	{
		return mDrawMode;
	}

	const string& ProjectiveTextureMappingMaterial::DrawModeString() const
	{
		return DrawModeDisplayNames.at(mDrawMode);
	}

	void ProjectiveTextureMappingMaterial::SetDrawMode(ProjectiveTextureMappingDrawModes drawMode)
	{
		mDrawMode = drawMode;
		SetShaderClassInstance(ShaderStages::PS, mShaderClassInstances[DrawModeShaderClassMap.at(mDrawMode)].get());
	}

	com_ptr<ID3D11SamplerState> ProjectiveTextureMappingMaterial::ColorMapSamplerState() const
	{
		return mColorMapSamplerState;
	}

	void ProjectiveTextureMappingMaterial::SetColorMapSamplerState(com_ptr<ID3D11SamplerState> samplerState)
	{
		mColorMapSamplerState = move(samplerState);
	}

	com_ptr<ID3D11SamplerState> ProjectiveTextureMappingMaterial::ProjectedMapSamplerState() const
	{
		return mProjectedMapSamplerState;
	}

	void ProjectiveTextureMappingMaterial::SetProjectedMapSamplerState(com_ptr<ID3D11SamplerState> samplerState)
	{
		mProjectedMapSamplerState = move(samplerState);
	}

	com_ptr<ID3D11SamplerState> ProjectiveTextureMappingMaterial::DepthMapSamplerState() const
	{
		return mDepthMapSamplerState;
	}

	void ProjectiveTextureMappingMaterial::SetDepthMapSamplerState(com_ptr<ID3D11SamplerState> samplerState)
	{
		mDepthMapSamplerState = move(samplerState);
	}

	shared_ptr<Texture2D> ProjectiveTextureMappingMaterial::ColorMap() const
	{
		return mColorMap;
	}

	void ProjectiveTextureMappingMaterial::SetColorMap(shared_ptr<Texture2D> texture)
	{
		mColorMap = move(texture);
		ResetPixelShaderResources();
	}

	shared_ptr<Texture2D> ProjectiveTextureMappingMaterial::ProjectedMap() const
	{
		return mProjectedMap;
	}

	void ProjectiveTextureMappingMaterial::SetProjectedMap(shared_ptr<Texture2D> texture)
	{
		mProjectedMap = move(texture);
		ResetPixelShaderResources();
	}

	com_ptr<ID3D11ShaderResourceView> ProjectiveTextureMappingMaterial::DepthMap() const
	{
		return mDepthMap;
	}

	void ProjectiveTextureMappingMaterial::SetDepthMap(com_ptr<ID3D11ShaderResourceView> depthMap)
	{
		mDepthMap = move(depthMap);
		ResetPixelShaderResources();
	}

	const XMFLOAT4& ProjectiveTextureMappingMaterial::AmbientColor() const
	{
		return mPixelCBufferPerFrameData.AmbientColor;
	}

	void ProjectiveTextureMappingMaterial::SetAmbientColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.AmbientColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const XMFLOAT3& ProjectiveTextureMappingMaterial::LightPosition() const
	{
		return mPixelCBufferPerFrameData.LightPosition;
	}

	void ProjectiveTextureMappingMaterial::SetLightPosition(const XMFLOAT3& position)
	{
		mPixelCBufferPerFrameData.LightPosition = position;
		mPixelCBufferPerFrameDataDirty = true;

		mVertexCBufferPerFrameData.LightPosition = position;
		mVertexCBufferPerFrameDataDirty = true;
	}

	const float ProjectiveTextureMappingMaterial::LightRadius() const
	{
		return mVertexCBufferPerFrameData.LightRadius;
	}

	void ProjectiveTextureMappingMaterial::SetLightRadius(float radius)
	{
		mVertexCBufferPerFrameData.LightRadius = radius;
		mVertexCBufferPerFrameDataDirty = true;
	}

	const XMFLOAT4& ProjectiveTextureMappingMaterial::LightColor() const
	{
		return mPixelCBufferPerFrameData.LightColor;
	}

	void ProjectiveTextureMappingMaterial::SetLightColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.LightColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	uint32_t ProjectiveTextureMappingMaterial::VertexSize() const
	{
		return sizeof(VertexPositionTextureNormal);
	}

	void ProjectiveTextureMappingMaterial::Initialize()
	{
		Material::Initialize();

		auto& content = mGame->Content();
		auto vertexShader = content.Load<VertexShader>(L"Shaders\\ProjectiveTextureMappingVS.cso"s);
		SetShader(vertexShader);

		auto direct3DDevice = mGame->Direct3DDevice();
		vertexShader->CreateInputLayout<VertexPositionTextureNormal>(direct3DDevice);
		SetInputLayout(vertexShader->InputLayout());

		// Create the projected texture mapping shader with class linkage
		auto classLinkage = Shader::CreateClassLinkage(direct3DDevice);
		PixelShaderWithClassLinkageReader pixelShaderContentReader(*mGame, classLinkage);

		auto pixelShader = content.Load<PixelShader>(L"Shaders\\ProjectiveTextureMappingPS.cso", false, pixelShaderContentReader);
		SetShader(pixelShader);
		
		ThrowIfFailed(classLinkage->CreateClassInstance("BasicProjectiveTextureMappingShader", 0, 0, 0, 0, mShaderClassInstances[ProjectiveTextureMappingShaderClasses::Basic].put()));
		ThrowIfFailed(classLinkage->CreateClassInstance("NoReverseProjectiveTextureMappingShader", 0, 0, 0, 0, mShaderClassInstances[ProjectiveTextureMappingShaderClasses::NoReverse].put()));
		ThrowIfFailed(classLinkage->CreateClassInstance("DepthMapProjectiveTextureMappingShader", 0, 0, 0, 0, mShaderClassInstances[ProjectiveTextureMappingShaderClasses::DepthMap].put()));
		SetDrawMode(ProjectiveTextureMappingDrawModes::Basic);		

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
	}

	void ProjectiveTextureMappingMaterial::UpdateTransforms(const VertexCBufferPerObject& transforms)
	{
		mVertexCBufferPerObjectData = transforms;
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
	}

	void ProjectiveTextureMappingMaterial::UpdateTransforms(FXMMATRIX worldViewProjectionMatrix, CXMMATRIX worldMatrix, CXMMATRIX projectiveTextureMatrix)
	{
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, worldViewProjectionMatrix);
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, worldMatrix);
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.ProjectiveTextureMatrix, projectiveTextureMatrix);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
	}

	void ProjectiveTextureMappingMaterial::BeginDraw()
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

		if (mDrawMode == ProjectiveTextureMappingDrawModes::WithDepthMap)
		{
			direct3DDeviceContext->PSSetShaderResources(0, narrow_cast<uint32_t>(mDepthMapPSShadersResources.size()), mDepthMapPSShadersResources.data());
		}
		else
		{
			direct3DDeviceContext->PSSetShaderResources(0, narrow_cast<uint32_t>(mNoDepthMapPSShadersResources.size()), mNoDepthMapPSShadersResources.data());
		}

		ID3D11SamplerState* const psSamplerStates[] = { mColorMapSamplerState.get(), mProjectedMapSamplerState.get(), mDepthMapSamplerState.get() };
		direct3DDeviceContext->PSSetSamplers(0, narrow_cast<uint32_t>(size(psSamplerStates)), psSamplerStates);
	}

	void ProjectiveTextureMappingMaterial::EndDraw()
	{
		UnbindShaderResources<3>(ShaderStages::PS);
	}

	void ProjectiveTextureMappingMaterial::ResetPixelShaderResources()
	{
		mDepthMapPSShadersResources = { mColorMap->ShaderResourceView().get(), mProjectedMap->ShaderResourceView().get(), mDepthMap.get() };
		mNoDepthMapPSShadersResources = { mColorMap->ShaderResourceView().get(), mProjectedMap->ShaderResourceView().get() };
	}
}