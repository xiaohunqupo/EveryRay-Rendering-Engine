#pragma once
#include "Common.h"
#include "GameComponent.h"
#include "ConstantBuffer.h"
#include "ER_GPUTexture.h"
#include "DepthTarget.h"
#include "ER_IlluminationProbeManager.h"

#define NUM_VOXEL_GI_CASCADES 2
#define NUM_VOXEL_GI_TEX_MIPS 6
#define VCT_GI_MAIN_PASS_DOWNSCALE 0.5

namespace Library
{
	class GameTime;
	class FullScreenRenderTarget;
	class DirectionalLight;
	class Camera;
	class Scene;
	class ER_GBuffer;
	class ER_ShadowMapper;
	class FoliageSystem;
	class IBLRadianceMap;
	class RenderableAABB;
	class RenderingObject;
	class ER_GPUBuffer;

	namespace IlluminationCBufferData {
		struct VoxelizationDebugCB
		{
			XMMATRIX WorldVoxelCube;
			XMMATRIX ViewProjection;
		};
		struct VoxelConeTracingCB
		{
			XMFLOAT4 VoxelCameraPositions[NUM_VOXEL_GI_CASCADES];
			XMFLOAT4 WorldVoxelScales[NUM_VOXEL_GI_CASCADES];
			XMFLOAT4 CameraPos;
			XMFLOAT2 UpsampleRatio;
			float IndirectDiffuseStrength;
			float IndirectSpecularStrength;
			float MaxConeTraceDistance;
			float AOFalloff;
			float SamplingFactor;
			float VoxelSampleOffset;
			float GIPower;
			XMFLOAT3 pad0;
		};
		struct UpsampleBlurCB
		{
			bool Upsample;
		};
		struct DeferredLightingCB
		{
			XMMATRIX ShadowMatrices[NUM_SHADOW_CASCADES];
			XMFLOAT4 ShadowCascadeDistances;
			XMFLOAT4 ShadowTexelSize;
			XMFLOAT4 SunDirection;
			XMFLOAT4 SunColor;
			XMFLOAT4 CameraPosition;
			bool SkipIndirectProbeLighting;
		};
		struct ForwardLightingCB
		{
			XMMATRIX ShadowMatrices[NUM_SHADOW_CASCADES];
			XMMATRIX ViewProjection;
			XMMATRIX World;
			XMFLOAT4 ShadowTexelSize;
			XMFLOAT4 ShadowCascadeDistances;
			XMFLOAT4 SunDirection;
			XMFLOAT4 SunColor;
			XMFLOAT4 CameraPosition;
			float UseGlobalDiffuseProbe;
		};
		struct LightProbesCB
		{
			XMFLOAT4 DiffuseProbesCellsCount[NUM_PROBE_VOLUME_CASCADES]; //x,y,z,total
			XMFLOAT4 DiffuseProbesVolumeSizes[NUM_PROBE_VOLUME_CASCADES];
			XMFLOAT4 SpecularProbesCellsCount[NUM_PROBE_VOLUME_CASCADES]; //x,y,z,total
			XMFLOAT4 SpecularProbesVolumeSizes[NUM_PROBE_VOLUME_CASCADES];
			XMFLOAT4 ProbesVolumeIndexSkips[NUM_PROBE_VOLUME_CASCADES]; //index skip, 0, 0, 0
			XMFLOAT4 SceneLightProbesBounds; //volume's extent of all scene's probes
			float DistanceBetweenDiffuseProbes;
			float DistanceBetweenSpecularProbes;
		};
	}

	class Illumination : public GameComponent
	{
	public:
		Illumination(Game& game, Camera& camera, const DirectionalLight& light, const ER_ShadowMapper& shadowMapper, const Scene* scene);
		~Illumination();

		void Initialize(const Scene* scene);

		void DrawLocalIllumination(ER_GBuffer* gbuffer, ER_GPUTexture* aRenderTarget, bool isEditorMode = false, bool clearInitTarget = false);
		void DrawGlobalIllumination(ER_GBuffer* gbuffer, const GameTime& gameTime);

		void Update(const GameTime& gameTime, const Scene* scene);
		void Config() { mShowDebug = !mShowDebug; }

		void SetShadowMapSRV(ID3D11ShaderResourceView* srv) { mShadowMapSRV = srv; }
		void SetFoliageSystemForGI(FoliageSystem* foliageSystem);

		void SetProbesManager(ER_IlluminationProbeManager* manager) { mProbesManager = manager; }

		bool GetDebugVoxels() { return mDrawVoxelization; }
		bool GetDebugAO() { return mDrawAmbientOcclusionOnly; }

		void PrepareForForwardLighting(RenderingObject* aObj, int meshIndex);

