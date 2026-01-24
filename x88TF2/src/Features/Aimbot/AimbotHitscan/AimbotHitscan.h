#pragma once
#include "../src/SDK/SDK.h"
#include <vector>

class CAimbotHitscan
{
private:
    struct Target_t
    {
        C_TFPlayer* pEntity = nullptr;
        Vec3 vHitboxPos = {};
        Vec3 vAngles = {};
        float fFOV = 0.0f;
        float fDist = 0.0f;
        int nHealth = 0;
        int nHitbox = 0;
        bool bCanHit = false;
        bool bCanShoot = false;
    };

    int m_iLastTargetIndex = 0;
    bool m_bTargetLocked = false;

    std::vector<Target_t> GetTargets(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
    bool ScanTarget(C_TFPlayer* pLocal, C_TFPlayer* pTarget,
        const Vec3& vLocalPos, const Vec3& vLocalAngles,
        const std::vector<int>& hitboxes, Target_t& out);

    std::vector<int> GetActiveHitboxes();
    float GetHitboxPriority(int hitbox);
    bool IsValidTarget(C_TFPlayer* pEntity, C_TFPlayer* pLocal);
    Vec3 GetHitboxPos(C_TFPlayer* pEntity, int nHitbox);

public:
    bool IsFiring(CUserCmd* pCmd, C_TFWeaponBase* pWeapon);
    void Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
};

MAKE_SINGLETON_SCOPED(CAimbotHitscan, AimbotHitscan, F);