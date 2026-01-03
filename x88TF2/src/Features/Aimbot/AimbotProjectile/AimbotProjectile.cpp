#include "AimbotProjectile.h"
#include "CFG.h"
#include "../src/Features/MovementSimulation/MovementSimulation.h"
#include "../src/Features/ProjectileSim/ProjectileSim.h"
#include <algorithm>
#include "../../../SDK/Helpers/AimUtils/AimUtils.h"

void DrawProjPath(const CUserCmd * pCmd, float time)
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

Vec3 GetProjectileFirePos(C_TFPlayer* local, C_TFWeaponBase* weapon, const Vec3& angles)
{
    Vec3 out = local->GetShootPos();
    Vec3 offset = { 0.0f, 0.0f, 0.0f };

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
        if (weapon->m_iItemDefinitionIndex() != Soldier_m_TheOriginal)
        {
            offset = { 23.5f, 12.0f, -3.0f };
            if (local->m_fFlags() & FL_DUCKING)
                offset.z = 8.0f;
        }
        break;

    case TF_WEAPON_COMPOUND_BOW:
        offset = { 20.5f, 12.0f, -3.0f };
        if (local->m_fFlags() & FL_DUCKING)
            offset.z = 8.0f;
        break;

    case TF_WEAPON_PIPEBOMBLAUNCHER:
    case TF_WEAPON_GRENADELAUNCHER:
    case TF_WEAPON_CANNON:
        offset = { 16.0f, 8.0f, -6.0f };
        break;

    default:
        return out; // No offset for other weapons
    }

    if (offset.x != 0.0f || offset.y != 0.0f || offset.z != 0.0f)
    {
        H::AimUtils->GetProjectileFireSetup(angles, offset, &out);
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
    {
        return false;
    }
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

bool CAimbotProjectile::CanSee(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const Vec3& vFrom, const Vec3& vTo, const ProjTarget_t& target, float flTargetTime)
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
        return CanArcReach(vFrom, vTo, target.AngleTo, flTargetTime, target.Entity);
    }

    if (m_CurProjInfo.Flamethrower)
    {
        return H::AimUtils->TraceFlames(target.Entity, vLocalPos, vTo);
    }
    return H::AimUtils->TraceProjectile(target.Entity, vLocalPos, vTo);
}

