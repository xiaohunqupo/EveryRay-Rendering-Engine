#include "ER_DirectionalLight.h"
#include "ER_VectorHelper.h"
#include "ER_Utility.h"
#include "ER_MatrixHelper.h"
#include "ER_Camera.h"
#include "ER_Core.h"

#include "Common.h"

namespace EveryRay_Core
{
	RTTI_DEFINITIONS(ER_DirectionalLight)

	ER_DirectionalLight::ER_DirectionalLight(ER_Core& game) : ER_Light(game),
		mDirection(ER_Vector3Helper::Forward),
		mUp(ER_Vector3Helper::Up), mRight(ER_Vector3Helper::Right), mProxyModel(nullptr)
	{
	}

	ER_DirectionalLight::ER_DirectionalLight(ER_Core& game, ER_Camera& camera) : ER_Light(game),
		mDirection(ER_Vector3Helper::Forward),
		mUp(ER_Vector3Helper::Up), mRight(ER_Vector3Helper::Right), mProxyModel(nullptr)
	{
		//directional gizmo model
		mProxyModel = new ER_DebugProxyObject(game, camera, ER_Utility::GetFilePath("content\\models\\proxy\\proxy_direction_arrow.obj"), 1.0f);
		mProxyModel->Initialize();
		mProxyModel->SetPosition(0.0f, 0.0f, 0.0f);
	}

	ER_DirectionalLight::~ER_DirectionalLight()
	{
		DeleteObject(mProxyModel);

		RotationUpdateEvent->RemoveAllListeners();
		DeleteObject(RotationUpdateEvent);
	}

	const XMFLOAT3& ER_DirectionalLight::Direction() const
	{
		return mDirection;
	}

	const XMFLOAT3& ER_DirectionalLight::Up() const
	{
		return mUp;
	}

	const XMFLOAT3& ER_DirectionalLight::Right() const
	{
		return mRight;
	}

	XMVECTOR ER_DirectionalLight::DirectionVector() const
	{
		return XMLoadFloat3(&mDirection);
	}

	XMVECTOR ER_DirectionalLight::UpVector() const
	{
		return XMLoadFloat3(&mUp);
	}

	XMVECTOR ER_DirectionalLight::RightVector() const
	{
		return XMLoadFloat3(&mRight);
	}

	void ER_DirectionalLight::ApplyRotation(CXMMATRIX transform)
	{
		mTransformMatrix = transform;

		XMVECTOR direction = XMLoadFloat3(&mDirection);
		XMVECTOR up = XMLoadFloat3(&mUp);

		direction = XMVector3TransformNormal(direction, transform);
		direction = XMVector3Normalize(direction);
		
		up = XMVector3TransformNormal(up, transform);
		up = XMVector3Normalize(up);

		XMVECTOR right = XMVector3Cross(direction, up);
		up = XMVector3Cross(right, direction);

		XMStoreFloat3(&mDirection, direction);
		XMStoreFloat3(&mUp, up);
		XMStoreFloat3(&mRight, right);

		if (mProxyModel)
			mProxyModel->ApplyRotation(transform);

		UpdateTransformArray(mTransformMatrix);
	}

	void ER_DirectionalLight::ApplyRotation(const XMFLOAT4X4& transform)
	{
		XMMATRIX transformMatrix = XMLoadFloat4x4(&transform);
		ApplyRotation(transformMatrix);
	}

	void ER_DirectionalLight::ApplyTransform(const float* transform)
	{
		XMFLOAT4X4 transformMatrixFloat = XMFLOAT4X4(transform);
		XMMATRIX transformMatrix = XMLoadFloat4x4(&transformMatrixFloat);
		
		XMVECTOR direction = XMVECTOR{ 0.0f, 0.0, -1.0f };
		XMVECTOR up = XMVECTOR{ 0.0f, 1.0, 0.0f };

		direction = XMVector3TransformNormal(direction, transformMatrix);
		direction = XMVector3Normalize(direction);

		up = XMVector3TransformNormal(up, transformMatrix);
		up = XMVector3Normalize(up);

		XMVECTOR right = XMVector3Cross(direction, up);
		up = XMVector3Cross(right, direction);

		XMStoreFloat3(&mDirection, direction);
		XMStoreFloat3(&mUp, up);
		XMStoreFloat3(&mRight, right);

		mTransformMatrix = transformMatrix;

		if (mProxyModel)
			mProxyModel->ApplyTransform(transformMatrix);

		for (auto listener : RotationUpdateEvent->GetListeners())
			listener();
	}

	void ER_DirectionalLight::DrawProxyModel(ER_RHI_GPUTexture* aRenderTarget, ER_RHI_GPUTexture* aDepth, const ER_CoreTime & time, ER_RHI_GPURootSignature* rs)
	{
		if (mProxyModel && ER_Utility::IsEditorMode && ER_Utility::IsSunLightEditor)
			mProxyModel->Draw(aRenderTarget, aDepth, time, rs);
	}

	void ER_DirectionalLight::UpdateProxyModel(const ER_CoreTime & time, const XMFLOAT4X4& viewMatrix, const XMFLOAT4X4& projectionMatrix)
	{
		ER_Camera* camera = (ER_Camera*)(GetCore()->GetServices().FindService(ER_Camera::TypeIdClass()));
		assert(camera);

		XMFLOAT3 gizmoPos = XMFLOAT3(
			(camera->Position().x + camera->Direction().x * mProxyModelGizmoTranslationDelta),
			(camera->Position().y + camera->Direction().y * mProxyModelGizmoTranslationDelta),
			(camera->Position().z + camera->Direction().z * mProxyModelGizmoTranslationDelta));

		if (mProxyModel)
		{
			mProxyModel->SetPosition(gizmoPos);
			mProxyModel->Update(time);

			ER_MatrixHelper::SetTranslation(mTransformMatrix, gizmoPos);
			ER_MatrixHelper::SetFloatArray(mTransformMatrix, mObjectTransformMatrix);
		}
	}
	void ER_DirectionalLight::UpdateTransformArray(CXMMATRIX transform)
	{
		XMFLOAT4X4 transformMatrix;
		XMStoreFloat4x4(&transformMatrix, transform);

		ER_MatrixHelper::SetFloatArray(transformMatrix, mObjectTransformMatrix);
	}

	const XMMATRIX& ER_DirectionalLight::LightMatrix(const XMFLOAT3& mPosition) const
	{
		XMVECTOR eyePosition = XMLoadFloat3(&mPosition);
		XMVECTOR direction = XMLoadFloat3(&mDirection);
		XMVECTOR upDirection = XMLoadFloat3(&mUp);

		XMMATRIX viewMatrix = XMMatrixLookToRH(eyePosition, direction, upDirection);
		return viewMatrix;
	}
}