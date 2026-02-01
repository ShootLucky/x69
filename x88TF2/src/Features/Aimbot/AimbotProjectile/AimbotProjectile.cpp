// ========================================
// AimbotProjectile.cpp - VERSÃO OTIMIZADA
// Sistema de Splash Bot REMOVIDO
// Sistema de Arc OTIMIZADO
// ========================================

#include "AimbotProjectile.h"
#include "CFG.h"
#include "../src/Features/MovementSimulation/MovementSimulation.h"
#include "../src/Features/ProjectileSim/ProjectileSim.h"
#include <algorithm>
#include "../../../SDK/Helpers/AimUtils/AimUtils.h"

void DrawProjPath(const CUserCmd* pCmd, float time)
{
    if (!pCmd || !G::bFiring)
        return;

    const auto pLocal = H::Entities->GetLocal();
    if (!pLocal || pLocal->deadflag())
        return;

    const auto pWeapon = H::Entities->GetWeapon();
    if (!pWeapon)
        return;

    ProjectileInfo info = {};
    if (!F::ProjectileSim->GetInfo(pLocal, pWeapon, pCmd->viewangles, info))
        return;

    if (!F::ProjectileSim->Init(info))
        return;

    for (auto n = 0; n < TIME_TO_TICKS(time); n++)
    {
        auto pre{ F::ProjectileSim->GetOrigin() };
        F::ProjectileSim->RunTick();
        auto post{ F::ProjectileSim->GetOrigin() };
        I::DebugOverlay->AddLineOverlay(pre, post, 255, 255, 255, false, 10.0f);
    }
}

void DrawMovePath(const std::vector<Vec3>& vPath)
{
    if (vPath.size() < 2)
        return;

    constexpr float duration = 10.0f;

    auto DrawLineOutlined = [&](const Vec3& a, const Vec3& b)
        {
            I::DebugOverlay->AddLineOverlay(a, b, 0, 0, 0, false, duration);
            I::DebugOverlay->AddLineOverlay(a, b, 255, 255, 255, false, duration);
        };

    // Line style
    if (CFG::Visuals_Draw_Movement_Path_Style == 1)
    {
        for (size_t n = 1; n < vPath.size(); n++)
            DrawLineOutlined(vPath[n], vPath[n - 1]);
    }
    // Dashed style
    else if (CFG::Visuals_Draw_Movement_Path_Style == 2)
    {
        for (size_t n = 1; n < vPath.size(); n++)
        {
            if (n % 2 == 0) continue;
            DrawLineOutlined(vPath[n], vPath[n - 1]);
        }
    }
    // Line + 3D Box
    else if (CFG::Visuals_Draw_Movement_Path_Style == 3)
    {
        for (size_t n = 1; n < vPath.size(); n++)
        {
            DrawLineOutlined(vPath[n], vPath[n - 1]);

            if (n == vPath.size() - 1)
            {
                const Vec3& impactPos = vPath[n];
                const Vec3 mins{ -24.f, -24.f, 0.f };
                const Vec3 maxs{ 24.f, 24.f, 82.f };

                I::DebugOverlay->AddBoxOverlay(impactPos, mins, maxs, Vec3(0.f, 0.f, 0.f), 0, 0, 0, 0, duration);
                I::DebugOverlay->AddBoxOverlay(impactPos, mins, maxs, Vec3(0.f, 0.f, 0.f), 255, 255, 255, 0, duration);
            }
        }
    }
}

Vec3 GetProjectileFirePos(C_TFPlayer* local, C_TFWeaponBase* weapon, const Vec3& angles)
{
    Vec3 out = local->GetShootPos();
    Vec3 offset = F::AimbotProjectile->GetWeaponFireOffset(weapon, local);

    if (offset.x != 0.0f || offset.y != 0.0f || offset.z != 0.0f)
        H::AimUtils->GetProjectileFireSetup(angles, offset, &out);

    return out;
}

bool IsChargingWeapon(int weaponID)
{
    return weaponID == TF_WEAPON_COMPOUND_BOW ||
        weaponID == TF_WEAPON_PIPEBOMBLAUNCHER ||
        weaponID == TF_WEAPON_CANNON;
}

float GetRequiredChargeTime(C_TFWeaponBase* pWeapon, const ProjTarget_t& target, const Vec3& vLocalPos)
{
    const int nWeaponID = pWeapon->GetWeaponID();
    const Vec3 vTo = target.Position - vLocalPos;
    float dx = sqrt(vTo.x * vTo.x + vTo.y * vTo.y);
    float dy = vTo.z;
    float g = SDKUtils::GetGravity();

    if (nWeaponID == TF_WEAPON_PIPEBOMBLAUNCHER)
    {
        g *= 1.0f;
        float min_u = g * (dy + sqrt(dx * dx + dy * dy));
        float min_v0 = sqrt(min_u);
        float required_speed = std::max(min_v0, 900.0f);
        float charge = (required_speed - 900.0f) / 375.0f;
        charge = std::max(0.0f, std::min(charge, 4.0f));
        return charge;
    }
    else if (nWeaponID == TF_WEAPON_COMPOUND_BOW)
    {
        float low = 0.0f, high = 1.0f;
        for (int i = 0; i < 10; i++)
        {
            float mid = (low + high) / 2.0f;
            float v0 = 1800.0f + mid * 800.0f;
            float g_mod = 0.5f - mid * 0.4f;
            float local_g = SDKUtils::GetGravity() * g_mod;
            float root = v0 * v0 * v0 * v0 - local_g * (local_g * dx * dx + 2.0f * dy * v0 * v0);

            if (root >= 0.0f)
                high = mid;
            else
                low = mid;
        }
        return high;
    }
    else if (nWeaponID == TF_WEAPON_CANNON)
    {
        if (CFG::Aimbot_Projectile_Auto_Double_Donk)
            return target.TimeToTarget * 0.8f;
        else
            return 0.0f;
    }

    return 0.0f;
}

float GetCurrentChargeTime(C_TFWeaponBase* pWeapon)
{
    float charge_begin = pWeapon->As<C_TFPipebombLauncher>()->m_flChargeBeginTime();
    if (charge_begin > 0.0f)
        return I::GlobalVars->curtime - charge_begin;

    return 0.0f;
}

