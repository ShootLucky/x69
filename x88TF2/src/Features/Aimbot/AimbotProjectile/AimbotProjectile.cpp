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

// ========== FUNÇÃO CalculateAngle DO AMALGAM ==========
void CAimbotProjectile::CalculateAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, int iSimTime, Solution_t& out, bool bAccuracy)
{
    if (out.m_iCalculated != CalculatedEnum::Pending)
        return;

    const float flGrav = m_tInfo.m_flGravity * 800.f;  // m_flGravity agora é o modifier (compatível com ProjectileSim)

    float flPitch, flYaw;
    {
        // Basic trajectory pass
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
            // Arch calculation
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
        // Calculate trajectory from projectile origin
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
        // Correct yaw
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
        // Correct pitch
        if (flGrav)
        {
            flPitch -= tProjInfo.m_ang.x;
            out.m_flPitch = -RAD2DEG(out.m_flPitch) + flPitch - m_tInfo.m_vAngFix.x;
        }
        else
        {
            Vec3 temp2d = tProjInfo.m_pos - vLocalPos; Vec2 rotated = Math::RotatePoint(Vec2(temp2d.x, temp2d.y), Vec2(0, 0), -flYaw); Vec3 vShootPos = Vec3(rotated.x, rotated.y, temp2d.z);
            vShootPos.y = 0;

            Vec3 tempTgt = vTargetPos - vLocalPos; Vec2 rotatedTgt = Math::RotatePoint(Vec2(tempTgt.x, tempTgt.y), Vec2(0, 0), -flYaw); Vec3 vTarget = Vec3(rotatedTgt.x, rotatedTgt.y, tempTgt.z);
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

// ========================================
// CORREÇÕES APLICADAS EM AimbotProjectile.cpp
// Linha 437-488: Função CanHit()
// ========================================

int CAimbotProjectile::CanHit(ProjTarget_t& tTarget, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, bool bVisuals)
{
    ProjectileInfo tProjInfo = {};
    if (!F::ProjectileSim->GetInfo(pLocal, pWeapon, {}, tProjInfo)
        || !F::ProjectileSim->Init(tProjInfo, false))
        return false;

    // ✅ CORRIGIDO: Removido "PlayerStorage tStorage" (não existe)
    // ✅ CORRIGIDO: Initialize() aceita apenas 1 parâmetro
    if (!F::MovementSimulation->Initialize(tTarget.Entity->As<C_TFPlayer>()))
        return false;

    // ✅ REMOVIDO: Verificação de tStorage.m_bInitialized (não existe)

    tTarget.Position = tTarget.Entity->m_vecOrigin();

    m_tInfo = { pLocal, pWeapon };
    m_tInfo.m_vLocalEye = pLocal->GetShootPos();
    m_tInfo.m_vTargetEye = tTarget.Entity->As<C_TFPlayer>()->m_vecViewOffset();
    m_tInfo.m_flLatency = 0.0f + TICKS_TO_TIME(0);

    m_tInfo.m_flVelocity = m_CurProjInfo.Speed;
    m_tInfo.m_vAngFix = {};

    // Hull fix: valor fixo baseado nos bbox do ProjectileSim.cpp
    m_tInfo.m_vHull = Vec3(3.f, 3.f, 3.f);

    m_tInfo.m_vOffset = tProjInfo.m_pos - m_tInfo.m_vLocalEye;
    m_tInfo.m_vOffset.y *= -1;
    m_tInfo.m_flOffsetTime = m_tInfo.m_vOffset.Length() / m_tInfo.m_flVelocity;

    // Gravity fix
    m_tInfo.m_flGravity = tProjInfo.m_gravity_mod;

    Vec3 vMins = tTarget.Entity->m_vecMins();
    Vec3 vMaxs = tTarget.Entity->m_vecMaxs();
    float flSize = (vMaxs - vMins).Length();

    // ✅ CORRIGIDO: Tipo de int para bool
    bool iReturn = false;
    int iMaxTime = TIME_TO_TICKS(CFG::Aimbot_Projectile_Max_Simulation_Time);

    Vec3 vAngleTo, vPredicted, vTarget;
    int iLowestPriority = std::numeric_limits<int>::max();
    float flLowestDist = std::numeric_limits<float>::max();

    for (int i = 1 - TIME_TO_TICKS(m_tInfo.m_flLatency); i <= iMaxTime; i++)
    {
        // ✅ CORRIGIDO: Removido if(!false) - condição inútil
        // ✅ CORRIGIDO: RunTick() sem parâmetros (aceita apenas float opcional)
        if (i > 0)
        {
            F::MovementSimulation->RunTick();

            // ✅ REMOVIDO: Verificação de tStorage.m_bInitialized (não existe)

            // ✅ CORRIGIDO: Usar método público GetSimulatedOrigin()
            // Antes: tTarget.Position = tStorage.m_MoveData.m_vecAbsOrigin;
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

    // ✅ REMOVIDO: Comentário sobre Storage cleanup (não existe mais tStorage)

    if (iReturn)
    {
        tTarget.Position = vTarget;
        tTarget.AngleTo = vAngleTo;
    }

    return iReturn;
}

bool CAimbotProjectile::CalcProjAngle(const Vec3& vFrom, const Vec3& vTo, Vec3& vAngleOut, float& flTimeOut, bool bHighArc)
{
    const auto pWeapon = H::Entities->GetWeapon();
    if (!pWeapon)
        return false;

    const Vec3 v = vTo - vFrom;
    const float dx = sqrt(v.x * v.x + v.y * v.y);
    const float dy = v.z;
    float v0 = m_CurProjInfo.Speed;
    const float g = SDKUtils::GetGravity() * m_CurProjInfo.GravityMod;

    if (g)
    {
        if (m_CurProjInfo.Pipes)
        {
            if (v0 > k_flMaxVelocity)
                v0 = k_flMaxVelocity;
        }

        const auto root{ v0 * v0 * v0 * v0 - g * (g * dx * dx + 2.0f * dy * v0 * v0) };
        if (root < 0.0f)
            return false;

        const float sign = bHighArc ? +1.0f : -1.0f;
        const float theta = atanf((v0 * v0 + sign * sqrtf(root)) / (g * dx));
        vAngleOut = { -RAD2DEG(theta), RAD2DEG(atan2f(v.y, v.x)), 0.0f };
        flTimeOut = dx / (cosf(theta) * v0);

        if (m_CurProjInfo.Pipes)
        {
            // 2nd pass for drag
            auto magic{ 0.0f };
            if (pWeapon->GetWeaponID() == TF_WEAPON_GRENADELAUNCHER)
            {
                if (pWeapon->m_iItemDefinitionIndex() == Demoman_m_TheLochnLoad)
                    magic = 0.07f;
                else
                    magic = 0.11f;
            }
            if (pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
                magic = 0.16f;

            if (pWeapon->GetWeaponID() == TF_WEAPON_CANNON)
                magic = 0.35f;

            v0 -= (v0 * flTimeOut) * magic;
            auto root{ v0 * v0 * v0 * v0 - g * (g * dx * dx + 2.0f * dy * v0 * v0) };

            if (root < 0.0f)
                return false;

            const float theta = atanf((v0 * v0 + sign * sqrtf(root)) / (g * dx));
            vAngleOut = { -RAD2DEG(theta), RAD2DEG(atan2f(v.y, v.x)), 0.0f };
            flTimeOut = dx / (cosf(theta) * v0);
        }
    }
    else
    {
        vAngleOut = Math::CalcAngle(vFrom, vTo);
        flTimeOut = vFrom.DistTo(vTo) / v0;
    }

    if (m_CurProjInfo.Pipes)
    {
        if (pWeapon->GetWeaponID() == TF_WEAPON_CANNON)
        {
            if (flTimeOut > 0.95f)
                return false;
        }
        else
        {
            if (pWeapon->m_iItemDefinitionIndex() == Demoman_m_TheIronBomber)
            {
                if (flTimeOut > 1.4f)
                    return false;
            }
            else
            {
                if (flTimeOut > 2.0f)
                    return false;
            }
        }
    }

    if ((pWeapon->GetWeaponID() == TF_WEAPON_FLAME_BALL || pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER) && flTimeOut > 0.18f)
        return false;

    return true;
}

void CAimbotProjectile::OffsetPlayerPosition(C_TFWeaponBase* pWeapon, Vec3& vPos, C_TFPlayer* pPlayer, bool bDucked, bool bOnGround)
{
    const float flMaxZ{ (bDucked ? 62.0f : 82.0f) * pPlayer->m_flModelScale() };

    switch (CFG::Aimbot_Projectile_AimPosition)
    {
    case 0: // Feet
        vPos.z += (flMaxZ * 0.2f);
        m_LastAimPos = 0;
        break;

    case 1: // Body
        vPos.z += (flMaxZ * 0.5f);
        m_LastAimPos = 1;
        break;

    case 2: // Head
        if (CFG::Aimbot_Projectile_Advanced_Head_Aim)
        {
            const Vec3 vDelta = pPlayer->GetHitboxPos(HITBOX_HEAD) - pPlayer->m_vecOrigin();
            vPos.x += vDelta.x;
            vPos.y += vDelta.y;
        }
        vPos.z += (flMaxZ * 0.85f);
        m_LastAimPos = 2;
        break;

    case 3: // Auto
        if (pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW)
        {
            if (CFG::Aimbot_Projectile_Advanced_Head_Aim)
            {
                const Vec3 vDelta = pPlayer->GetHitboxPos(HITBOX_HEAD) - pPlayer->m_vecOrigin();
                vPos.x += vDelta.x;
                vPos.y += vDelta.y;
            }
            vPos.z += (flMaxZ * 0.92f);
            m_LastAimPos = 2;
        }
        else
        {
            switch (pWeapon->GetWeaponID())
            {
            case TF_WEAPON_ROCKETLAUNCHER:
            case TF_WEAPON_PARTICLE_CANNON:
            case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
            case TF_WEAPON_GRENADELAUNCHER:
            case TF_WEAPON_CANNON:
                if (bOnGround)
                {
                    vPos.z += (flMaxZ * 0.2f);
                    m_LastAimPos = 0;
                }
                else
                {
                    vPos.z += (flMaxZ * 0.5f);
                    m_LastAimPos = 1;
                }
                break;
            default:
                vPos.z += (flMaxZ * 0.5f);
                m_LastAimPos = 1;
            }
        }
        break;
    }
}

bool CAimbotProjectile::CanArcReach(const Vec3& vFrom, const Vec3& vTo, const Vec3& vAngleTo, float flTargetTime, C_BaseEntity* pTarget)
{
    // Verifica se o projétil pode alcançar o alvo considerando a trajetória em arco
    const auto pWeapon = H::Entities->GetWeapon();
    if (!pWeapon)
        return false;

    // Se não tem gravidade, sempre pode alcançar (linha reta)
    if (!m_CurProjInfo.GravityMod)
        return true;

    const Vec3 v = vTo - vFrom;
    const float dx = sqrt(v.x * v.x + v.y * v.y);
    const float dy = v.z;
    const float v0 = m_CurProjInfo.Speed;
    const float g = SDKUtils::GetGravity() * m_CurProjInfo.GravityMod;

    // Verifica se a equação tem solução real (pode alcançar o alvo)
    const float root = v0 * v0 * v0 * v0 - g * (g * dx * dx + 2.0f * dy * v0 * v0);

    if (root < 0.0f)
        return false; // Não pode alcançar

    // Calcula o tempo de voo
    const float theta = atanf((v0 * v0 - sqrtf(root)) / (g * dx));
    const float flTime = dx / (cosf(theta) * v0);

    // Verifica se o tempo é compatível com o tempo esperado
    const float flTimeDiff = fabsf(flTime - flTargetTime);

    return flTimeDiff < TICK_INTERVAL * 2; // Tolerância de 2 ticks
}

bool CAimbotProjectile::CanSee(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const Vec3& vFrom, const Vec3& vTo, const ProjTarget_t& target, float flTargetTime)
{
    // ✅ PROTEÇÃO CONTRA CRASH: Verifica ponteiros nulos
    if (!pLocal || !pWeapon || !target.Entity)
        return false;

    // ✅ PROTEÇÃO: Verifica se entidade ainda é válida (SEM GetDormant)
    int idx = target.Entity->entindex();
    if (idx <= 0 || idx > 64)
        return false;

    // Verifica se a entidade ainda existe
    C_BaseEntity* pCheck = reinterpret_cast<C_BaseEntity*>(I::ClientEntityList->GetClientEntity(idx));
    if (!pCheck || pCheck != target.Entity)
        return false;

    // Verifica se há linha de visão clara do ponto de origem ao alvo
    CGameTrace trace;
    CTraceFilterWorldCustom filter;
    filter.m_pTarget = target.Entity;

    // Trace hull para projéteis maiores (rockets, pipes)
    Vec3 vHull = Vec3(3, 3, 3);
    if (pWeapon)
    {
        switch (pWeapon->GetWeaponID())
        {
        case TF_WEAPON_ROCKETLAUNCHER:
        case TF_WEAPON_PARTICLE_CANNON:
        case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
        case TF_WEAPON_GRENADELAUNCHER:
        case TF_WEAPON_PIPEBOMBLAUNCHER:
        case TF_WEAPON_CANNON:
            vHull = Vec3(4, 4, 4);
            break;
        case TF_WEAPON_COMPOUND_BOW:
        case TF_WEAPON_CROSSBOW:
            vHull = Vec3(1, 1, 1);
            break;
        default:
            vHull = Vec3(2, 2, 2);
            break;
        }
    }

    // Usar TraceHull do EngineTrace
    Ray_t ray;
    ray.Init(vFrom, vTo, vHull * -1.0f, vHull);

    I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &trace);

    // Se não acertou nada, caminho livre
    if (trace.fraction >= 0.99f)
        return true;

    // Se acertou o próprio alvo, ok
    if (trace.m_pEnt && trace.m_pEnt == target.Entity)
        return true;

    // Verificação adicional para céu (skybox)
    if (trace.surface.flags & SURF_SKY)
        return false;

    // Se acertou algo sólido antes de chegar, bloqueado
    return false;
}

bool CAimbotProjectile::TrySplashShot(
    C_TFPlayer* pLocal,
    C_TFWeaponBase* pWeapon,
    const CUserCmd* pCmd,
    const ProjTarget_t& target,
    Vec3& outAngle,
    float& outTime,
    bool isPlayer)
{
    // ✅ PROTEÇÃO CONTRA CRASH: Verifica ponteiros nulos
    if (!pLocal || !pWeapon || !pCmd || !target.Entity)
        return false;

    // ✅ PROTEÇÃO: Verifica se entidade ainda é válida (SEM GetDormant)
    int idx = target.Entity->entindex();
    if (idx <= 0 || idx > 64)
        return false;

    // Verifica se a entidade ainda existe
    C_BaseEntity* pEntity = reinterpret_cast<C_BaseEntity*>(I::ClientEntityList->GetClientEntity(idx));
    if (!pEntity || pEntity != target.Entity)
        return false;

    // Verifica se splash bot está ativado
    if (!CFG::Aimbot_Projectile_SplashBot)
        return false;

    // Verifica se a arma tem splash damage
    float flSplashRadius = 0.0f;
    switch (pWeapon->GetWeaponID())
    {
    case TF_WEAPON_ROCKETLAUNCHER:
    case TF_WEAPON_PARTICLE_CANNON:
    case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
        flSplashRadius = 146.0f;
        break;
    case TF_WEAPON_GRENADELAUNCHER:
    case TF_WEAPON_PIPEBOMBLAUNCHER:
    case TF_WEAPON_CANNON:
        flSplashRadius = 146.0f;
        break;
    case TF_WEAPON_FLAREGUN:
    case TF_WEAPON_FLAREGUN_REVENGE:
        flSplashRadius = 110.0f;
        break;
    default:
        return false;
    }

    flSplashRadius *= CFG::Aimbot_Projectile_SplashRadius;

    if (flSplashRadius <= 0.0f)
        return false;

    Vec3 vLocalPos = pLocal->GetShootPos();
    Vec3 vTargetPos = target.Position;

    std::vector<Vec3> vSplashPoints;

    const int iPointCount = CFG::Aimbot_Projectile_SplashTestPoints;
    const float flRadius = CFG::Aimbot_Projectile_SplashMaxDist;

    // Pontos ao redor do alvo
    for (int i = 0; i < iPointCount; i++)
    {
        float flAngle = (360.0f / iPointCount) * i;
        float flRad = DEG2RAD(flAngle);
        Vec3 vOffset = Vec3(
            cosf(flRad) * flRadius,
            sinf(flRad) * flRadius,
            0.0f
        );

        Vec3 vTestPoint = vTargetPos + vOffset;

        CGameTrace trace;
        CTraceFilterWorldCustom filter;
        filter.m_pTarget = target.Entity;

        Ray_t ray;
        ray.Init(vTestPoint + Vec3(0, 0, 50), vTestPoint - Vec3(0, 0, 100));

        // ✅ PROTEÇÃO: Verifica se TraceRay é válido (sem try-catch que pode não compilar)
        if (!I::EngineTrace)
            continue;

        I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &trace);

        if (trace.fraction < 1.0f && !(trace.surface.flags & SURF_SKY))
        {
            float flDistToTarget = (trace.endpos - vTargetPos).Length();
            if (flDistToTarget <= flSplashRadius)
                vSplashPoints.push_back(trace.endpos);
        }
    }

    // Testa ponto embaixo do alvo
    CGameTrace groundTrace;
    CTraceFilterWorldCustom groundFilter;
    groundFilter.m_pTarget = target.Entity;

    Ray_t groundRay;
    groundRay.Init(vTargetPos, vTargetPos - Vec3(0, 0, 200));

    if (I::EngineTrace)
    {
        I::EngineTrace->TraceRay(groundRay, MASK_SOLID, &groundFilter, &groundTrace);

        if (groundTrace.fraction < 1.0f && !(groundTrace.surface.flags & SURF_SKY))
        {
            float flDistToTarget = (groundTrace.endpos - vTargetPos).Length();
            if (flDistToTarget <= flSplashRadius)
            {
                if (CFG::Aimbot_Projectile_SplashPrioritizeGround)
                    vSplashPoints.insert(vSplashPoints.begin(), groundTrace.endpos);
                else
                    vSplashPoints.push_back(groundTrace.endpos);
            }
        }
    }

    // ✅ PROTEÇÃO: Verifica se encontrou pontos
    if (vSplashPoints.empty())
        return false;

    // Testa cada ponto de splash
    for (const auto& vSplashPoint : vSplashPoints)
    {
        Vec3 vAngle;
        float flTime;

        if (!CalcProjAngle(vLocalPos, vSplashPoint, vAngle, flTime, false))
            continue;

        CGameTrace trace;
        CTraceFilterWorldCustom visFilter;
        visFilter.m_pTarget = nullptr;

        Ray_t visRay;
        visRay.Init(vLocalPos, vSplashPoint);

        if (!I::EngineTrace)
            continue;

        I::EngineTrace->TraceRay(visRay, MASK_SOLID, &visFilter, &trace);

        if (trace.fraction >= 0.95f || (trace.endpos - vSplashPoint).Length() < 32.0f)
        {
            float flSplashDist = (vSplashPoint - vTargetPos).Length();
            if (flSplashDist <= flSplashRadius)
            {
                if (CFG::Aimbot_Projectile_SplashUseNN)
                {
                    C_TFPlayer* pTargetPlayer = target.Entity->As<C_TFPlayer>();
                    if (pTargetPlayer && !NeuralNetworkSplashPrediction(vSplashPoint, pTargetPlayer))
                        continue;
                }

                outAngle = vAngle;
                outTime = flTime;
                return true;
            }
        }
    }

    return false;
}

bool CAimbotProjectile::SolveTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const CUserCmd* pCmd, ProjTarget_t& target)
{
    // ✅ PROTEÇÃO CONTRA CRASH
    if (!pLocal || !pWeapon || !pCmd || !target.Entity)
        return false;

    // ✅ PROTEÇÃO: Verifica se entidade ainda é válida (SEM GetDormant)
    int idx = target.Entity->entindex();
    if (idx <= 0 || idx > 64)
        return false;

    // Verifica se a entidade ainda existe
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

    if (target.Entity->GetClassId() == ETFClassIds::CTFPlayer)
    {
        const auto pPlayer = target.Entity->As<C_TFPlayer>();

        // ✅ PROTEÇÃO: Verifica se conversão foi bem-sucedida
        if (!pPlayer)
            return false;

        const bool bDucked = pPlayer->m_fFlags() & FL_DUCKING;
        const bool bOnGround = pPlayer->m_fFlags() & FL_ONGROUND;

        if (!F::MovementSimulation->Initialize(pPlayer))
            return false;

        // Verifica se a inicialização foi bem-sucedida
        Vec3 testOrigin = F::MovementSimulation->GetOrigin();
        if (isnan(testOrigin.x) || isnan(testOrigin.y) || isnan(testOrigin.z))
            return false;

        // ✅ PROTEÇÃO: Limite de ticks mais alto para alvos distantes
        float flMaxSimTime = CFG::Aimbot_Projectile_Max_Simulation_Time;
        const float flDistToTarget = vLocalPos.DistTo(target.Position);

        // Se alvo está muito longe, aumenta o tempo de simulação
        if (flDistToTarget > 1000.0f)
            flMaxSimTime = std::min(flMaxSimTime * 1.5f, 5.0f);

        for (int nTick = 0; nTick < TIME_TO_TICKS(flMaxSimTime); nTick++)
        {
            // ✅ PROTEÇÃO: Verifica se simulação retornou valor válido (SEM IsValid)
            Vec3 simOrigin = F::MovementSimulation->GetOrigin();

            // Verifica se Vec3 é válido manualmente
            if (isnan(simOrigin.x) || isnan(simOrigin.y) || isnan(simOrigin.z) ||
                isinf(simOrigin.x) || isinf(simOrigin.y) || isinf(simOrigin.z))
                break;

            m_TargetPath.push_back(simOrigin);

            // Verifica se RunTick foi bem-sucedido
            F::MovementSimulation->RunTick(TICKS_TO_TIME(nTick));

            Vec3 vTarget = F::MovementSimulation->GetOrigin();

            // Verifica se o resultado é válido
            if (isnan(vTarget.x) || isnan(vTarget.y) || isnan(vTarget.z) ||
                isinf(vTarget.x) || isinf(vTarget.y) || isinf(vTarget.z))
                break;

            OffsetPlayerPosition(pWeapon, vTarget, pPlayer, bDucked, bOnGround);

            float flTimeToTarget = 0.0f;
            if (!CalcProjAngle(vLocalPos, vTarget, target.AngleTo, flTimeToTarget, false))
                continue;

            target.TimeToTarget = flTimeToTarget;
            int nTargetTick = TIME_TO_TICKS(flTimeToTarget + SDKUtils::GetLatency());

            if (pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
            {
                const auto sticky_arm_time = SDKUtils::AttribHookValue(0.8f, "sticky_arm_time", pLocal);
                if (TICKS_TO_TIME(nTargetTick) < sticky_arm_time)
                    nTargetTick += TIME_TO_TICKS(fabsf(flTimeToTarget - sticky_arm_time));
            }

            if ((nTargetTick == nTick || nTargetTick == nTick - 1))
            {
                // Tiro direto primeiro
                if (CanSee(pLocal, pWeapon, vLocalPos, vTarget, target, flTimeToTarget))
                {
                    F::MovementSimulation->Restore();
                    return true;
                }

                // Splash se falhar
                if (CFG::Aimbot_Projectile_SplashBot)
                {
                    Vec3 splashAngle;
                    float splashTime;
                    if (TrySplashShot(pLocal, pWeapon, pCmd, target, splashAngle, splashTime, true))
                    {
                        target.AngleTo = splashAngle;
                        target.TimeToTarget = splashTime;
                        F::MovementSimulation->Restore();
                        return true;
                    }
                }
            }
        }
        F::MovementSimulation->Restore();
    }
    else // Buildings
    {
        const Vec3 vTarget = target.Position;
        float flTimeToTarget = 0.0f;

        if (!CalcProjAngle(vLocalPos, vTarget, target.AngleTo, flTimeToTarget, false))
            return false;

        target.TimeToTarget = flTimeToTarget;

        if (CanSee(pLocal, pWeapon, vLocalPos, vTarget, target, flTimeToTarget))
            return true;

        if (CFG::Aimbot_Projectile_SplashBot)
        {
            Vec3 splashAngle;
            float splashTime;
            if (TrySplashShot(pLocal, pWeapon, pCmd, target, splashAngle, splashTime, false))
            {
                target.AngleTo = splashAngle;
                target.Position = vTarget;
                target.TimeToTarget = splashTime;
                return true;
            }
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

            // Enemy checks
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
            // Ally checks (Crossbow)
            else
            {
                if (pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW)
                {
                    if (pPlayer->m_iHealth() >= pPlayer->GetMaxHealth() || pPlayer->IsInvulnerable())
                        continue;
                }
            }

            Vec3 vPos = pPlayer->GetCenter();
            Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
            const float flFOVTo = CFG::Aimbot_Projectile_Sort == 0 ? Math::CalcFov(vLocalAngles, vAngleTo) : 0.0f;
            const float flDistTo = vLocalPos.DistTo(vPos);

            // FOV filter
            if (CFG::Aimbot_Projectile_Sort == 0 && flFOVTo > CFG::Aimbot_Projectile_FOV)
                continue;

            m_vecTargets.emplace_back(ProjTarget_t{ pPlayer, vPos, vAngleTo, flFOVTo, flDistTo });
        }
    }

    // ========== BUILDINGS ==========
    if (CFG::Aimbot_Target_Buildings)
    {
        const auto isRescueRanger = pWeapon->GetWeaponID() == TF_WEAPON_SHOTGUN_BUILDING_RESCUE;
        const auto nGroup = isRescueRanger ? EEntGroup::BUILDINGS_ALL : EEntGroup::BUILDINGS_ENEMIES;

        for (const auto pEntity : H::Entities->GetGroup(nGroup))
        {
            if (!pEntity)
                continue;

            const auto pBuilding = pEntity->As<C_BaseObject>();
            if (pBuilding->m_bPlacing())
                continue;

            // Rescue Ranger checks
            if (isRescueRanger && pBuilding->m_iTeamNum() == pLocal->m_iTeamNum() &&
                pBuilding->m_iHealth() >= pBuilding->m_iMaxHealth())
                continue;

            Vec3 vPos = pBuilding->GetCenter();
            Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
            const float flFOVTo = CFG::Aimbot_Projectile_Sort == 0 ? Math::CalcFov(vLocalAngles, vAngleTo) : 0.0f;
            const float flDistTo = vLocalPos.DistTo(vPos);

            // FOV filter
            if (CFG::Aimbot_Projectile_Sort == 0 && flFOVTo > CFG::Aimbot_Projectile_FOV)
                continue;

            m_vecTargets.emplace_back(ProjTarget_t{ pBuilding, vPos, vAngleTo, flFOVTo, flDistTo });
        }
    }

    if (m_vecTargets.empty())
        return false;

    // ========== SORTING ==========
    F::AimbotCommon->Sort(m_vecTargets, CFG::Aimbot_Projectile_Sort);

    // ========== TARGET PROCESSING ==========
    const int maxTargets = std::min(CFG::Aimbot_Projectile_Max_Processing_Targets, static_cast<int>(m_vecTargets.size()));
    const float maxRangeForFullScan = 2000.0f; // ✅ AUMENTADO para funcionar à distância

    for (int i = 0; i < static_cast<int>(m_vecTargets.size()); i++)
    {
        auto& target = m_vecTargets[i];

        // ✅ PROTEÇÃO CONTRA CRASH (SEM GetDormant)
        if (!target.Entity)
            continue;

        int idx = target.Entity->entindex();
        if (idx <= 0 || idx > 64)
            continue;

        // Verifica se a entidade ainda existe
        C_BaseEntity* pCheck = reinterpret_cast<C_BaseEntity*>(I::ClientEntityList->GetClientEntity(idx));
        if (!pCheck || pCheck != target.Entity)
            continue;

        const float distToTarget = target.Position.DistTo(vLocalPos);

        // ✅ AUMENTADO para funcionar à distância
        const int maxTargets = std::min(CFG::Aimbot_Projectile_Max_Processing_Targets, static_cast<int>(m_vecTargets.size()));
        const float maxRangeForFullScan = 2000.0f; // Era 400!

        if (distToTarget > maxRangeForFullScan && i >= maxTargets)
            continue;

        // Solve target
        if (!SolveTarget(pLocal, pWeapon, pCmd, target))
            continue;

        // Final FOV check
        if (CFG::Aimbot_Projectile_Sort == 0 && Math::CalcFov(vLocalAngles, target.AngleTo) > CFG::Aimbot_Projectile_FOV)
            continue;

        outTarget = target;
        return true;
    }
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
            pCmd->buttons |= IN_ATTACK; // Hold to charge
        else
            pCmd->buttons &= ~IN_ATTACK; // Release to fire
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
    return Vec3(0.0f, 0.0f, 0.0f); // Placeholder
}

void CAimbotProjectile::Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    if (!CFG::Aimbot_Projectile_Active)
        return;

    if (!GetProjectileInfo(pWeapon))
        return;

    if (CFG::Aimbot_Projectile_Sort == 0)
        G::flAimbotFOV = CFG::Aimbot_Projectile_FOV;

    if (Shifting::bShifting && !Shifting::bShiftingWarp)
        return;

    // ========== KEYBIND SYSTEM ==========
    int mode = CFG::Aimbot_Projectile_KeyMode;
    int key = CFG::Aimbot_Key;

    bool bActive = false;
    bool bKeyDown = (key > 0 && (GetAsyncKeyState(key) & 0x8000) != 0);

    static bool bToggleState = false;
    static bool bLastKeyDown = false;

    switch (mode)
    {
    case 0: // Always On
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

    default:
        bActive = false;
        break;
    }

    bLastKeyDown = bKeyDown;

    if (!bActive)
        return;

    // ========== MAIN AIMBOT LOGIC ==========
    ProjTarget_t target = {};

    if (GetTarget(pLocal, pWeapon, pCmd, target) && target.Entity)
    {
        G::nTargetIndexEarly = target.Entity->entindex();
        G::nTargetIndex = target.Entity->entindex();

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
                I::DebugOverlay->ClearAllOverlays();
                DrawMovePath(m_TargetPath);
                m_TargetPath.clear();
            }
        }
    }
}

// ========== NEURAL NETWORK SPLASH PREDICTION ==========
bool CAimbotProjectile::NeuralNetworkSplashPrediction(const Vec3& impactPoint, C_BaseEntity* pTargetEntity)
{
    // ✅ IMPLEMENTAÇÃO BÁSICA (PLACEHOLDER)
    // Se você não tem uma rede neural implementada, retorna true para aceitar todos os pontos
    // Você pode adicionar lógica mais complexa aqui futuramente

    if (!pTargetEntity)
        return false;

    // TODO: Implementar rede neural para predição de splash damage
    // Por enquanto, usa heurísticas simples:

    C_TFPlayer* pTarget = pTargetEntity->As<C_TFPlayer>();
    if (!pTarget)
        return true; // Aceita para buildings

    // Verifica se o alvo está se movendo muito rápido
    Vec3 vVelocity = pTarget->m_vecVelocity();
    float flSpeed = vVelocity.Length();

    // Se está muito rápido, splash é menos confiável
    if (flSpeed > 450.0f)
        return false;

    // Verifica altura do ponto de impacto relativo ao alvo
    Vec3 vTargetPos = pTarget->GetAbsOrigin();
    float flHeightDiff = abs(impactPoint.z - vTargetPos.z);

    // Se o ponto está muito abaixo ou acima, splash pode não funcionar
    if (flHeightDiff > 100.0f)
        return false;

    // Aceita o splash shot
    return true;
}