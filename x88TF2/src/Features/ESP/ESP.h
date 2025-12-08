#pragma once

#include "../src/SDK/SDK.h"
#include "../src/SDK/Helpers/Draw/Draw.h"
#include "../src/SDK/Helpers/Entities/Entities.h"
#include "../src/CFG.h"

class CESP {
public:
	// Executa a lógica do ESP e desenha na tela (chamar por frame).
	void Run();

	// Inicializa recursos necessários (opcional).
	void Init();

	// Libera recursos (opcional).
	void Shutdown();

	// Estado (consulta rápida).
	bool IsEnabled() const noexcept { return CFG::ESP_Enable; }

private:
	// Helpers de desenho — implementados em ESP.cpp
	void DrawBox(int left, int top, int w, int h, const Color_t& clr);
	void DrawName(int x, int y, const std::string& name, const Color_t& clr);
	void DrawHealthBar(int left, int top, int h, int health, int maxHealth);

	bool m_bInitialized = false;
};

extern CESP gESP;