#pragma once

#include "DrawableGameComponent.h"

namespace Library
{
	class ER_BasicColorMaterial;
	class Pass;

	class RenderableAABB : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(RenderableAABB, DrawableGameComponent)

	public:
		RenderableAABB(Game& game, Camera& camera);
		RenderableAABB(Game& game, Camera& camera, const XMFLOAT4& color);
		~RenderableAABB();


		void InitializeGeometry(const std::vector<XMFLOAT3>& aabb, XMMATRIX matrix);
		void SetPosition(const XMFLOAT3& position);
		void SetScale(const XMFLOAT3& scale) { mScale = scale; };
		void SetColor(const XMFLOAT4& color);
		void UpdateColor(const XMFLOAT4& color);
		void SetRotationMatrix(const XMMATRIX& rotationMat);
		void SetAABB(const std::vector<XMFLOAT3>& aabb);


		virtual void Initialize() override;
		void Update();
		void Draw();

		bool isColliding = false;

	private:
		RenderableAABB();
		RenderableAABB(const RenderableAABB& rhs);
		RenderableAABB& operator=(const RenderableAABB& rhs);

		void InitializeVertexBuffer(const std::vector<XMFLOAT3>& aabb);
		void InitializeIndexBuffer();
		void ResizeAABB();

		static const XMVECTORF32 DefaultColor;
		static const UINT AABBVertexCount;
		static const UINT AABBPrimitiveCount;
		static const UINT AABBIndicesPerPrimitive;
		static const UINT AABBIndexCount;
		static const USHORT AABBIndices[];

		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		ER_BasicColorMaterial* mMaterial;

		XMFLOAT4 mColor;
		XMFLOAT3 mPosition;
		XMFLOAT3 mScale;
		XMFLOAT3 mDirection;
		XMFLOAT3 mUp;
		XMFLOAT3 mRight;

		XMFLOAT4X4 mWorldMatrix;

		XMMATRIX mTransformMatrix;

		std::vector<XMFLOAT3> mVertices;

		ER_AABB mAABB;
	};
}