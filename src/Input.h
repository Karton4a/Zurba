#pragma once
#include <basetsd.h>
#include <directxmath.h>

class Input
{
public:
	enum class MouseKey
	{
		LeftKey,
		RightKey,
		MiddleKey,
		MouseKeyCount,
	};
	Input();
	void Update();
	void RegisterKeyUp(UINT8 key);
	void RegisterKeyDown(UINT8 key);
	void RegisterMouseDown(MouseKey key);
	void RegisterMouseUp(MouseKey key);
	void SetMousePosition(uint32_t x, uint32_t y);
	inline bool IsKeyPressed(UINT8 key) const { return m_InputTable[key]; };
	inline bool IsMouseKeyPressed(MouseKey key) const { return m_MouseTable[(uint8_t)key]; };
	inline DirectX::XMUINT2 GetMousePosition() const { return m_MousePosition; };
	inline DirectX::XMINT2 GetMouseDelta() const { return { (int32_t)m_MousePosition.x - (int32_t)m_PreviousMousePosition.x, (int32_t)m_MousePosition.y - (int32_t)m_PreviousMousePosition.y }; };
private:
	bool m_InputTable[256];
	bool m_MouseTable[3];
	DirectX::XMUINT2 m_MousePosition;
	DirectX::XMUINT2 m_PreviousMousePosition;
};