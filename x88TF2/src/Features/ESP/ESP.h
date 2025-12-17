#pragma once
#include "../src/SDK/SDK.h"
#include "../src/SDK/Helpers/Draw/Draw.h"
#include "../src/SDK/Helpers/Entities/Entities.h"
#include "../src/CFG.h"
#include "../LagRecords/Backtrack.h" // Adjusted include path if necessary
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
	void CustomFOV(CViewSetup* pSetup);
private:
	// Helpers de desenho — implementados em ESP.cpp
	void DrawBox(int left, int top, int w, int h, const Color_t& clr);
	void DrawBox2D(int left, int top, int w, int h, const Color_t& clr);
	void DrawBox3D(Vec3 scr[8], const Color_t& clr, bool useAA = true);
	void DrawBoxCorner(int left, int top, int w, int h, const Color_t& clr);
	void DrawName(int x, int y, const std::string& name, const Color_t& clr);
	void DrawHealthBar(int left, int top, int h, int health, int maxHealth);
	void DrawBoneLine(const Vec3& a, const Vec3& b, const Color_t& clr);
	void DrawThinLine(const Vec3& a, const Vec3& b, const Color_t& clr);
	void DrawOutlinedLine(const Vec3& a, const Vec3& b, const Color_t& clr, const Color_t& outlineClr = Color_t(0, 0, 0, 255));
	void DrawSmoothBoneLine(const Vec3& a, const Vec3& b, const Color_t& clr, bool useAA = true);
	void DrawScreenLine(const Vec3& a, const Vec3& b, const Color_t& clr);
	void DrawImpactBox(const Vec3& worldPos, const Color_t& clr);
	void DrawProjectedHitboxWire(const Vec3 proj[8], const Color_t& clr, bool useAA = true);
	void DrawOffscreenArrow(const Vec3& origin, const Color_t& clr);
	void DrawFOVCircle(float fov, const Color_t& color);
	bool m_bInitialized = false;
};
extern CESP gESP;