bool CAimbotProjectile::GetProjectileInfo(C_TFWeaponBase* pWeapon)
{
    m_CurProjInfo = {};

    auto curTime = [&]() -> float
        {
            if (const auto pLocal = H::Entities->GetLocal())
                return static_cast<float>(pLocal->m_nTickBase()) * I::GlobalVars->interval_per_tick;
            return I::GlobalVars->curtime;
        };

    switch (pWeapon->GetWeaponID())
    {
    case TF_WEAPON_ROCKETLAUNCHER:
    case TF_WEAPON_PARTICLE_CANNON:
    case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
    {
        m_CurProjInfo = { 1100.0f, 0.0f };
        m_CurProjInfo.Speed = SDKUtils::AttribHookValue(m_CurProjInfo.Speed, "mult_projectile_speed", pWeapon);

        if (C_TFPlayer * local{ H::Entities->GetLocal() })
        {
            if (const int rocket_specialist{ static_cast<int>(SDKUtils::AttribHookValue(0.0f, "rocket_specialist", local)) })
            {
                m_CurProjInfo.Speed *= Math::RemapValClamped(static_cast<float>(rocket_specialist), 1.0f, 4.0f, 1.15f, 1.6f);
                m_CurProjInfo.Speed = std::min(m_CurProjInfo.Speed, 3000.0f);
            }
        }
        break;
    }
    case TF_WEAPON_GRENADELAUNCHER:
    {
        m_CurProjInfo = { 1200.0f, 1.0f, true };
        m_CurProjInfo.Speed = SDKUtils::AttribHookValue(m_CurProjInfo.Speed, "mult_projectile_speed", pWeapon);
        break;
    }
    case TF_WEAPON_PIPEBOMBLAUNCHER:
    {
        const float flChargeBeginTime = pWeapon->As<C_TFPipebombLauncher>()->m_flChargeBeginTime();
        const float flCharge = curTime() - flChargeBeginTime;

        if (flChargeBeginTime)
        {
            m_CurProjInfo.Speed = Math::RemapValClamped(
                flCharge,
                0.0f,
                SDKUtils::AttribHookValue(4.0f, "stickybomb_charge_rate", pWeapon),
                900.0f,
                2400.0f
            );
        }
        else
        {
            m_CurProjInfo.Speed = 900.0f;
        }
        m_CurProjInfo.GravityMod = 1.0f;
        m_CurProjInfo.Pipes = true;
        break;
    }
    case TF_WEAPON_CANNON:
    {
        m_CurProjInfo = { 1454.0f, 1.0f, true };
        break;
    }
    case TF_WEAPON_COMPOUND_BOW:
    {
        const float flChargeBeginTime = pWeapon->As<C_TFPipebombLauncher>()->m_flChargeBeginTime();
        const float flCharge = curTime() - flChargeBeginTime;

        if (flChargeBeginTime)
        {
            m_CurProjInfo.Speed = 1800.0f + std::clamp<float>(flCharge, 0.0f, 1.0f) * 800.0f;
            m_CurProjInfo.GravityMod = Math::RemapValClamped(flCharge, 0.0f, 1.0f, 0.5f, 0.1f);
        }
        else
        {
            m_CurProjInfo.Speed = 1800.0f;
            m_CurProjInfo.GravityMod = 0.5f;
        }
        break;
    }
    case TF_WEAPON_CROSSBOW:
    case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
    {
        m_CurProjInfo = { 2400.0f, 0.2f };
        break;
    }
    case TF_WEAPON_SYRINGEGUN_MEDIC:
    {
        m_CurProjInfo = { 1000.0f, 0.3f };
        break;
    }
    case TF_WEAPON_FLAREGUN:
    {
        m_CurProjInfo = { 2000.0f, 0.3f };
        break;
    }
    case TF_WEAPON_FLAREGUN_REVENGE:
    {
        m_CurProjInfo = { 3000.0f, 0.45f };
        break;
    }
    case TF_WEAPON_FLAME_BALL:
    {
        m_CurProjInfo = { 3000.0f, 0.0f };
        break;
    }
    case TF_WEAPON_FLAMETHROWER:
    {
        m_CurProjInfo = { 2000.0f, 0.0f };
        m_CurProjInfo.Flamethrower = true;
        break;
    }
    case TF_WEAPON_RAYGUN:
    case TF_WEAPON_DRG_POMSON:
    {
        m_CurProjInfo = { 1200.0f, 0.0f };
        break;
    }
    default:
        break;
    }

    return m_CurProjInfo.Speed > 0.0f;
}

// ========================================
// CACHE HELPER FUNCTIONS - OTIMIZADAS
// ========================================

inline float CAimbotProjectile::GetCachedGravity()
{
    int nFrameCount = I::GlobalVars->framecount;
    if (m_nLastGravityFrame != nFrameCount)
    {
        m_flCachedGravity = SDKUtils::GetGravity();
        m_nLastGravityFrame = nFrameCount;
    }
    return m_flCachedGravity;
}

inline bool CAimbotProjectile::IsCacheValid(const ArcCache_t& cache, const Vec3& vFrom, const Vec3& vTo, float flSpeed, bool bHighArc)
{
    int nCurrentFrame = I::GlobalVars->framecount;
    if (nCurrentFrame - cache.nFrameCalculated > CACHE_VALIDITY_FRAMES)
        return false;

    constexpr float EPSILON = 0.1f;
    if (abs(cache.vFrom.x - vFrom.x) > EPSILON || abs(cache.vFrom.y - vFrom.y) > EPSILON || abs(cache.vFrom.z - vFrom.z) > EPSILON)
        return false;
    if (abs(cache.vTo.x - vTo.x) > EPSILON || abs(cache.vTo.y - vTo.y) > EPSILON || abs(cache.vTo.z - vTo.z) > EPSILON)
        return false;
    if (abs(cache.flSpeed - flSpeed) > EPSILON)
        return false;
    if (cache.bHighArc != bHighArc)
        return false;

    return cache.bValid;
}

// ========================================
// OPTIMIZED ARC CALCULATION
// ========================================

