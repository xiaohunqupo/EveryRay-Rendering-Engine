#pragma once
#include "..\Common.h"

#define ER_RHI_MAX_GRAPHICS_COMMAND_LISTS 4
#define ER_RHI_MAX_COMPUTE_COMMAND_LISTS 2
#define ER_RHI_MAX_BOUND_VERTEX_BUFFERS 2 //we only support 1 vertex buffer + 1 instance buffer

namespace Library
{
	static const int DefaultFrameRate = 60;

	enum ER_GRAPHICS_API
	{
		DX11,
		DX12
	};

	enum ER_RHI_SHADER_TYPE
	{
		ER_VERTEX,
		ER_GEOMETRY,
		ER_TESSELLATION_HULL,
		ER_TESSELLATION_DOMAIN,
		ER_PIXEL,
		ER_COMPUTE
	};

	enum ER_RHI_DEPTH_STENCIL_STATE
	{
		ER_DISABLED,
		ER_DEPTH_ONLY_READ_COMPARISON_NEVER,
		ER_DEPTH_ONLY_READ_COMPARISON_LESS,
		ER_DEPTH_ONLY_READ_COMPARISON_EQUAL,
		ER_DEPTH_ONLY_READ_COMPARISON_LESS_EQUAL,
		ER_DEPTH_ONLY_READ_COMPARISON_GREATER,
		ER_DEPTH_ONLY_READ_COMPARISON_NOT_EQUAL,
		ER_DEPTH_ONLY_READ_COMPARISON_GREATER_EQUAL,
		ER_DEPTH_ONLY_READ_COMPARISON_ALWAYS,
		ER_DEPTH_ONLY_WRITE_COMPARISON_NEVER,
		ER_DEPTH_ONLY_WRITE_COMPARISON_LESS,
		ER_DEPTH_ONLY_WRITE_COMPARISON_EQUAL,
		ER_DEPTH_ONLY_WRITE_COMPARISON_LESS_EQUAL,
		ER_DEPTH_ONLY_WRITE_COMPARISON_GREATER,
		ER_DEPTH_ONLY_WRITE_COMPARISON_NOT_EQUAL,
		ER_DEPTH_ONLY_WRITE_COMPARISON_GREATER_EQUAL,
		ER_DEPTH_ONLY_WRITE_COMPARISON_ALWAYS
		//TODO: add support for stencil
	};

	enum ER_RHI_BLEND_STATE
	{
		ER_NO_BLEND,
		ER_ALPHA_TO_COVERAGE
	};

	enum ER_RHI_SAMPLER_STATE
	{
		ER_BILINEAR_WRAP,
		ER_BILINEAR_CLAMP,
		ER_BILINEAR_BORDER,
		ER_BILINEAR_MIRROR,
		ER_TRILINEAR_WRAP,
		ER_TRILINEAR_CLAMP,
		ER_TRILINEAR_BORDER,
		ER_TRILINEAR_MIRROR,
		ER_ANISOTROPIC_WRAP,
		ER_ANISOTROPIC_CLAMP,
		ER_ANISOTROPIC_BORDER,
		ER_ANISOTROPIC_MIRROR,
		ER_SHADOW_SS /* this is dirty */
	};

	enum ER_RHI_RASTERIZER_STATE
	{
		ER_NO_CULLING,
		ER_BACK_CULLING,
		ER_FRONT_CULLING,
		ER_WIREFRAME,
		ER_NO_CULLING_NO_DEPTH_SCISSOR_ENABLED,
		ER_SHADOW_RS/* this is dirty */
	};

	enum ER_RHI_RESOURCE_STATE
	{

	};

