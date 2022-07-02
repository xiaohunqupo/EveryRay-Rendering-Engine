#include "ER_BasicColorMaterial.h"
#include "ShaderCompiler.h"
#include "ER_Utility.h"
#include "ER_CoreException.h"
#include "ER_Core.h"
#include "ER_Camera.h"
#include "ER_RenderingObject.h"
#include "ER_Mesh.h"
#include "ER_MaterialsCallbacks.h"
namespace Library
{
	ER_BasicColorMaterial::ER_BasicColorMaterial(ER_Core& game, const MaterialShaderEntries& entries, unsigned int shaderFlags, bool instanced)
		: ER_Material(game, entries, shaderFlags)
	{
		//TODO instanced support
		if (shaderFlags & HAS_VERTEX_SHADER)
		{
			D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
			ER_Material::CreateVertexShader("content\\shaders\\BasicColor.hlsl", inputElementDescriptions, ARRAYSIZE(inputElementDescriptions));
		}
		
		if (shaderFlags & HAS_PIXEL_SHADER)
			ER_Material::CreatePixelShader("content\\shaders\\BasicColor.hlsl");

		mConstantBuffer.Initialize(ER_Material::GetCore()->Direct3DDevice());
	}

	ER_BasicColorMaterial::~ER_BasicColorMaterial()
	{
		mConstantBuffer.Release();
		ER_Material::~ER_Material();
	}

	void ER_BasicColorMaterial::PrepareForRendering(ER_MaterialSystems neededSystems, ER_RenderingObject* aObj, int meshIndex)
	{
		auto context = ER_Material::GetCore()->Direct3DDeviceContext();
		ER_Camera* camera = (ER_Camera*)(ER_Material::GetCore()->Services().GetService(ER_Camera::TypeIdClass()));
		
		assert(aObj);
		assert(camera);

		ER_Material::PrepareForRendering(neededSystems, aObj, meshIndex);

		mConstantBuffer.Data.World = XMMatrixTranspose(aObj->GetTransformationMatrix());
		mConstantBuffer.Data.ViewProjection = XMMatrixTranspose(camera->ViewMatrix() * camera->ProjectionMatrix());
		mConstantBuffer.Data.Color = XMFLOAT4{0.0, 1.0, 0.0, 0.0};
		mConstantBuffer.ApplyChanges(context);
		ID3D11Buffer* CBs[1] = { mConstantBuffer.Buffer() };

		context->VSSetConstantBuffers(0, 1, CBs);
		context->PSSetConstantBuffers(0, 1, CBs);
	}

	// non-callback method for non-"RenderingObject" draws
	void ER_BasicColorMaterial::PrepareForRendering(const XMMATRIX& worldTransform, const XMFLOAT4& color)
	{
		auto context = ER_Material::GetCore()->Direct3DDeviceContext();
		ER_Camera* camera = (ER_Camera*)(ER_Material::GetCore()->Services().GetService(ER_Camera::TypeIdClass()));

		assert(camera);

		ER_Material::PrepareForRendering({}, nullptr, 0);

		mConstantBuffer.Data.World = XMMatrixTranspose(worldTransform);
		mConstantBuffer.Data.ViewProjection = XMMatrixTranspose(camera->ViewMatrix() * camera->ProjectionMatrix());
		mConstantBuffer.Data.Color = color;
		mConstantBuffer.ApplyChanges(context);
		ID3D11Buffer* CBs[1] = { mConstantBuffer.Buffer() };

		context->VSSetConstantBuffers(0, 1, CBs);
		context->PSSetConstantBuffers(0, 1, CBs);
	}

	void ER_BasicColorMaterial::CreateVertexBuffer(const ER_Mesh& mesh, ID3D11Buffer** vertexBuffer)
	{
		mesh.CreateVertexBuffer_Position(vertexBuffer);
	}

	int ER_BasicColorMaterial::VertexSize()
	{
		return sizeof(VertexPosition);
	}

}