#include "stdafx.h"

#include "TerrainMaterial.h"
#include "GameException.h"
#include "Mesh.h"
#include "ColorHelper.h"

namespace Library
{
	RTTI_DEFINITIONS(TerrainMaterial)

	TerrainMaterial::TerrainMaterial() : Material("main"),
		MATERIAL_VARIABLE_INITIALIZATION(World),
		MATERIAL_VARIABLE_INITIALIZATION(View),
		MATERIAL_VARIABLE_INITIALIZATION(Projection)
	{
	}

	MATERIAL_VARIABLE_DEFINITION(TerrainMaterial, World)
	MATERIAL_VARIABLE_DEFINITION(TerrainMaterial, View)
	MATERIAL_VARIABLE_DEFINITION(TerrainMaterial, Projection)

	void TerrainMaterial::Initialize(Effect* effect)
	{
		Material::Initialize(effect);

		MATERIAL_VARIABLE_RETRIEVE(World)
		MATERIAL_VARIABLE_RETRIEVE(View)
		MATERIAL_VARIABLE_RETRIEVE(Projection)

		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		CreateInputLayout("main", "p0", inputElementDescriptions, ARRAYSIZE(inputElementDescriptions));
	}

	void TerrainMaterial::CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const
	{
		const std::vector<XMFLOAT3>& sourceVertices = mesh.Vertices();
		//std::vector<XMFLOAT3>* textureCoordinates = mesh.TextureCoordinates().at(0);
		const std::vector<XMFLOAT3>& normals = mesh.Normals();

		std::vector<VertexPositionNormal> vertices;
		vertices.reserve(sourceVertices.size());
		for (UINT i = 0; i < sourceVertices.size(); i++)
		{
			XMFLOAT3 position = sourceVertices.at(i);
			//XMFLOAT3 uv = textureCoordinates->at(i);
			XMFLOAT3 normal = normals.at(i);
			vertices.push_back(VertexPositionNormal(XMFLOAT4(position.x, position.y, position.z, 1.0f), normal));
		}

		CreateVertexBuffer(device, &vertices[0], vertices.size(), vertexBuffer);
	}

	void TerrainMaterial::CreateVertexBuffer(ID3D11Device* device, VertexPositionNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = VertexSize() * vertexCount;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = vertices;
		if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer)))
		{
			throw GameException("ID3D11Device::CreateBuffer() failed.");
		}
	}

	UINT TerrainMaterial::VertexSize() const
	{
		return sizeof(VertexPositionNormal);
	}
}