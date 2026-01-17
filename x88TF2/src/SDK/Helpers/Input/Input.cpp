#include "Input.h"
bool CInput::IsPressedAndHeld(short key)
{
	auto Now = std::chrono::steady_clock::now();
	static std::chrono::time_point<std::chrono::steady_clock> KeyTimes[256] = { Now };
	if (m_Keys[key] == PRESSED) {
		KeyTimes[key] = Now;
		return true;
	}
	if (m_Keys[key] == HELD && std::chrono::duration_cast<std::chrono::milliseconds>(Now - KeyTimes[key]).count() > 400)
		return true;
	return false;
}
bool CInput::KeyReleased(short key)
{
	return !IsDown(key) && m_bPrevKeyState[key];
}
bool CInput::MouseInRegion(int x, int y, int w, int h)
{
	return m_nMouseX > x && m_nMouseY > y && m_nMouseX < w + x && m_nMouseY < h + y;
}
bool CInput::KeybindMethod(int iKey, int iMethod, bool* pToggled, bool old_method)
{
	switch (iMethod)
	{
	case 0: // Always on
		return true;
		break;
	case 1: // Hold
		return IsDown(iKey);
		break;
	case 2: // Toggle
	{
		if (old_method) {
			if (IsPressed(iKey))
				*pToggled = !*pToggled;
			return *pToggled;
		}
		else {
			return GetKeyState(iKey);
		}
	}
	break;
	case 3: // Force off
		return !IsDown(iKey);
		break;
	}
	return false;
}
void CInput::Update()
{
	m_bGameFocused = SDKUtils::IsGameWindowInFocus();
	for (int n = 0; n < 256; n++)
	{
		m_bPrevKeyState[n] = IsDown(n);
		if (!m_bGameFocused) {
			m_Keys[n] = NONE;
			continue;
		}
		bool bDown = GetAsyncKeyState(n) & 0x8000;
		if (bDown)
		{
			if (m_Keys[n] == PRESSED)
				m_Keys[n] = HELD;
			else if (m_Keys[n] != HELD)
				m_Keys[n] = PRESSED;
		}
		else m_Keys[n] = NONE;
	}
	I::MatSystemSurface->SurfaceGetCursorPos(m_nMouseX, m_nMouseY);
}