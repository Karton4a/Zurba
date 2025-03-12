#include "Camera.h"

Camera::Camera(const DirectX::XMFLOAT3& position, float fovDeg, float aspectRatio, float nearZ, float farZ)
	: m_Fov(DirectX::XMConvertToRadians(fovDeg)), m_AspectRatio(aspectRatio), m_NearZ(nearZ), m_FarZ(farZ)
{
	m_ViewDirection = DirectX::XMVectorSet(0, 0, 1, 0);
	m_ViewMatrix = DirectX::XMMatrixIdentity();
	m_ViewProjectionMatrix = DirectX::XMMatrixIdentity();
	m_Position = DirectX::XMLoadFloat3(&position);
	m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(m_Fov, m_AspectRatio, m_NearZ, m_FarZ);
}

void Camera::SetPosition(const DirectX::XMVECTOR& position)
{
	m_Position = position;
	m_ViewDirty = true;
}

void Camera::Rotate(float yaw, float pitch, float roll)
{
	m_Yaw += DirectX::XMConvertToRadians(yaw);
	m_Pitch += DirectX::XMConvertToRadians(pitch);
	m_Roll += DirectX::XMConvertToRadians(roll);
	m_ViewDirty = true;
}

void Camera::FlushCalculations()
{
	if (m_ProjectionDirty)
	{
		m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(m_Fov, m_AspectRatio, m_NearZ, m_FarZ);
	}
	if (m_ViewDirty)
	{
		m_ViewDirection = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0,0,1,0), DirectX::XMQuaternionRotationRollPitchYaw(m_Pitch, m_Yaw, m_Roll));
		auto viewPosition = DirectX::XMVectorAdd(m_Position, m_ViewDirection);
		m_ViewMatrix = DirectX::XMMatrixLookAtLH(m_Position, viewPosition, DirectX::XMVectorSet(0, 1, 0, 0));
	}

	if (m_ViewDirty || m_ProjectionDirty)
	{
		m_ViewProjectionMatrix = DirectX::XMMatrixMultiply(m_ViewMatrix, m_ProjectionMatrix);
	}
	m_ViewDirty = false;
	m_ProjectionDirty = false;
}

