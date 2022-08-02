#pragma once
#include "Common.h"
#include "ER_CoreComponent.h"
#include "RHI/ER_RHI.h"

namespace Library
{
	class ER_Frustum;
	class ER_Projector;
	class ER_Camera;
	class DirectionalLight;
	class ER_Scene;
	class ER_Terrain;

	class ER_ShadowMapper : public ER_CoreComponent 
	{
	public:
		ER_ShadowMapper(ER_Core& pCore, ER_Camera& camera, DirectionalLight& dirLight, UINT pWidth, UINT pHeight, bool isCascaded = true);
		~ER_ShadowMapper();

		void Draw(const ER_Scene* scene, ER_Terrain* terrain = nullptr);
		void Update(const ER_CoreTime& gameTime);
		void BeginRenderingToShadowMap(int cascadeIndex = 0);
		void StopRenderingToShadowMap(int cascadeIndex = 0);
		XMMATRIX GetViewMatrix(int cascadeIndex = 0) const;
		XMMATRIX GetProjectionMatrix(int cascadeIndex = 0) const;
		ER_RHI_GPUTexture* GetShadowTexture(int cascadeIndex = 0) const;
		UINT GetResolution() const { return mResolution; }
		void ApplyTransform();
		//void ApplyRotation();

	private:
		XMMATRIX GetLightProjectionMatrixInFrustum(int index, ER_Frustum& cameraFrustum, DirectionalLight& light);
		XMMATRIX GetProjectionBoundingSphere(int index);

		ER_Camera& mCamera;
		DirectionalLight& mDirectionalLight;

		std::vector<ER_RHI_GPUTexture*> mShadowMaps;
		std::vector<ER_Projector*> mLightProjectors;
		std::vector<ER_Frustum> mCameraCascadesFrustums;
		std::vector<XMFLOAT3> mLightProjectorCenteredPositions;

		ER_RHI_RASTERIZER_STATE mOriginalRS;
		ER_RHI_Viewport mOriginalViewport;
		XMMATRIX mShadowMapViewMatrix;
		XMMATRIX mShadowMapProjectionMatrix;
		UINT mResolution = 0;
		bool mIsCascaded = true;
	};
}