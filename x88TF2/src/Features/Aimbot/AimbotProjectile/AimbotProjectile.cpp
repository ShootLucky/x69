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

    // ========== USAR A FUNÇÃO AUXILIAR ==========
    Vec3 offset = F::AimbotProjectile->GetWeaponFireOffset(weapon, local);

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
    if (!pLocal) return false;

    const auto pWeapon = H::Entities->GetWeapon();
    if (!pWeapon) return false;

    // ========== SETUP DE INFORMAÇÕES DO PROJÉTIL ==========
    ProjectileInfo info{};
    if (!F::ProjectileSim->GetInfo(pLocal, pWeapon, vAngleTo, info))
        return false;

    // Correção especial para Loch-n-Load
    if (pWeapon->m_iItemDefinitionIndex() == Demoman_m_TheLochnLoad)
    {
        info.m_speed += 45.0f;
    }

    if (!F::ProjectileSim->Init(info, true))
        return false;

    // ========== OTIMIZAÇÕES DE EARLY EXIT ==========
    const float maxDistance = vFrom.DistTo(vTo);
    const float projectileSpeed = info.m_speed;
    const float estimatedTime = maxDistance / (projectileSpeed + 0.001f);

    // Se o tempo estimado é muito maior que o esperado, já falha
    if (estimatedTime > flTargetTime * 1.5f)
        return false;

    // ========== SETUP DE TRACE FILTER ==========
    CTraceFilterWorldCustom filter{};
    filter.m_pTarget = pTarget;

    // ========== TAMANHO DO HULL BASEADO NO PROJÉTIL ==========
    Vec3 mins, maxs;

    switch (info.m_type)
    {
    case TF_PROJECTILE_PIPEBOMB:
    case TF_PROJECTILE_PIPEBOMB_REMOTE:
    case TF_PROJECTILE_PIPEBOMB_PRACTICE:
    case TF_PROJECTILE_CANNONBALL:
        mins = { -8.0f, -8.0f, -8.0f };
        maxs = { 8.0f, 8.0f, 20.0f };
        break;

    case TF_PROJECTILE_FLARE:
        mins = { -8.0f, -8.0f, -8.0f };
        maxs = { 8.0f, 8.0f, 8.0f };
        break;

    default:
        mins = { -6.0f, -6.0f, -6.0f };
        maxs = { 6.0f, 6.0f, 6.0f };
        break;
    }

    // ========== SIMULAÇÃO OTIMIZADA ==========
    const int maxTicks = TIME_TO_TICKS(flTargetTime * 1.2f);
    const float targetDistSqr = vTo.DistToSqr(vFrom); // Usa distância ao quadrado (mais rápido)

    // Variáveis para detecção de "muito próximo"
    float closestDistSqr = FLT_MAX;
    bool wasGettingCloser = false;
    int ticksSinceClosest = 0;

    for (int n = 0; n < maxTicks; n++)
    {
        const Vec3 pre = F::ProjectileSim->GetOrigin();
        F::ProjectileSim->RunTick();
        const Vec3 post = F::ProjectileSim->GetOrigin();

        // ========== EARLY EXIT: Se está se afastando muito do alvo ==========
        const float currentDistSqr = post.DistToSqr(vTo);

        if (currentDistSqr < closestDistSqr)
        {
            closestDistSqr = currentDistSqr;
            wasGettingCloser = true;
            ticksSinceClosest = 0;
        }
        else
        {
            ticksSinceClosest++;

            // Se estava ficando perto mas agora está se afastando por muito tempo, desiste
            if (wasGettingCloser && ticksSinceClosest > 5)
            {
                // Só desiste se está realmente longe (30 HU)
                if (currentDistSqr > 900.0f) // 30^2
                    return false;
            }
        }

        // ========== TRACE HULL OTIMIZADO ==========
        trace_t trace{};
        H::AimUtils->TraceHull(pre, post, mins, maxs, MASK_SOLID, &filter, &trace);

        // ========== VERIFICAÇÃO DE ACERTO DIRETO ==========
        if (trace.m_pEnt == pTarget)
        {
            return true;
        }

        // ========== VERIFICAÇÃO DE COLISÃO ==========
        if (trace.DidHit())
        {
            const float distToImpact = info.m_pos.DistTo(trace.endpos);
            const float distToTarget = info.m_pos.DistTo(vTo);

            // Se o projétil já passou do alvo, considera que pode acertar
            if (distToImpact > distToTarget)
            {
                return true;
            }

            // Se bateu muito longe do alvo (40 HU+), falhou
            const float impactToTargetDist = trace.endpos.DistTo(vTo);
            if (impactToTargetDist > 40.0f)
            {
                return false;
            }

            // ========== VERIFICAÇÃO FINAL: Linha de visão do impacto até o alvo ==========
            trace_t finalTrace{};
            H::AimUtils->Trace(trace.endpos, vTo, MASK_SOLID, &filter, &finalTrace);

            // Se não bateu em nada OU bateu no alvo = sucesso
            return !finalTrace.DidHit() || finalTrace.m_pEnt == pTarget;
        }

        // ========== EARLY EXIT: Passou muito longe do tempo esperado ==========
        if (n > TIME_TO_TICKS(flTargetTime * 1.3f))
        {
            // Verifica se pelo menos está perto do alvo
            if (post.DistToSqr(vTo) < 625.0f) // 25^2 HU
                continue; // Continua simulando se estiver perto
            else
                return false;
        }
    }

    return false;
}

