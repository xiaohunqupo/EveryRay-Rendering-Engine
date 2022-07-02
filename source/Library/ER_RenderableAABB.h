#pragma once
#include "Common.h"

namespace Library
{
	class ER_Core;
	class ER_BasicColorMaterial;
	class ER_GPUBuffer;

	class ER_RenderableAABB
	{
	public:
		ER_RenderableAABB(ER_Core& game, const XMFLOAT4& color);
		~ER_RenderableAABB();

		void InitializeGeometry(const std::vector<XMFLOAT3>& aabb);
		void Update(ER_AABB& aabb);
		void Draw();
		void SetColor(const XMFLOAT4& color);
		void SetAABB(const std::vector<XMFLOAT3>& aabb);

	private:
		void UpdateVertices();

		ER_Core& mCore;

		ER_GPUBuffer* mVertexBuffer;
		ER_GPUBuffer* mIndexBuffer;
		ER_BasicColorMaterial* mMaterial;
		
		XMFLOAT4 mColor;

		XMFLOAT4 mVertices[8];
		ER_AABB mAABB;
	};
}