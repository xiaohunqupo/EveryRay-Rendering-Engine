#include "stdafx.h"

#include "ShadowMapper.h"
#include "Frustum.h"
#include "Projector.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "GameTime.h"
#include "Game.h"
#include "DepthMap.h"
#include "GameException.h"
#include "Scene.h"
#include "MaterialHelper.h"
#include "RenderingObject.h"
#include "ER_ShadowMapMaterial.h"
#include "ER_MaterialsCallbacks.h"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <limits>

using namespace std;

namespace Library
{
	ShadowMapper::ShadowMapper(Game& pGame, Camera& camera, DirectionalLight& dirLight,  UINT pWidth, UINT pHeight, bool isCascaded)
		: GameComponent(pGame),
		mShadowMaps(0, nullptr), 
		mShadowRasterizerState(nullptr),
		mGame(pGame),
		mDirectionalLight(dirLight),
		mCamera(camera),
		mResolution(pWidth),
		mIsCascaded(isCascaded)
	{
		for (int i = 0; i < NUM_SHADOW_CASCADES; i++)
		{
			mLightProjectorCenteredPositions.push_back(XMFLOAT3(0, 0, 0));
			mShadowMaps.push_back(new DepthMap(pGame, pWidth, pHeight));

			mCameraCascadesFrustums.push_back(XMMatrixIdentity());
			(isCascaded) ? mCameraCascadesFrustums[i].SetMatrix(mCamera.GetCustomViewProjectionMatrixForCascade(i)) : mCameraCascadesFrustums[i].SetMatrix(mCamera.ProjectionMatrix());

			mLightProjectors.push_back(new Projector(mGame));
			mLightProjectors[i]->Initialize();
			mLightProjectors[i]->SetProjectionMatrix(GetProjectionBoundingSphere(i));
			//mLightProjectors[i]->ApplyRotation(mDirectionalLight.GetTransform());

		}

		D3D11_RASTERIZER_DESC rasterizerStateDesc;
		ZeroMemory(&rasterizerStateDesc, sizeof(rasterizerStateDesc));
		rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
		rasterizerStateDesc.DepthClipEnable = false;
		rasterizerStateDesc.DepthBias = 0.05f;
		rasterizerStateDesc.SlopeScaledDepthBias = 3.0f;
		rasterizerStateDesc.FrontCounterClockwise = false;
		HRESULT hr = mGame.Direct3DDevice()->CreateRasterizerState(&rasterizerStateDesc, &mShadowRasterizerState);
		if (FAILED(hr))
			throw GameException("CreateRasterizerState() failed while generating a shadow mapper.", hr);

		D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
		depthStencilStateDesc.DepthEnable = TRUE;
		depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		depthStencilStateDesc.StencilEnable = FALSE;
		hr = mGame.Direct3DDevice()->CreateDepthStencilState(&depthStencilStateDesc, &mDepthStencilState);
		if (FAILED(hr))
			throw GameException("CreateDepthStencilState() failed while generating a shadow mapper.", hr);
	}

	ShadowMapper::~ShadowMapper()
	{
		DeletePointerCollection(mShadowMaps);
		DeletePointerCollection(mLightProjectors);
		ReleaseObject(mShadowRasterizerState);
		ReleaseObject(mDepthStencilState);
	}

	void ShadowMapper::Update(const GameTime& gameTime)
	{
		for (size_t i = 0; i < NUM_SHADOW_CASCADES; i++)
		{
			(mIsCascaded) ? mCameraCascadesFrustums[i].SetMatrix(mCamera.GetCustomViewProjectionMatrixForCascade(i)) : mCameraCascadesFrustums[i].SetMatrix(mCamera.ProjectionMatrix());

			mLightProjectors[i]->SetPosition(mLightProjectorCenteredPositions[i].x, mLightProjectorCenteredPositions[i].y, mLightProjectorCenteredPositions[i].z);
			mLightProjectors[i]->SetProjectionMatrix(GetProjectionBoundingSphere(i));
			mLightProjectors[i]->SetViewMatrix(mLightProjectorCenteredPositions[i], mDirectionalLight.Direction(), mDirectionalLight.Up());
			mLightProjectors[i]->Update(gameTime);
		}
	}

