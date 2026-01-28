#pragma once
#include "../AimbotCommon/AimbotCommon.h"

// ========== ENUMS DO AMALGAM ==========
enum CalculatedEnum
{
    Pending,
    Good,
    Time,
    Bad
};

enum ProjSimEnum
{
    None = 0,
    Trace = 1 << 0,
    InitCheck = 1 << 1,
    NoRandomAngles = 1 << 2,
    PredictCmdNum = 1 << 3
};

// ========== STRUCTS DO AMALGAM ==========
struct Solution_t
{
    float m_flPitch = 0.f;
    float m_flYaw = 0.f;
    float m_flTime = 0.f;
    int m_iCalculated = CalculatedEnum::Pending;
};

struct Point_t
{
    Vec3 m_vPoint = {};
    Solution_t m_tSolution = {};
};

struct Info_t
{
    C_TFPlayer* m_pLocal = nullptr;
    C_TFWeaponBase* m_pWeapon = nullptr;

    Vec3 m_vLocalEye = {};
    Vec3 m_vTargetEye = {};

    float m_flLatency = 0.f;

    Vec3 m_vHull = {};
    Vec3 m_vOffset = {};
    Vec3 m_vAngFix = {};
    float m_flVelocity = 0.f;
    float m_flGravity = 0.f;
    float m_flRadius = 0.f;
    float m_flRadiusTime = 0.f;
    float m_flBoundingTime = 0.f;
    float m_flOffsetTime = 0.f;
    int m_iSplashCount = 0;
    int m_iSplashMode = 0;
    float m_flPrimeTime = 0;
    int m_iPrimeTime = 0;
};

// ========== PROJECTILE TARGET DATA ==========
struct ProjTarget_t : AimTarget_t
{
    float TimeToTarget = 0.0f;
    float ProjectileTime = 0.0f;
    float RequiredCharge = 0.0f;
};

// ========== PROJECTILE AIMBOT ==========
class CAimbotProjectile
{
private:
    std::vector<ProjTarget_t> m_vecTargets = {};
    std::vector<Vec3> m_TargetPath = {};
    int m_LastAimPos = 0;

    struct ProjectileInfo_t
    {
        float Speed = 0.0f;
        float GravityMod = 0.0f;
        bool Pipes = false;
        bool Flamethrower = false;
    };
    ProjectileInfo_t m_CurProjInfo = {};

    // Info structure para CalculateAngle
    Info_t m_tInfo = {};

private:
    bool GetProjectileInfo(C_TFWeaponBase* pWeapon);

    bool CalcProjAngle(
        const Vec3& vFrom,
        const Vec3& vTo,
        Vec3& vAngleOut,
        float& flTimeOut,
        bool bHighArc = false
    );

    void OffsetPlayerPosition(
        C_TFWeaponBase* pWeapon,
        Vec3& vPos,
        C_TFPlayer* pPlayer,
        bool bDucked,
        bool bOnGround
    );

    bool CanArcReach(
        const Vec3& vFrom,
        const Vec3& vTo,
        const Vec3& vAngleTo,
        float flTargetTime,
        C_BaseEntity* pTarget
    );

    bool CanSee(
        C_TFPlayer* pLocal,
        C_TFWeaponBase* pWeapon,
        const Vec3& vFrom,
        const Vec3& vTo,
        const ProjTarget_t& target,
        float flTargetTime
    );

    bool TrySplashShot(
        C_TFPlayer* pLocal,
        C_TFWeaponBase* pWeapon,
        const CUserCmd* pCmd,
        const ProjTarget_t& target,
        Vec3& outAngle,
        float& outTime,
        bool isPlayer
    );

    bool SolveTarget(
        C_TFPlayer* pLocal,
        C_TFWeaponBase* pWeapon,
        const CUserCmd* pCmd,
        ProjTarget_t& target
    );

    bool GetTarget(
        C_TFPlayer* pLocal,
        C_TFWeaponBase* pWeapon,
        const CUserCmd* pCmd,
        ProjTarget_t& outTarget
    );

    bool KeyDown(const CUserCmd* pCmd);
    bool ShouldAim(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);

    void Aim(
        CUserCmd* pCmd,
        C_TFPlayer* pLocal,
        C_TFWeaponBase* pWeapon,
        const Vec3& vAngles
    );

    bool ShouldFire(
        CUserCmd* pCmd,
        C_TFPlayer* pLocal,
        C_TFWeaponBase* pWeapon
    );

    void HandleFire(
        CUserCmd* pCmd,
        C_TFWeaponBase* pWeapon,
        C_TFPlayer* pLocal,
        const ProjTarget_t& target
    );

    bool NeuralNetworkSplashPrediction(const Vec3& impactPoint, C_BaseEntity* pTargetEntity);

    // ========== FUNÇÕES DO AMALGAM ==========
    void CalculateAngle(
        const Vec3& vLocalPos,
        const Vec3& vTargetPos,
        int iSimTime,
        Solution_t& out,
        bool bAccuracy = true
    );

    int CanHit(
        ProjTarget_t& tTarget,
        C_TFPlayer* pLocal,
        C_TFWeaponBase* pWeapon,
        bool bVisuals = true
    );

public:
    // ========== PUBLIC FUNCTIONS ==========
    Vec3 GetWeaponFireOffset(C_TFWeaponBase* pWeapon, C_TFPlayer* pLocal);
    bool IsFiring(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
    bool ShouldAimKey();
    void Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon);
};

MAKE_SINGLETON_SCOPED(CAimbotProjectile, AimbotProjectile, F);