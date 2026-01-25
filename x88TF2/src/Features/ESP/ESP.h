#pragma once
#include "../src/SDK/SDK.h"
#include "../src/SDK/Helpers/Draw/Draw.h"
#include "../src/SDK/Helpers/Entities/Entities.h"
#include "../src/CFG.h"
#include "../LagRecords/Backtrack.h"
#include <map>
#include <unordered_map>

// Forward declarations
struct BoneMatrixes_t;

class CESP {
public:
	void Run();
	void Init();
	void Shutdown();
	bool IsEnabled() const noexcept { return CFG::ESP_Enable; }
	void CustomFOV(CViewSetup* pSetup);
	void Rain();
	void PlayerArrow(C_TFPlayer* Player, Color_t Clr);

	// Bone system - Amalgam style
	void StoreBoneMatrix(C_TFPlayer* pPlayer);
	void StoreBoneHitboxes(C_TFPlayer* pPlayer);
	Vec3 GetBonePosition(C_TFPlayer* pPlayer, int iBone, const BoneMatrixes_t* pBoneMatrix = nullptr);

	// Snapshots for on-shot / on-hit visuals
	void StoreShotSnapshot(C_TFPlayer* pPlayer);
	void StoreHitSnapshot(C_TFPlayer* pPlayer, const Vec3& hitPos);

	// Aimbot -> ESP link for dynamic aim points
	void SetAimbotAimPoint(C_TFPlayer* pPlayer, const Vec3& point);

	// Skeleton drawing functions
	void DrawSkeleton(C_TFPlayer* pPlayer, const Color_t& clr);
	void DrawSkeletonOnShot(C_TFPlayer* pPlayer);
	void DrawSkeletonOnHit(C_TFPlayer* pPlayer, const Vec3& hitPos);
	void DrawBacktrackSkeleton(C_TFPlayer* pPlayer);
	void DrawBounds(C_TFPlayer* pPlayer, const Color_t& clr);
	void DrawAimPoints(C_TFPlayer* pPlayer, const Color_t& clr);
	void DrawSkeletonBones(C_TFPlayer* pPlayer, matrix3x4_t* aBones, std::vector<int> vecBones, const Color_t& clr);

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

	// Bone storage maps - não static
	std::unordered_map<C_TFPlayer*, BoneMatrixes_t> m_mBones;
	std::unordered_map<C_TFPlayer*, std::vector<Vec3>> m_mBoneHitboxes;

	std::unordered_map<C_TFPlayer*, std::pair<BoneMatrixes_t, float>> m_mBonesOnShot;
	std::unordered_map<C_TFPlayer*, std::pair<std::vector<Vec3>, float>> m_mBoneHitboxesOnShot;

	std::unordered_map<C_TFPlayer*, std::pair<BoneMatrixes_t, float>> m_mBonesOnHit;
	std::unordered_map<C_TFPlayer*, std::pair<std::vector<Vec3>, float>> m_mBoneHitboxesOnHit;
	std::unordered_map<C_TFPlayer*, std::pair<Vec3, float>> m_mHitPosOnHit;

	std::unordered_map<C_TFPlayer*, std::pair<Vec3, float>> m_mAimbotAimPoints;

	// Sistema de animação de vida
	std::map<int, float> m_AnimatedHealth; // entIndex -> vida animada
	std::map<int, float> m_InitialAppearTime; // entIndex -> tempo de aparição
};

extern CESP gESP;