bool CAimbotProjectile::CalcProjAngle_Optimized(const Vec3& vFrom, const Vec3& vTo, Vec3& vAngleOut, float& flTimeOut, bool bHighArc)
{
    // Check cache
    if (IsCacheValid(m_ArcCache, vFrom, vTo, m_CurProjInfo.Speed, bHighArc))
    {
        vAngleOut = m_ArcCache.vAngleResult;
        flTimeOut = m_ArcCache.flTimeResult;
        return true;
    }

    const float g = GetCachedGravity() * m_CurProjInfo.GravityMod;
    float v0 = m_CurProjInfo.Speed;

    // Fast path - no gravity
    if (g <= 0.01f)
    {
        vAngleOut = Math::CalcAngle(vFrom, vTo);
        flTimeOut = vFrom.DistTo(vTo) / v0;

        m_ArcCache.vFrom = vFrom;
        m_ArcCache.vTo = vTo;
        m_ArcCache.flSpeed = v0;
        m_ArcCache.flGravity = g;
        m_ArcCache.bHighArc = bHighArc;
        m_ArcCache.vAngleResult = vAngleOut;
        m_ArcCache.flTimeResult = flTimeOut;
        m_ArcCache.bValid = true;
        m_ArcCache.nFrameCalculated = I::GlobalVars->framecount;
        return true;
    }

    // Ballistic calculation
    const Vec3 v = vTo - vFrom;
    const float dx = sqrtf(v.x * v.x + v.y * v.y);
    const float dy = v.z;

    if (m_CurProjInfo.Pipes && v0 > k_flMaxVelocity)
        v0 = k_flMaxVelocity;

    const float v0_sq = v0 * v0;
    const float v0_quad = v0_sq * v0_sq;
    const float g_dx_sq = g * dx * dx;
    const float discriminant = v0_quad - g * (g_dx_sq + 2.0f * dy * v0_sq);

    if (discriminant < 0.0f)
    {
        m_ArcCache.bValid = false;
        return false;
    }

    const float sqrt_discriminant = sqrtf(discriminant);
    const float sign = bHighArc ? +1.0f : -1.0f;
    const float theta = atanf((v0_sq + sign * sqrt_discriminant) / (g * dx));
    const float cos_theta = cosf(theta);

    vAngleOut.x = -RAD2DEG(theta);
    vAngleOut.y = RAD2DEG(atan2f(v.y, v.x));
    vAngleOut.z = 0.0f;
    flTimeOut = dx / (cos_theta * v0);

    // Drag correction for pipes
    if (m_CurProjInfo.Pipes)
    {
        const auto pWeapon = H::Entities->GetWeapon();
        if (!pWeapon)
        {
            m_ArcCache.bValid = false;
            return false;
        }

        float dragCoeff = 0.0f;
        const int weaponID = pWeapon->GetWeaponID();

        if (weaponID == TF_WEAPON_GRENADELAUNCHER)
            dragCoeff = (pWeapon->m_iItemDefinitionIndex() == Demoman_m_TheLochnLoad) ? 0.07f : 0.11f;
        else if (weaponID == TF_WEAPON_PIPEBOMBLAUNCHER)
            dragCoeff = 0.16f;
        else if (weaponID == TF_WEAPON_CANNON)
            dragCoeff = 0.35f;

        if (dragCoeff > 0.0f)
        {
            const float v0_drag = v0 - (v0 * flTimeOut * dragCoeff);
            const float v0_drag_sq = v0_drag * v0_drag;
            const float v0_drag_quad = v0_drag_sq * v0_drag_sq;
            const float discriminant_drag = v0_drag_quad - g * (g_dx_sq + 2.0f * dy * v0_drag_sq);

            if (discriminant_drag < 0.0f)
            {
                m_ArcCache.bValid = false;
                return false;
            }

            const float sqrt_discriminant_drag = sqrtf(discriminant_drag);
            const float theta_drag = atanf((v0_drag_sq + sign * sqrt_discriminant_drag) / (g * dx));
            const float cos_theta_drag = cosf(theta_drag);

            vAngleOut.x = -RAD2DEG(theta_drag);
            flTimeOut = dx / (cos_theta_drag * v0_drag);
        }

        // Time validation
        if (weaponID == TF_WEAPON_CANNON)
        {
            if (flTimeOut > 0.95f)
            {
                m_ArcCache.bValid = false;
                return false;
            }
        }
        else if (pWeapon->m_iItemDefinitionIndex() == Demoman_m_TheIronBomber)
        {
            if (flTimeOut > 1.4f)
            {
                m_ArcCache.bValid = false;
                return false;
            }
        }
        else if (flTimeOut > 2.0f)
        {
            m_ArcCache.bValid = false;
            return false;
        }
    }

    // Flamethrower time limit
    if ((m_CurProjInfo.Flamethrower || H::Entities->GetWeapon()->GetWeaponID() == TF_WEAPON_FLAME_BALL) && flTimeOut > 0.18f)
    {
        m_ArcCache.bValid = false;
        return false;
    }

    // Update cache
    m_ArcCache.vFrom = vFrom;
    m_ArcCache.vTo = vTo;
    m_ArcCache.flSpeed = m_CurProjInfo.Speed;
    m_ArcCache.flGravity = g;
    m_ArcCache.bHighArc = bHighArc;
    m_ArcCache.vAngleResult = vAngleOut;
    m_ArcCache.flTimeResult = flTimeOut;
    m_ArcCache.bValid = true;
    m_ArcCache.nFrameCalculated = I::GlobalVars->framecount;

    return true;
}

bool CAimbotProjectile::CalcProjAngle(const Vec3& vFrom, const Vec3& vTo, Vec3& vAngleOut, float& flTimeOut, bool bHighArc)
{
    return CalcProjAngle_Optimized(vFrom, vTo, vAngleOut, flTimeOut, bHighArc);
}

// ========================================
// FUNÇÃO CalculateAngle DO AMALGAM
// ========================================