		ID3D11ShaderResourceView* GetGlobaIlluminationSRV() const {
			if (mDrawVoxelization)
				return mVCTVoxelizationDebugRT->GetSRV();
			else
				return mVCTUpsampleAndBlurRT->GetSRV();
		}
	private:
		void DrawDeferredLighting(ER_GBuffer* gbuffer, ER_GPUTexture* aRenderTarget, bool clearTarget = false);
		void DrawForwardLighting(ER_GBuffer* gbuffer, ER_GPUTexture* aRenderTarget);
		void DrawDebugGizmos();

		void UpdateImGui();
		void UpdateVoxelCameraPosition();

		void CPUCullObjectsAgainstVoxelCascades(const Scene* scene);

		Camera& mCamera;
		const DirectionalLight& mDirectionalLight;
		const ER_ShadowMapper& mShadowMapper;

		ER_IlluminationProbeManager* mProbesManager = nullptr;
		FoliageSystem* mFoliageSystem = nullptr;

		using RenderingObjectInfo = std::map<std::string, RenderingObject*>;
		RenderingObjectInfo mVoxelizationObjects[NUM_VOXEL_GI_CASCADES];

		ConstantBuffer<IlluminationCBufferData::VoxelizationDebugCB> mVoxelizationDebugConstantBuffer;
		ConstantBuffer<IlluminationCBufferData::VoxelConeTracingCB> mVoxelConeTracingConstantBuffer;
		ConstantBuffer<IlluminationCBufferData::UpsampleBlurCB> mUpsampleBlurConstantBuffer;
		ConstantBuffer<IlluminationCBufferData::DeferredLightingCB> mDeferredLightingConstantBuffer;
		ConstantBuffer<IlluminationCBufferData::ForwardLightingCB> mForwardLightingConstantBuffer;
		ConstantBuffer<IlluminationCBufferData::LightProbesCB> mLightProbesConstantBuffer;

		std::vector<ER_GPUTexture*> mVCTVoxelCascades3DRTs;
		ER_GPUTexture* mVCTVoxelizationDebugRT = nullptr;
		ER_GPUTexture* mVCTMainRT = nullptr;
		ER_GPUTexture* mVCTUpsampleAndBlurRT = nullptr;

		ER_GBuffer* mGbuffer = nullptr;

		DepthTarget* mDepthBuffer = nullptr;

		ID3D11VertexShader* mVCTVoxelizationDebugVS = nullptr;
		ID3D11GeometryShader* mVCTVoxelizationDebugGS = nullptr;
		ID3D11PixelShader* mVCTVoxelizationDebugPS = nullptr;
		ID3D11ComputeShader* mVCTMainCS = nullptr;
		ID3D11ComputeShader* mUpsampleBlurCS = nullptr;
		ID3D11ComputeShader* mDeferredLightingCS = nullptr;
		ID3D11VertexShader* mForwardLightingVS = nullptr;
		ID3D11VertexShader* mForwardLightingVS_Instancing = nullptr;
		ID3D11PixelShader* mForwardLightingPS = nullptr;
		ID3D11PixelShader* mForwardLightingDiffuseProbesPS = nullptr;
		ID3D11PixelShader* mForwardLightingSpecularProbesPS = nullptr;

		ID3D11ShaderResourceView* mShadowMapSRV = nullptr;

		ID3D11DepthStencilState* mDepthStencilStateRW = nullptr;
		
		ID3D11InputLayout* mForwardLightingRenderingObjectInputLayout = nullptr;
		ID3D11InputLayout* mForwardLightingRenderingObjectInputLayout_Instancing = nullptr;

		float mWorldVoxelScales[NUM_VOXEL_GI_CASCADES] = { 2.0f, 0.5f };
		XMFLOAT4 mVoxelCameraPositions[NUM_VOXEL_GI_CASCADES];
		ER_AABB mVoxelCascadesAABBs[NUM_VOXEL_GI_CASCADES];
		
		std::vector<RenderableAABB*> mDebugVoxelZonesGizmos;

		float mVCTIndirectDiffuseStrength = 1.0f;
		float mVCTIndirectSpecularStrength = 1.0f;
		float mVCTMaxConeTraceDistance = 100.0f;
		float mVCTAoFalloff = 2.0f;
		float mVCTSamplingFactor = 0.5f;
		float mVCTVoxelSampleOffset = 0.0f;
		float mVCTGIPower = 1.0f;
		bool mVoxelCameraPositionsUpdated = true;

		bool mDrawVoxelization = false;
		bool mDrawVoxelZonesGizmos = false;
		bool mDrawAmbientOcclusionOnly = false;
		bool mDrawDiffuseProbes = false;
		bool mDrawSpecularProbes = false;
		bool mDrawProbesVolumeGizmo = false;
		int mCurrentDebugProbeVolumeIndex = 0;
		bool mDebugSkipIndirectProbeLighting = false;

		bool mEnabled = false;
		bool mShowDebug = false;

		RenderingObjectInfo mForwardPassObjects;
	};
}