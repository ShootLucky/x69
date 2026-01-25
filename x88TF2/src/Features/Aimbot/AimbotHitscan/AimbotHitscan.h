#pragma once
#include "../src/SDK/SDK.h"
#include <vector>

// Adapted from Amalgam by rei-2
// https://github.com/rei-2/Amalgam

// Identificadores de hitbox locais ao módulo para evitar colisões com
// símbolos globais já definidos em outros headers/SDKs.
constexpr int AIMBOT_HITBOX_HEAD = 0;
constexpr int AIMBOT_HITBOX_PELVIS = 1;
constexpr int AIMBOT_HITBOX_SPINE0 = 2;
constexpr int AIMBOT_HITBOX_SPINE1 = 3;
constexpr int AIMBOT_HITBOX_SPINE2 = 4;
constexpr int AIMBOT_HITBOX_SPINE3 = 5;

class CAimbotHitscan
{
private:
    struct Target_t
    {
        C_TFPlayer* pEntity = nullptr;
        Vec3 vPos = {};
        Vec3 vAngles = {};
        float fFOV = 0.0f;
        float fDist = 0.0f;
        int nHitbox = 0;
        int nPriority = 0;
        bool bBacktrack = false;
        int nTickCount = 0;
    };

    // Core functions
    std::vector<Target_t> GetTargets(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
    int CanHit(Target_t& target, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
    bool ShouldFire(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, CUserCmd* pCmd, const Target_t& target);
    void Aim(CUserCmd* pCmd, Vec3 vAngle);

    // Helper functions
    std::vector<int> GetActiveHitboxes();
    int GetHitboxPriority(int nHitbox);
    bool IsValidTarget(C_TFPlayer* pEntity, C_TFPlayer* pLocal);
    Vec3 GetHitboxPos(C_TFPlayer* pEntity, int nHitbox);

public:
    void Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
    bool IsFiring(CUserCmd* pCmd, C_TFWeaponBase* pWeapon);
};

MAKE_SINGLETON_SCOPED(CAimbotHitscan, AimbotHitscan, F);