void CAimbotProjectile::CalculateAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, int iSimTime, Solution_t& out, bool bAccuracy)
{
    if (out.m_iCalculated != CalculatedEnum::Pending)
        return;

    const float flGrav = m_tInfo.m_flGravity * 800.f;

    float flPitch, flYaw;
    {
        float flVelocity = m_tInfo.m_flVelocity;
        Vec3 vDelta = vTargetPos - vLocalPos;
        float flDist = vDelta.Length2D();
        Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vTargetPos);

        if (!flGrav)
        {
            flPitch = -DEG2RAD(vAngleTo.x);
        }
        else
        {
            float flRoot = pow(flVelocity, 4) - flGrav * (flGrav * pow(flDist, 2) + 2.f * vDelta.z * pow(flVelocity, 2));

            if (flRoot < 0.f)
            {
                out.m_iCalculated = CalculatedEnum::Bad;
                return;
            }

            flPitch = atan((pow(flVelocity, 2) - sqrt(flRoot)) / (flGrav * flDist));
        }

        out.m_flTime = flDist / (cos(flPitch) * flVelocity) - m_tInfo.m_flOffsetTime;
        out.m_flPitch = flPitch = -RAD2DEG(flPitch) - m_tInfo.m_vAngFix.x;
        out.m_flYaw = flYaw = vAngleTo.y - m_tInfo.m_vAngFix.y;
    }

    int iTimeTo = int(out.m_flTime / TICK_INTERVAL) + 1;

    if (!m_tInfo.m_vOffset.IsZero())
    {
        if (iTimeTo > iSimTime)
        {
            out.m_iCalculated = CalculatedEnum::Time;
            return;
        }
    }
    else
    {
        out.m_iCalculated = iTimeTo > iSimTime ? CalculatedEnum::Time : CalculatedEnum::Good;
        return;
    }

    int iFlags = (bAccuracy ? ProjSimEnum::Trace : ProjSimEnum::None) | ProjSimEnum::NoRandomAngles | ProjSimEnum::PredictCmdNum;

    ProjectileInfo tProjInfo = {};
    if (!F::ProjectileSim->GetInfo(m_tInfo.m_pLocal, m_tInfo.m_pWeapon, { flPitch, flYaw, 0 }, tProjInfo, iFlags))
    {
        out.m_iCalculated = CalculatedEnum::Bad;
        return;
    }

    {
        float flVelocity = m_tInfo.m_flVelocity;
        Vec3 vDelta = vTargetPos - tProjInfo.m_pos;
        float flDist = vDelta.Length2D();
        Vec3 vAngleTo = Math::CalcAngle(tProjInfo.m_pos, vTargetPos);

        if (!flGrav)
        {
            out.m_flPitch = -DEG2RAD(vAngleTo.x);
        }
        else
        {
            float flRoot = pow(flVelocity, 4) - flGrav * (flGrav * pow(flDist, 2) + 2.f * vDelta.z * pow(flVelocity, 2));

            if (flRoot < 0.f)
            {
                out.m_iCalculated = CalculatedEnum::Bad;
                return;
            }

            out.m_flPitch = atan((pow(flVelocity, 2) - sqrt(flRoot)) / (flGrav * flDist));
        }

        out.m_flTime = flDist / (cos(out.m_flPitch) * flVelocity);
    }

    {
        Vec3 vShootPos = Vec3((tProjInfo.m_pos - vLocalPos).x, (tProjInfo.m_pos - vLocalPos).y, 0.0f);
        Vec3 vTarget = vTargetPos - vLocalPos;
        Vec3 vForward; Math::AngleVectors(tProjInfo.m_ang, &vForward); Math::Normalized2D(vForward);

        float flB = 2 * (vShootPos.x * vForward.x + vShootPos.y * vForward.y);
        float flC = vShootPos.Length2DSqr() - vTarget.Length2DSqr();

        float x1, x2;
        if (Math::SolveQuadratic(1.f, flB, flC, x1, x2))
        {
            vShootPos += vForward * x1;
            out.m_flYaw = flYaw - (RAD2DEG(atan2(vShootPos.y, vShootPos.x)) - flYaw);
            flYaw = RAD2DEG(atan2(vShootPos.y, vShootPos.x));
        }
    }

    {
        if (flGrav)
        {
            flPitch -= tProjInfo.m_ang.x;
            out.m_flPitch = -RAD2DEG(out.m_flPitch) + flPitch - m_tInfo.m_vAngFix.x;
        }
        else
        {
            Vec3 temp2d = tProjInfo.m_pos - vLocalPos;
            Vec2 rotated = Math::RotatePoint(Vec2(temp2d.x, temp2d.y), Vec2(0, 0), -flYaw);
            Vec3 vShootPos = Vec3(rotated.x, rotated.y, temp2d.z);
            vShootPos.y = 0;

            Vec3 tempTgt = vTargetPos - vLocalPos;
            Vec2 rotatedTgt = Math::RotatePoint(Vec2(tempTgt.x, tempTgt.y), Vec2(0, 0), -flYaw);
            Vec3 vTarget = Vec3(rotatedTgt.x, rotatedTgt.y, tempTgt.z);
            Vec3 vForward; Math::AngleVectors(tProjInfo.m_ang - Vec3(0, flYaw, 0), &vForward);
            vForward.y = 0; vForward.Normalize();

            float flB = 2 * (vShootPos.x * vForward.x + vShootPos.z * vForward.z);
            float flC = (powf(vShootPos.x, 2) + powf(vShootPos.z, 2)) - (powf(vTarget.x, 2) + powf(vTarget.z, 2));

            float x1, x2;
            if (Math::SolveQuadratic(1.f, flB, flC, x1, x2))
            {
                vShootPos += vForward * x1;
                out.m_flPitch = flPitch - (RAD2DEG(atan2(-vShootPos.z, vShootPos.x)) - flPitch);
            }
        }
    }

    iTimeTo = int(out.m_flTime / TICK_INTERVAL) + 1;
    out.m_iCalculated = iTimeTo > iSimTime ? CalculatedEnum::Time : CalculatedEnum::Good;
}

int CAimbotProjectile::CanHit(ProjTarget_t& tTarget, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, bool bVisuals)
{
    ProjectileInfo tProjInfo = {};
    if (!F::ProjectileSim->GetInfo(pLocal, pWeapon, {}, tProjInfo)
        || !F::ProjectileSim->Init(tProjInfo, false))
        return false;

    if (!F::MovementSimulation->Initialize(tTarget.Entity->As<C_TFPlayer>()))
        return false;

    tTarget.Position = tTarget.Entity->m_vecOrigin();

    m_tInfo = { pLocal, pWeapon };
    m_tInfo.m_vLocalEye = pLocal->GetShootPos();
    m_tInfo.m_vTargetEye = tTarget.Entity->As<C_TFPlayer>()->m_vecViewOffset();
    m_tInfo.m_flLatency = 0.0f + TICKS_TO_TIME(0);

    m_tInfo.m_flVelocity = m_CurProjInfo.Speed;
    m_tInfo.m_vAngFix = {};
    m_tInfo.m_vHull = Vec3(3.f, 3.f, 3.f);

    m_tInfo.m_vOffset = tProjInfo.m_pos - m_tInfo.m_vLocalEye;
    m_tInfo.m_vOffset.y *= -1;
    m_tInfo.m_flOffsetTime = m_tInfo.m_vOffset.Length() / m_tInfo.m_flVelocity;
    m_tInfo.m_flGravity = tProjInfo.m_gravity_mod;

    Vec3 vMins = tTarget.Entity->m_vecMins();
    Vec3 vMaxs = tTarget.Entity->m_vecMaxs();
    float flSize = (vMaxs - vMins).Length();

    bool iReturn = false;
    int iMaxTime = TIME_TO_TICKS(CFG::Aimbot_Projectile_Max_Simulation_Time);

    Vec3 vAngleTo, vPredicted, vTarget;
    int iLowestPriority = std::numeric_limits<int>::max();
    float flLowestDist = std::numeric_limits<float>::max();

    for (int i = 1 - TIME_TO_TICKS(m_tInfo.m_flLatency); i <= iMaxTime; i++)
    {
        if (i > 0)
        {
            F::MovementSimulation->RunTick();
            tTarget.Position = F::MovementSimulation->GetSimulatedOrigin();
        }

        if (i < 0)
            continue;

        Solution_t solution;
        CalculateAngle(m_tInfo.m_vLocalEye, tTarget.Position, i, solution, true);

        if (solution.m_iCalculated == CalculatedEnum::Good)
        {
            vAngleTo = { solution.m_flPitch, solution.m_flYaw, 0.f };
            vPredicted = tTarget.Position;
            vTarget = tTarget.Position;
            iReturn = true;
            break;
        }
    }

    if (iReturn)
    {
        tTarget.Position = vTarget;
        tTarget.AngleTo = vAngleTo;
    }

    return iReturn;
}

