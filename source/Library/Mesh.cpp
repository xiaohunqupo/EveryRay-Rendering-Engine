#include "stdafx.h"

#include "Mesh.h"
#include "Game.h"
#include "GameException.h"
#include "VertexDeclarations.h"

#include "assimp\scene.h"

namespace Library
{

	Mesh::Mesh(Model& model, aiMesh& mesh) : mModel(model), mMaterial(nullptr), mName(mesh.mName.C_Str()), mVertices(), mNormals(), mTangents(), mBiNormals(), mTextureCoordinates(), mVertexColors(), mFaceCount(0), mIndices()
	{
		mMaterial = mModel.Materials().at(mesh.mMaterialIndex);

		// Vertices
		mVertices.reserve(mesh.mNumVertices);
		for (UINT i = 0; i < mesh.mNumVertices; i++)
		{
			mVertices.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mVertices[i])));
		}

		// Normals
		if (mesh.HasNormals())
		{
			mNormals.reserve(mesh.mNumVertices);
			for (UINT i = 0; i < mesh.mNumVertices; i++)
			{
				mNormals.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mNormals[i])));
			}
		}

		// Tangents and Binormals
		if (mesh.HasTangentsAndBitangents())
		{
			mTangents.reserve(mesh.mNumVertices);
			mBiNormals.reserve(mesh.mNumVertices);
			for (UINT i = 0; i < mesh.mNumVertices; i++)
			{
				mTangents.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mTangents[i])));
				mBiNormals.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mBitangents[i])));
			}
		}

		// Texture Coordinates
		UINT uvChannelCount = mesh.GetNumUVChannels();
		for (UINT i = 0; i < uvChannelCount; i++)
		{
			std::vector<XMFLOAT3>* textureCoordinates = new std::vector<XMFLOAT3>();
			textureCoordinates->reserve(mesh.mNumVertices);
			mTextureCoordinates.push_back(textureCoordinates);

			aiVector3D* aiTextureCoordinates = mesh.mTextureCoords[i];
			for (UINT j = 0; j < mesh.mNumVertices; j++)
			{
				textureCoordinates->push_back(XMFLOAT3(reinterpret_cast<const float*>(&aiTextureCoordinates[j])));
			}
		}

		// Vertex Colors
		UINT colorChannelCount = mesh.GetNumColorChannels();
		for (UINT i = 0; i < colorChannelCount; i++)
		{
			std::vector<XMFLOAT4>* vertexColors = new std::vector<XMFLOAT4>();
			vertexColors->reserve(mesh.mNumVertices);
			mVertexColors.push_back(vertexColors);

			aiColor4D* aiVertexColors = mesh.mColors[i];
			for (UINT j = 0; j < mesh.mNumVertices; j++)
			{
				vertexColors->push_back(XMFLOAT4(reinterpret_cast<const float*>(&aiVertexColors[j])));
			}
		}

		// Faces (note: could pre-reserve if we limit primitive types)
		if (mesh.HasFaces())
		{
			mFaceCount = mesh.mNumFaces;
			for (UINT i = 0; i < mFaceCount; i++)
			{
				aiFace* face = &mesh.mFaces[i];

				for (UINT j = 0; j < face->mNumIndices; j++)
				{
					mIndices.push_back(face->mIndices[j]);
				}
			}
		}
	}

	/*Mesh::Mesh(Model & model, ModelMaterial * material)
	{
	}*/

	Mesh::~Mesh()
	{
		for (std::vector<XMFLOAT3>* textureCoordinates : mTextureCoordinates)
		{
			delete textureCoordinates;
		}

		for (std::vector<XMFLOAT4>* vertexColors : mVertexColors)
		{
			delete vertexColors;
		}
	}

	Model& Mesh::GetModel()
	{
		return mModel;
	}

	ModelMaterial* Mesh::GetMaterial()
	{
		return mMaterial;
	}

	const std::string& Mesh::Name() const
	{
		return mName;
	}

	const std::vector<XMFLOAT3>& Mesh::Vertices() const
	{
		return mVertices;
	}

	const std::vector<XMFLOAT3>& Mesh::Normals() const
	{
		return mNormals;
	}

	const std::vector<XMFLOAT3>& Mesh::Tangents() const
	{
		return mTangents;
	}

	const std::vector<XMFLOAT3>& Mesh::BiNormals() const
	{
		return mBiNormals;
	}

	const std::vector<std::vector<XMFLOAT3>*>& Mesh::TextureCoordinates() const
	{
		return mTextureCoordinates;
	}

	const std::vector<std::vector<XMFLOAT4>*>& Mesh::VertexColors() const
	{
		return mVertexColors;
	}

	UINT Mesh::FaceCount() const
	{
		return mFaceCount;
	}

	const std::vector<UINT>& Mesh::Indices() const
	{
		return mIndices;
	}

	void Mesh::CreateIndexBuffer(ID3D11Buffer** indexBuffer)
	{
		assert(indexBuffer != nullptr);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
		indexBufferDesc.ByteWidth = sizeof(UINT) * mIndices.size();
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubResourceData;
		ZeroMemory(&indexSubResourceData, sizeof(indexSubResourceData));
		indexSubResourceData.pSysMem = &mIndices[0];
		if (FAILED(mModel.GetGame().Direct3DDevice()->CreateBuffer(&indexBufferDesc, &indexSubResourceData, indexBuffer)))
		{
			throw GameException("ID3D11Device::CreateBuffer() failed during the creation of the index buffer in Mesh.");
		}
	}

	void Mesh::CreateVertexBuffer_PositionUvNormalTangent(ID3D11Buffer** vertexBuffer)
	{
		const std::vector<XMFLOAT3>& sourceVertices = Vertices();
		std::vector<XMFLOAT3>* textureCoordinates = TextureCoordinates().at(0);
		assert(textureCoordinates->size() == sourceVertices.size());

		const std::vector<XMFLOAT3>& normals = Normals();
		assert(normals.size() == sourceVertices.size());

		const std::vector<XMFLOAT3>& tangents = Tangents();
		assert(tangents.size() == sourceVertices.size());

		std::vector<VertexPositionTextureNormalTangent> vertices;
		vertices.reserve(sourceVertices.size());

		for (UINT i = 0; i < sourceVertices.size(); i++)
		{
			XMFLOAT3 position = sourceVertices.at(i);
			XMFLOAT3 uv = textureCoordinates->at(i);
			XMFLOAT3 normal = normals.at(i);
			XMFLOAT3 tangent = tangents.at(i);

			vertices.push_back(VertexPositionTextureNormalTangent(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y), normal, tangent));
		}

		ID3D11Device* device = mModel.GetGame().Direct3DDevice();

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = sizeof(VertexPositionTextureNormalTangent) * vertices.size();
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = &vertices[0];
		if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer)))
			throw GameException("ID3D11Device::CreateBuffer() failed during vertex buffer creation for Forward Lighting.");

	}

}