#pragma once
#include "Common.h"
#include "Material.h"
#include "VertexDeclarations.h"

namespace Library
{
	class TerrainMaterial : public Material
	{
		RTTI_DECLARATIONS(TerrainMaterial, Material)

		MATERIAL_VARIABLE_DECLARATION(World)
		MATERIAL_VARIABLE_DECLARATION(View)
		MATERIAL_VARIABLE_DECLARATION(Projection)
		MATERIAL_VARIABLE_DECLARATION(albedoTexture)

			MATERIAL_VARIABLE_DECLARATION(SunDirection)
			MATERIAL_VARIABLE_DECLARATION(SunColor)
			MATERIAL_VARIABLE_DECLARATION(AmbientColor)
			MATERIAL_VARIABLE_DECLARATION(ShadowTexelSize)
			MATERIAL_VARIABLE_DECLARATION(ShadowCascadeDistances)

	public:
		TerrainMaterial();

		virtual void Initialize(Effect* effect) override;
		virtual void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const override;
		void CreateVertexBuffer(ID3D11Device* device, VertexPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
		virtual UINT VertexSize() const override;
	};
}