void CAimbotProjectile::OffsetPlayerPosition(C_TFWeaponBase* pWeapon, Vec3& vPos, C_TFPlayer* pPlayer, bool bDucked, bool bOnGround)
{
    const float flMaxZ{ (bDucked ? 62.0f : 82.0f) * pPlayer->m_flModelScale() };

    // Ponto padrão (Body)
    vPos.z += (flMaxZ * 0.5f);
}

bool CAimbotProjectile::CanSee(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const Vec3& vFrom, const Vec3& vTo, const ProjTarget_t& target, float flTargetTime)
{
    if (!pLocal || !pWeapon || !target.Entity)
        return false;

    int idx = target.Entity->entindex();
    if (idx <= 0 || idx > 64)
        return false;

    C_BaseEntity* pCheck = reinterpret_cast<C_BaseEntity*>(I::ClientEntityList->GetClientEntity(idx));
    if (!pCheck || pCheck != target.Entity)
        return false;

    CGameTrace trace;
    CTraceFilterWorldCustom filter;
    filter.m_pTarget = target.Entity;

    Vec3 vHull = Vec3(3, 3, 3); // Padrão (Hitscan/Setas)
    if (pWeapon)
    {
        switch (pWeapon->GetWeaponID())
        {
        case TF_WEAPON_ROCKETLAUNCHER:
        case TF_WEAPON_PARTICLE_CANNON:
        case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
            // MODO AGRESSIVO: 6.0f
            // Permite que o Rocket passe por frestas onde a linha central passa.
            // Pode acertar o frame da janela, mas exploda o alvo se passar.
            vHull = Vec3(6.0f, 6.0f, 6.0f);
            break;
        case TF_WEAPON_GRENADELAUNCHER:
        case TF_WEAPON_PIPEBOMBLAUNCHER:
        case TF_WEAPON_CANNON:
            // MODO AGRESSIVO: 4.0f
            // Pipes também podem passar em frestas apertadas.
            vHull = Vec3(4.0f, 4.0f, 4.0f);
            break;
        case TF_WEAPON_COMPOUND_BOW:
        case TF_WEAPON_CROSSBOW:
            vHull = Vec3(2.0f, 2.0f, 2.0f);
            break;
        default:
            vHull = Vec3(3.0f, 3.0f, 3.0f);
            break;
        }
    }

    Ray_t ray;
    ray.Init(vFrom, vTo, vHull * -1.0f, vHull);
    I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &trace);

    // Tolerância aumentada para 0.98 (permite raspões)
    if (trace.fraction >= 0.98f)
        return true;

    if (trace.m_pEnt && trace.m_pEnt == target.Entity)
        return true;

    if (trace.surface.flags & SURF_SKY)
        return false;

    return false;
}