	enum ER_RHI_FORMAT
	{
		ER_FORMAT_UNKNOWN = 0,
		ER_FORMAT_R32G32B32A32_TYPELESS,
		ER_FORMAT_R32G32B32A32_FLOAT,
		ER_FORMAT_R32G32B32A32_UINT,
		ER_FORMAT_R32G32B32_TYPELESS,
		ER_FORMAT_R32G32B32_FLOAT,
		ER_FORMAT_R32G32B32_UINT,
		ER_FORMAT_R16G16B16A16_TYPELESS,
		ER_FORMAT_R16G16B16A16_FLOAT,
		ER_FORMAT_R16G16B16A16_UNORM,
		ER_FORMAT_R16G16B16A16_UINT,
		ER_FORMAT_R32G32_TYPELESS,
		ER_FORMAT_R32G32_FLOAT,
		ER_FORMAT_R32G32_UINT,
		ER_FORMAT_R10G10B10A2_TYPELESS,
		ER_FORMAT_R10G10B10A2_UNORM,
		ER_FORMAT_R10G10B10A2_UINT,
		ER_FORMAT_R11G11B10_FLOAT,
		ER_FORMAT_R8G8B8A8_TYPELESS,
		ER_FORMAT_R8G8B8A8_UNORM,
		ER_FORMAT_R8G8B8A8_UINT,
		ER_FORMAT_R16G16_TYPELESS,
		ER_FORMAT_R16G16_FLOAT,
		ER_FORMAT_R16G16_UNORM,
		ER_FORMAT_R16G16_UINT,
		ER_FORMAT_R32_TYPELESS,
		ER_FORMAT_D32_FLOAT,
		ER_FORMAT_R32_FLOAT,
		ER_FORMAT_R32_UINT,
		ER_FORMAT_D24_UNORM_S8_UINT,
		ER_FORMAT_R8G8_TYPELESS,
		ER_FORMAT_R8G8_UNORM,
		ER_FORMAT_R8G8_UINT,
		ER_FORMAT_R16_TYPELESS,
		ER_FORMAT_R16_FLOAT,
		ER_FORMAT_R16_UNORM,
		ER_FORMAT_R16_UINT,
		ER_FORMAT_R8_TYPELESS,
		ER_FORMAT_R8_UNORM,
		ER_FORMAT_R8_UINT
	};

	enum ER_RHI_PRIMITIVE_TYPE
	{
		ER_PRIMITIVE_TOPOLOGY_POINTLIST,
		ER_PRIMITIVE_TOPOLOGY_LINELIST,
		ER_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		ER_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
		ER_PRIMITIVE_TOPOLOGY_CONTROL_POINT_PATCHLIST
	};

	enum ER_RHI_BIND_FLAG
	{
		ER_BIND_NONE = 0x0L,
		ER_BIND_VERTEX_BUFFER = 0x1L,
		ER_BIND_INDEX_BUFFER = 0x2L,
		ER_BIND_CONSTANT_BUFFER = 0x4L,
		ER_BIND_SHADER_RESOURCE = 0x8L,
		ER_BIND_STREAM_OUTPUT = 0x10L,
		ER_BIND_RENDER_TARGET = 0x20L,
		ER_BIND_DEPTH_STENCIL = 0x40L,
		ER_BIND_UNORDERED_ACCESS = 0x80L
	};

	struct ER_RHI_INPUT_ELEMENT_DESC
	{
		LPCSTR SemanticName;
		UINT SemanticIndex;
		ER_RHI_FORMAT Format;
		UINT InputSlot;
		UINT AlignedByteOffset;
		bool IsPerVertex = true;
		UINT InstanceDataStepRate = 0;
	};

	struct ER_RHI_Viewport
	{
		float TopLeftX;
		float TopLeftY;
		float Width;
		float Height;
		float MinDepth = 0.0f;
		float MaxDepth = 1.0f;
	};

	struct ER_RHI_Rect
	{
		LONG left;
		LONG top;
		LONG right;
		LONG bottom;
	};

	class ER_RHI_InputLayout
	{
	public:
		ER_RHI_InputLayout(ER_RHI_INPUT_ELEMENT_DESC* inputElementDescriptions, UINT inputElementDescriptionCount)
		{
			mInputElementDescriptions = inputElementDescriptions;
			mInputElementDescriptionCount = inputElementDescriptionCount;
		};
		virtual ~ER_RHI_InputLayout();

		ER_RHI_INPUT_ELEMENT_DESC* mInputElementDescriptions;
		UINT mInputElementDescriptionCount;
	};
	
	class ER_RHI_GPUResource;
	class ER_RHI_GPUTexture;
	class ER_RHI_GPUBuffer;
	class ER_RHI_GPUShader;

	class ER_RHI
	{
	public:
		ER_RHI();
		virtual ~ER_RHI();

		virtual bool Initialize(UINT width, UINT height, bool isFullscreen) = 0;
		
		virtual void BeginGraphicsCommandList() = 0;
		virtual void EndGraphicsCommandList() = 0;

		virtual void BeginComputeCommandList() = 0;
		virtual void EndComputeCommandList() = 0;

