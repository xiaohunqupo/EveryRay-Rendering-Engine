#include "stdafx.h"

#include "TestSceneDemo.h"
#include "..\Library\Game.h"
#include "..\Library\GameException.h"
#include "..\Library\VectorHelper.h"
#include "..\Library\MatrixHelper.h"
#include "..\Library\ColorHelper.h"
#include "..\Library\MaterialHelper.h"
#include "..\Library\Camera.h"
#include "..\Library\Editor.h"
#include "..\Library\Model.h"
#include "..\Library\Mesh.h"
#include "..\Library\Utility.h"
#include "..\Library\DirectionalLight.h"
#include "..\Library\Keyboard.h"
#include "..\Library\Light.h"
#include "..\Library\Skybox.h"
#include "..\Library\Grid.h"
#include "..\Library\DemoLevel.h"
#include "..\Library\RenderStateHelper.h"
#include "..\Library\RenderingObject.h"
#include "..\Library\DepthMap.h"
#include "..\Library\Frustum.h"
#include "..\Library\StandardLightingMaterial.h"
#include "..\Library\ScreenSpaceReflectionsMaterial.h"
#include "..\Library\DepthMapMaterial.h"
#include "..\Library\PostProcessingStack.h"
#include "..\Library\FullScreenRenderTarget.h"
#include "..\Library\IBLRadianceMap.h"
#include "..\Library\DeferredMaterial.h"
#include "..\Library\GBuffer.h"
#include "..\Library\FullScreenQuad.h"
#include "..\Library\ShadowMapper.h"
#include "..\Library\Terrain.h"
#include "..\Library\Foliage.h"
#include "..\Library\Scene.h"
#include "..\Library\ShaderCompiler.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <sstream>

namespace Rendering
{
	RTTI_DEFINITIONS(TestSceneDemo)

	TestSceneDemo::TestSceneDemo(Game& game, Camera& camera, Editor& editor)
		: DrawableGameComponent(game, camera, editor),
		mWorldMatrix(MatrixHelper::Identity),
		mRenderStateHelper(nullptr),
		mDirectionalLight(nullptr),
		mSkybox(nullptr),
		mIrradianceTextureSRV(nullptr),
		mRadianceTextureSRV(nullptr),
		mIntegrationMapTextureSRV(nullptr),
		mGrid(nullptr),
		mGBuffer(nullptr),
		mSSRQuad(nullptr),
		mShadowMapper(nullptr),
		mFoliageCollection(0, nullptr),
		mScene(nullptr)
		//mTerrain(nullptr)
	{
	}

	TestSceneDemo::~TestSceneDemo()
	{
		//DeleteObject(mTerrain)
		DeleteObject(mRenderStateHelper);
		DeleteObject(mDirectionalLight);
		DeleteObject(mSkybox);
		DeleteObject(mGrid);
		DeleteObject(mPostProcessingStack);
		DeleteObject(mGBuffer);
		DeleteObject(mSSRQuad);
		DeleteObject(mShadowMapper);
		DeletePointerCollection(mFoliageCollection);
		ReleaseObject(mIrradianceTextureSRV);
		ReleaseObject(mRadianceTextureSRV);
		ReleaseObject(mIntegrationMapTextureSRV);
		DeleteObject(mScene);
	}

#pragma region COMPONENT_METHODS
	/////////////////////////////////////////////////////////////
	// 'DemoLevel' ugly methods...
	bool TestSceneDemo::IsComponent()
	{
		return mGame->IsInGameLevels<TestSceneDemo*>(mGame->levels, this);
	}
	void TestSceneDemo::Create()
	{
		Initialize();
		mGame->levels.push_back(this);
	}
	void TestSceneDemo::Destroy()
	{
		this->~TestSceneDemo();
		mGame->levels.clear();
	}
	/////////////////////////////////////////////////////////////  
#pragma endregion