bool CAimbotProjectile::NeuralNetworkSplashPrediction(const Vec3& impactPoint, C_BaseEntity* pTargetEntity)
{
    if (!pTargetEntity) return false;
    // --- 1. Calculate Required Inputs ---
    Vec3 playerPosition = pTargetEntity->m_vecOrigin();
    Vec3 playerVelocity = pTargetEntity->m_vecVelocity();
    // A. Distance Factor (How far the player is from the splash point)
    float distanceToImpact = impactPoint.DistTo(playerPosition);
    float normalizedDistance = std::min(distanceToImpact / 1000.0f, 1.0f);
    // B. Speed Factor (How fast the player is moving)
    float playerSpeed = playerVelocity.Length();
    float normalizedVelocity = std::min(playerSpeed / 300.0f, 1.0f); // 300 HU/s is max walk speed
    // C. Direction Factor (New Crucial Input: Is the player moving towards or away from the impact?)
    // Calculate the vector pointing from the player to the impact zone.
    Vec3 directionToImpact = (impactPoint - playerPosition).Normalized();
    // The dot product measures the projection of the velocity onto the direction vector.
    // Positive value means moving TOWARDS the impact; negative means AWAY.
    float playerSpeedTowardsImpact = playerVelocity.Dot(directionToImpact);
    // Normalize the speed projection to a 0.0 to 1.0 range for the NN input.
    // (Assuming max speed is 300 HU/s, so range is -300 to 300).
    float normalizedDirection = (playerSpeedTowardsImpact + 300.0f) / 600.0f;
    // Input Layer: [Distance, Speed, Direction]
    float inputLayer[3] = { normalizedDistance, normalizedVelocity, normalizedDirection };
    // --- 2. Hidden Layer Calculation (Cleaner Matrix Math) ---
    const float hiddenLayerWeights[2][3] = {
        {0.2f, 0.3f, 0.5f},
        {0.4f, 0.1f, 0.2f}
    };
    const float hiddenLayerBias[2] = { 0.1f, -0.2f };
    float hiddenLayerOutput[2];
    // Refined loop structure for the dot product (addressing the 'TODO: optimize this' for clarity)
    for (int i = 0; i < 2; ++i) {
        float dotProduct = 0.0f;
        // J loop performs the vector dot product: Input[j] * Weight[i][j]
        for (int j = 0; j < 3; ++j) {
            dotProduct += inputLayer[j] * hiddenLayerWeights[i][j];
        }
        // Apply Bias and Activation Function
        hiddenLayerOutput[i] = 1.0f / (1.0f + expf(-(dotProduct + hiddenLayerBias[i]))); // Sigmoid
    }
    // --- 3. Output Layer Calculation ---
    const float outputLayerWeights[2] = { 0.7f, 0.9f };
    const float outputLayerBias = 0.1f;
    float finalDotProduct = (hiddenLayerOutput[0] * outputLayerWeights[0]) +
        (hiddenLayerOutput[1] * outputLayerWeights[1]);
    float output = 1.0f / (1.0f + expf(-(finalDotProduct + outputLayerBias))); // Sigmoid
    const float predictionThreshold = 0.5f; // Adjust as needed; add CFG if desired
    return output > predictionThreshold;
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
        for (int nTick = 0; nTick < TIME_TO_TICKS(CFG::Aimbot_Projectile_Max_Simulation_Time); nTick++)
        {
            m_TargetPath.push_back(F::MovementSimulation->GetOrigin());
            F::MovementSimulation->RunTick(TICKS_TO_TIME(nTick));
            Vec3 vTarget = F::MovementSimulation->GetOrigin();
            OffsetPlayerPosition(pWeapon, vTarget, pPlayer, bDucked, bOnGround);
            float flTimeToTarget = 0.0f;
            if (!CalcProjAngle(vLocalPos, vTarget, target.AngleTo, flTimeToTarget, false))
                continue;
            target.TimeToTarget = flTimeToTarget;
            int nTargetTick = TIME_TO_TICKS(flTimeToTarget + SDKUtils::GetLatency());
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
                auto runSplash = [&]() -> bool
                    {
                        // 1. Get Weapon Info & Splash Radius
                        const int weaponID = pWeapon->GetWeaponID();
                        const int defIndex = pWeapon->m_iItemDefinitionIndex();
                        bool isRocketLauncher = (weaponID == TF_WEAPON_ROCKETLAUNCHER);
                        bool isDirectHit = (weaponID == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT);
                        bool isAirStrike = (defIndex == Soldier_m_TheAirStrike);
                        // Filter invalid weapons immediately
                        if (!isRocketLauncher && !isDirectHit && !isAirStrike)
                            return false;
                        // Define Radius
                        float splashRadius = 146.0f; // Standard Rocket Radius
                        if (isRocketLauncher) splashRadius = 146.0f; // Adjust based on strict TF2 values if needed (standard is ~146hu)
                        if (isDirectHit) splashRadius = 44.0f; // DH is ~30% of standard
                        if (isAirStrike) splashRadius = 110.0f; // Airstrike is smaller
                        // 2. Setup Scanning Center
                        Vec3 mins = target.Entity->m_vecMins();
                        Vec3 maxs = target.Entity->m_vecMaxs();
                        Vec3 targetCenter = F::MovementSimulation->GetOrigin() + Vec3(0.0f, 0.0f, (mins.z + maxs.z) * 0.5f);
                        // 3. Generate Points (Fibonacci Sphere)
                        // Decreased count for performance; 80 is overkill, 45 covers most geometry.
                        const int numPoints = static_cast<int>(CFG::Aimbot_Projectile_SplashPoints);
                        std::vector<Vec3> potentialPoints;
                        potentialPoints.reserve(numPoints);
                        // Extend the scan radius slightly beyond the splash radius to find walls just out of range
                        // that might still clip the edge of the splash.
                        float scanRadius = splashRadius * 1.1f;
                        for (int n = 0; n < numPoints; n++)
                        {
                            // Fibonacci Sphere Math
                            float t = static_cast<float>(n) / static_cast<float>(numPoints);
                            float inclination = acosf(1.0f - 2.0f * t);
                            float azimuth = (PI * (3.0f - sqrtf(5.0f))) * static_cast<float>(n);
                            float x = sinf(inclination) * cosf(azimuth);
                            float y = sinf(inclination) * sinf(azimuth);
                            float z = cosf(inclination);
                            // Optimization: Skip points that are significantly above the target (Ceiling shots are rare/bad)
                            if (z > 0.5f) continue;
                            Vec3 dir(x, y, z);
                            Vec3 scanEnd = targetCenter + (dir * scanRadius);
                            // Trace from Target -> Outwards (Find walls around them)
                            CTraceFilterWorldCustom filter;
                            trace_t trace;
                            H::AimUtils->Trace(targetCenter, scanEnd, MASK_SOLID, &filter, &trace);
                            // If fraction is 1.0, we hit air. We need to hit a wall/floor.
                            if (trace.fraction >= 0.99f) continue;
                            // Verify the wall point is actually within lethal splash range of the target
                            // (The trace might have hit a wall far away if the scanRadius is huge)
                            if (trace.endpos.DistTo(targetCenter) > splashRadius) continue;
                            potentialPoints.push_back(trace.endpos);
                        }
                        if (potentialPoints.empty()) return false;
                        // 4. Sort Points by Damage Potential
                        // Logic: The closer the explosion is to the target's center, the more damage it deals.
                        std::sort(potentialPoints.begin(), potentialPoints.end(), [&](const Vec3& a, const Vec3& b) {
                            return a.DistTo(targetCenter) < b.DistTo(targetCenter);
                            });
                        // 5. Validate Firing
                        Vec3 localShootPos = GetProjectileFirePos(pLocal, pWeapon, pCmd->viewangles);
                        for (const auto& splashPoint : potentialPoints)
                        {
                            // Safety: Don't shoot if the splash point is too close to ourselves (Self-Damage check)
                            if (splashPoint.DistTo(localShootPos) < splashRadius) continue;
                            // Can we compute a firing solution?
                            Vec3 outAngle;
                            if (!CalcProjAngle(localShootPos, splashPoint, outAngle, target.TimeToTarget, false))
                            {
                                continue;
                            }
                            // Trace Hull: Can our rocket physically reach this spot?
                            trace_t trace = {};
                            CTraceFilterWorldCustom filter = {};
                            // Use a small hull for the rocket size
                            H::AimUtils->TraceHull(
                                localShootPos,
                                splashPoint,
                                { -2.0f, -2.0f, -2.0f }, // Slightly tighter hull than 4.0 for leniency
                                { 2.0f, 2.0f, 2.0f },
                                MASK_SOLID,
                                &filter,
                                &trace
                            );
                            // Did we hit something unexpected?
                            if (trace.startsolid || trace.allsolid || trace.fraction < 0.9f)
                            {
                                // If we hit something, was it the intended wall point?
                                // If the hit point is very close to our desired splashPoint, it's valid.
                                if (trace.endpos.DistTo(splashPoint) > 15.0f)
                                    continue;
                            }
                            // 6. Neural Network / Final Validation
                            // This is your "Is this a good idea?" check
                            if (NeuralNetworkSplashPrediction(splashPoint, target.Entity))
                            {
                                // Set the aim angles here (assuming your bot needs to set them)
                                target.AngleTo = outAngle;
                                return true;
                            }
                            I::DebugOverlay->AddBoxOverlay(splashPoint, { -4.0f, -4.0f, -4.0f }, { 4.0f, 4.0f, 4.0f }, Vec3{ 0.0f, 0.0f, 0.0f }, 0, 255, 0, 100, 1.5f);
                        }
                        return false;
                    };
                if (CFG::Aimbot_Projectile_SplashBot && runSplash())
                {
                    F::MovementSimulation->Restore();
                    return true;
                }
                if (CanSee(pLocal, pWeapon, vLocalPos, vTarget, target, flTimeToTarget))
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
        auto runSplash = [&]() -> bool
            {
                // 1. Setup Constants and Weapon Info
                const int weaponID = pWeapon->GetWeaponID();
                const int defIndex = pWeapon->m_iItemDefinitionIndex();
                const auto isRocketLauncher = (weaponID == TF_WEAPON_ROCKETLAUNCHER);
                const auto isDirectHit = (weaponID == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT);
                const auto isAirStrike = (defIndex == Soldier_m_TheAirStrike);
                if (!isRocketLauncher && !isDirectHit && !isAirStrike)
                    return false;
                // Use actual TF2 splash radiuses for max effectiveness (e.g., standard rocket is 146 HU)
                float splashRadius = 146.0f;
                if (isDirectHit) splashRadius = 44.0f; // Direct Hit radius is significantly smaller
                if (isAirStrike) splashRadius = 110.0f; // Air Strike is between standard and DH
                // Use a scan radius slightly larger than the splash radius to find external walls
                const float scanRadius = splashRadius * 1.1f;
                const Vec3 targetCenter = target.Entity->GetCenter();
                // If the projectile path is clear (fraction close to 1.0) and we hit the player,
                // we should use direct aim, NOT splash. So, we abort the splash logic.
                // We only continue if the player is blocked (trace hit something or fraction is low).
                if (H::AimUtils->TraceProjectile(target.Entity, vLocalPos, targetCenter))
                {
                    // The target is visible/hittable directly.
                    // If direct aim is available, we usually prefer it.
                    return false;
                }
                // If we reach here, the target is confirmed to be blocked by world geometry.
                // 2. Point Generation and Filtering (Optimized)
                // Reduced points from 80 to 40-50 for performance while maintaining good coverage.
                const int numPoints = static_cast<int>(CFG::Aimbot_Projectile_SplashPoints);
                std::vector<Vec3> potential{};
                potential.reserve(numPoints);
                for (int n = 0; n < numPoints; n++)
                {
                    // Fibonacci Sphere Generation
                    const float t = static_cast<float>(n) / static_cast<float>(numPoints);
                    const float inclination = acosf(1.0f - 2.0f * t);
                    const float azimuth = (PI * (3.0f - sqrtf(5.0f))) * static_cast<float>(n);
                    const float x = sinf(inclination) * cosf(azimuth);
                    const float y = sinf(inclination) * sinf(azimuth);
                    const float z = cosf(inclination);
                    // Optimization: Skip points significantly above the target (Z > 0.5 is upper hemisphere)
                    if (z > 0.5f) continue;
                    auto point = targetCenter + Vec3{ x, y, z } *scanRadius;
                    // Trace from Target -> Outwards (Find nearby geometry/walls)
                    CTraceFilterWorldCustom filter{};
                    trace_t trace{};
                    H::AimUtils->Trace(targetCenter, point, MASK_SOLID, &filter, &trace);
                    // If fraction >= 0.99f, we hit air. We need to hit a solid surface for splash.
                    if (trace.fraction >= 0.99f) continue;
                    // Pre-Filter: Ensure the wall point is close enough to the target for damage.
                    if (trace.endpos.DistTo(targetCenter) > splashRadius) continue;
                    // Safety: Prevent self-damage splash.
                    if (trace.endpos.DistTo(vLocalPos) < splashRadius * 0.9f) continue;
                    potential.push_back(trace.endpos);
                }
                if (potential.empty()) return false;
                // 3. Sort Points by Damage Potential
                // Sort by distance to the target's center. Closest point = highest splash damage.
                std::sort(potential.begin(), potential.end(), [&](const Vec3& a, const Vec3& b)
                    {
                        return a.DistTo(targetCenter) < b.DistTo(targetCenter);
                    });
                // 4. Validate Projectile Path and Prediction
                for (const auto& point : potential)
                {
                    Vec3 outAngle;
                    // Attempt to calculate the required firing angle and velocity arc
                    if (!CalcProjAngle(vLocalPos, point, outAngle, flTimeToTarget, false))
                    {
                        continue;
                    }
                    // Trace Hull: Check if the projectile path is clear to the splash point.
                    trace_t trace = {};
                    CTraceFilterWorldCustom filter = {};
                    H::AimUtils->TraceHull
                    (
                        vLocalPos,
                        point,
                        { -2.0f, -2.0f, -2.0f }, // Smaller hull for better clearance
                        { 2.0f, 2.0f, 2.0f },
                        MASK_SOLID,
                        &filter,
                        &trace
                    );
                    // Check 1: If the hull was stopped far short, it's not a clear shot.
                    // We tolerate a slight miss (< 15 HU) because the hull trace is conservative.
                    if (trace.startsolid || trace.allsolid || trace.endpos.DistTo(point) > 15.0f)
                    {
                        continue;
                    }
                    // The original code's second trace is redundant if the hull check passes and hits
                    // close to the intended point. If you want maximum safety, you can keep the
                    // secondary check (Trace from endpos of hull trace to final point) but it's often overkill.
                    // We rely on the distance check above instead.
                    bool splashDetected = NeuralNetworkSplashPrediction(point, target.Entity);
                    if (splashDetected)
                    {
                        target.AngleTo = outAngle; // Set the computed angle
                        target.Position = point;
                        return true;
                    }
                    I::DebugOverlay->AddBoxOverlay(point, { -4.0f, -4.0f, -4.0f }, { 4.0f, 4.0f, 4.0f }, Vec3{ 0.0f, 0.0f, 0.0f }, 0, 255, 0, 100, 1.5f);
                }
                return false;
            };
        if (CFG::Aimbot_Projectile_SplashBot && runSplash())
        {
            return true;
        }
        if (CanSee(pLocal, pWeapon, vLocalPos, vTarget, target, flTimeToTarget))
        {
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

bool CAimbotProjectile::KeyDown(const CUserCmd* pCmd)
{
    return false; // Implement if needed
}

bool CAimbotProjectile::ShouldAimKey()
{
    return false; // Implement if needed
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
    else
    {
        // Updated charging logic
        float requiredCharge = GetRequiredChargeTime(pWeapon, target, pLocal->GetShootPos());
        float currentCharge = GetCurrentChargeTime(pWeapon);
        if (currentCharge < requiredCharge)
        {
            pCmd->buttons |= IN_ATTACK; // Hold to charge
        }
        else
        {
            pCmd->buttons &= ~IN_ATTACK; // Release to fire
        }
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