		virtual void ClearMainRenderTarget(float colors[4]) = 0;
		virtual void ClearMainDepthStencilTarget(float depth, UINT stencil = 0) = 0;
		virtual void ClearRenderTarget(ER_RHI_GPUTexture* aRenderTarget, float colors[4]) = 0;
		virtual void ClearDepthStencilTarget(ER_RHI_GPUTexture* aDepthTarget, float depth, UINT stencil = 0) = 0;
		virtual void ClearUAV(ER_RHI_GPUResource* aRenderTarget, float colors[4]) = 0;
		virtual void CreateInputLayout(ER_RHI_InputLayout* aOutInputLayout, ER_RHI_INPUT_ELEMENT_DESC* inputElementDescriptions, UINT inputElementDescriptionCount, const void* shaderBytecodeWithInputSignature, UINT byteCodeLength) = 0;
		
		virtual void CreateTexture(ER_RHI_GPUTexture* aOutTexture, UINT width, UINT height, UINT samples, ER_RHI_FORMAT format, ER_RHI_BIND_FLAG bindFlags,
			int mip = 1, int depth = -1, int arraySize = 1, bool isCubemap = false, int cubemapArraySize = -1) = 0;
		virtual void CreateTexture(ER_RHI_GPUTexture* aOutTexture, const std::string& aPath, bool isFullPath = false) = 0;
		virtual void CreateTexture(ER_RHI_GPUTexture* aOutTexture, const std::wstring& aPath, bool isFullPath = false) = 0;

		virtual void CreateBuffer(ER_RHI_GPUBuffer* aOutBuffer, void* aData, UINT objectsCount, UINT byteStride, bool isDynamic = false, ER_RHI_BIND_FLAG bindFlags = ER_BIND_NONE, UINT cpuAccessFlags = 0, UINT miscFlags = 0, ER_RHI_FORMAT format = ER_FORMAT_UNKNOWN) = 0;
		virtual void CopyBuffer(ER_RHI_GPUBuffer* aDestBuffer, ER_RHI_GPUBuffer* aSrcBuffer) = 0;
		//TODO UpdateBuffer()