	void TestSceneDemo::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		mGBuffer = new GBuffer(*mGame, *mCamera, mGame->ScreenWidth(), mGame->ScreenHeight());
		mGBuffer->Initialize();

		mScene = new Scene(*mGame, *mCamera, Utility::GetFilePath("content\\levels\\testScene.json"));
		for (auto& object : mScene->objects) {
			object.second->MeshMaterialVariablesUpdateEvent->AddListener(MaterialHelper::lightingMaterialName, [&](int meshIndex) { UpdateStandardLightingPBRMaterialVariables(object.first, meshIndex); });
			object.second->MeshMaterialVariablesUpdateEvent->AddListener(MaterialHelper::deferredPrepassMaterialName, [&](int meshIndex) { UpdateDeferredPrepassMaterialVariables(object.first, meshIndex); });
			object.second->MeshMaterialVariablesUpdateEvent->AddListener(MaterialHelper::shadowMapMaterialName + " " + std::to_string(0), [&](int meshIndex) { UpdateShadow0MaterialVariables(object.first, meshIndex); });
			object.second->MeshMaterialVariablesUpdateEvent->AddListener(MaterialHelper::shadowMapMaterialName + " " + std::to_string(1), [&](int meshIndex) { UpdateShadow1MaterialVariables(object.first, meshIndex); });
			object.second->MeshMaterialVariablesUpdateEvent->AddListener(MaterialHelper::shadowMapMaterialName + " " + std::to_string(2), [&](int meshIndex) { UpdateShadow2MaterialVariables(object.first, meshIndex); });
		}
		mEditor->LoadScene(mScene);

		mKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
		assert(mKeyboard != nullptr);

		mSkybox = new Skybox(*mGame, *mCamera, Utility::GetFilePath(Utility::ToWideString(mScene->skyboxPath)), 10000);
		mSkybox->Initialize();

		mGrid = new Grid(*mGame, *mCamera, 200, 56, XMFLOAT4(0.961f, 0.871f, 0.702f, 1.0f));
		mGrid->Initialize();
		mGrid->SetColor((XMFLOAT4)ColorHelper::LightGray);


		//directional light
		mDirectionalLight = new DirectionalLight(*mGame, *mCamera);
		mDirectionalLight->ApplyRotation(XMMatrixRotationAxis(mDirectionalLight->RightVector(), -XMConvertToRadians(70.0f)) * XMMatrixRotationAxis(mDirectionalLight->UpVector(), -XMConvertToRadians(25.0f)));
		mDirectionalLight->SetAmbientColor(mScene->ambientColor);
		mDirectionalLight->SetSunColor(mScene->sunColor);

		mShadowMapper = new ShadowMapper(*mGame, *mCamera, *mDirectionalLight, 4096, 4096);
		mDirectionalLight->RotationUpdateEvent->AddListener("shadow mapper", [&]() {mShadowMapper->ApplyTransform(); });

		mRenderStateHelper = new RenderStateHelper(*mGame);

		//PP
		mPostProcessingStack = new PostProcessingStack(*mGame, *mCamera);
		mPostProcessingStack->Initialize(false, false, true, true, true, false);

		//foliage
		mFoliageCollection.push_back(new Foliage(*mGame, *mCamera, *mDirectionalLight, 1500, Utility::GetFilePath("content\\textures\\foliage\\grass_type1.png"), 2.5f, 100.0f));
		//mFoliageCollection.push_back(new Foliage(*mGame, *mCamera, *mDirectionalLight, 10000, Utility::GetFilePath("content\\textures\\foliage\\grass_type2.png"), 1.3f, 100.0f));
		//mFoliageCollection.push_back(new Foliage(*mGame, *mCamera, *mDirectionalLight, 1000, Utility::GetFilePath("content\\textures\\foliage\\grass_type3.png"), 1.5f, 100.0f));
		mFoliageCollection.push_back(new Foliage(*mGame, *mCamera, *mDirectionalLight, 3500, Utility::GetFilePath("content\\textures\\foliage\\grass_type4.png"), 2.0f, 100.0f));
		mFoliageCollection.push_back(new Foliage(*mGame, *mCamera, *mDirectionalLight, 100, Utility::GetFilePath("content\\textures\\foliage\\grass_flower_type1.png"), 3.5f, 100.0f));
		mFoliageCollection.push_back(new Foliage(*mGame, *mCamera, *mDirectionalLight, 50, Utility::GetFilePath("content\\textures\\foliage\\grass_flower_type3.png"), 2.5f, 100.0f));
		mFoliageCollection.push_back(new Foliage(*mGame, *mCamera, *mDirectionalLight, 100, Utility::GetFilePath("content\\textures\\foliage\\grass_flower_type10.png"), 3.5f, 100.0f));

