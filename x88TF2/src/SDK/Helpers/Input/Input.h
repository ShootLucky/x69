#pragma once
#include "../../SDK.h"

class CInput
{
private:
	bool m_bKeyState[256] = {};
	bool m_bPrevKeyState[256] = {};
	int m_nMouseX = 0, m_nMouseY = 0;
	bool m_bGameFocused = false;
	bool m_bInputLoopStarted = false;

public:
	void Update();

	bool KeybindMethod(int iKey, int iMethod, bool* pToggled, bool old_method = false);
	bool KeyPressed(int key);
	bool KeyDown(int key);
	bool KeyReleased(int key);

	bool MouseInRegion(int x, int y, int w, int h);

	inline int GetMouseX() { return m_nMouseX; }
	inline int GetMouseY() { return m_nMouseY; }
	inline bool IsGameFocused() { return m_bGameFocused; }
};

MAKE_SINGLETON_SCOPED(CInput, Input, H);