// ========== FUNÇÃO AUXILIAR PARA CALCULAR OFFSET ==========
Vec3 CAimbotProjectile::GetWeaponFireOffset(C_TFWeaponBase* pWeapon, C_TFPlayer* pLocal)
{
    const int weaponID = pWeapon->GetWeaponID();
    const bool bDucking = (pLocal->m_fFlags() & FL_DUCKING) != 0;
    const int defIndex = pWeapon->m_iItemDefinitionIndex();

    switch (weaponID)
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
        if (defIndex != Soldier_m_TheOriginal)
        {
            return { 23.5f, 12.0f, bDucking ? 8.0f : -3.0f };
        }
        break;
    }
    case TF_WEAPON_COMPOUND_BOW:
    {
        return { 20.5f, 12.0f, bDucking ? 8.0f : -3.0f };
    }
    case TF_WEAPON_PIPEBOMBLAUNCHER:
    case TF_WEAPON_GRENADELAUNCHER:
    case TF_WEAPON_CANNON:
    {
        return { 16.0f, 8.0f, -6.0f };
    }
    default:
        break;
    }

    return { 0.0f, 0.0f, 0.0f };
}

bool CAimbotProjectile::CanSee(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const Vec3& vFrom, const Vec3& vTo, const ProjTarget_t& target, float flTargetTime)
{
    Vec3 vLocalPos = vFrom;

    // ========== CALCULAR OFFSET UMA ÚNICA VEZ ==========
    const Vec3 vOffset = GetWeaponFireOffset(pWeapon, pLocal);

    if (vOffset.x != 0.0f || vOffset.y != 0.0f || vOffset.z != 0.0f)
    {
        H::AimUtils->GetProjectileFireSetup(target.AngleTo, vOffset, &vLocalPos);
    }

    // ========== VERIFICAÇÕES POR TIPO DE PROJÉTIL ==========

    // 1. PROJÉTEIS COM GRAVIDADE (arco/pipes/rockets)
    if (m_CurProjInfo.GravityMod != 0.0f)
    {
        return CanArcReach(vFrom, vTo, target.AngleTo, flTargetTime, target.Entity);
    }

    // 2. FLAMETHROWER (trace especial)
    if (m_CurProjInfo.Flamethrower)
    {
        return H::AimUtils->TraceFlames(target.Entity, vLocalPos, vTo);
    }

    // 3. PROJÉTEIS DIRETOS (flechas, seringas, etc)
    return H::AimUtils->TraceProjectile(target.Entity, vLocalPos, vTo);
}

