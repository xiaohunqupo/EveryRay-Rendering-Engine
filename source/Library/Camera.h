#pragma once

#include "GameComponent.h"
#include "Frustum.h"

namespace Library
{
	class ER_RenderingObject;
	class GameTime;

	class Camera : public GameComponent
	{
		RTTI_DECLARATIONS(Camera, GameComponent)

	public:
		Camera(Game& game);
		Camera(Game& game, float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance);

		virtual ~Camera();

		const XMFLOAT3& Position() const;
		const XMFLOAT3& Direction() const;
		const XMFLOAT3& Up() const;
		const XMFLOAT3& Right() const;

		XMVECTOR PositionVector() const;
		XMVECTOR DirectionVector() const;
		XMVECTOR UpVector() const;
		XMVECTOR RightVector() const;

		float AspectRatio() const;
		float FieldOfView() const;
		float NearPlaneDistance() const;
		float FarPlaneDistance() const;
		Frustum GetFrustum() const { return mFrustum; }

		XMMATRIX ViewMatrix() const;
		XMFLOAT4X4 ViewMatrix4X4() const;
		XMMATRIX ProjectionMatrix() const;
		XMFLOAT4X4 ProjectionMatrix4X4() const;
		XMMATRIX ViewProjectionMatrix() const;
		XMMATRIX RotationTransformMatrix() const;

		float GetCameraFarShadowCascadeDistance (int index) const;
		float GetCameraNearShadowCascadeDistance (int index) const;

		virtual void SetPosition(FLOAT x, FLOAT y, FLOAT z);
		virtual void SetPosition(FXMVECTOR position);
		virtual void SetPosition(const XMFLOAT3& position);
		virtual void SetDirection(const XMFLOAT3& direction);
		virtual void SetUp(const XMFLOAT3& up);
		virtual void SetFOV(float fov);
		virtual void SetNearPlaneDistance(float value);
		virtual void SetFarPlaneDistance(float value);
		virtual XMMATRIX GetCustomViewProjectionMatrixForCascade(int cascadeIndex);


		virtual void Reset();
		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void UpdateViewMatrix(bool leftHanded = false);
		virtual void UpdateProjectionMatrix(bool leftHanded = false);
		void ApplyRotation(CXMMATRIX transform);
		void ApplyRotation(const XMFLOAT4X4& transform);

		static const float DefaultFieldOfView;
		static const float DefaultAspectRatio;
		static const float DefaultNearPlaneDistance;
		static const float DefaultFarPlaneDistance;

	protected:
		float mFieldOfView;
		float mAspectRatio;
		float mNearPlaneDistance;
		float mFarPlaneDistance;

		XMFLOAT3 mPosition;
		XMFLOAT3 mDirection;
		XMFLOAT3 mUp;
		XMFLOAT3 mRight;

		XMMATRIX mRotationMatrix;
		XMFLOAT4X4 mViewMatrix;
		XMFLOAT4X4 mProjectionMatrix;

	private:
		Camera(const Camera& rhs);
		Camera& operator=(const Camera& rhs);

		Frustum mFrustum;
	};
}