#include "AimbotProjectile.h"
#include "CFG.h"
#include "../src/Features/MovementSimulation/MovementSimulation.h"
#include "../src/Features/ProjectileSim/ProjectileSim.h"
#include <algorithm>
void DrawProjPath(const CUserCmd* pCmd, float time)
{
    if (!pCmd || !G::bFiring)
    {
        return;
    }
    const auto pLocal = H::Entities->GetLocal();
    if (!pLocal || pLocal->deadflag())
    {
        return;
    }
    const auto pWeapon = H::Entities->GetWeapon();
    if (!pWeapon)
    {
        return;
    }
    ProjectileInfo info = {};
    if (!F::ProjectileSim->GetInfo(pLocal, pWeapon, pCmd->viewangles, info))
    {
        return;
    }
    if (!F::ProjectileSim->Init(info))
    {
        return;
    }
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
            // Outline (preto)
            I::DebugOverlay->AddLineOverlay(a, b, 0, 0, 0, false, duration);
            // Linha principal
            I::DebugOverlay->AddLineOverlay(a, b, 255, 255, 255, false, duration);
        };
    // ================= Line =================
    if (CFG::Visuals_Draw_Movement_Path_Style == 1)
    {
        for (size_t n = 1; n < vPath.size(); n++)
        {
            DrawLineOutlined(vPath[n], vPath[n - 1]);
        }
    }
    // ================= Dashed =================
    if (CFG::Visuals_Draw_Movement_Path_Style == 2)
    {
        for (size_t n = 1; n < vPath.size(); n++)
        {
            if (n % 2 == 0)
                continue;
            DrawLineOutlined(vPath[n], vPath[n - 1]);
        }
    }
    // ================= Alternative line + 3D Box =================
    if (CFG::Visuals_Draw_Movement_Path_Style == 3)
    {
        for (size_t n = 1; n < vPath.size(); n++)
        {
            // Linha com outline
            DrawLineOutlined(vPath[n], vPath[n - 1]);
            if (n == vPath.size() - 1)
            {
                // ===== BOX 3D no ponto de impacto =====
                const Vec3& impactPos = vPath[n];
                // Bounding box padrão do player TF2
                const Vec3 mins{ -24.f, -24.f, 0.f };
                const Vec3 maxs{ 24.f, 24.f, 82.f };
                // Outline da box
                I::DebugOverlay->AddBoxOverlay(
                    impactPos,
                    mins,
                    maxs,
                    Vec3(0.f, 0.f, 0.f),
                    0, 0, 0, 0,
                    duration
                );
                // Box principal
                I::DebugOverlay->AddBoxOverlay(
                    impactPos,
                    mins,
                    maxs,
                    Vec3(0.f, 0.f, 0.f),
                    255, 255, 255, 0,
                    duration
                );
            }
        }
    }
}
Vec3 GetOffsetShootPos(C_TFPlayer* local, C_TFWeaponBase* weapon, const CUserCmd* pCmd)
{
    auto out{ local->GetShootPos() };
    switch (weapon->GetWeaponID())
    {
    case TF_WEAPON_ROCKETLAUNCHER:
    case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
    case TF_WEAPON_FLAREGUN:
    case TF_WEAPON_FLAREGUN_REVENGE:
    case TF_WEAPON_SYRINGEGUN_MEDIC:
    case TF_WEAPON_FLAME_BALL:
    case TF_WEAPON_CROSSBOW:
    case TF_WEAPON_FLAMETHROWER:
    case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
    {
        if (weapon->m_iItemDefinitionIndex() != Soldier_m_TheOriginal)
        {
            Vec3 vOffset = { 23.5f, 12.0f, -3.0f };
            if (local->m_fFlags() & FL_DUCKING)
                vOffset.z = 8.0f;
            H::AimUtils->GetProjectileFireSetup(pCmd->viewangles, vOffset, &out);
        }
        break;
    }
    case TF_WEAPON_COMPOUND_BOW:
    {
        Vec3 vOffset = { 20.5f, 12.0f, -3.0f };
        if (local->m_fFlags() & FL_DUCKING)
            vOffset.z = 8.0f;
        H::AimUtils->GetProjectileFireSetup(pCmd->viewangles, vOffset, &out);
        break;
    }
    default: break;
    }
    return out;
}
bool IsChargingWeapon(int weaponID)
{
    return weaponID == TF_WEAPON_COMPOUND_BOW || weaponID == TF_WEAPON_PIPEBOMBLAUNCHER || weaponID == TF_WEAPON_CANNON;
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
        g *= 1.0f; // GravityMod for pipes
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
        {
            return target.TimeToTarget * 0.8f;
        }
        else
        {
            return 0.0f;
        }
    }
    return 0.0f;
}
float GetCurrentChargeTime(C_TFWeaponBase* pWeapon)
{
    float charge_begin = pWeapon->As<C_TFPipebombLauncher>()->m_flChargeBeginTime();
    if (charge_begin > 0.0f)
    {
        return I::GlobalVars->curtime - charge_begin;
    }
    return 0.0f;
}
bool CAimbotProjectile::GetProjectileInfo(C_TFWeaponBase* pWeapon)
{
    m_CurProjInfo = {};
    auto curTime = [&]() -> float
        {
            if (const auto pLocal = H::Entities->GetLocal())
            {
                return static_cast<float>(pLocal->m_nTickBase()) * I::GlobalVars->interval_per_tick;
            }
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
            m_CurProjInfo.Speed = Math::RemapValClamped
            (
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
    default: break;
    }
    return m_CurProjInfo.Speed > 0.0f;
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
            {
                v0 = k_flMaxVelocity;
            }
        }
        const auto root{ v0 * v0 * v0 * v0 - g * (g * dx * dx + 2.0f * dy * v0 * v0) };
        if (root < 0.0f)
        {
            return false;
        }
        const float sign = bHighArc ? +1.0f : -1.0f;
        const float theta = atanf((v0 * v0 + sign * sqrtf(root)) / (g * dx));
        vAngleOut = { -RAD2DEG(theta), RAD2DEG(atan2f(v.y, v.x)), 0.0f };
        flTimeOut = dx / (cosf(theta) * v0);
        if (m_CurProjInfo.Pipes)
        {
            //do 2nd pass for drag | TODO: Math > Magic
            auto magic{ 0.0f };
            if (pWeapon->GetWeaponID() == TF_WEAPON_GRENADELAUNCHER)
            {
                if (pWeapon->m_iItemDefinitionIndex() == Demoman_m_TheLochnLoad)
                {
                    magic = 0.07f;
                }
                else
                {
                    magic = 0.11f;
                }
            }
            if (pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
            {
                magic = 0.16f;
            }
            if (pWeapon->GetWeaponID() == TF_WEAPON_CANNON)
            {
                magic = 0.35f;
            }
            v0 -= (v0 * flTimeOut) * magic;
            auto root{ v0 * v0 * v0 * v0 - g * (g * dx * dx + 2.0f * dy * v0 * v0) };
            if (root < 0.0f)
            {
                return false;
            }
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
            {
                return false;
            }
        }
        else
        {
            if (pWeapon->m_iItemDefinitionIndex() == Demoman_m_TheIronBomber)
            {
                if (flTimeOut > 1.4f)
                {
                    return false;
                }
            }
            else
            {
                if (flTimeOut > 2.0f)
                {
                    return false;
                }
            }
        }
    }
    if ((pWeapon->GetWeaponID() == TF_WEAPON_FLAME_BALL || pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER) && flTimeOut > 0.18f)
    {
        return false;
    }
    return true;
}
void CAimbotProjectile::OffsetPlayerPosition(C_TFWeaponBase* pWeapon, Vec3& vPos, C_TFPlayer* pPlayer, bool bDucked, bool bOnGround)
{
    const float flMaxZ{ (bDucked ? 62.0f : 82.0f) * pPlayer->m_flModelScale() };
    switch (CFG::Aimbot_Projectile_AimPosition)
    {
        // Feet
    case 0:
    {
        vPos.z += (flMaxZ * 0.2f);
        m_LastAimPos = 0;
        break;
    }
    // Body
    case 1:
    {
        vPos.z += (flMaxZ * 0.5f);
        m_LastAimPos = 1;
        break;
    }
    // Head
    case 2:
    {
        if (CFG::Aimbot_Projectile_Advanced_Head_Aim)
        {
            const Vec3 vDelta = pPlayer->GetHitboxPos(HITBOX_HEAD) - pPlayer->m_vecOrigin();
            vPos.x += vDelta.x;
            vPos.y += vDelta.y;
        }
        vPos.z += (flMaxZ * 0.85f);
        m_LastAimPos = 2;
        break;
    }
    // Auto
    case 3:
    {
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
            {
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
            }
            case TF_WEAPON_PIPEBOMBLAUNCHER:
            {
                vPos.z += (flMaxZ * 0.1f);
                m_LastAimPos = 0;
                break;
            }
            default:
            {
                vPos.z += (flMaxZ * 0.5f);
                m_LastAimPos = 1;
                break;
            }
            }
        }
        break;
    }
    default: break;
    }
}
bool CAimbotProjectile::CanArcReach(const Vec3& vFrom, const Vec3& vTo, const Vec3& vAngleTo, float flTargetTime, C_BaseEntity* pTarget)
{
    const auto pLocal = H::Entities->GetLocal();
    if (!pLocal)
    {
        return false;
    }
    const auto pWeapon = H::Entities->GetWeapon();
    if (!pWeapon)
    {
        return false;
    }
    ProjectileInfo info{};
    if (!F::ProjectileSim->GetInfo(pLocal, pWeapon, vAngleTo, info))
    {
        return false;
    }
    if (pWeapon->m_iItemDefinitionIndex() == Demoman_m_TheLochnLoad)
    {
        info.m_speed += 45.0f; //need to do this for some reason
    }
    if (!F::ProjectileSim->Init(info, true))
    {
        return false;
    }
    CTraceFilterWorldCustom filter{};
    filter.m_pTarget = pTarget;
    //I::DebugOverlay->ClearAllOverlays();
    for (auto n = 0; n < TIME_TO_TICKS(flTargetTime * 1.2f); n++)
    {
        auto pre{ F::ProjectileSim->GetOrigin() };
        F::ProjectileSim->RunTick();
        auto post{ F::ProjectileSim->GetOrigin() };
        trace_t trace{};
        Vec3 mins{ -6.0f, -6.0f, -6.0f };
        Vec3 maxs{ 6.0f, 6.0f, 6.0f };
        switch (info.m_type)
        {
        case TF_PROJECTILE_PIPEBOMB:
        case TF_PROJECTILE_PIPEBOMB_REMOTE:
        case TF_PROJECTILE_PIPEBOMB_PRACTICE:
        case TF_PROJECTILE_CANNONBALL:
        {
            mins = { -8.0f, -8.0f, -8.0f };
            maxs = { 8.0f, 8.0f, 20.0f };
            break;
        }
        case TF_PROJECTILE_FLARE:
        {
            mins = { -8.0f, -8.0f, -8.0f };
            maxs = { 8.0f, 8.0f, 8.0f };
            break;
        }
        default:
        {
            break;
        }
        }
        H::AimUtils->TraceHull(pre, post, mins, maxs, MASK_SOLID, &filter, &trace);
        if (trace.m_pEnt == pTarget)
        {
            return true;
        }
        if (trace.DidHit())
        {
            if (info.m_pos.DistTo(trace.endpos) > info.m_pos.DistTo(vTo))
            {
                return true;
            }
            if (trace.endpos.DistTo(vTo) > 40.0f)
            {
                return false;
            }
            H::AimUtils->Trace(trace.endpos, vTo, MASK_SOLID, &filter, &trace);
            return !trace.DidHit() || trace.m_pEnt == pTarget;
        }
        //I::DebugOverlay->AddBoxOverlay(post, mins, maxs, Math::CalcAngle(pre, post), 255, 255, 255, 2, 60.0f);
    }
    return true;
}
bool CAimbotProjectile::CanSee(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const Vec3& vFrom, const Vec3& vTo, const ProjTarget_t& target, float flTimeToTarget)
{
    Vec3 vLocalPos = vFrom;
    switch (pWeapon->GetWeaponID())
    {
    case TF_WEAPON_ROCKETLAUNCHER:
    case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
    case TF_WEAPON_FLAREGUN:
    case TF_WEAPON_FLAREGUN_REVENGE:
    case TF_WEAPON_SYRINGEGUN_MEDIC:
    case TF_WEAPON_FLAME_BALL:
    case TF_WEAPON_CROSSBOW:
    case TF_WEAPON_FLAMETHROWER:
    case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
    {
        if (pWeapon->m_iItemDefinitionIndex() != Soldier_m_TheOriginal)
        {
            Vec3 vOffset = { 23.5f, 12.0f, -3.0f };
            if (pLocal->m_fFlags() & FL_DUCKING)
                vOffset.z = 8.0f;
            H::AimUtils->GetProjectileFireSetup(target.AngleTo, vOffset, &vLocalPos);
        }
        break;
    }
    case TF_WEAPON_COMPOUND_BOW:
    {
        Vec3 vOffset = { 20.5f, 12.0f, -3.0f };
        if (pLocal->m_fFlags() & FL_DUCKING)
            vOffset.z = 8.0f;
        H::AimUtils->GetProjectileFireSetup(target.AngleTo, vOffset, &vLocalPos);
        break;
    }
    default: break;
    }
    if (m_CurProjInfo.GravityMod != 0.f)
    {
        return CanArcReach(vFrom, vTo, target.AngleTo, flTimeToTarget, target.Entity);
    }
    if (m_CurProjInfo.Flamethrower)
    {
        return H::AimUtils->TraceFlames(target.Entity, vLocalPos, vTo);
    }
    return H::AimUtils->TraceProjectile(target.Entity, vLocalPos, vTo);
}
bool CAimbotProjectile::SolveTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const CUserCmd* pCmd, ProjTarget_t& target)
{
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
        const bool bDucked = pPlayer->m_fFlags() & FL_DUCKING;
        const bool bOnGround = pPlayer->m_fFlags() & FL_ONGROUND;
        if (!F::MovementSimulation->Initialize(pPlayer))
            return false;
        float prev_z = pPlayer->m_vecOrigin().z;
        for (int nTick = 0; nTick < TIME_TO_TICKS(CFG::Aimbot_Projectile_Max_Simulation_Time); nTick++)
        {
            m_TargetPath.push_back(F::MovementSimulation->GetOrigin());
            F::MovementSimulation->RunTick(TICKS_TO_TIME(nTick));
            float current_z = F::MovementSimulation->GetOrigin().z;
            if (current_z < prev_z - 100.0f) // Detect potential fall into gap
            {
                break; // Stop simulation to avoid long predicts
            }
            prev_z = current_z;
            Vec3 vTarget = F::MovementSimulation->GetOrigin();
            // Amalgamation: combine methods for better prediction (adapted for bow and stick)
            const bool bSimOnGround = F::MovementSimulation->IsSimulatedOnGround();
            const Vec3 vel = F::MovementSimulation->GetSimulatedVelocity();
            const bool isBow = pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW;
            const bool isStick = pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER;
            if ((isBow || isStick) && (CFG::Aimbot_Projectile_PredictionMethod == 0 ||
                CFG::Aimbot_Projectile_GroundStrafePrediction ||
                CFG::Aimbot_Projectile_AdvancedAirStrafe))
            {
                const float speed = vel.Length2D();
                if (speed > 0.01f)
                {
                    Vec3 accel_dir = { vel.x, vel.y, 0.0f };
                    const float len = accel_dir.Length2D();
                    if (len > 0.0f)
                        accel_dir /= len; // normalize
                    float accel = 0.0f;
                    if (CFG::Aimbot_Projectile_PredictionMethod == 0)
                    {
                        accel = bSimOnGround ? 300.0f : 12.0f;
                    }
                    else if (CFG::Aimbot_Projectile_GroundStrafePrediction && bSimOnGround)
                    {
                        accel = 300.0f;
                    }
                    else if (CFG::Aimbot_Projectile_AdvancedAirStrafe && !bSimOnGround)
                    {
                        accel = 12.0f;
                    }
                    if (accel > 0.0f)
                    {
                        const float t = target.TimeToTarget;
                        vTarget += accel_dir * (0.5f * accel * t * t);
                    }
                }
            }
            OffsetPlayerPosition(pWeapon, vTarget, pPlayer, bDucked, bOnGround);
            float flTimeToTarget = 0.0f;
            Vec3 vAngleTo;
            if (!CalcProjAngle(vLocalPos, vTarget, vAngleTo, flTimeToTarget, false))
            {
                if (!CalcProjAngle(vLocalPos, vTarget, vAngleTo, flTimeToTarget, true))
                    continue;
            }
            target.AngleTo = vAngleTo;
            target.TimeToTarget = flTimeToTarget;
            int nTargetTick = TIME_TO_TICKS(flTimeToTarget + SDKUtils::GetLatency()) + CFG::Aimbot_Projectile_TicksPredict;
            if (pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
            {
                const auto sticky_arm_time{ SDKUtils::AttribHookValue(0.8f, "sticky_arm_time", pLocal) };
                if (TICKS_TO_TIME(nTargetTick) < sticky_arm_time)
                {
                    nTargetTick += TIME_TO_TICKS(fabsf(flTimeToTarget - sticky_arm_time));
                }
            }
            if ((nTargetTick == nTick || nTargetTick == nTick - 1))
            {
                auto runSplash = [&]()
                    {
                        if (!CFG::Aimbot_Projectile_SplashBot) return false;
                        auto isRocketLauncher{ pWeapon->GetWeaponID() == TF_WEAPON_ROCKETLAUNCHER };
                        auto isDirectHit{ pWeapon->GetWeaponID() == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT };
                        auto isAirStrike{ pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheAirStrike };
                        auto isPipebomb{ pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER };
                        auto isBow{ pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW };
                        if (!isRocketLauncher && !isDirectHit && !isAirStrike && !isPipebomb && !isBow)
                        {
                            return false;
                        }
                        Vec3 mins{ target.Entity->m_vecMins() };
                        Vec3 maxs{ target.Entity->m_vecMaxs() };
                        auto center{ F::MovementSimulation->GetOrigin() + Vec3(0.0f, 0.0f, (mins.z + maxs.z) * 0.5f) };
                        center.x = vTarget.x;
                        center.y = vTarget.y;
                        auto numPoints{ static_cast<int>(CFG::Aimbot_Projectile_SplashPoints) };
                        auto radius{ isRocketLauncher ? 146.0f : (isPipebomb ? 146.0f : (isBow ? 100.0f : 44.0f)) }; // Adapted radius for bow
                        if (isAirStrike)
                        {
                            radius = 124.0f;
                        }
                        // Adjust radius based on movement speed
                        float speed = F::MovementSimulation->GetSimulatedVelocity().Length2D();
                        radius *= (1.0f - speed / 800.0f); // Reduce radius for faster moving targets
                        // Prefer ground points if grounded
                        bool preferGround = F::MovementSimulation->IsSimulatedOnGround();
                        std::vector<Vec3> potential{};
                        for (int n = 0; n < numPoints; n++)
                        {
                            auto a1{ acosf(1.0f - 2.0f * (static_cast<float>(n) / static_cast<float>(numPoints))) };
                            auto a2{ (static_cast<float>(PI) * (3.0f - sqrtf(5.0f))) * static_cast<float>(n) };
                            Vec3 dir{ sinf(a1) * cosf(a2), sinf(a1) * sinf(a2), cosf(a1) };
                            if (preferGround && dir.z > 0.0f)
                                continue; // Skip upper hemisphere if preferring ground
                            dir *= radius;
                            auto point{ center + dir };
                            CTraceFilterWorldCustom filter{};
                            trace_t trace{};
                            H::AimUtils->Trace(center, point, MASK_SOLID, &filter, &trace);
                            if (trace.fraction > 0.99f)
                            {
                                continue;
                            }
                            potential.push_back(trace.endpos);
                        }
                        std::ranges::sort(potential, [&](const Vec3& a, const Vec3& b)
                            {
                                // Sort by distance to center (closer for higher damage)
                                float distA = a.DistTo(center);
                                float distB = b.DistTo(center);
                                return distA < distB;
                            });
                        for (auto& point : potential)
                        {
                            if (!CalcProjAngle(vLocalPos, point, target.AngleTo, target.TimeToTarget, false))
                            {
                                continue;
                            }
                            trace_t trace = {};
                            CTraceFilterWorldCustom filter = {};
                            H::AimUtils->TraceHull
                            (
                                GetOffsetShootPos(pLocal, pWeapon, pCmd),
                                point,
                                { -4.0f, -4.0f, -4.0f },
                                { 4.0f, 4.0f, 4.0f },
                                MASK_SOLID,
                                &filter,
                                &trace
                            );
                            if (trace.fraction < 0.9f || trace.startsolid || trace.allsolid)
                            {
                                continue;
                            }
                            H::AimUtils->Trace(trace.endpos, point, MASK_SOLID, &filter, &trace);
                            if (trace.fraction < 1.0f)
                            {
                                continue;
                            }
                            if (CFG::Debug_SplashPoints)
                            {
                                I::DebugOverlay->AddBoxOverlay(point, Vec3(-1, -1, -1), Vec3(1, 1, 1), Vec3(0, 0, 0), 255, 0, 0, 128, 5.0f);
                            }
                            return true;
                        }
                        return false;
                    };
                if (CFG::Aimbot_Projectile_RocketSplashPoint && runSplash())
                {
                    F::MovementSimulation->Restore();
                    return true;
                }
                if (CanSee(pLocal, pWeapon, vLocalPos, vTarget, target, flTimeToTarget))
                {
                    F::MovementSimulation->Restore();
                    return true;
                }
                if (CFG::Aimbot_Projectile_BBox_Multipoint && pWeapon->GetWeaponID() != TF_WEAPON_COMPOUND_BOW)
                {
                    const int nOld = CFG::Aimbot_Projectile_AimPosition;
                    for (int n = 0; n < 3; n++)
                    {
                        if (n == m_LastAimPos)
                            continue;
                        CFG::Aimbot_Projectile_AimPosition = n;
                        Vec3 vTargetMp = F::MovementSimulation->GetOrigin();
                        OffsetPlayerPosition(pWeapon, vTargetMp, pPlayer, bDucked, bOnGround);
                        CFG::Aimbot_Projectile_AimPosition = nOld;
                        if (CalcProjAngle(vLocalPos, vTargetMp, target.AngleTo, target.TimeToTarget, false))
                        {
                            if (CanSee(pLocal, pWeapon, vLocalPos, vTargetMp, target, target.TimeToTarget))
                            {
                                F::MovementSimulation->Restore();
                                return true;
                            }
                        }
                    }
                }
                if (runSplash())
                {
                    F::MovementSimulation->Restore();
                    return true;
                }
            }
        }
        F::MovementSimulation->Restore();
    }
    else
    {
        const Vec3 vTarget = target.Position;
        float flTimeToTarget = 0.0f;
        auto runSplash = [&]()
            {
                if (!CFG::Aimbot_Projectile_SplashBot) return false;
                const auto isRocketLauncher{ pWeapon->GetWeaponID() == TF_WEAPON_ROCKETLAUNCHER };
                const auto isDirectHit{ pWeapon->GetWeaponID() == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT };
                const auto isAirStrike{ pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheAirStrike };
                const auto isPipebomb{ pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER };
                const auto isBow{ pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW };
                if (!isRocketLauncher && !isDirectHit && !isAirStrike && !isPipebomb && !isBow)
                {
                    return false;
                }
                const auto center{ target.Entity->GetCenter() };
                auto numPoints{ static_cast<int>(CFG::Aimbot_Projectile_SplashPoints) };
                auto radius{ isRocketLauncher ? 146.0f : (isPipebomb ? 146.0f : (isBow ? 100.0f : 44.0f)) }; // Adapted radius for bow
                if (isAirStrike)
                {
                    radius = 124.0f;
                }
                std::vector<Vec3> potential{};
                for (int n = 0; n < numPoints; n++)
                {
                    const auto a1{ acosf(1.0f - 2.0f * (static_cast<float>(n) / static_cast<float>(numPoints))) };
                    const auto a2{ (static_cast<float>(PI) * (3.0f - sqrtf(5.0f))) * static_cast<float>(n) };
                    Vec3 dir{ sinf(a1) * cosf(a2), sinf(a1) * sinf(a2), cosf(a1) };
                    dir *= radius;
                    auto point{ center + dir };
                    CTraceFilterWorldCustom filter{};
                    trace_t trace{};
                    H::AimUtils->Trace(center, point, MASK_SOLID, &filter, &trace);
                    if (trace.fraction > 0.99f)
                    {
                        continue;
                    }
                    potential.push_back(trace.endpos);
                }
                std::ranges::sort(potential, [&](const Vec3& a, const Vec3& b)
                    {
                        return a.DistTo(center) < b.DistTo(center);
                    });
                //I::DebugOverlay->ClearAllOverlays();
                for (auto& point : potential)
                {
                    if (!CalcProjAngle(vLocalPos, point, target.AngleTo, target.TimeToTarget, false))
                    {
                        continue;
                    }
                    trace_t trace = {};
                    CTraceFilterWorldCustom filter = {};
                    H::AimUtils->TraceHull
                    (
                        GetOffsetShootPos(pLocal, pWeapon, pCmd),
                        point,
                        { -4.0f, -4.0f, -4.0f },
                        { 4.0f, 4.0f, 4.0f },
                        MASK_SOLID,
                        &filter,
                        &trace
                    );
                    if (trace.fraction < 0.9f || trace.startsolid || trace.allsolid)
                    {
                        continue;
                    }
                    H::AimUtils->Trace(trace.endpos, point, MASK_SOLID, &filter, &trace);
                    if (trace.fraction < 1.0f)
                    {
                        continue;
                    }
                    if (CFG::Debug_SplashPoints)
                    {
                        I::DebugOverlay->AddBoxOverlay(point, Vec3(-1, -1, -1), Vec3(1, 1, 1), Vec3(0, 0, 0), 255, 0, 0, 128, 5.0f);
                    }
                    return true;
                }
                return false;
            };
        if (!CalcProjAngle(vLocalPos, vTarget, target.AngleTo, flTimeToTarget, false))
        {
            if (!CalcProjAngle(vLocalPos, vTarget, target.AngleTo, flTimeToTarget, true))
                return false;
        }
        target.TimeToTarget = flTimeToTarget;
        int nTargetTick = TIME_TO_TICKS(flTimeToTarget + SDKUtils::GetLatency()) + CFG::Aimbot_Projectile_TicksPredict;
        if (pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
        {
            nTargetTick += TIME_TO_TICKS(fabsf(flTimeToTarget - SDKUtils::AttribHookValue(0.8f, "sticky_arm_time", pLocal)));
        }
        if (nTargetTick <= TIME_TO_TICKS(CFG::Aimbot_Projectile_Max_Simulation_Time))
        {
            if (CFG::Aimbot_Projectile_RocketSplashPoint && runSplash())
            {
                return true;
            }
            if (CanSee(pLocal, pWeapon, vLocalPos, vTarget, target, flTimeToTarget))
            {
                return true;
            }
            if (runSplash())
            {
                return true;
            }
        }
    }
    m_TargetPath.clear();
    return false;
}
bool CAimbotProjectile::GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const CUserCmd* pCmd, ProjTarget_t& outTarget)
{
    const Vec3 vLocalPos = pLocal->GetShootPos();
    const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();
    m_vecTargets.clear();
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
                    {
                        continue;
                    }
                }
            }
            Vec3 vPos = pPlayer->GetCenter();
            Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
            const float flFOVTo = CFG::Aimbot_Projectile_Sort == 0 ? Math::CalcFov(vLocalAngles, vAngleTo) : 0.0f;
            const float flDistTo = vLocalPos.DistTo(vPos);
            if (CFG::Aimbot_Projectile_Sort == 0 && flFOVTo > CFG::Aimbot_Projectile_FOV)
                continue;
            m_vecTargets.emplace_back(ProjTarget_t{ pPlayer, vPos, vAngleTo, flFOVTo, flDistTo });
        }
    }
    if (CFG::Aimbot_Target_Buildings)
    {
        const auto isRescueRanger{ pWeapon->GetWeaponID() == TF_WEAPON_SHOTGUN_BUILDING_RESCUE };
        const auto nGroup = isRescueRanger ? EEntGroup::BUILDINGS_ALL : EEntGroup::BUILDINGS_ENEMIES;
        for (const auto pEntity : H::Entities->GetGroup(nGroup))
        {
            if (!pEntity)
                continue;
            const auto pBuilding = pEntity->As<C_BaseObject>();
            if (pBuilding->m_bPlacing())
                continue;
            if (isRescueRanger && pBuilding->m_iTeamNum() == pLocal->m_iTeamNum() && pBuilding->m_iHealth() >= pBuilding->m_iMaxHealth())
            {
                continue;
            }
            Vec3 vPos = pBuilding->GetCenter(); //fuck teleporters when aimed at with pipes lma
            Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
            const float flFOVTo = CFG::Aimbot_Projectile_Sort == 0 ? Math::CalcFov(vLocalAngles, vAngleTo) : 0.0f;
            const float flDistTo = vLocalPos.DistTo(vPos);
            if (CFG::Aimbot_Projectile_Sort == 0 && flFOVTo > CFG::Aimbot_Projectile_FOV)
                continue;
            m_vecTargets.emplace_back(ProjTarget_t{ pBuilding, vPos, vAngleTo, flFOVTo, flDistTo });
        }
    }
    if (m_vecTargets.empty())
        return false;
    // Sort by target priority
    F::AimbotCommon->Sort(m_vecTargets, CFG::Aimbot_Projectile_Sort);
    const auto maxTargets{ std::min(CFG::Aimbot_Projectile_Max_Processing_Targets, static_cast<int>(m_vecTargets.size())) };
    auto targetsScanned{ 0 };
    for (auto& target : m_vecTargets)
    {
        if (target.Position.DistTo(vLocalPos) > 400.0f && targetsScanned >= maxTargets)
        {
            continue;
        }
        if (!SolveTarget(pLocal, pWeapon, pCmd, target))
        {
            targetsScanned++;
            continue;
        }
        if (CFG::Aimbot_Projectile_Sort == 0 && Math::CalcFov(vLocalAngles, target.AngleTo) > CFG::Aimbot_Projectile_FOV)
        {
            continue;
        }
        outTarget = target;
        return true;
    }
    return false;
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
    case 0:
    {
        pCmd->viewangles = vAngleTo;
        break;
    }
    case 1:
    {
        if (m_CurProjInfo.Flamethrower ? true : G::bCanPrimaryAttack)
        {
            H::AimUtils->FixMovement(pCmd, vAngleTo);
            pCmd->viewangles = vAngleTo;
            if (m_CurProjInfo.Flamethrower)
            {
                G::bSilentAngles = true;
            }
            else
            {
                G::bPSilentAngles = true;
            }
        }
        break;
    }
    default: break;
    }
}
bool CAimbotProjectile::ShouldFire(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    if (!CFG::Aimbot_AutoShoot)
    {
        //fucking fuck
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
    if (bIsBazooka && pWeapon->HasPrimaryAmmoForShot())
        pCmd->buttons &= ~IN_ATTACK;
}
bool CAimbotProjectile::IsFiring(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    if (!pWeapon->HasPrimaryAmmoForShot())
        return false;
    const int nWeaponID = pWeapon->GetWeaponID();
    if (IsChargingWeapon(nWeaponID))
    {
        return (G::nOldButtons & IN_ATTACK) && !(pCmd->buttons & IN_ATTACK);
    }
    if (nWeaponID == TF_WEAPON_FLAME_BALL)
    {
        return pLocal->m_flTankPressure() >= 100.0f && (pCmd->buttons & IN_ATTACK);
    }
    if (pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka)
        return G::bCanPrimaryAttack;
    if (nWeaponID == TF_WEAPON_FLAMETHROWER)
    {
        return pCmd->buttons & IN_ATTACK;
    }
    return (pCmd->buttons & IN_ATTACK) && G::bCanPrimaryAttack;
}
void CAimbotProjectile::Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    if (!CFG::Aimbot_Projectile_Enable)
        return;
    if (!GetProjectileInfo(pWeapon))
        return;
    if (CFG::Aimbot_Projectile_Sort == 0)
        G::flAimbotFOV = CFG::Aimbot_Projectile_FOV;
    if (Shifting::bShifting && !Shifting::bShiftingWarp)
        return;
    // Handle key mode with GetAsyncKeyState
    static bool bToggled = false;
    static bool bLastDown = false;
    bool bShouldRun = false;
    if (CFG::Aimbot_KeyMode == 2) // Always On
    {
        bShouldRun = true;
    }
    else
    {
        bool bDown = (GetAsyncKeyState(CFG::Aimbot_Key) & 0x8000) != 0;
        if (CFG::Aimbot_KeyMode == 1) // Toggle
        {
            if (bDown && !bLastDown)
            {
                bToggled = !bToggled;
            }
            bLastDown = bDown;
            bShouldRun = bToggled;
        }
        else // Hold (default 0)
        {
            bShouldRun = bDown;
            bLastDown = bDown;
        }
    }
    if (!bShouldRun)
        return;
    ProjTarget_t target = {};
    if (GetTarget(pLocal, pWeapon, pCmd, target) && target.Entity)
    {
        G::nTargetIndexEarly = target.Entity->entindex();
        G::nTargetIndex = target.Entity->entindex();
        if (ShouldFire(pCmd, pLocal, pWeapon))
            HandleFire(pCmd, pWeapon, pLocal, target);
        const bool bIsFiring = IsFiring(pCmd, pLocal, pWeapon);
        G::bFiring = bIsFiring;
        if (ShouldAim(pCmd, pLocal, pWeapon) || bIsFiring || (IsChargingWeapon(pWeapon->GetWeaponID()) && (pCmd->buttons & IN_ATTACK)))
        {
            Aim(pCmd, pLocal, pWeapon, target.AngleTo);
            if (bIsFiring && m_TargetPath.size() > 1)
            {
                I::DebugOverlay->ClearAllOverlays();
                //drawProjPath(pCmd, Target.TimeToTarget);
                DrawMovePath(m_TargetPath);
                m_TargetPath.clear();
            }
        }
    }
}