bool CAimbotProjectile::NeuralNetworkSplashPrediction(const Vec3& impactPoint, C_BaseEntity* pTargetEntity)
{
    if (!pTargetEntity) return false;

    // ========== CACHE DE DADOS BÁSICOS ==========
    const Vec3 playerPosition = pTargetEntity->m_vecOrigin();
    const Vec3 playerVelocity = pTargetEntity->m_vecVelocity();

    // ========== EARLY EXIT: Distância ==========
    const float distanceToImpact = impactPoint.DistTo(playerPosition);
    if (distanceToImpact > 200.0f)
        return false;

    // ========== EARLY EXIT: Altura ==========
    const float heightDifference = fabsf(impactPoint.z - playerPosition.z);
    if (heightDifference > 150.0f)
        return false;

    // ========== VERIFICAÇÃO CRÍTICA: LINHA DE VISÃO DO SPLASH ==========
    // O splash precisa ter linha de visão até o jogador para causar dano
    const Vec3 playerCenter = playerPosition + Vec3(0.0f, 0.0f, 41.0f); // Centro do player (meio da altura)

    CTraceFilterWorldCustom filter{};
    filter.m_pTarget = pTargetEntity;
    trace_t splashTrace{};

    // Trace do ponto de impacto até o centro do jogador
    H::AimUtils->Trace(impactPoint, playerCenter, MASK_SOLID, &filter, &splashTrace);

    // Se bateu em algo que NÃO é o jogador, o splash está bloqueado
    if (splashTrace.DidHit() && splashTrace.m_pEnt != pTargetEntity)
    {
        // A parede está bloqueando o splash completamente
        return false;
    }

    // Verificação adicional: trace para os pés do jogador também
    const Vec3 playerFeet = playerPosition + Vec3(0.0f, 0.0f, 10.0f);
    trace_t feetTrace{};
    H::AimUtils->Trace(impactPoint, playerFeet, MASK_SOLID, &filter, &feetTrace);

    // Se ambos traces (centro e pés) estão bloqueados, definitivamente não vai causar dano
    if (feetTrace.DidHit() && feetTrace.m_pEnt != pTargetEntity)
    {
        // Splash completamente bloqueado
        return false;
    }

    // ========== INPUT LAYER (3 inputs otimizados) ==========
    const float normalizedDistance = std::min(distanceToImpact * 0.001f, 1.0f);
    const float playerSpeed = playerVelocity.Length();
    const float normalizedVelocity = std::min(playerSpeed * 0.00333f, 1.0f);

    const float invDistance = 1.0f / (distanceToImpact + 0.001f);
    const Vec3 directionToImpact = (impactPoint - playerPosition) * invDistance;
    const float playerSpeedTowardsImpact = playerVelocity.Dot(directionToImpact);
    const float normalizedDirection = (playerSpeedTowardsImpact + 300.0f) * 0.001666f;

    // ========== HIDDEN LAYER (2 neurons) ==========
    static constexpr float hiddenWeights[2][3] = {
        {-0.8f,  0.4f,  0.6f},
        { 0.3f, -0.2f,  0.7f}
    };
    static constexpr float hiddenBias[2] = { 0.3f, -0.1f };

    float hidden[2];
    {
        const float dot0 = normalizedDistance * hiddenWeights[0][0] +
            normalizedVelocity * hiddenWeights[0][1] +
            normalizedDirection * hiddenWeights[0][2];

        const float dot1 = normalizedDistance * hiddenWeights[1][0] +
            normalizedVelocity * hiddenWeights[1][1] +
            normalizedDirection * hiddenWeights[1][2];

        hidden[0] = 1.0f / (1.0f + expf(-(dot0 + hiddenBias[0])));
        hidden[1] = 1.0f / (1.0f + expf(-(dot1 + hiddenBias[1])));
    }

    // ========== OUTPUT LAYER ==========
    static constexpr float outputWeights[2] = { 0.85f, 0.75f };
    static constexpr float outputBias = 0.15f;

    const float finalDot = (hidden[0] * outputWeights[0]) + (hidden[1] * outputWeights[1]);
    const float output = 1.0f / (1.0f + expf(-(finalDot + outputBias)));

    // ========== THRESHOLD DINÂMICO ==========
    float dynamicThreshold = 0.45f;

    if (distanceToImpact < 50.0f)
        dynamicThreshold = 0.35f;
    else if (distanceToImpact < 100.0f)
        dynamicThreshold = 0.45f;
    else
        dynamicThreshold = 0.60f;

    return output > dynamicThreshold;
}