		for (auto& foliage : mFoliageCollection) {
			foliage->CreateBufferGPU();
		}

		mCamera->SetPosition(mScene->cameraPosition);
		mCamera->SetDirection(mScene->cameraDirection);
		mCamera->SetFarPlaneDistance(100000.0f);

		//IBL
		if (FAILED(DirectX::CreateDDSTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), Utility::GetFilePath(L"content\\textures\\Sky_Type_4_PBRDiffuseHDR.dds").c_str(), nullptr, &mIrradianceTextureSRV)))
			throw GameException("Failed to create Irradiance Map.");

		// Create an IBL Radiance Map from Environment Map
		mIBLRadianceMap.reset(new IBLRadianceMap(*mGame, Utility::GetFilePath(L"content\\textures\\Sky_Type_4.dds")));
		mIBLRadianceMap->Initialize();
		mIBLRadianceMap->Create(*mGame);

		mRadianceTextureSRV = *mIBLRadianceMap->GetShaderResourceViewAddress();
		if (mRadianceTextureSRV == nullptr)
			throw GameException("Failed to create Radiance Map.");
		mIBLRadianceMap.release();
		mIBLRadianceMap.reset(nullptr);

		// Load a pre-computed Integration Map
		if (FAILED(DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), Utility::GetFilePath(L"content\\textures\\PBR\\Skyboxes\\ibl_brdf_lut.png").c_str(), nullptr, &mIntegrationMapTextureSRV)))
			throw GameException("Failed to create Integration Texture.");

		//volumetric clouds
		ID3DBlob* blob = nullptr;
		if (FAILED(ShaderCompiler::CompileShader(Utility::GetFilePath(L"content\\shaders\\VolumetricClouds\\VolumetricClouds.hlsl").c_str(), "main", "ps_5_0", &blob)))
			throw GameException("Failed to load main pass from shader: VolumetricClouds.hlsl!");
		if (FAILED(mGame->Direct3DDevice()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &VCMainPS)))
			throw GameException("Failed to create shader from VolumetricClouds.hlsl!");
		blob->Release();

		if (FAILED(DirectX::CreateDDSTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), Utility::GetFilePath(L"content\\textures\\VolumetricClouds\\cloud.dds").c_str(), nullptr, &mCloudTextureSRV)))
			throw GameException("Failed to create Cloud Map.");

		if (FAILED(DirectX::CreateDDSTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), Utility::GetFilePath(L"content\\textures\\VolumetricClouds\\weather.dds").c_str(), nullptr, &mWeatherTextureSRV)))
			throw GameException("Failed to create Weather Map.");

		if (FAILED(DirectX::CreateDDSTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), Utility::GetFilePath(L"content\\textures\\VolumetricClouds\\worley.dds").c_str(), nullptr, &mWorleyTextureSRV)))
			throw GameException("Failed to create Worley Map.");

		mVolumetricCloudsFrameConstantBuffer.Initialize(mGame->Direct3DDevice());
		mVolumetricCloudsCloudsConstantBuffer.Initialize(mGame->Direct3DDevice());

		D3D11_SAMPLER_DESC sam_desc;
		sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sam_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sam_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sam_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sam_desc.MipLODBias = 0;
		sam_desc.MaxAnisotropy = 1;
		sam_desc.MinLOD = -1000.0f;
		sam_desc.MaxLOD = 1000.0f;
		if (FAILED(mGame->Direct3DDevice()->CreateSamplerState(&sam_desc, &mCloudSS)))
			throw GameException("Failed to create sampler mCloudSS!");

		sam_desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		if (FAILED(mGame->Direct3DDevice()->CreateSamplerState(&sam_desc, &mWeatherSS)))
			throw GameException("Failed to create sampler mWeatherSS!");

	}

	void TestSceneDemo::UpdateLevel(const GameTime& gameTime)
	{
		UpdateImGui();

		mDirectionalLight->UpdateProxyModel(gameTime, mCamera->ViewMatrix4X4(), mCamera->ProjectionMatrix4X4());
		mSkybox->SetUseCustomColor(mEditor->IsSkyboxUsingCustomColor());
		mSkybox->SetColors(mEditor->GetBottomSkyColor(), mEditor->GetTopSkyColor());
		mSkybox->Update(gameTime);
		mGrid->Update(gameTime);
		mPostProcessingStack->Update();

		//for (auto object : mFoliageCollection) 
		//{
		//	object->SetWindParams(mWindGustDistance, mWindStrength, mWindFrequency);
		//	object->Update(gameTime);
		//}
		
		mCamera->Cull(mScene->objects);
		mShadowMapper->Update(gameTime);

		//for (auto& object : mScene->objects)
		//	object.second->Update(gameTime);

		mEditor->Update(gameTime);
	}

	void TestSceneDemo::UpdateImGui()
	{

		ImGui::Begin("Test Demo Scene");

		ImGui::Checkbox("Show Post Processing Stack", &mPostProcessingStack->isWindowOpened);
		if (mPostProcessingStack->isWindowOpened) mPostProcessingStack->ShowPostProcessingWindow();

		ImGui::Separator();

		ImGui::SliderFloat("Wind strength", &mWindStrength, 0.0f, 100.0f);
		ImGui::SliderFloat("Wind gust distance", &mWindGustDistance, 0.0f, 100.0f);
		ImGui::SliderFloat("Wind frequency", &mWindFrequency, 0.0f, 100.0f);

		ImGui::ColorEdit3("Clouds ambient color", mCloudsAmbientColor);
		ImGui::SliderFloat("Clouds bottom height", &mCloudsLayerInnerHeight, 1000.0f, 10000.0f);
		ImGui::SliderFloat("Clouds top height", &mCloudsLayerOuterHeight, 10000.0f, 50000.0f);
		ImGui::SliderFloat("Clouds crispiness", &mCloudsCrispiness, 0.0f, 100.0f);
		ImGui::SliderFloat("Clouds curliness", &mCloudsCurliness, 0.0f, 5.0f);
		ImGui::SliderFloat("Clouds coverage", &mCloudsCoverage, 0.0f, 1.0f);
		ImGui::SliderFloat("Clouds wind speed factor", &mCloudsWindSpeedMultiplier, 0.0f, 10000.0f);

		ImGui::End();
	}

	void TestSceneDemo::DrawLevel(const GameTime& gameTime)
	{
		float clear_color[4] = { 0.0f, 1.0f, 1.0f, 0.0f };

		mRenderStateHelper->SaveRasterizerState();

		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		/*
#pragma region DEFERRED_PREPASS
		
		mGBuffer->Start();
		for (auto it = mScene->objects.begin(); it != mScene->objects.end(); it++)
			it->second->Draw(MaterialHelper::deferredPrepassMaterialName, true);
		mGBuffer->End();

#pragma endregion

#pragma region DRAW_SHADOWS

		for (int i = 0; i < MAX_NUM_OF_CASCADES; i++)
		{
			mShadowMapper->BeginRenderingToShadowMap(i);
			const std::string name = MaterialHelper::shadowMapMaterialName + " " + std::to_string(i);
		
			XMMATRIX lvp = mShadowMapper->GetViewMatrix(i) * mShadowMapper->GetProjectionMatrix(i);
			int objectIndex = 0;
			for (auto it = mScene->objects.begin(); it != mScene->objects.end(); it++, objectIndex++)
			{
				static_cast<DepthMapMaterial*>(it->second->GetMaterials()[name])->LightViewProjection() << lvp;
				//static_cast<DepthMapMaterial*>(it->second->GetMaterials()[name])->AlbedoAlphaMap() << it->second->GetTextureData(objectIndex).AlbedoMap;
				it->second->Draw(name, true);
			}
		
			mShadowMapper->StopRenderingToShadowMap(i);
		}

		mRenderStateHelper->RestoreRasterizerState();
		
#pragma endregion
*/
		mPostProcessingStack->Begin();

#pragma region DRAW_LIGHTING

		//skybox
		mSkybox->Draw(gameTime);

		//grid
		//if (Utility::IsEditorMode)
		//	mGrid->Draw(gameTime);

		//gizmo
		if (Utility::IsEditorMode)
			mDirectionalLight->DrawProxyModel(gameTime);

		//lighting
		//for (auto it = mScene->objects.begin(); it != mScene->objects.end(); it++)
		//	it->second->Draw(MaterialHelper::lightingMaterialName);

		////foliage 
		//for (auto object : mFoliageCollection)
		//	object->Draw(gameTime);

#pragma endregion

		mPostProcessingStack->End();
		DrawVolumetricClouds(gameTime);


		mPostProcessingStack->UpdateSSRMaterial(mGBuffer->GetNormals()->getSRV(), mGBuffer->GetDepth()->getSRV(), mGBuffer->GetExtraBuffer()->getSRV(), (float)gameTime.TotalGameTime());
		mPostProcessingStack->DrawEffects(gameTime);
		
		mRenderStateHelper->SaveAll();

#pragma region DRAW_IMGUI
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

#pragma endregion
	}

	void TestSceneDemo::DrawVolumetricClouds(const GameTime& gameTime)
	{
		ID3D11DeviceContext* context = mGame->Direct3DDeviceContext();
		
		//mPostProcessingStack->Begin(false);
		mVolumetricCloudsFrameConstantBuffer.Data.invProj = XMMatrixInverse(nullptr, mCamera->ProjectionMatrix());
		mVolumetricCloudsFrameConstantBuffer.Data.invView = XMMatrixInverse(nullptr, mCamera->ViewMatrix());
		mVolumetricCloudsFrameConstantBuffer.Data.lightDir = -mDirectionalLight->DirectionVector();
		mVolumetricCloudsFrameConstantBuffer.Data.lightCol = XMVECTOR{ mDirectionalLight->GetDirectionalLightColor().x, mDirectionalLight->GetDirectionalLightColor().y, mDirectionalLight->GetDirectionalLightColor().z, 1.0f };
		mVolumetricCloudsFrameConstantBuffer.Data.cameraPos = mCamera->PositionVector();
		mVolumetricCloudsFrameConstantBuffer.Data.resolution = XMFLOAT2(mGame->ScreenWidth(), mGame->ScreenHeight());
		mVolumetricCloudsFrameConstantBuffer.ApplyChanges(context);

		mVolumetricCloudsCloudsConstantBuffer.Data.AmbientColor = XMVECTOR{ mCloudsAmbientColor[0], mCloudsAmbientColor[1], mCloudsAmbientColor[2], 1.0f };
		mVolumetricCloudsCloudsConstantBuffer.Data.WindDir = XMVECTOR{1.0f, 0.0f, 0.0f, 1.0f};
		mVolumetricCloudsCloudsConstantBuffer.Data.WindSpeed = mWindStrength * mCloudsWindSpeedMultiplier;
		mVolumetricCloudsCloudsConstantBuffer.Data.Time = static_cast<float>(gameTime.TotalGameTime());
		mVolumetricCloudsCloudsConstantBuffer.Data.Crispiness = mCloudsCrispiness;
		mVolumetricCloudsCloudsConstantBuffer.Data.Curliness = mCloudsCurliness;
		mVolumetricCloudsCloudsConstantBuffer.Data.Coverage = mCloudsCoverage;
		mVolumetricCloudsCloudsConstantBuffer.Data.CloudsLayerSphereInnerRadius = mCloudsLayerInnerHeight;
		mVolumetricCloudsCloudsConstantBuffer.Data.CloudsLayerSphereOuterRadius = mCloudsLayerOuterHeight;
		mVolumetricCloudsCloudsConstantBuffer.ApplyChanges(context);

		ID3D11Buffer* CBs[2] = { mVolumetricCloudsFrameConstantBuffer.Buffer(), mVolumetricCloudsCloudsConstantBuffer.Buffer() };
		ID3D11ShaderResourceView* SR[4] = { mPostProcessingStack->GetPrepassColorOutputTexture(), mWeatherTextureSRV, mCloudTextureSRV, mWorleyTextureSRV };
		ID3D11SamplerState* SS[2] = { mCloudSS, mWeatherSS };
		context->PSSetShaderResources(0, 4, SR);
		context->PSSetConstantBuffers(0, 2, CBs);
		context->PSSetSamplers(0, 2, SS);
		context->PSSetShader(VCMainPS, NULL, NULL);
		mPostProcessingStack->DrawFullscreenQuad(context);

		//mPostProcessingStack->End();
	}

	void TestSceneDemo::UpdateStandardLightingPBRMaterialVariables(const std::string& objectName, int meshIndex)
	{
		XMMATRIX worldMatrix = XMLoadFloat4x4(&(mScene->objects[objectName]->GetTransformationMatrix4X4()));
		XMMATRIX vp = /*worldMatrix * */mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		//XMMATRIX shadowMatrix = /*worldMatrix **/ mShadowMapper->GetViewMatrix() * mShadowMapper->GetProjectionMatrix() /*mShadowMapViewMatrix * mShadowMapProjectionMatrix*//* mShadowProjector->ViewMatrix() * mShadowProjector->ProjectionMatrix()*/ * XMLoadFloat4x4(&MatrixHelper::GetProjectionShadowMatrix());

		XMMATRIX shadowMatrices[3] = 
		{ 
			mShadowMapper->GetViewMatrix(0) *  mShadowMapper->GetProjectionMatrix(0) * XMLoadFloat4x4(&MatrixHelper::GetProjectionShadowMatrix()) ,
			mShadowMapper->GetViewMatrix(1) *  mShadowMapper->GetProjectionMatrix(1) * XMLoadFloat4x4(&MatrixHelper::GetProjectionShadowMatrix()) ,
			mShadowMapper->GetViewMatrix(2) *  mShadowMapper->GetProjectionMatrix(2) * XMLoadFloat4x4(&MatrixHelper::GetProjectionShadowMatrix()) 
		};

		ID3D11ShaderResourceView* shadowMaps[3] = 
		{
			mShadowMapper->GetShadowTexture(0),
			mShadowMapper->GetShadowTexture(1),
			mShadowMapper->GetShadowTexture(2)
		};

		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->ViewProjection() << vp;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->World() << worldMatrix;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->ShadowMatrices().SetMatrixArray(shadowMatrices, 0, MAX_NUM_OF_CASCADES);
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->CameraPosition() << mCamera->PositionVector();
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->SunDirection() << XMVectorNegate(mDirectionalLight->DirectionVector());
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->SunColor() << XMVECTOR{ mDirectionalLight->GetDirectionalLightColor().x,  mDirectionalLight->GetDirectionalLightColor().y, mDirectionalLight->GetDirectionalLightColor().z , 1.0f };
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->AmbientColor() << XMVECTOR{ mDirectionalLight->GetAmbientLightColor().x,  mDirectionalLight->GetAmbientLightColor().y, mDirectionalLight->GetAmbientLightColor().z , 1.0f };
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->ShadowTexelSize() << XMVECTOR{ 1.0f / mShadowMapper->GetResolution(), 1.0f, 1.0f , 1.0f };
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->ShadowCascadeDistances() << XMVECTOR{ mCamera->GetCameraFarCascadeDistance(0), mCamera->GetCameraFarCascadeDistance(1), mCamera->GetCameraFarCascadeDistance(2), 1.0f };
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->AlbedoTexture() << mScene->objects[objectName]->GetTextureData(meshIndex).AlbedoMap;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->NormalTexture() << mScene->objects[objectName]->GetTextureData(meshIndex).NormalMap;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->SpecularTexture() << mScene->objects[objectName]->GetTextureData(meshIndex).SpecularMap;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->RoughnessTexture() << mScene->objects[objectName]->GetTextureData(meshIndex).RoughnessMap;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->MetallicTexture() << mScene->objects[objectName]->GetTextureData(meshIndex).MetallicMap;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->CascadedShadowTextures().SetResourceArray(shadowMaps, 0, MAX_NUM_OF_CASCADES);
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->IrradianceTexture() << mIrradianceTextureSRV;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->RadianceTexture() << mRadianceTextureSRV;
		static_cast<StandardLightingMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::lightingMaterialName])->IntegrationTexture() << mIntegrationMapTextureSRV;
	}
	void TestSceneDemo::UpdateDeferredPrepassMaterialVariables(const std::string & objectName, int meshIndex)
	{
		XMMATRIX worldMatrix = XMLoadFloat4x4(&(mScene->objects[objectName]->GetTransformationMatrix4X4()));
		XMMATRIX vp = /*worldMatrix * */mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		static_cast<DeferredMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::deferredPrepassMaterialName])->ViewProjection() << vp;
		static_cast<DeferredMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::deferredPrepassMaterialName])->World() << worldMatrix;
		static_cast<DeferredMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::deferredPrepassMaterialName])->AlbedoMap() << mScene->objects[objectName]->GetTextureData(meshIndex).AlbedoMap;
		static_cast<DeferredMaterial*>(mScene->objects[objectName]->GetMaterials()[MaterialHelper::deferredPrepassMaterialName])->ReflectionMaskFactor() << mScene->objects[objectName]->GetMeshReflectionFactor(meshIndex);
	}
	void TestSceneDemo::UpdateShadow0MaterialVariables(const std::string & objectName, int meshIndex)
	{
		const std::string name = MaterialHelper::shadowMapMaterialName + " " + std::to_string(0);
		static_cast<DepthMapMaterial*>(mScene->objects[objectName]->GetMaterials()[name])->AlbedoAlphaMap() << mScene->objects[objectName]->GetTextureData(meshIndex).AlbedoMap;
	}
	void TestSceneDemo::UpdateShadow1MaterialVariables(const std::string & objectName, int meshIndex)
	{
		const std::string name = MaterialHelper::shadowMapMaterialName + " " + std::to_string(1);
		static_cast<DepthMapMaterial*>(mScene->objects[objectName]->GetMaterials()[name])->AlbedoAlphaMap() << mScene->objects[objectName]->GetTextureData(meshIndex).AlbedoMap;
	}
	void TestSceneDemo::UpdateShadow2MaterialVariables(const std::string & objectName, int meshIndex)
	{
		const std::string name = MaterialHelper::shadowMapMaterialName + " " + std::to_string(2);
		static_cast<DepthMapMaterial*>(mScene->objects[objectName]->GetMaterials()[name])->AlbedoAlphaMap() << mScene->objects[objectName]->GetTextureData(meshIndex).AlbedoMap;
	}
}