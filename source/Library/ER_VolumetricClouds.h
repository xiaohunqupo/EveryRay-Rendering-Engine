#pragma once
#include "Common.h"
#include "ConstantBuffer.h"
#include "ER_GPUTexture.h"
#include "ER_CoreComponent.h"

namespace Library
{
	class DirectionalLight;
	class ER_CoreTime;
	class ER_Camera;
	class ER_Skybox;

	namespace VolumetricCloudsCBufferData {
		struct FrameCB
		{
			XMMATRIX	InvProj;
			XMMATRIX	InvView;
			XMVECTOR	LightDir;
			XMVECTOR	LightCol;
			XMVECTOR	CameraPos;
			XMFLOAT2	UpsampleRatio;
		};

		struct CloudsCB
		{
			XMVECTOR AmbientColor;
			XMVECTOR WindDir;
			float WindSpeed;
			float Time;
			float Crispiness;
			float Curliness;
			float Coverage;
			float Absorption;
			float CloudsBottomHeight;
			float CloudsTopHeight;
			float DensityFactor;
		};

		struct UpsampleBlurCB
		{
			bool Upsample;
		};
	}

	class ER_VolumetricClouds : public ER_CoreComponent
	{
	public:
		ER_VolumetricClouds(ER_Core& game, ER_Camera& camera, DirectionalLight& light, ER_Skybox& skybox);
		~ER_VolumetricClouds();

		void Initialize(ER_GPUTexture* aIlluminationDepth);

		void Draw(const ER_CoreTime& gametime);
		void Update(const ER_CoreTime& gameTime);
		void Config() { mShowDebug = !mShowDebug; }
		void Composite(ER_GPUTexture* aRenderTarget);
		bool IsEnabled() { return mEnabled; }
		void SetDownscaleFactor(float val) { mDownscaleFactor = val; }
	private:
		void UpdateImGui();

		ER_Camera& mCamera;
		DirectionalLight& mDirectionalLight;
		ER_Skybox& mSkybox;
		
		ConstantBuffer<VolumetricCloudsCBufferData::FrameCB> mFrameConstantBuffer;
		ConstantBuffer<VolumetricCloudsCBufferData::CloudsCB> mCloudsConstantBuffer;
		ConstantBuffer<VolumetricCloudsCBufferData::UpsampleBlurCB> mUpsampleBlurConstantBuffer;

		ER_GPUTexture* mIlluminationResultDepthTarget = nullptr; // not allocated here, just a pointer

		ER_GPUTexture* mSkyRT = nullptr;
		ER_GPUTexture* mSkyAndSunRT = nullptr;
		ER_GPUTexture* mMainRT = nullptr;
		ER_GPUTexture* mUpsampleAndBlurRT = nullptr;
		ER_GPUTexture* mBlurRT = nullptr;

		ID3D11ComputeShader* mMainCS = nullptr;
		ID3D11PixelShader* mCompositePS = nullptr;
		ID3D11PixelShader* mBlurPS = nullptr;
		ID3D11ComputeShader* mUpsampleBlurCS = nullptr;

		ID3D11ShaderResourceView* mCloudTextureSRV = nullptr;
		ID3D11ShaderResourceView* mWeatherTextureSRV = nullptr;
		ID3D11ShaderResourceView* mWorleyTextureSRV = nullptr;

		ID3D11SamplerState* mCloudSS = nullptr;
		ID3D11SamplerState* mWeatherSS = nullptr;

		float mCrispiness = 43.0f;
		float mCurliness = 1.1f;
		float mCoverage = 0.305f;
		float mAmbientColor[3] = { 102.0f / 255.0f, 104.0f / 255.0f, 105.0f / 255.0f };
		float mWindSpeedMultiplier = 175.0f;
		float mLightAbsorption = 0.003f;
		float mCloudsBottomHeight = 2340.0f;
		float mCloudsTopHeight = 16400.0f;
		float mDensityFactor = 0.012f;
		float mDownscaleFactor = 0.5f;

		bool mEnabled = true;
		bool mShowDebug = false;
	};
}