bool CAimbotProjectile::TrySplashShot(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon,
    const CUserCmd* pCmd, const ProjTarget_t& target, Vec3& outAngle, float& outTime, bool isPlayer)
{
    const int weaponID = pWeapon->GetWeaponID();
    const int defIndex = pWeapon->m_iItemDefinitionIndex();

    const bool isRocketLauncher = (weaponID == TF_WEAPON_ROCKETLAUNCHER);
    const bool isDirectHit = (weaponID == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT);
    const bool isAirStrike = (defIndex == Soldier_m_TheAirStrike);

    if (!isRocketLauncher && !isDirectHit && !isAirStrike)
        return false;

    constexpr float ROCKET_LAUNCHER_SPLASH = 146.0f;
    constexpr float DIRECT_HIT_SPLASH = 44.0f;
    constexpr float AIRSTRIKE_SPLASH = 110.0f;

    float splashRadius = ROCKET_LAUNCHER_SPLASH;
    if (isDirectHit) splashRadius = DIRECT_HIT_SPLASH;
    if (isAirStrike) splashRadius = AIRSTRIKE_SPLASH;

    const float scanRadius = splashRadius * 1.1f;
    const Vec3 vLocalPos = GetProjectileFirePos(pLocal, pWeapon, pCmd->viewangles);
    const Vec3 targetCenter = isPlayer ?
        F::MovementSimulation->GetOrigin() + Vec3(0.0f, 0.0f, 41.0f) :
        target.Entity->GetCenter();

    // ========== PRÉ-VERIFICAÇÃO: VISÃO DIRETA ==========
    CTraceFilterWorldCustom directFilter{};
    trace_t directTrace{};
    H::AimUtils->Trace(vLocalPos, targetCenter, MASK_SOLID, &directFilter, &directTrace);

    if (!directTrace.DidHit() || directTrace.fraction > 0.95f || directTrace.m_pEnt == target.Entity)
    {
        return false; // Visão direta disponível, não precisa splash
    }

    float distToBlock = vLocalPos.DistTo(directTrace.endpos);
    float distToTarget = vLocalPos.DistTo(targetCenter);

    if (distToBlock >= distToTarget * 0.7f)
    {
        return false; // Bloqueio muito perto do alvo
    }

    // ========== GERAÇÃO DE PONTOS DE SPLASH ==========
    std::vector<Vec3> potentialPoints;
    potentialPoints.reserve(CFG::Aimbot_Projectile_SplashPoints);

    const int numPoints = static_cast<int>(CFG::Aimbot_Projectile_SplashPoints);

    for (int n = 0; n < numPoints; n++)
    {
        const float t = static_cast<float>(n) / static_cast<float>(numPoints);
        const float inclination = acosf(1.0f - 2.0f * t);
        const float azimuth = (PI * (3.0f - sqrtf(5.0f))) * static_cast<float>(n);

        const float x = sinf(inclination) * cosf(azimuth);
        const float y = sinf(inclination) * sinf(azimuth);
        const float z = cosf(inclination);

        if (z > 0.5f) continue;

        const Vec3 scanPoint = targetCenter + Vec3(x, y, z) * scanRadius;

        CTraceFilterWorldCustom filter{};
        trace_t trace{};
        H::AimUtils->Trace(targetCenter, scanPoint, MASK_SOLID, &filter, &trace);

        if (trace.fraction >= 0.99f) continue;
        if (trace.endpos.DistTo(targetCenter) > splashRadius) continue;
        if (trace.endpos.DistTo(vLocalPos) < splashRadius * 0.9f) continue;

        // Verificação 1: Ponto até alvo
        trace_t losTrace{};
        H::AimUtils->Trace(trace.endpos, targetCenter, MASK_SOLID, &filter, &losTrace);

        if (losTrace.DidHit() && losTrace.fraction < 0.95f)
            continue;

        // Verificação 2: Nós até ponto
        trace_t visTrace{};
        H::AimUtils->Trace(vLocalPos, trace.endpos, MASK_SOLID, &filter, &visTrace);

        if (visTrace.DidHit() && visTrace.fraction < 0.85f)
        {
            float distToVBlock = vLocalPos.DistTo(visTrace.endpos);
            float distToPoint = vLocalPos.DistTo(trace.endpos);

            if (distToVBlock < distToPoint * 0.85f)
                continue;
        }

        potentialPoints.push_back(trace.endpos);
    }

    if (potentialPoints.empty())
        return false;

    std::sort(potentialPoints.begin(), potentialPoints.end(), [&](const Vec3& a, const Vec3& b)
        {
            return a.DistTo(targetCenter) < b.DistTo(targetCenter);
        });

    // ========== TESTE DE TRAJETÓRIA ==========
    for (const auto& splashPoint : potentialPoints)
    {
        if (splashPoint.DistTo(vLocalPos) < splashRadius)
            continue;

        Vec3 calcAngle;
        float splashTime = 0.0f;

        if (!CalcProjAngle(vLocalPos, splashPoint, calcAngle, splashTime, false))
            continue;

        trace_t projTrace{};
        CTraceFilterWorldCustom projFilter{};

        H::AimUtils->TraceHull(
            vLocalPos,
            splashPoint,
            { -2.0f, -2.0f, -2.0f },
            { 2.0f, 2.0f, 2.0f },
            MASK_SOLID,
            &projFilter,
            &projTrace
        );

        if (projTrace.startsolid || projTrace.allsolid)
            continue;

        if (projTrace.fraction < 0.85f && projTrace.endpos.DistTo(splashPoint) > 20.0f)
            continue;

        if (NeuralNetworkSplashPrediction(splashPoint, target.Entity))
        {
            outAngle = calcAngle;
            outTime = splashTime;
            return true;
        }
    }

    return false;
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
                const auto sticky_arm_time = SDKUtils::AttribHookValue(0.8f, "sticky_arm_time", pLocal);
                if (TICKS_TO_TIME(nTargetTick) < sticky_arm_time)
                {
                    nTargetTick += TIME_TO_TICKS(fabsf(flTimeToTarget - sticky_arm_time));
                }
            }

            if ((nTargetTick == nTick || nTargetTick == nTick - 1))
            {
                // IMPORTANTE: Verifica tiro direto PRIMEIRO (mais confiável)
                if (CanSee(pLocal, pWeapon, vLocalPos, vTarget, target, flTimeToTarget))
                {
                    F::MovementSimulation->Restore();
                    return true;
                }

                // Se tiro direto falhou, tenta splash
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
    else // Alvo não é jogador (buildings, etc)
    {
        const Vec3 vTarget = target.Position;
        float flTimeToTarget = 0.0f;

        if (!CalcProjAngle(vLocalPos, vTarget, target.AngleTo, flTimeToTarget, false))
            return false;

        target.TimeToTarget = flTimeToTarget;

        // IMPORTANTE: Verifica tiro direto PRIMEIRO
        if (CanSee(pLocal, pWeapon, vLocalPos, vTarget, target, flTimeToTarget))
        {
            return true;
        }

        // Se tiro direto falhou, tenta splash
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

    // ========== COLETA DE PLAYERS ==========
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

            // ========== VERIFICAÇÕES DE INIMIGOS ==========
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
            // ========== VERIFICAÇÕES DE ALIADOS (CROSSBOW) ==========
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

            // ========== FILTRO FOV ==========
            if (CFG::Aimbot_Projectile_Sort == 0 && flFOVTo > CFG::Aimbot_Projectile_FOV)
                continue;

            m_vecTargets.emplace_back(ProjTarget_t{ pPlayer, vPos, vAngleTo, flFOVTo, flDistTo });
        }
    }

    // ========== COLETA DE BUILDINGS ==========
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

            // ========== VERIFICAÇÕES DE RESCUE RANGER ==========
            if (isRescueRanger && pBuilding->m_iTeamNum() == pLocal->m_iTeamNum() &&
                pBuilding->m_iHealth() >= pBuilding->m_iMaxHealth())
            {
                continue;
            }

            Vec3 vPos = pBuilding->GetCenter();
            Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
            const float flFOVTo = CFG::Aimbot_Projectile_Sort == 0 ? Math::CalcFov(vLocalAngles, vAngleTo) : 0.0f;
            const float flDistTo = vLocalPos.DistTo(vPos);

            // ========== FILTRO FOV ==========
            if (CFG::Aimbot_Projectile_Sort == 0 && flFOVTo > CFG::Aimbot_Projectile_FOV)
                continue;

            m_vecTargets.emplace_back(ProjTarget_t{ pBuilding, vPos, vAngleTo, flFOVTo, flDistTo });
        }
    }

    if (m_vecTargets.empty())
        return false;

    // ========== ORDENAÇÃO POR PRIORIDADE ==========
    F::AimbotCommon->Sort(m_vecTargets, CFG::Aimbot_Projectile_Sort);

    // ========== PROCESSAMENTO DE ALVOS COM LIMITE INTELIGENTE ==========
    const int maxTargets = std::min(CFG::Aimbot_Projectile_Max_Processing_Targets, static_cast<int>(m_vecTargets.size()));
    const float maxRangeForFullScan = 400.0f;

    for (int i = 0; i < static_cast<int>(m_vecTargets.size()); i++)
    {
        auto& target = m_vecTargets[i];
        const float distToTarget = target.Position.DistTo(vLocalPos);

        // ========== LÓGICA DE LIMITE: Alvos longe só são processados se dentro do limite ==========
        // Se alvo está longe AND já processamos max_targets, pula
        if (distToTarget > maxRangeForFullScan && i >= maxTargets)
        {
            continue;
        }

        // ========== RESOLVER ALVO ==========
        if (!SolveTarget(pLocal, pWeapon, pCmd, target))
            continue;

        // ========== VERIFICAÇÃO FOV FINAL ==========
        if (CFG::Aimbot_Projectile_Sort == 0 && Math::CalcFov(vLocalAngles, target.AngleTo) > CFG::Aimbot_Projectile_FOV)
            continue;

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