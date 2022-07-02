#pragma once
#include "Common.h"

namespace Library
{
	class ER_Core;
	class ER_CoreTime;
	class ER_BasicColorMaterial;
	class ER_Camera;

	class ER_DebugProxyObject
	{
	public:
		ER_DebugProxyObject(ER_Core& game, ER_Camera& camera, const std::string& modelFileName, float scale = 1.0f);
		~ER_DebugProxyObject();

		const XMFLOAT3& Position() const;
		const XMFLOAT3& Direction() const;
		const XMFLOAT3& Up() const;
		const XMFLOAT3& Right() const;

		XMVECTOR PositionVector() const;
		XMVECTOR DirectionVector() const;
		XMVECTOR UpVector() const;
		XMVECTOR RightVector() const;

		bool& DisplayWireframe();

		void SetPosition(FLOAT x, FLOAT y, FLOAT z);
		void SetPosition(FXMVECTOR position);
		void SetPosition(const XMFLOAT3& position);

		void ApplyRotation(CXMMATRIX transform);
		void ApplyTransform(XMMATRIX transformMatrix);
		void ApplyRotation(const XMFLOAT4X4& transform);
		void ApplyRotaitonAroundPoint(float radius, float angle);

		void Initialize();
		void Update(const ER_CoreTime& gameTime);
		void Draw(const ER_CoreTime& gameTime);

	private:
		ER_DebugProxyObject();
		ER_DebugProxyObject(const ER_DebugProxyObject& rhs);
		ER_DebugProxyObject& operator=(const ER_DebugProxyObject& rhs);

		ER_Core& mCore;

		std::string mModelFileName;
		ER_BasicColorMaterial* mMaterial;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;

		XMFLOAT4X4 mWorldMatrix;
		XMFLOAT4X4 mScaleMatrix;

		bool mDisplayWireframe;
		XMFLOAT3 mPosition;
		XMFLOAT3 mDirection;
		XMFLOAT3 mUp;
		XMFLOAT3 mRight;
	};
}