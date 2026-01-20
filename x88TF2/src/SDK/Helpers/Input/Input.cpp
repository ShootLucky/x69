#include "Input.h"

void CInput::Update()
{
	m_bGameFocused = SDKUtils::IsGameWindowInFocus();

	for (int i = 0; i < 256; i++)
	{
		m_bPrevKeyState[i] = m_bKeyState[i];

		if (!m_bGameFocused)
		{
			m_bKeyState[i] = false;
			continue;
		}

		m_bKeyState[i] = GetAsyncKeyState(i);
	}

	I::MatSystemSurface->SurfaceGetCursorPos(m_nMouseX, m_nMouseY);

	if (!m_bInputLoopStarted)
		m_bInputLoopStarted = true;
}

bool CInput::KeybindMethod(int iKey, int iMethod, bool* pToggled, bool old_method)
{
	switch (iMethod)
	{
	case 0: // Always on
		return true;
		break;
	case 1: // Hold
		return GetAsyncKeyState(iKey);
		break;
	case 2: // Toggle
	{
		if (old_method)
		{
			if (KeyPressed(iKey))
				*pToggled = !*pToggled;

			return *pToggled;
		}
		else
		{
			return GetKeyState(iKey);
		}
	}
	break;
	case 3: // Force off
		return !KeyDown(iKey);
		break;
	}
	return false;
}

bool CInput::KeyPressed(int key)
{
	return m_bKeyState[key] && !m_bPrevKeyState[key];
}

bool CInput::KeyDown(int key)
{
	return m_bKeyState[key];
}

bool CInput::KeyReleased(int key)
{
	return !m_bKeyState[key] && m_bPrevKeyState[key];
}

bool CInput::MouseInRegion(int x, int y, int w, int h)
{
	return m_nMouseX > x && m_nMouseY > y && m_nMouseX < w + x && m_nMouseY < h + y;
}