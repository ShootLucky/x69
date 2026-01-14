#pragma once
#include "../AimbotCommon/AimbotCommon.h"
#include "../src/Features/LagRecords/Backtrack.h"

// Hitbox type flags for multi-selection
enum EHitboxType
{
    HITBOX_TYPE_HEAD = (1 << 0), // 1
    HITBOX_TYPE_BODY = (1 << 1), // 2
    HITBOX_TYPE_PELVIS = (1 << 2), // 4
    HITBOX_TYPE_ARMS = (1 << 3), // 8
    HITBOX_TYPE_LEGS = (1 << 4)  // 16
};

class CAimbotHitscan
{
private:
    struct HitscanTarget_t : AimTarget_t
    {
        int AimedHitbox = -1;
        float SimulationTime = -1.0f;
        const LagRecord_t* LagRecord = nullptr;
        bool WasMultiPointed = false;
        int HitboxGroup = -1;
        float Accuracy = 0.0f;
        float Damage = 0.0f;
    };

    struct ScanPoint_t
    {
        Vec3 Position;
        float FOVTo;
        float DistanceToCenter;
        int Hitbox;
        float quality; // lowercase to match usage in .cpp
    };

    // Target management
    std::vector<HitscanTarget_t> m_vecTargets = {};
    HitscanTarget_t m_LastTarget{};
    bool m_bActive = false;
    bool m_bLastShotMissed = false;

    // Core scanning functions
    bool ScanHitboxGroup(C_TFPlayer* pLocal, C_TFPlayer* pTarget, HitscanTarget_t& target,
        const Vec3& vLocalAngles, const std::vector<int>& hitboxes, int group);
    bool ScanBuilding(C_TFPlayer* pLocal, HitscanTarget_t& target, const Vec3& vLocalAngles);

    // Multipoint generation
    std::vector<Vec3> GenerateMultipoints(const mstudiobbox_t* pBox, const matrix3x4_t& boneMatrix);

    // Target selection
    bool GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, HitscanTarget_t& outTarget);
    bool ValidateTarget(C_BaseEntity* pEntity, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);

    // Aiming
    bool ShouldAim(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
    void Aim(CUserCmd* pCmd, C_TFPlayer* pLocal, const Vec3& vTargetAngles);
    Vec3 CalculateSmoothAngles(const Vec3& vCurrentAngles, const Vec3& vTargetAngles, float smoothing);

    // Shooting
    bool ShouldFire(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const HitscanTarget_t& target);
    void HandleFire(CUserCmd* pCmd, C_TFWeaponBase* pWeapon);
    bool VerifyHitchance(C_TFPlayer* pLocal, const CUserCmd* pCmd, const HitscanTarget_t& target);

    // Utilities
    std::vector<int> GetActiveHitboxes();
    int GetHitboxGroup(int hitbox);
    float CalculateHitboxPriority(int hitbox, C_TFWeaponBase* pWeapon, float distance);
    bool IsHitboxEnabled(int hitbox);

public:
    void Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
    bool IsFiring(CUserCmd* pCmd, C_TFWeaponBase* pWeapon);
    void DrawDebug();
};

MAKE_SINGLETON_SCOPED(CAimbotHitscan, AimbotHitscan, F);