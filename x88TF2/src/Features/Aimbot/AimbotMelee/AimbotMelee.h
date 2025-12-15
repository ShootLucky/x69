#pragma once
#include "../AimbotCommon/AimbotCommon.h"
#include "../src/Features/LagRecords/Backtrack.h"
class CAimbotMelee
{
	struct MeleeTarget_t : AimTarget_t
	{
		float SimulationTime = -1.0f;
		const LagRecord_t* LagRecord = nullptr;
		bool MeleeTraceHit = false;
	};
	std::vector<MeleeTarget_t> m_vecTargets = {};
	bool CanSee(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, MeleeTarget_t& target);
	bool GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, MeleeTarget_t& outTarget);
	bool KeyDown(const CUserCmd* pCmd);
	bool ShouldAim(const CUserCmd* pCmd, C_TFWeaponBase* pWeapon);
	void Aim(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const Vec3& vAngles);
	bool ShouldFire(const MeleeTarget_t& target);
	void HandleFire(CUserCmd* pCmd, C_TFWeaponBase* pWeapon);
public:
	bool IsFiring(const CUserCmd* pCmd, C_TFWeaponBase* pWeapon);
	void Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
};
MAKE_SINGLETON_SCOPED(CAimbotMelee, AimbotMelee, F);