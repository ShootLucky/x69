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
    Vec3 m_vSmoothTargetAngles = {};
    bool m_bTargetLocked = false;
    int m_iLastShotTick = 0;
    int m_iTapfireDelay = 0;
    bool m_bWaitingForRelease = false;

    bool GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, Target_t& outTarget);
    bool ScanTargetHitboxes(C_TFPlayer* pLocal, C_TFPlayer* pTarget,
        const Vec3& vLocalPos, const Vec3& vLocalAngles,
        const std::vector<int>& hitboxes, Target_t& outTarget);

    std::vector<int> GetActiveHitboxes();
    float CalculateHitboxScore(int hitbox, float fov, float distance);

    bool IsValidTarget(C_TFPlayer* pEntity, C_TFPlayer* pLocal);
    Vec3 GetHitboxPosition(C_TFPlayer* pEntity, int nHitbox);
    float CalculateFOV(const Vec3& vLocalAngles, const Vec3& vTargetPos, const Vec3& vLocalPos);

public:
    bool IsFiring(CUserCmd* pCmd, C_TFWeaponBase* pWeapon);
    void Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
};

MAKE_SINGLETON_SCOPED(CAimbotHitscan, AimbotHitscan, F);