bool CAimbotProjectile::SolveTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const CUserCmd* pCmd, ProjTarget_t& target)
{
    if (!pLocal || !pWeapon || !pCmd || !target.Entity)
        return false;

    int idx = target.Entity->entindex();
    if (idx <= 0 || idx > 64)
        return false;

    C_BaseEntity* pCheck = reinterpret_cast<C_BaseEntity*>(I::ClientEntityList->GetClientEntity(idx));
    if (!pCheck || pCheck != target.Entity)
        return false;

    Vec3 vLocalPos = pLocal->GetShootPos();

    if (m_CurProjInfo.Pipes)
    {
        const Vec3 vOffset = { 16.0f, 8.0f, -6.0f };
        H::AimUtils->GetProjectileFireSetup(pCmd->viewangles, vOffset, &vLocalPos);
    }

    m_TargetPath.clear();

    // ========================================
    // OTIMIZAÇÃO: Dynamic Simulation Limit
    // ========================================
    float flDistToTarget = vLocalPos.DistTo(target.Position);
    float flEstTimeToTarget = flDistToTarget / m_CurProjInfo.Speed;

    float flMaxSimTime = flEstTimeToTarget + 0.3f;

    if (flMaxSimTime > CFG::Aimbot_Projectile_Max_Simulation_Time)
        flMaxSimTime = CFG::Aimbot_Projectile_Max_Simulation_Time;

    if (m_CurProjInfo.Speed > 1000.0f && flMaxSimTime > 1.5f)
        flMaxSimTime = 1.5f;

    int nMaxTicks = TIME_TO_TICKS(flMaxSimTime);

    // ========================================
    // DEFINIÇÃO DA ESTRUTURA (Movida para fora do loop se possível, mas aqui está ok)
    // ========================================
    struct PointData_t
    {
        Vec3 vPos;
        int nPriority; // 3=Head, 2=Neck, 1=Body, 0=Feet
    };

    // ========================================
    // HELPERS
    // ========================================

    // 1. Verificação rápida de visibilidade (Ray Trace)
    auto IsPointVisible = [&](const Vec3& vStart, const Vec3& vEnd) -> bool
        {
            trace_t tr;
            Ray_t ray;
            CTraceFilterWorldOnly filter; // Ignora o inimigo, bate só no mundo
            ray.Init(vStart, vEnd);
            I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &tr);
            return tr.fraction >= 0.98f; // Se não bateu na parede, está visível
        };

    // 2. Estrutura de Candidatos
    struct CandidateShot_t
    {
        Vec3 vPos;
        Vec3 vAngle;
        float flTime;
        float flFOV;
    };
    std::vector<CandidateShot_t> vCandidates;
    vCandidates.reserve(64); // Reserva memória

    // ========================================
    // FASE 1: SIMULAÇÃO E FILTRO (Otimizado)
    // ========================================
    if (target.Entity->GetClassId() == ETFClassIds::CTFPlayer)
    {
        const auto pPlayer = target.Entity->As<C_TFPlayer>();
        if (!pPlayer)
            return false;

        const bool bDucked = pPlayer->m_fFlags() & FL_DUCKING;
        const bool bOnGround = pPlayer->m_fFlags() & FL_ONGROUND;

        if (!F::MovementSimulation->Initialize(pPlayer))
            return false;

        for (int nTick = 0; nTick <= nMaxTicks; nTick++)
        {
            // Simula movimento do INIMIGO
            if (nTick > 0)
            {
                F::MovementSimulation->RunTick();
            }

            Vec3 simOrigin = F::MovementSimulation->GetOrigin();
            if (isnan(simOrigin.x) || isnan(simOrigin.y) || isnan(simOrigin.z) ||
                isinf(simOrigin.x) || isinf(simOrigin.y) || isinf(simOrigin.z))
                break;

            m_TargetPath.push_back(simOrigin);

            // ========================================
            // GERAÇÃO DE HITBOXES (HEAD/NECK/BODY/FEET)
            // ========================================
            const float flMaxZ{ (bDucked ? 62.0f : 82.0f) * pPlayer->m_flModelScale() };
            std::vector<PointData_t> vAimPoints;

            // --- PRIORIDADE 3: CABEÇA ---
            if (CFG::Aimbot_Projectile_AimPosition == 2 || CFG::Aimbot_Projectile_AimPosition == 3)
            {
                Vec3 vHead = pPlayer->GetHitboxPos(HITBOX_HEAD);
                if (!vHead.IsZero())
                {
                    // FIX COMPILADOR: Variável temporária
                    PointData_t pHead;
                    pHead.vPos = vHead;
                    pHead.nPriority = 3;
                    vAimPoints.push_back(pHead);

                    // --- PRIORIDADE 2: PESCOÇO (FALLBACK DA CABEÇA) ---
                    PointData_t pNeck;
                    pNeck.vPos = { vHead.x, vHead.y, vHead.z - 5.0f };
                    pNeck.nPriority = 2;
                    vAimPoints.push_back(pNeck);
                }
                else
                {
                    // Fallback Offset
                    PointData_t pFallbackHead;
                    pFallbackHead.vPos = { simOrigin.x, simOrigin.y, simOrigin.z + flMaxZ * 0.92f };
                    pFallbackHead.nPriority = 3;
                    vAimPoints.push_back(pFallbackHead);
                }
            }

            // --- PRIORIDADE 1: CORPO ---
            if (CFG::Aimbot_Projectile_AimPosition == 1 || CFG::Aimbot_Projectile_AimPosition == 3)
            {
                PointData_t pBody;
                pBody.vPos = { simOrigin.x, simOrigin.y, simOrigin.z + flMaxZ * 0.5f };
                pBody.nPriority = 1;
                vAimPoints.push_back(pBody);
            }

            // --- PRIORIDADE 0: PÉS ---
            if (CFG::Aimbot_Projectile_AimPosition == 0 || CFG::Aimbot_Projectile_AimPosition == 3)
            {
                PointData_t pFeet;
                pFeet.vPos = { simOrigin.x, simOrigin.y, simOrigin.z + flMaxZ * 0.2f };
                pFeet.nPriority = 0;
                vAimPoints.push_back(pFeet);
            }

            // ========================================
            // FILTRO DE VISIBILIDADE (SMART PRIORITY)
            // ========================================
            std::vector<PointData_t> vVisiblePoints;

            for (const auto& point : vAimPoints)
            {
                // LÓGICA INTELIGENTE DE PESCOÇO
                if (point.nPriority == 3)
                {
                    if (IsPointVisible(vLocalPos, point.vPos))
                    {
                        vVisiblePoints.push_back(point);
                    }
                }
                else if (point.nPriority == 2) // Pescoço
                {
                    if (IsPointVisible(vLocalPos, point.vPos))
                    {
                        vVisiblePoints.push_back(point);
                    }
                }
                else // Corpo e Pés
                {
                    if (IsPointVisible(vLocalPos, point.vPos))
                    {
                        vVisiblePoints.push_back(point);
                    }
                }
            }

            // Se nada estiver visível, pula a física deste tick (Economia de CPU)
            if (vVisiblePoints.empty())
                continue;

            // ========================================
            // MATEMÁTICA APENAS NOS VISÍVEIS
            // ========================================
            for (const auto& point : vVisiblePoints)
            {
                Vec3 vAngleOut;
                float flTimeToTarget = 0.0f;

                if (CalcProjAngle(vLocalPos, point.vPos, vAngleOut, flTimeToTarget, false))
                {
                    // Verificação de Sticky Arm
                    int nTargetTick = TIME_TO_TICKS(flTimeToTarget + SDKUtils::GetLatency());
                    if (pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
                    {
                        const auto sticky_arm_time = SDKUtils::AttribHookValue(0.8f, "sticky_arm_time", pLocal);
                        if (TICKS_TO_TIME(nTargetTick) < sticky_arm_time)
                            nTargetTick += TIME_TO_TICKS(fabsf(flTimeToTarget - sticky_arm_time));
                    }

                    // Verificação de Tick
                    if (nTargetTick == nTick || nTargetTick == nTick + 1 || nTargetTick == nTick - 1)
                    {
                        // Verificação FOV
                        Vec3 vRealView = I::EngineClient->GetViewAngles();
                        float flFOV = Math::CalcFov(vRealView, vAngleOut);

                        if (flFOV < CFG::Aimbot_Projectile_FOV)
                        {
                            // Bônus de Prioridade: Cabeça > Pescoço > Corpo
                            float flScore = flFOV;
                            if (point.nPriority == 3) flScore -= 1.0f; // Cabeça super prioridade
                            else if (point.nPriority == 2) flScore -= 0.5f; // Pescoço prioridade alta

                            vCandidates.push_back({ point.vPos, vAngleOut, flTimeToTarget, flScore });
                        }
                    }
                }
            }
        }
        F::MovementSimulation->Restore();
    }
    else // BUILDINGS
    {
        const Vec3 vTarget = target.Position;
        Vec3 vAngleOut;
        float flTimeToTarget = 0.0f;

        if (CalcProjAngle(vLocalPos, vTarget, vAngleOut, flTimeToTarget, false))
        {
            Vec3 vRealView = I::EngineClient->GetViewAngles();
            float flFOV = Math::CalcFov(vRealView, vAngleOut);

            if (flFOV < CFG::Aimbot_Projectile_FOV)
            {
                vCandidates.push_back({ vTarget, vAngleOut, flTimeToTarget, flFOV });
            }
        }
    }

    // ========================================
    // FASE 2: ORDENAÇÃO (Fim da Simulação)
    // ========================================
    if (vCandidates.empty())
        return false;

    // Ordenar por Score (FOV + Prioridade de Hitbox)
    std::sort(vCandidates.begin(), vCandidates.end(), [](const CandidateShot_t& a, const CandidateShot_t& b) {
        return a.flFOV < b.flFOV;
        });

    // ========================================
    // FASE 3: VERIFICAÇÃO FINAL (Trace Pesado - Sem Peek Check)
    // ========================================
    for (const auto& candidate : vCandidates)
    {
        // REMOVIDO: Peek Check. Agora só verificamos se a linha chega lá.
        if (CanSee(pLocal, pWeapon, vLocalPos, candidate.vPos, target, candidate.flTime))
        {
            target.Position = candidate.vPos;
            target.AngleTo = candidate.vAngle;
            target.TimeToTarget = candidate.flTime;
            return true;
        }
    }

    return false;
}

