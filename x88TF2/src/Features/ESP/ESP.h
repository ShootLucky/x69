#pragma once
#include "../src/SDK/SDK.h"
#include "../src/SDK/Helpers/Draw/Draw.h"
#include "../src/SDK/Helpers/Entities/Entities.h"
#include "../src/CFG.h"
#include "../LagRecords/Backtrack.h"
#include <map>

class CESP {
public:
	void Run();
	void Init();
	void Shutdown();
	bool IsEnabled() const noexcept { return CFG::ESP_Enable; }
	void CustomFOV(CViewSetup* pSetup);
	void Rain();
	void PlayerArrow(C_TFPlayer* Player, Color_t Clr);

private:
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
	float GetAnimatedHealthValue(int entIndex, int currentHealth, int maxHealth);
	Color_t GetHealthBarColor(int health, int maxHealth);
	bool m_bInitialized = false;
	static C_BaseEntity* RainEntity;
	static IClientNetworkable* RainNetworkable;
	static C_BaseEntity* WindEntity;
	static IClientNetworkable* WindNetworkable;

	// Sistema de animação de vida
	std::map<int, float> m_AnimatedHealth; // entIndex -> vida animada
	std::map<int, float> m_InitialAppearTime; // entIndex -> tempo de aparição
};

extern CESP gESP;