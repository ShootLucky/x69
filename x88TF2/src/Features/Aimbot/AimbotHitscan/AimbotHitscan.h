// AimbotHitscan.h
#pragma once
#include "../AimbotCommon/AimbotCommon.h" // Assuming this exists, but since base is SEOwned, perhaps not needed or adjust

class CAimbotHitscan
{
	struct HitscanTarget_t
	{
		C_BaseEntity* Entity = nullptr;
		Vec3 Position = {};
		Vec3 AngleTo = {};
		float FOVTo = 0.0f;
		float DistTo = 0.0f;
		int AimedHitbox = -1;
	};

	std::vector<HitscanTarget_t> m_vecTargets = {};
	static C_BaseEntity* m_pLastTarget; // For aimlock

	int GetAimHitbox();
	bool GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, HitscanTarget_t& outTarget);
	bool ShouldAim(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
	void Aim(CUserCmd* pCmd, C_TFPlayer* pLocal, const Vec3& vAngles);
	bool ShouldFire(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const HitscanTarget_t& target);
	void HandleFire(CUserCmd* pCmd, C_TFWeaponBase* pWeapon);

public:
	void Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
	bool IsFiring(const CUserCmd* pCmd, C_TFWeaponBase* pWeapon);
};

MAKE_SINGLETON_SCOPED(CAimbotHitscan, AimbotHitscan, F);