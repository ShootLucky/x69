#pragma once
#include "../../SDK.h"
class CInput
{
private:
	enum EKeyState { NONE, PRESSED, HELD };
	EKeyState m_Keys[256] = {};
	bool m_bPrevKeyState[256] = {};
	int m_nMouseX = 0, m_nMouseY = 0;
	bool m_bGameFocused = false;
public:
	inline bool IsPressed(short key) { return m_Keys[key] == PRESSED; }
	inline bool IsHeld(short key) { return m_Keys[key] == HELD; }
	inline bool IsDown(short key) { return IsPressed(key) || IsHeld(key); }
	inline int GetMouseX() { return m_nMouseX; }
	inline int GetMouseY() { return m_nMouseY; }
	inline bool IsGameFocused() { return m_bGameFocused; }
	bool IsPressedAndHeld(short key);
	bool KeyReleased(short key);
	bool MouseInRegion(int x, int y, int w, int h);
	bool KeybindMethod(int iKey, int iMethod, bool* pToggled, bool old_method = false);
	void Update();
};
MAKE_SINGLETON_SCOPED(CInput, Input, H);