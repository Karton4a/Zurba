module;
#include <cstring>
#include <cassert>
#include <cstdint>
module Input;

Input::Input()
	:m_MousePosition(0, 0),
	m_PreviousMousePosition(0, 0)
{
	memset(m_InputTable, false, 256);
	memset(m_MouseTable, false, (size_t)MouseKey::MouseKeyCount);
}
void Input::Update() {};
void Input::RegisterKeyUp(UINT8 key)
{
	m_InputTable[key] = false;
}
void Input::RegisterKeyDown(UINT8 key)
{
	m_InputTable[key] = true;
}

void Input::RegisterMouseDown(MouseKey key)
{
	assert(key < MouseKey::MouseKeyCount);
	m_MouseTable[(uint8_t)key] = true;
}

void Input::RegisterMouseUp(MouseKey key)
{
	assert(key < MouseKey::MouseKeyCount);
	m_MouseTable[(uint8_t)key] = false;
}

void Input::SetMousePosition(uint32_t x, uint32_t y)
{
	m_PreviousMousePosition = m_MousePosition;
	m_MousePosition = { x, y };
}