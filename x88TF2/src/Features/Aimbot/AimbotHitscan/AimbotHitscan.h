#pragma once
#include "../src/SDK/SDK.h"
#include <vector>

// Adapted from Amalgam by rei-2
// https://github.com/rei-2/Amalgam

constexpr int AIMBOT_HITBOX_HEAD = 0;
constexpr int AIMBOT_HITBOX_PELVIS = 1;
constexpr int AIMBOT_HITBOX_SPINE0 = 2;
constexpr int AIMBOT_HITBOX_SPINE1 = 3;
constexpr int AIMBOT_HITBOX_SPINE2 = 4;
constexpr int AIMBOT_HITBOX_SPINE3 = 5;

class CAimbotHitscan
{
private:
    struct TickRecord
    {
        float m_flSimTime = 0.f;
        Vec3 m_vOrigin = {};
        matrix3x4_t m_aBones[MAXSTUDIOBONES];
    };

    struct Target_t
    {
        C_TFPlayer* pEntity = nullptr;
        Vec3 vPos = {};
        Vec3 vAngles = {};
        float fFOV = 0.0f;
        float fDist = 0.0f;
        int nHitbox = 0;
        int nPriority = 0;
        int nAimedHitbox = 0;
        bool bBacktrack = false;
        TickRecord* pRecord = nullptr;
    };

    Vec3 m_vEyePos = {};

    // Core functions
    std::vector<Target_t> GetTargets(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
    int GetHitboxPriority(int nHitbox, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, C_BaseEntity* pTarget);
    int CanHit(Target_t& tTarget, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, CUserCmd* pCmd);
    bool ShouldFire(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, CUserCmd* pCmd, const Target_t& tTarget);

    // ========== DUAS FUNÇÕES AIM (AMALGAM) ==========
    // Overload 1: Calcula o ângulo para o target
    bool Aim(Vec3 vCurAngle, Vec3 vToAngle, Vec3& vOut, int iMethod = 0);

    // Overload 2: Aplica o ângulo ao comando
    void Aim(CUserCmd* pCmd, Vec3& vAngle, int iMethod = 0);

    // Helper functions
    std::vector<int> GetActiveHitboxes();
    bool IsValidTarget(C_TFPlayer* pEntity, C_TFPlayer* pLocal);
    Vec3 GetHitboxPos(C_TFPlayer* pEntity, int nHitbox);

public:
    void Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
    bool IsFiring(CUserCmd* pCmd, C_TFWeaponBase* pWeapon);
};

MAKE_SINGLETON_SCOPED(CAimbotHitscan, AimbotHitscan, F);