bool CAimbotProjectile::GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const CUserCmd* pCmd, ProjTarget_t& outTarget)
{
    const Vec3 vLocalPos = pLocal->GetShootPos();
    const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();
    m_vecTargets.clear();

    // ========== PLAYERS ==========
    if (CFG::Aimbot_Target_Players)
    {
        const auto nGroup = pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW ? EEntGroup::PLAYERS_ALL : EEntGroup::PLAYERS_ENEMIES;

        for (const auto pEntity : H::Entities->GetGroup(nGroup))
        {
            if (!pEntity || pEntity == pLocal)
                continue;

            const auto pPlayer = pEntity->As<C_TFPlayer>();
            if (pPlayer->deadflag() || pPlayer->InCond(TF_COND_HALLOWEEN_GHOST_MODE))
                continue;

            // Verificação de Time/Amigos/Condições
            if (pPlayer->m_iTeamNum() != pLocal->m_iTeamNum())
            {
                if (CFG::Aimbot_Ignore_Friends && pPlayer->IsPlayerOnSteamFriendsList())
                    continue;
                if (CFG::Aimbot_Ignore_Invisible && pPlayer->IsInvisible())
                    continue;
                if (CFG::Aimbot_Ignore_Invulnerable && pPlayer->IsInvulnerable())
                    continue;
                if (CFG::Aimbot_Ignore_Taunting && pPlayer->InCond(TF_COND_TAUNTING))
                    continue;
            }
            else
            {
                if (pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW)
                {
                    if (pPlayer->m_iHealth() >= pPlayer->GetMaxHealth() || pPlayer->IsInvulnerable())
                        continue;
                }
                else
                {
                    continue; // Não mira em aliados a não ser com crossbow
                }
            }

            Vec3 vPos = pPlayer->GetCenter();

            // OTIMIZAÇÃO: Check de distância rápido antes de criar o struct
            float flDistTo = vLocalPos.DistTo(vPos);
            if (flDistTo > m_CurProjInfo.Speed * 2.0f) // Ignora se estiver muito longe para a velocidade do projétil
                continue;

            Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
            const float flFOVTo = CFG::Aimbot_Projectile_Sort == 0 ? Math::CalcFov(vLocalAngles, vAngleTo) : 0.0f;

            if (CFG::Aimbot_Projectile_Sort == 0 && flFOVTo > CFG::Aimbot_Projectile_FOV)
                continue;

            m_vecTargets.emplace_back(ProjTarget_t{ pPlayer, vPos, vAngleTo, flFOVTo, flDistTo });
        }
    }

    // ========== BUILDINGS ==========
    if (CFG::Aimbot_Target_Buildings)
    {
        for (const auto pEntity : H::Entities->GetGroup(EEntGroup::BUILDINGS_ENEMIES))
        {
            if (!pEntity || pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
                continue;

            Vec3 vPos = pEntity->GetCenter();
            float flDistTo = vLocalPos.DistTo(vPos);

            // OTIMIZAÇÃO: Distância
            if (flDistTo > m_CurProjInfo.Speed * 2.0f)
                continue;

            Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
            const float flFOVTo = CFG::Aimbot_Projectile_Sort == 0 ? Math::CalcFov(vLocalAngles, vAngleTo) : 0.0f;

            if (CFG::Aimbot_Projectile_Sort == 0 && flFOVTo > CFG::Aimbot_Projectile_FOV)
                continue;

            m_vecTargets.emplace_back(ProjTarget_t{ pEntity, vPos, vAngleTo, flFOVTo, flDistTo });
        }
    }

    if (m_vecTargets.empty())
        return false;

    // ========== SORTING ==========
    switch (CFG::Aimbot_Projectile_Sort)
    {
    case 0: // FOV
        std::sort(m_vecTargets.begin(), m_vecTargets.end(),
            [](const ProjTarget_t& a, const ProjTarget_t& b) { return a.FOV < b.FOV; });
        break;
    case 1: // Distance
        std::sort(m_vecTargets.begin(), m_vecTargets.end(),
            [](const ProjTarget_t& a, const ProjTarget_t& b) { return a.DistTo < b.DistTo; });
        break;
    }

    // ========== SOLVE TARGETS ==========
    for (auto& target : m_vecTargets)
    {
        // A Função SolveTarget agora é muito mais rápida
        if (SolveTarget(pLocal, pWeapon, pCmd, target))
        {
            if (CFG::Aimbot_Projectile_Sort == 0 && Math::CalcFov(vLocalAngles, target.AngleTo) > CFG::Aimbot_Projectile_FOV)
                continue;

            outTarget = target;
            return true;
        }
    }

    return false;
}

bool CAimbotProjectile::ShouldAimKey()
{
    if (!CFG::Aimbot_Projectile_Active)
        return false;

    int mode = CFG::Aimbot_Projectile_KeyMode;
    int key = CFG::Aimbot_Key;

    bool bKeyDown = (key > 0 && (GetAsyncKeyState(key) & 0x8000) != 0);
    static bool bToggleState = false;
    static bool bLastKeyDown = false;

    bool bActive = false;

    switch (mode)
    {
    case 0: // Always
        bActive = true;
        break;
    case 1: // Hold
        bActive = bKeyDown;
        break;
    case 2: // Toggle
        if (bKeyDown && !bLastKeyDown)
            bToggleState = !bToggleState;
        bActive = bToggleState;
        break;
    case 3: // Hold Off
        bActive = !bKeyDown;
        break;
    }

    bLastKeyDown = bKeyDown;
    return bActive;
}

bool CAimbotProjectile::ShouldAim(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    return CFG::Aimbot_Projectile_Mode != 1 || IsFiring(pCmd, pLocal, pWeapon) && pWeapon->HasPrimaryAmmoForShot();
}

void CAimbotProjectile::Aim(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const Vec3& vAngles)
{
    Vec3 vAngleTo = vAngles - pLocal->m_vecPunchAngle();

    if (m_CurProjInfo.Pipes)
    {
        Vec3 vAngle = {}, vForward = {}, vUp = {};
        Math::AngleVectors(vAngleTo, &vForward, nullptr, &vUp);
        const Vec3 vVelocity = (vForward * m_CurProjInfo.Speed) - (vUp * 200.0f);
        Math::VectorAngles(vVelocity, vAngle);
        vAngleTo.x = vAngle.x;
    }

    Math::ClampAngles(vAngleTo);

    switch (CFG::Aimbot_Projectile_Mode)
    {
    case 0: // Plain
        pCmd->viewangles = vAngleTo;
        break;

    case 1: // Silent
        if (m_CurProjInfo.Flamethrower ? true : G::bCanPrimaryAttack)
        {
            H::AimUtils->FixMovement(pCmd, vAngleTo);
            pCmd->viewangles = vAngleTo;

            if (m_CurProjInfo.Flamethrower)
                G::bSilentAngles = true;
            else
                G::bPSilentAngles = true;
        }
        break;
    }
}

bool CAimbotProjectile::ShouldFire(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    if (!CFG::Aimbot_AutoShoot)
    {
        if (pWeapon->GetWeaponID() == TF_WEAPON_FLAME_BALL && pLocal->m_flTankPressure() < 100.0f)
            pCmd->buttons &= ~IN_ATTACK;
        return false;
    }
    return true;
}

void CAimbotProjectile::HandleFire(CUserCmd* pCmd, C_TFWeaponBase* pWeapon, C_TFPlayer* pLocal, const ProjTarget_t& target)
{
    const bool bIsBazooka = pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka;

    if (!bIsBazooka && !pWeapon->HasPrimaryAmmoForShot())
        return;

    const int nWeaponID = pWeapon->GetWeaponID();

    if (!IsChargingWeapon(nWeaponID))
    {
        pCmd->buttons |= IN_ATTACK;
    }
    else
    {
        float requiredCharge = GetRequiredChargeTime(pWeapon, target, pLocal->GetShootPos());
        float currentCharge = GetCurrentChargeTime(pWeapon);

        if (currentCharge < requiredCharge)
            pCmd->buttons |= IN_ATTACK;
        else
            pCmd->buttons &= ~IN_ATTACK;
    }

    if (bIsBazooka && pWeapon->HasPrimaryAmmoForShot())
        pCmd->buttons &= ~IN_ATTACK;
}

bool CAimbotProjectile::IsFiring(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    if (!pWeapon->HasPrimaryAmmoForShot())
        return false;

    const int nWeaponID = pWeapon->GetWeaponID();

    if (IsChargingWeapon(nWeaponID))
        return (G::nOldButtons & IN_ATTACK) && !(pCmd->buttons & IN_ATTACK);

    if (nWeaponID == TF_WEAPON_FLAME_BALL)
        return pLocal->m_flTankPressure() >= 100.0f && (pCmd->buttons & IN_ATTACK);

    if (pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka)
        return G::bCanPrimaryAttack;

    if (nWeaponID == TF_WEAPON_FLAMETHROWER)
        return pCmd->buttons & IN_ATTACK;

    return (pCmd->buttons & IN_ATTACK) && G::bCanPrimaryAttack;
}

Vec3 CAimbotProjectile::GetWeaponFireOffset(C_TFWeaponBase* pWeapon, C_TFPlayer* pLocal)
{
    return Vec3(0.0f, 0.0f, 0.0f);
}

void CAimbotProjectile::Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    // ========================================
    // 1. ATIVAÇÃO E CACHING
    // ========================================
    if (!CFG::Aimbot_Projectile_Active)
        return;

    // Limpa o cache de ARC se a arma trocar
    static C_TFWeaponBase* s_pLastWeapon = nullptr;
    if (s_pLastWeapon != pWeapon)
    {
        m_ArcCache = {};
        s_pLastWeapon = pWeapon;
    }

    // Busca as informações do projétil (Velocidade, Gravidade, etc)
    if (!GetProjectileInfo(pWeapon))
        return;

    // Atualiza o FOV global para a ESP/Indicadores
    if (CFG::Aimbot_Projectile_Sort == 0)
        G::flAimbotFOV = CFG::Aimbot_Projectile_FOV;

    // ========================================
    // 2. EXPLOITS (SHIFTING)
    // ========================================
    if (Shifting::bShifting && !Shifting::bShiftingWarp)
        return;

    // ========================================
    // 3. VERIFICAÇÃO DE KEYBIND (LIMPEZA)
    // ========================================
    // Removemos o bloco switch/case gigante que estava aqui.
    // Essa função cuida de Toggle/Hold/Always/Hold-Off automaticamente.
    if (!ShouldAimKey())
        return;

    // ========================================
    // 4. BUSCA DE ALVO (GET TARGET)
    // ========================================
    // O GetTarget agora contém o SolveTarget com Smart Priority e Optimization.
    ProjTarget_t target = {};

    if (GetTarget(pLocal, pWeapon, pCmd, target) && target.Entity)
    {
        // Define índices globais para ESP/Indicadores
        G::nTargetIndexEarly = target.Entity->entindex();
        G::nTargetIndex = target.Entity->entindex();

        // ========================================
        // 5. LÓGICA DE TIRO (AUTOFIRE)
        // ========================================
        if (ShouldFire(pCmd, pLocal, pWeapon))
            HandleFire(pCmd, pWeapon, pLocal, target);

        const bool bIsFiring = IsFiring(pCmd, pLocal, pWeapon);
        G::bFiring = bIsFiring;

        if (ShouldAim(pCmd, pLocal, pWeapon) || bIsFiring ||
            (IsChargingWeapon(pWeapon->GetWeaponID()) && (pCmd->buttons & IN_ATTACK)))
        {
            Aim(pCmd, pLocal, pWeapon, target.AngleTo);

            if (bIsFiring && m_TargetPath.size() > 1)
            {
                // Limpa overlays antigos para não poluir a tela
                I::DebugOverlay->ClearAllOverlays();
                DrawMovePath(m_TargetPath);
                m_TargetPath.clear();
            }
        }
    }
}