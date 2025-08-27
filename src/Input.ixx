module;
#include <basetsd.h>
#include <cstring>
#include <directxmath.h>
export module Input;

export class Input
{
public:
	enum class MouseKey
	{
		LeftKey,
		RightKey,
		MiddleKey,
		MouseKeyCount,
	};
	Input()
		:m_MousePosition(0, 0),
		m_PreviousMousePosition(0, 0)
	{
		memset(m_InputTable, false, 256);
		memset(m_MouseTable, false, (size_t)MouseKey::MouseKeyCount);
	}
	void Update() {};
	void RegisterKeyUp(UINT8 key)
	{
		m_InputTable[key] = false;
	}
	void RegisterKeyDown(UINT8 key)
	{
		m_InputTable[key] = true;
	}

	void RegisterMouseDown(MouseKey key)
	{
		assert(key < MouseKey::MouseKeyCount);
		m_MouseTable[(uint8_t)key] = true;
	}

	void RegisterMouseUp(MouseKey key)
	{
		assert(key < MouseKey::MouseKeyCount);
		m_MouseTable[(uint8_t)key] = false;
	}

	void SetMousePosition(uint32_t x, uint32_t y)
	{
		m_PreviousMousePosition = m_MousePosition;
		m_MousePosition = { x, y };
	}
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