		virtual void Draw(UINT VertexCount) = 0;
		virtual void DrawIndexed(UINT IndexCount) = 0;
		virtual void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) = 0;
		virtual void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) = 0;
		//TODO DrawIndirect

		virtual void Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ) = 0;
		//TODO DispatchIndirect

		virtual void GenerateMips(ER_RHI_GPUTexture* aTexture) = 0; // not every API supports that!

		virtual ER_RHI_PRIMITIVE_TYPE GetCurrentTopologyType() = 0;

		virtual void PresentGraphics() = 0;
		virtual void PresentCompute() = 0;

		virtual void SetRenderTargets(const std::vector<ER_RHI_GPUTexture*>& aRenderTargets, ER_RHI_GPUTexture* aDepthTarget = nullptr, ER_RHI_GPUTexture* aUAV = nullptr) = 0;
		virtual void SetDepthTarget(ER_RHI_GPUTexture* aDepthTarget) = 0;

		virtual void SetDepthStencilState(ER_RHI_DEPTH_STENCIL_STATE aDS, UINT stencilRef = 0xffffffff) = 0;
		virtual ER_RHI_DEPTH_STENCIL_STATE GetCurrentDepthStencilState() = 0;

		virtual void SetBlendState(ER_RHI_BLEND_STATE aBS, const float BlendFactor[4], UINT SampleMask) = 0;
		virtual ER_RHI_BLEND_STATE GetCurrentBlendState() { return mCurrentBS; }

		virtual void SetRasterizerState(ER_RHI_RASTERIZER_STATE aRS) = 0;
		virtual ER_RHI_RASTERIZER_STATE GetCurrentRasterizerState() { return mCurrentRS; }

		virtual void SetViewport(const ER_RHI_Viewport& aViewport) = 0;
		virtual const ER_RHI_Viewport& GetCurrentViewport() { return mCurrentViewport; }

		virtual void SetRect(const ER_RHI_Rect& rect) = 0;

		virtual void SetShader(ER_RHI_GPUShader* aShader) = 0;
		virtual void SetShaderResources(ER_RHI_SHADER_TYPE aShaderType, const std::vector<ER_RHI_GPUResource*>& aSRVs, UINT startSlot = 0) = 0;
		virtual void SetUnorderedAccessResources(ER_RHI_SHADER_TYPE aShaderType, const std::vector<ER_RHI_GPUResource*>& aUAVs, UINT startSlot = 0) = 0;
		virtual void SetConstantBuffers(ER_RHI_SHADER_TYPE aShaderType, const std::vector<ER_RHI_GPUBuffer*>& aCBs, UINT startSlot = 0) = 0;
		virtual void SetSamplers(ER_RHI_SHADER_TYPE aShaderType, const std::vector<ER_RHI_SAMPLER_STATE>& aSamplers, UINT startSlot = 0) = 0;

		virtual void SetIndexBuffer(ER_RHI_GPUBuffer* aBuffer, UINT offset = 0) = 0;
		virtual void SetVertexBuffers(const std::vector<ER_RHI_GPUBuffer*>& aVertexBuffers) = 0;
		virtual void SetInputLayout(ER_RHI_InputLayout* aIL) = 0;
		virtual void SetEmptyInputLayout() = 0;
		virtual void SetTopologyType(ER_RHI_PRIMITIVE_TYPE aType) = 0;

		virtual void UnbindRenderTargets() = 0;
		virtual void UnbindResourcesFromShader(ER_RHI_SHADER_TYPE aShaderType, bool unbindShader = true) = 0;

		virtual void UpdateBuffer(ER_RHI_GPUBuffer* aBuffer, void* aData, int dataSize) = 0;

		virtual void InitImGui() = 0;
		virtual void StartNewImGuiFrame() = 0;
		virtual void RenderDrawDataImGui() = 0;
		virtual void ShutdownImGui() = 0;

		virtual void SetWindowHandle(void* handle) { (HWND)mWindowHandle; }

		ER_GRAPHICS_API GetAPI() { return mAPI; }
	protected:
		HWND mWindowHandle;

		ER_GRAPHICS_API mAPI;
		bool mIsFullScreen = false;

		ER_RHI_RASTERIZER_STATE mCurrentRS;
		ER_RHI_BLEND_STATE mCurrentBS;

		ER_RHI_Viewport mCurrentViewport;

		int mCurrentGraphicsCommandListIndex = 0;
		int mCurrentComputeCommandListIndex = 0;
	};

	class ER_RHI_GPUResource
	{
	public:
		ER_RHI_GPUResource();
		virtual ~ER_RHI_GPUResource();

		virtual void* GetSRV() = 0;
		virtual void* GetUAV() = 0;
	};

	class ER_RHI_GPUTexture : public ER_RHI_GPUResource
	{
	public:
		ER_RHI_GPUTexture();
		virtual ~ER_RHI_GPUTexture();

		virtual void CreateGPUTextureResource(ER_RHI* aRHI, UINT width, UINT height, UINT samples, ER_RHI_FORMAT format, ER_RHI_BIND_FLAG bindFlags = ER_BIND_NONE,
			int mip = 1, int depth = -1, int arraySize = 1, bool isCubemap = false, int cubemapArraySize = -1) = 0;
		virtual void CreateGPUTextureResource(ER_RHI* aRHI, const std::string& aPath, bool isFullPath = false) = 0;
		virtual void CreateGPUTextureResource(ER_RHI* aRHI, const std::wstring& aPath, bool isFullPath = false) = 0;

		virtual void* GetRTV(void* aEmpty = nullptr) = 0;
		virtual void* GetDSV() = 0;
	};

	class ER_RHI_GPUBuffer : public ER_RHI_GPUResource
	{
	public:
		ER_RHI_GPUBuffer();
		virtual ~ER_RHI_GPUBuffer();

		virtual void CreateGPUBufferResource(ER_RHI* aRHI, void* aData, UINT objectsCount, UINT byteStride, bool isDynamic = false, ER_RHI_BIND_FLAG bindFlags = ER_BIND_NONE, UINT cpuAccessFlags = 0, UINT miscFlags = 0, ER_RHI_FORMAT format = ER_FORMAT_UNKNOWN) = 0;
		virtual void* GetBuffer() = 0;
		virtual int GetSize() = 0;
		virtual UINT GetStride() = 0;
		virtual ER_RHI_FORMAT GetFormatRhi() = 0;
	};

	class ER_RHI_GPUShader
	{
	public:
		ER_RHI_GPUShader();
		virtual ~ER_RHI_GPUShader();

		virtual void CompileShader(ER_RHI* aRHI, const std::string& path, const std::string& shaderEntry, ER_RHI_SHADER_TYPE type, ER_RHI_InputLayout* aIL = nullptr) = 0;
		virtual void* GetShaderObject() = 0;

		ER_RHI_SHADER_TYPE mShaderType;
	};
}