	void ShadowMapper::BeginRenderingToShadowMap(int cascadeIndex)
	{
		assert(cascadeIndex < NUM_SHADOW_CASCADES);
		mGame.Direct3DDeviceContext()->RSGetState(&mOriginalRasterizerState);

		mShadowMaps[cascadeIndex]->Begin();
		mGame.Direct3DDeviceContext()->ClearDepthStencilView(mShadowMaps[cascadeIndex]->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		mGame.Direct3DDeviceContext()->RSSetState(mShadowRasterizerState);
	}

	void ShadowMapper::StopRenderingToShadowMap(int cascadeIndex)
	{
		assert(cascadeIndex < NUM_SHADOW_CASCADES);

		mShadowMaps[cascadeIndex]->End();

		mGame.Direct3DDeviceContext()->RSSetState(mOriginalRasterizerState);
	}

	XMMATRIX ShadowMapper::GetViewMatrix(int cascadeIndex /*= 0*/) const
	{
		assert(cascadeIndex < NUM_SHADOW_CASCADES);
		return mLightProjectors.at(cascadeIndex)->ViewMatrix();
	}
	XMMATRIX ShadowMapper::GetProjectionMatrix(int cascadeIndex /*= 0*/) const
	{
		assert(cascadeIndex < NUM_SHADOW_CASCADES);
		return mLightProjectors.at(cascadeIndex)->ProjectionMatrix();
	}

	ID3D11ShaderResourceView* ShadowMapper::GetShadowTexture(int cascadeIndex) const
	{
		assert(cascadeIndex < NUM_SHADOW_CASCADES);
		return mShadowMaps.at(cascadeIndex)->OutputTexture();
	}

	//void ShadowMapper::ApplyRotation()
	//{
	//	for (int i = 0; i < MAX_NUM_OF_CASCADES; i++)
	//		mLightProjectors[i]->ApplyRotation(mDirectionalLight.GetTransform());
	//}	
	void ShadowMapper::ApplyTransform()
	{
		for (int i = 0; i < NUM_SHADOW_CASCADES; i++)
			mLightProjectors[i]->ApplyTransform(mDirectionalLight.GetTransform());
	}

	XMMATRIX ShadowMapper::GetLightProjectionMatrixInFrustum(int index, Frustum& cameraFrustum, DirectionalLight& light)
	{
		assert(index < NUM_SHADOW_CASCADES);

		//create corners
		XMFLOAT3 frustumCorners[8] = {};

		frustumCorners[0] = (cameraFrustum.Corners()[0]);
		frustumCorners[1] = (cameraFrustum.Corners()[1]);
		frustumCorners[2] = (cameraFrustum.Corners()[2]);
		frustumCorners[3] = (cameraFrustum.Corners()[3]);
		frustumCorners[4] = (cameraFrustum.Corners()[4]);
		frustumCorners[5] = (cameraFrustum.Corners()[5]);
		frustumCorners[6] = (cameraFrustum.Corners()[6]);
		frustumCorners[7] = (cameraFrustum.Corners()[7]);

		XMFLOAT3 frustumCenter = { 0, 0, 0 };

		for (size_t i = 0; i < 8; i++)
		{
			frustumCenter = XMFLOAT3(frustumCenter.x + frustumCorners[i].x,
				frustumCenter.y + frustumCorners[i].y,
				frustumCenter.z + frustumCorners[i].z);
		}

		//calculate frustum's center position
		frustumCenter = XMFLOAT3(
			frustumCenter.x * (1.0f / 8.0f),
			frustumCenter.y * (1.0f / 8.0f),
			frustumCenter.z * (1.0f / 8.0f));

		//mLightProjectorCenteredPositions[index] = frustumCenter;

		float minX = (std::numeric_limits<float>::max)();
		float maxX = (std::numeric_limits<float>::min)();
		float minY = (std::numeric_limits<float>::max)();
		float maxY = (std::numeric_limits<float>::min)();
		float minZ = (std::numeric_limits<float>::max)();
		float maxZ = (std::numeric_limits<float>::min)();

		for (int j = 0; j < 8; j++) {

			// Transform the frustum coordinate from world to light space
			XMVECTOR frustumCornerVector = XMLoadFloat3(&frustumCorners[j]);
			frustumCornerVector = XMVector3Transform(frustumCornerVector, (light.LightMatrix(frustumCenter)));

			XMStoreFloat3(&frustumCorners[j], frustumCornerVector);

			minX = min(minX, frustumCorners[j].x);
			maxX = max(maxX, frustumCorners[j].x);
			minY = min(minY, frustumCorners[j].y);
			maxY = max(maxY, frustumCorners[j].y);
			minZ = min(minZ, frustumCorners[j].z);
			maxZ = max(maxZ, frustumCorners[j].z);
		}

		mLightProjectorCenteredPositions[index] =
			XMFLOAT3(
				frustumCenter.x + light.Direction().x * -maxZ,
				frustumCenter.y + light.Direction().y * -maxZ,
				frustumCenter.z + light.Direction().z * -maxZ
			);

		//set orthographic proj with proper boundaries
		float delta = 10.0f;
		XMMATRIX projectionMatrix = XMMatrixOrthographicRH(maxX - minX, maxY - minY, -delta, maxZ - minZ);
		return projectionMatrix;
	}
	XMMATRIX ShadowMapper::GetProjectionBoundingSphere(int index)
	{
		// Create a bounding sphere around the camera frustum for 360 rotation
		float nearV = mCamera.GetCameraNearCascadeDistance(index);
		float farV = mCamera.GetCameraFarCascadeDistance(index);
		float endV = nearV + farV;
		XMFLOAT3 sphereCenter = XMFLOAT3(
			mCamera.Position().x + mCamera.Direction().x * (nearV + 0.5f * endV),
			mCamera.Position().y + mCamera.Direction().y * (nearV + 0.5f * endV),
			mCamera.Position().z + mCamera.Direction().z * (nearV + 0.5f * endV)
		);
		// Create a vector to the frustum far corner
		float tanFovY = tanf(mCamera.FieldOfView());
		float tanFovX = mCamera.AspectRatio() * tanFovY;

		XMFLOAT3 farCorner = XMFLOAT3(
			mCamera.Direction().x + mCamera.Right().x * tanFovX + mCamera.Up().x * tanFovY,
			mCamera.Direction().y + mCamera.Right().y * tanFovX + mCamera.Up().y * tanFovY,
			mCamera.Direction().z + mCamera.Right().z * tanFovX + mCamera.Up().z * tanFovY);
		// Compute the frustumBoundingSphere radius
		XMFLOAT3 boundVec = XMFLOAT3(
			mCamera.Position().x + farCorner.x  * farV - sphereCenter.x,
			mCamera.Position().y + farCorner.y  * farV - sphereCenter.y,
			mCamera.Position().z + farCorner.z  * farV - sphereCenter.z);
		float sphereRadius = sqrt(boundVec.x * boundVec.x + boundVec.y * boundVec.y + boundVec.z * boundVec.z);

		mLightProjectorCenteredPositions[index] =
			XMFLOAT3(
				mCamera.Position().x + mCamera.Direction().x * 0.5f * mCamera.GetCameraFarCascadeDistance(index),
				mCamera.Position().y + mCamera.Direction().y * 0.5f * mCamera.GetCameraFarCascadeDistance(index),
				mCamera.Position().z + mCamera.Direction().z * 0.5f * mCamera.GetCameraFarCascadeDistance(index)
			);

		XMMATRIX projectionMatrix = XMMatrixOrthographicRH(sphereRadius, sphereRadius, -sphereRadius, sphereRadius);
		return projectionMatrix;
	}

	void ShadowMapper::Draw(const Scene* scene)
	{
		mGame.Direct3DDeviceContext()->OMSetDepthStencilState(mDepthStencilState, 0);

		ER_MaterialSystems materialSystems;
		materialSystems.mShadowMapper = this;

		for (int i = 0; i < NUM_SHADOW_CASCADES; i++)
		{
			BeginRenderingToShadowMap(i);
			const std::string name = MaterialHelper::shadowMapMaterialName + " " + std::to_string(i);

			int objectIndex = 0;
			for (auto it = scene->objects.begin(); it != scene->objects.end(); it++, objectIndex++)
			{
				auto materialInfo = it->second->GetNewMaterials().find(name);
				if (materialInfo != it->second->GetNewMaterials().end())
				{
					for (int meshIndex = 0; meshIndex < it->second->GetMeshCount(); meshIndex++)
					{
						static_cast<ER_ShadowMapMaterial*>(materialInfo->second)->PrepareForRendering(materialSystems, it->second, meshIndex, i);
						it->second->Draw(name, true);
					}
				}
			}

			StopRenderingToShadowMap(i);
		}
	}

}