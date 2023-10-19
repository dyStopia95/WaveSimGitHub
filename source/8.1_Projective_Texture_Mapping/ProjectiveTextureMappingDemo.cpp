#include "pch.h"
#include "ProjectiveTextureMappingDemo.h"
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
	ProjectiveTextureMappingDemo::ProjectiveTextureMappingDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera),
		mDepthMap(game, DepthMapWidth, DepthMapHeight),
		mRenderStateHelper(game)
	{
	}

	ProjectiveTextureMappingDemo::~ProjectiveTextureMappingDemo()
	{
	}

	ProjectiveTextureMappingDrawModes ProjectiveTextureMappingDemo::DrawMode() const
	{
		return mMaterial->DrawMode();
	}

	const string& ProjectiveTextureMappingDemo::DrawModeString() const
	{
		return mMaterial->DrawModeString();
	}

	void ProjectiveTextureMappingDemo::SetDrawMode(ProjectiveTextureMappingDrawModes drawMode)
	{
		mMaterial->SetDrawMode(drawMode);
	}

	float ProjectiveTextureMappingDemo::AmbientLightIntensity() const
	{
		return mMaterial->AmbientColor().x;
	}

	void ProjectiveTextureMappingDemo::SetAmbientLightIntensity(float intensity)
	{
		mMaterial->SetAmbientColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	float ProjectiveTextureMappingDemo::PointLightIntensity() const
	{
		return mMaterial->LightColor().x;
	}

	void ProjectiveTextureMappingDemo::SetPointLightIntensity(float intensity)
	{
		mMaterial->SetLightColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	float ProjectiveTextureMappingDemo::PointLightRadius() const
	{
		return mMaterial->LightRadius();
	}

	void ProjectiveTextureMappingDemo::SetPointLightRadius(float radius)
	{
		mMaterial->SetLightRadius(radius);
	}

	const XMFLOAT3& ProjectiveTextureMappingDemo::ProjectorPosition() const
	{
		return mProjector->Position();
	}

	const XMVECTOR ProjectiveTextureMappingDemo::ProjectorPositionVector() const
	{
		return mProjector->PositionVector();
	}

	void ProjectiveTextureMappingDemo::SetProjectorPosition(const XMFLOAT3& position)
	{
		XMVECTOR positionVector = XMLoadFloat3(&position);
		mPointLight.SetPosition(positionVector);
		mProxyModel->SetPosition(positionVector);
		mProjector->SetPosition(positionVector);
		mRenderableProjectorFrustum->SetPosition(positionVector);
		mMaterial->SetLightPosition(position);
		mUpdateMaterial = true;
	}

	void ProjectiveTextureMappingDemo::SetProjectorPosition(FXMVECTOR position)
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

	const XMFLOAT3& ProjectiveTextureMappingDemo::ProjectorDirection() const
	{
		return mProjector->Direction();
	}

	void ProjectiveTextureMappingDemo::RotateProjector(const DirectX::XMFLOAT2& amount)
	{
		XMMATRIX rotationMatrix = XMMatrixRotationY(amount.x) * XMMatrixRotationAxis(mProjector->RightVector(), amount.y);
		mProjector->ApplyRotation(rotationMatrix);
		mRenderableProjectorFrustum->ApplyRotation(rotationMatrix);
		mUpdateMaterial = true;
	}

	void ProjectiveTextureMappingDemo::Initialize()
	{
		// Color texture for the plane we'll be projecting onto
		auto& content = mGame->Content();
		auto colorMap = content.Load<Texture2D>(L"Textures\\Checkerboard.png"s);

		// Initialize the projected scaling matrix from the size of the projected texture
		auto projectedMap = content.Load<Texture2D>(L"Textures\\ProjectedTexture.png"s);
		InitializeProjectedTextureScalingMatrix(projectedMap->Width(), projectedMap->Height());

		mMaterial = make_shared<ProjectiveTextureMappingMaterial>(*mGame, colorMap, projectedMap, mDepthMap.OutputTexture());
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

		// Depth map for occluding the projected texture
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
	}

	void ProjectiveTextureMappingDemo::Update(const GameTime& gameTime)
	{
		mProjector->Update(gameTime);
		mProxyModel->Update(gameTime);		
		mRenderableProjectorFrustum->Update(gameTime);
	}

	void ProjectiveTextureMappingDemo::Draw(const GameTime& gameTime)
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

		if (DrawMode() == ProjectiveTextureMappingDrawModes::WithDepthMap)
		{
			DrawWithDepthMap();
		}
		else
		{
			DrawWithoutDepthMap();
		}		
	}

	void ProjectiveTextureMappingDemo::DrawWithDepthMap()
	{
		// Depth map pass (render the teapot model only)
		mDepthMap.Begin();
		
		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->ClearDepthStencilView(mDepthMap.DepthStencilView().get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		mDepthMapMaterial->DrawIndexed(not_null<ID3D11Buffer*>(mTeapotPositionOnlyVertexBuffer.get()), not_null<ID3D11Buffer*>(mTeapotIndexBuffer.get()), mTeapotIndexCount);

		mDepthMap.End();

		// Render the plane
		mMaterial->UpdateTransforms(mPlaneTransforms);
		mMaterial->Draw(not_null<ID3D11Buffer*>(mPlaneVertexBuffer.get()), mPlaneVertexCount);

		// Render the teapot
		mMaterial->UpdateTransforms(mTeapotTransforms);
		mMaterial->DrawIndexed(not_null<ID3D11Buffer*>(mTeapotVertexBuffer.get()), not_null<ID3D11Buffer*>(mTeapotIndexBuffer.get()), mTeapotIndexCount);

		// Render the depth map in the lower left-hand corner (debug output)
		mRenderStateHelper.SaveAll();
		mSpriteBatch->Begin();
		mSpriteBatch->Draw(mDepthMap.OutputTexture().get(), DepthMapDestinationRectangle);
		mSpriteBatch->End();
		Material::UnbindShaderResources<1>(direct3DDeviceContext, ShaderStages::PS);
		mRenderStateHelper.RestoreAll();
	}

	void ProjectiveTextureMappingDemo::DrawWithoutDepthMap()
	{		
		mMaterial->UpdateTransforms(mPlaneTransforms);
		mMaterial->Draw(not_null<ID3D11Buffer*>(mPlaneVertexBuffer.get()), mPlaneVertexCount);
	}

	void ProjectiveTextureMappingDemo::UpdateTransforms(ProjectiveTextureMappingMaterial::VertexCBufferPerObject& transforms, FXMMATRIX worldViewProjectionMatrix, CXMMATRIX worldMatrix, CXMMATRIX projectiveTextureMatrix)
	{
		XMStoreFloat4x4(&transforms.WorldViewProjection, worldViewProjectionMatrix);
		XMStoreFloat4x4(&transforms.World, worldMatrix);
		XMStoreFloat4x4(&transforms.ProjectiveTextureMatrix, projectiveTextureMatrix);
	}

	void ProjectiveTextureMappingDemo::InitializeProjectedTextureScalingMatrix(uint32_t textureWidth, uint32_t textureHeight)
	{
		float scalingBiasX = 0.5f + (0.5f / textureWidth);
		float scalingBiasY = 0.5f + (0.5f / textureHeight);

		mProjectedTextureScalingMatrix._11 = 0.5f;
		mProjectedTextureScalingMatrix._22 = -0.5f;
		mProjectedTextureScalingMatrix._33 = 1.0f;
		mProjectedTextureScalingMatrix._41 = scalingBiasX;
		mProjectedTextureScalingMatrix._42 = scalingBiasY;
		mProjectedTextureScalingMatrix._44 = 1.0f;
	}
}