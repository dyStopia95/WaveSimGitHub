#include "pch.h"
#include "ShadowMappingDemo.h"
#include "Game.h"
#include "Texture2D.h"
#include "TexturedModelMaterial.h"
#include "VertexDeclarations.h"
#include "PerspectiveCamera.h"
#include "VectorHelper.h"
#include "Utility.h"
#include "ProxyModel.h"
#include "Frustum.h"
#include "RenderableFrustum.h"
#include "DepthMapMaterial.h"
#include "Model.h"
#include "Mesh.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace winrt;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	ShadowMappingDemo::ShadowMappingDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera),
		mShadowMap(game, ShadowMapWidth, ShadowMapHeight),
		mRenderStateHelper(game)
	{
	}

	ShadowMappingDemo::~ShadowMappingDemo()
	{
	}

	ShadowMappingDrawModes ShadowMappingDemo::DrawMode() const
	{
		return mMaterial->DrawMode();
	}

	const string& ShadowMappingDemo::DrawModeString() const
	{
		return mMaterial->DrawModeString();
	}

	void ShadowMappingDemo::SetDrawMode(ShadowMappingDrawModes drawMode)
	{
		mMaterial->SetDrawMode(drawMode);
	}

	float ShadowMappingDemo::DepthBias() const
	{
		return mDepthBias;
	}

	void ShadowMappingDemo::SetDepthBias(float bias)
	{
		mDepthBias = bias;
		UpdateDepthBiasRasterizerState();
	}

	float ShadowMappingDemo::SlopeScaledDepthBias() const
	{
		return mSlopeScaledDepthBias;
	}

	void ShadowMappingDemo::SetSlopeScaledDepthBias(float bias)
	{
		mSlopeScaledDepthBias = bias;
		UpdateDepthBiasRasterizerState();
	}

	float ShadowMappingDemo::AmbientLightIntensity() const
	{
		return mMaterial->AmbientColor().x;
	}

	void ShadowMappingDemo::SetAmbientLightIntensity(float intensity)
	{
		mMaterial->SetAmbientColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	float ShadowMappingDemo::PointLightIntensity() const
	{
		return mMaterial->LightColor().x;
	}

	void ShadowMappingDemo::SetPointLightIntensity(float intensity)
	{
		mMaterial->SetLightColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	float ShadowMappingDemo::PointLightRadius() const
	{
		return mMaterial->LightRadius();
	}

	void ShadowMappingDemo::SetPointLightRadius(float radius)
	{
		mMaterial->SetLightRadius(radius);
	}

	const Camera& ShadowMappingDemo::Projector() const
	{
		return *mProjector;
	}

	const XMFLOAT3& ShadowMappingDemo::ProjectorPosition() const
	{
		return mProjector->Position();
	}

	const XMVECTOR ShadowMappingDemo::ProjectorPositionVector() const
	{
		return mProjector->PositionVector();
	}

	void ShadowMappingDemo::SetProjectorPosition(const XMFLOAT3& position)
	{
		XMVECTOR positionVector = XMLoadFloat3(&position);
		mPointLight.SetPosition(positionVector);
		mProxyModel->SetPosition(positionVector);
		mProjector->SetPosition(positionVector);
		mRenderableProjectorFrustum->SetPosition(positionVector);
		mMaterial->SetLightPosition(position);
		mUpdateMaterial = true;
	}

	void ShadowMappingDemo::SetProjectorPosition(FXMVECTOR position)
	{
		mPointLight.SetPosition(position);
		mProxyModel->SetPosition(position);
		mProjector->SetPosition(position);
		mRenderableProjectorFrustum->SetPosition(position);
		
		XMFLOAT3 lightPosition;
		XMStoreFloat3(&lightPosition, position);
		mMaterial->SetLightPosition(lightPosition);
		mUpdateMaterial = true;
	}

	const XMFLOAT3& ShadowMappingDemo::ProjectorDirection() const
	{
		return mProjector->Direction();
	}

	void ShadowMappingDemo::RotateProjector(const DirectX::XMFLOAT2& amount)
	{
		XMMATRIX rotationMatrix = XMMatrixRotationY(amount.x) * XMMatrixRotationAxis(mProjector->RightVector(), amount.y);
		mProjector->ApplyRotation(rotationMatrix);
		mRenderableProjectorFrustum->ApplyRotation(rotationMatrix);
		mUpdateMaterial = true;
	}

	void ShadowMappingDemo::Initialize()
	{
		// Color texture for the plane we'll be projecting onto
		auto& content = mGame->Content();
		auto colorMap = content.Load<Texture2D>(L"Textures\\Checkerboard.png"s);

		mMaterial = make_shared<ShadowMappingMaterial>(*mGame, colorMap, mShadowMap.OutputTexture());
		mMaterial->Initialize();

		// Proxy model for the point light
		mProxyModel = make_unique<ProxyModel>(*mGame, mCamera, "Models\\PointLightProxy.obj.bin"s, 0.5f);
		mProxyModel->Initialize();

		// Set up projector
		mProjector = make_unique<PerspectiveCamera>(*mGame);
		mProjector->SetNearPlaneDistance(0.5f);
		mProjector->SetFarPlaneDistance(100.0f);
		mProjector->Initialize();		

		// Renderable frustum for visualizing the projector
		mRenderableProjectorFrustum = make_unique<RenderableFrustum>(*mGame, mCamera);
		mRenderableProjectorFrustum->Initialize();
		mRenderableProjectorFrustum->InitializeGeometry(Frustum(mProjector->ViewProjectionMatrix()));
		
		// The projector, point light, point light proxy model,
		// and renderable frustum are positioned collectively
		SetProjectorPosition(XMFLOAT3(0.0f, 5.0f, 6.0f));

		// Material for rendering the scene to the shadow (depth) map
		mDepthMapMaterial = make_shared<DepthMapMaterial>(*mGame);
		mDepthMapMaterial->Initialize();

		auto updateMaterialFunc = [this]() { mUpdateMaterial = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateMaterialFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateMaterialFunc);

		const VertexPositionTextureNormal planeSourceVertices[]
		{
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),

			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), Vector3Helper::Backward),
		};

		auto direct3DDevice = mGame->Direct3DDevice();
		const span<const VertexPositionTextureNormal> planeVertices{ planeSourceVertices };
		mPlaneVertexCount = narrow_cast<uint32_t>(planeVertices.size());
		VertexPositionTextureNormal::CreateVertexBuffer(direct3DDevice, planeVertices, not_null<ID3D11Buffer**>(mPlaneVertexBuffer.put()));
		
		// Scale the plane
		XMStoreFloat4x4(&mPlaneWorldMatrix, XMMatrixScaling(10.0f, 10.0f, 10.0f));

		// Load teapot model and create its vertex and index buffers
		const auto model = mGame->Content().Load<Model>(L"Models\\Teapot.obj.bin"s);
		Mesh* mesh = model->Meshes().at(0).get();
		VertexPositionTextureNormal::CreateVertexBuffer(direct3DDevice, *mesh, not_null<ID3D11Buffer**>(mTeapotVertexBuffer.put()));
		VertexPosition::CreateVertexBuffer(direct3DDevice, *mesh, not_null<ID3D11Buffer**>(mTeapotPositionOnlyVertexBuffer.put()));
		mesh->CreateIndexBuffer(*direct3DDevice, not_null<ID3D11Buffer**>(mTeapotIndexBuffer.put()));
		mTeapotIndexCount = narrow<uint32_t>(mesh->Indices().size());

		// Position and scale the teapot
		XMStoreFloat4x4(&mTeapotWorldMatrix, XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixTranslation(0.0f, 5.0f, 2.5f));

		// Sprite batch for rendering the depth map to the screen
		mSpriteBatch = make_unique<SpriteBatch>(mGame->Direct3DDeviceContext());

		mMaterial->SetShadowMapSize(static_cast<float>(mShadowMap.Width()), static_cast<float>(mShadowMap.Height()));
		UpdateDepthBiasRasterizerState();
	}

	void ShadowMappingDemo::Update(const GameTime& gameTime)
	{
		mProjector->Update(gameTime);
		mProxyModel->Update(gameTime);		
		mRenderableProjectorFrustum->Update(gameTime);
	}

	void ShadowMappingDemo::Draw(const GameTime& gameTime)
	{
		mProxyModel->Draw(gameTime);
		mRenderableProjectorFrustum->Draw(gameTime);

		if (mUpdateMaterial)
		{
			const XMMATRIX projectiveTextureMatrix = mProjector->ViewProjectionMatrix() * XMLoadFloat4x4(&mProjectedTextureScalingMatrix);

			{
				// Update plane transforms
				const XMMATRIX worldMatrix = XMLoadFloat4x4(&mPlaneWorldMatrix);
				const XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
				XMMATRIX worldProjectiveTextureMatrix = worldMatrix * projectiveTextureMatrix;
				UpdateTransforms(mPlaneTransforms, XMMatrixTranspose(wvp), XMMatrixTranspose(worldMatrix), XMMatrixTranspose(worldProjectiveTextureMatrix));
			}

			{
				// Update teapot transforms
				const XMMATRIX worldMatrix = XMLoadFloat4x4(&mTeapotWorldMatrix);
				const XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
				XMMATRIX worldProjectiveTextureMatrix = worldMatrix * projectiveTextureMatrix;
				UpdateTransforms(mTeapotTransforms, XMMatrixTranspose(wvp), XMMatrixTranspose(worldMatrix), XMMatrixTranspose(worldProjectiveTextureMatrix));
			
				const XMMATRIX worldLightViewProjection = worldMatrix * mProjector->ViewProjectionMatrix();
				mDepthMapMaterial->UpdateTransform(XMMatrixTranspose(worldLightViewProjection));
			}
			
			mUpdateMaterial = false;
		}
	
		// Shadow (depth) map pass (render the teapot model only)
		mRenderStateHelper.SaveRasterizerState();
		mShadowMap.Begin();

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->ClearDepthStencilView(mShadowMap.DepthStencilView().get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		
		direct3DDeviceContext->RSSetState(mDepthBiasState.get());
		mDepthMapMaterial->DrawIndexed(not_null<ID3D11Buffer*>(mTeapotPositionOnlyVertexBuffer.get()), not_null<ID3D11Buffer*>(mTeapotIndexBuffer.get()), mTeapotIndexCount);
		
		mShadowMap.End();
		mRenderStateHelper.RestoreRasterizerState();

		// Render the plane
		mMaterial->UpdateTransforms(mPlaneTransforms);
		mMaterial->Draw(not_null<ID3D11Buffer*>(mPlaneVertexBuffer.get()), mPlaneVertexCount);

		// Render the teapot
		mMaterial->UpdateTransforms(mTeapotTransforms);
		mMaterial->DrawIndexed(not_null<ID3D11Buffer*>(mTeapotVertexBuffer.get()), not_null<ID3D11Buffer*>(mTeapotIndexBuffer.get()), mTeapotIndexCount);

		// Render the shadow map in the lower left-hand corner (debug output)
		mRenderStateHelper.SaveAll();
		mSpriteBatch->Begin();
		mSpriteBatch->Draw(mShadowMap.OutputTexture().get(), ShadowMapDestinationRectangle);
		mSpriteBatch->End();
		Material::UnbindShaderResources<1>(direct3DDeviceContext, ShaderStages::PS);
		mRenderStateHelper.RestoreAll();
	}

	void ShadowMappingDemo::UpdateTransforms(ShadowMappingMaterial::VertexCBufferPerObject& transforms, FXMMATRIX worldViewProjectionMatrix, CXMMATRIX worldMatrix, CXMMATRIX projectiveTextureMatrix)
	{
		XMStoreFloat4x4(&transforms.WorldViewProjection, worldViewProjectionMatrix);
		XMStoreFloat4x4(&transforms.World, worldMatrix);
		XMStoreFloat4x4(&transforms.ProjectiveTextureMatrix, projectiveTextureMatrix);
	}

	void ShadowMappingDemo::UpdateDepthBiasRasterizerState()
	{
		D3D11_RASTERIZER_DESC rasterizerStateDesc;
		ZeroMemory(&rasterizerStateDesc, sizeof(rasterizerStateDesc));
		rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
		rasterizerStateDesc.DepthClipEnable = true;
		rasterizerStateDesc.DepthBias = static_cast<int>(mDepthBias);
		rasterizerStateDesc.SlopeScaledDepthBias = mSlopeScaledDepthBias;

		mDepthBiasState = nullptr;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateRasterizerState(&rasterizerStateDesc, mDepthBiasState.put()), "ID3D11Device::CreateRasterizerState() failed.");
	}
}