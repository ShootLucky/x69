#pragma once

#include "../../TF2/IMatSystemSurface.h"

class CInput
{
private:
	enum EKeyState { NONE, PRESSED, HELD };
	EKeyState m_Keys[256] = {};
	int m_nMouseX = 0, m_nMouseY = 0;
	bool m_bGameFocused = false;

public:
	inline bool IsPressed(short key) { return m_Keys[key] == PRESSED; }
	inline bool IsHeld(short key) { return m_Keys[key] == HELD; }
	inline bool IsDown(short key) { return IsPressed(key) || IsHeld(key); }
	inline int GetMouseX() { return m_nMouseX; }
	inline int GetMouseY() { return m_nMouseY; }
	inline bool IsGameFocused() { return m_bGameFocused; }

public:
	bool IsPressedAndHeld(short key);

public:
	void Update();

public:
	// Retorna o delta do scroll do mouse (positivo para cima, negativo para baixo, 0 se não houver)
	int GetMouseScroll() const
	{
		// Exemplo de implementação usando Windows API
		// Você pode adaptar conforme seu sistema de input real
		static int last_scroll = 0;
		int current_scroll = 0;

		// Supondo que você armazene o delta do scroll em algum lugar a cada frame,
		// por exemplo, via mensagem WM_MOUSEWHEEL ou equivalente.
		// Aqui está um exemplo genérico:
		// current_scroll = this->m_nMouseWheelDelta;

		// Se você não tem um campo para isso, pode usar GetAsyncKeyState para testes:
		if (GetAsyncKeyState(VK_MBUTTON) & 0x8000) // Apenas exemplo, não detecta scroll real
			current_scroll = 1;

		int delta = current_scroll - last_scroll;
		last_scroll = current_scroll;
		return delta;
	}
};

MAKE_SINGLETON_SCOPED(CInput, Input, H);