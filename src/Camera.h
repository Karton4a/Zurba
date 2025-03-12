#pragma once
#include <directxmath.h>
class Camera
{
public:
	Camera(const DirectX::XMFLOAT3& position, float fovDeg, float aspectRatio, float nearZ, float farZ);
	void SetPosition(const DirectX::XMVECTOR& position);
	void Rotate(float yaw, float pitch, float roll);
	void FlushCalculations();
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