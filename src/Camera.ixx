module;

#include <directxmath.h>

export module Camera;

export class Camera
{
public:
	Camera(const DirectX::XMFLOAT3& position, float fovDeg, float aspectRatio, float nearZ, float farZ)
		: m_Fov(DirectX::XMConvertToRadians(fovDeg)), m_AspectRatio(aspectRatio), m_NearZ(nearZ), m_FarZ(farZ)
	{
		m_ViewDirection = DirectX::XMVectorSet(0, 0, 1, 0);
		m_ViewMatrix = DirectX::XMMatrixIdentity();
		m_ViewProjectionMatrix = DirectX::XMMatrixIdentity();
		m_Position = DirectX::XMLoadFloat3(&position);
		m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(m_Fov, m_AspectRatio, m_NearZ, m_FarZ);
	}
	void SetPosition(const DirectX::XMVECTOR& position)
	{
		m_Position = position;
		m_ViewDirty = true;
	}
	void Rotate(float yaw, float pitch, float roll)
	{
		m_Yaw += DirectX::XMConvertToRadians(yaw);
		m_Pitch += DirectX::XMConvertToRadians(pitch);
		m_Roll += DirectX::XMConvertToRadians(roll);
		m_ViewDirty = true;
	}
	void FlushCalculations()
	{
		if (m_ProjectionDirty)
		{
			m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(m_Fov, m_AspectRatio, m_NearZ, m_FarZ);
		}
		if (m_ViewDirty)
		{
			m_ViewDirection = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0), DirectX::XMQuaternionRotationRollPitchYaw(m_Pitch, m_Yaw, m_Roll));
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
	const DirectX::XMMATRIX& GetViewMatrix() const { return m_ViewMatrix; };
	const DirectX::XMMATRIX& GetViewProjectionMatrix() { return m_ViewProjectionMatrix; };

	const DirectX::XMVECTOR& GetPosition() const { return m_Position; };
	const DirectX::XMVECTOR& GetViewDirection() const { return m_ViewDirection; };

private:
	DirectX::XMVECTOR m_Position;
	DirectX::XMVECTOR m_ViewDirection;
	DirectX::XMMATRIX m_ViewMatrix;
	DirectX::XMMATRIX m_ProjectionMatrix;
	DirectX::XMMATRIX m_ViewProjectionMatrix;

	float m_Yaw = 0;
	float m_Pitch = 0;
	float m_Roll = 0;
	bool m_ProjectionDirty = false;
	bool m_ViewDirty = false;
	float m_Fov = 0.0f;
	float m_AspectRatio = 0.0f;
	float m_NearZ = 0.0f;
	float m_FarZ = 0.0f;
};