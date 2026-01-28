#include "ProjectileSim.h"

bool CProjectileSim::GetInfo(C_TFPlayer* player, C_TFWeaponBase* weapon, const Vec3& angles, ProjectileInfo& out)
{
    if (!player || !weapon)
    {
        return false;
    }

    auto cur_time = static_cast<float>(player->m_nTickBase()) * TICK_INTERVAL;
    auto ducking = player->m_fFlags() & FL_DUCKING;
    Vec3 pos{};
    Vec3 ang{};
    float radius = 0.0f;

    switch (weapon->GetWeaponID())
    {
    case TF_WEAPON_ROCKETLAUNCHER:
    case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
    case TF_WEAPON_PARTICLE_CANNON:
    {
        Vec3 offset = { 23.5f, 12.0f, ducking ? 8.0f : -3.0f };
        if (weapon->m_iItemDefinitionIndex() == Soldier_m_TheOriginal)
        {
            offset = { 0.0f, 0.0f, 0.0f };
        }
        SDKUtils::GetProjectileFireSetupRebuilt(player, offset, angles, pos, ang, false);
        float speed = SDKUtils::AttribHookValue(1100.0f, "mult_projectile_speed", weapon);
        int specialist = static_cast<int>(SDKUtils::AttribHookValue(0.0f, "rocket_specialist", player));
        if (specialist)
        {
            speed *= Math::RemapValClamped(static_cast<float>(specialist), 1.0f, 4.0f, 1.15f, 1.6f);
            speed = std::min(speed, 3000.0f);
        }
        radius = SDKUtils::AttribHookValue(146.0f, "mult_explosion_radius", weapon);
        out.m_type = SimProjType::ROCKET;
        out.m_pos = pos;
        out.m_ang = ang;
        out.m_speed = speed;
        out.m_gravity_mod = 0.0f;
        out.no_spin = true;
        out.m_radius = radius;
        break;
    }
    case TF_WEAPON_GRENADELAUNCHER:
    {
        SDKUtils::GetProjectileFireSetupRebuilt(player, { 16.0f, 8.0f, -6.0f }, angles, pos, ang, true);
        auto is_lochnload = weapon->m_iItemDefinitionIndex() == Demoman_m_TheLochnLoad;
        auto speed = SDKUtils::AttribHookValue(1200.0f, "mult_projectile_speed", weapon);
        radius = SDKUtils::AttribHookValue(146.0f, "mult_explosion_radius", weapon);
        out.m_type = SimProjType::PIPEBOMB;
        out.m_pos = pos;
        out.m_ang = ang;
        out.m_speed = speed;
        out.m_gravity_mod = 1.0f;
        out.no_spin = is_lochnload;
        out.m_radius = radius;
        break;
    }
    case TF_WEAPON_PIPEBOMBLAUNCHER:
    {
        SDKUtils::GetProjectileFireSetupRebuilt(player, { 16.0f, 8.0f, -6.0f }, angles, pos, ang, true);

        // ✅ Cast correto usando C_TFPipebombLauncher que está definido
        auto pPipeLauncher = weapon->As<C_TFPipebombLauncher>();
        if (!pPipeLauncher)
            return false;

        auto charge_begin_time = pPipeLauncher->m_flChargeBeginTime();
        auto charge = cur_time - charge_begin_time;
        auto speed = Math::RemapValClamped(charge, 0.0f, SDKUtils::AttribHookValue(4.0f, "stickybomb_charge_rate", weapon), 900.0f, 2400.0f);
        if (charge_begin_time <= 0.0f)
        {
            speed = 900.0f;
        }
        radius = SDKUtils::AttribHookValue(146.0f, "mult_explosion_radius", weapon);
        out.m_type = SimProjType::PIPEBOMB_REMOTE;
        out.m_pos = pos;
        out.m_ang = ang;
        out.m_speed = speed;
        out.m_gravity_mod = 1.0f;
        out.no_spin = false;
        out.m_radius = radius;
        break;
    }
    case TF_WEAPON_CANNON:
    {
        SDKUtils::GetProjectileFireSetupRebuilt(player, { 16.0f, 8.0f, -6.0f }, angles, pos, ang, true);
        radius = SDKUtils::AttribHookValue(146.0f, "mult_explosion_radius", weapon);
        out.m_type = SimProjType::CANNONBALL;
        out.m_pos = pos;
        out.m_ang = ang;
        out.m_speed = 1454.0f;
        out.m_gravity_mod = 1.0f;
        out.no_spin = false;
        out.m_radius = radius;
        break;
    }
    case TF_WEAPON_FLAREGUN:
    {
        SDKUtils::GetProjectileFireSetupRebuilt(player, { 23.5f, 12.0f, ducking ? 8.0f : -3.0f }, angles, pos, ang, false);
        out.m_type = SimProjType::FLARE;
        out.m_pos = pos;
        out.m_ang = ang;
        out.m_speed = 2000.0f;
        out.m_gravity_mod = 0.3f;
        out.no_spin = true;
        out.m_radius = 0.0f;
        break;
    }
    case TF_WEAPON_FLAREGUN_REVENGE:
    {
        SDKUtils::GetProjectileFireSetupRebuilt(player, { 23.5f, 12.0f, ducking ? 8.0f : -3.0f }, angles, pos, ang, false);
        out.m_type = SimProjType::FLARE;
        out.m_pos = pos;
        out.m_ang = ang;
        out.m_speed = 3000.0f;
        out.m_gravity_mod = 0.45f;
        out.no_spin = true;
        out.m_radius = 0.0f;
        break;
    }
    case TF_WEAPON_FLAME_BALL:
    {
        SDKUtils::GetProjectileFireSetupRebuilt(player, { 23.5f, 12.0f, ducking ? 8.0f : -3.0f }, angles, pos, ang, false);
        out.m_type = SimProjType::FLARE;
        out.m_pos = pos;
        out.m_ang = ang;
        out.m_speed = 3000.0f;
        out.m_gravity_mod = 0.0f;
        out.no_spin = true;
        out.m_radius = 0.0f;
        break;
    }
    case TF_WEAPON_COMPOUND_BOW:
    {
        SDKUtils::GetProjectileFireSetupRebuilt(player, { 23.5f, 8.0f, -3.0f }, angles, pos, ang, false);

        // ✅ CORREÇÃO: Usa C_TFPipebombLauncher ao invés de C_TFCompoundBow inexistente
        // O Huntsman (Compound Bow) compartilha a mesma classe base de charge que pipebomb launcher
        auto pBowLauncher = weapon->As<C_TFPipebombLauncher>();
        float speed = 1800.0f;
        float grav_mod = 0.5f;

        if (pBowLauncher)
        {
            auto charge_begin_time = pBowLauncher->m_flChargeBeginTime();
            if (charge_begin_time > 0.0f)
            {
                auto charge = cur_time - charge_begin_time;
                speed = Math::RemapValClamped(charge, 0.0f, 1.0f, 1800.0f, 2600.0f);
                grav_mod = Math::RemapValClamped(charge, 0.0f, 1.0f, 0.5f, 0.1f);
            }
        }

        out.m_type = SimProjType::ARROW;
        out.m_pos = pos;
        out.m_ang = ang;
        out.m_speed = speed;
        out.m_gravity_mod = grav_mod;
        out.no_spin = true;
        out.m_radius = 0.0f;
        break;
    }
    case TF_WEAPON_CROSSBOW:
    case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
    {
        SDKUtils::GetProjectileFireSetupRebuilt(player, { 23.5f, 8.0f, -3.0f }, angles, pos, ang, false);
        out.m_type = SimProjType::ARROW;
        out.m_pos = pos;
        out.m_ang = ang;
        out.m_speed = 2400.0f;
        out.m_gravity_mod = 0.2f;
        out.no_spin = true;
        out.m_radius = 0.0f;
        break;
    }
    case TF_WEAPON_SYRINGEGUN_MEDIC:
    {
        SDKUtils::GetProjectileFireSetupRebuilt(player, { 16.0f, 6.0f, -8.0f }, angles, pos, ang, false);
        out.m_type = SimProjType::SYRINGE;
        out.m_pos = pos;
        out.m_ang = ang;
        out.m_speed = 1000.0f;
        out.m_gravity_mod = 0.3f;
        out.no_spin = true;
        out.m_radius = 0.0f;
        break;
    }
    case TF_WEAPON_RAYGUN:
    case TF_WEAPON_DRG_POMSON:
    {
        SDKUtils::GetProjectileFireSetupRebuilt(player, { 23.5f, 12.0f, ducking ? 8.0f : -3.0f }, angles, pos, ang, false);
        out.m_type = SimProjType::ENERGY_BALL;
        out.m_pos = pos;
        out.m_ang = ang;
        out.m_speed = 1200.0f;
        out.m_gravity_mod = 0.0f;
        out.no_spin = true;
        out.m_radius = 0.0f;
        break;
    }
    default:
        return false;
    }
    return true;
}

bool CProjectileSim::GetInfo(C_TFPlayer* player, C_TFWeaponBase* weapon, const Vec3& angles, ProjectileInfo& out, int flags)
{
    // Sobrecarga para compatibilidade
    return GetInfo(player, weapon, angles, out);
}

bool CProjectileSim::Init(const ProjectileInfo& info, bool no_vec_up)
{
    if (!m_env)
    {
        m_env = I::Physics->CreateEnvironment();
    }

    // Determine bbox size
    Vec3 bbox_min = { -1.0f, -1.0f, -1.0f };
    Vec3 bbox_max = { 1.0f, 1.0f, 1.0f };
    switch (info.m_type)
    {
    case SimProjType::PIPEBOMB:
    case SimProjType::PIPEBOMB_REMOTE:
    case SimProjType::PIPEBOMB_PRACTICE:
    case SimProjType::CANNONBALL:
        bbox_min = { -2.0f, -2.0f, -2.0f };
        bbox_max = { 2.0f, 2.0f, 2.0f };
        break;
    case SimProjType::ROCKET:
    case SimProjType::FLARE:
    case SimProjType::SYRINGE:
    case SimProjType::ENERGY_BALL:
    case SimProjType::ARROW:
        bbox_min = { -0.5f, -0.5f, -0.5f };
        bbox_max = { 0.5f, 0.5f, 0.5f };
        break;
    default:
        break;
    }

    if (!m_obj)
    {
        auto col = I::PhysicsCollision->BBoxToCollide(bbox_min, bbox_max);
        auto params = g_PhysDefaultObjectParams;
        params.damping = 0.0f;
        params.rotdamping = 0.0f;
        params.inertia = 0.0f;
        params.rotInertiaLimit = 0.0f;
        params.enableCollisions = true;
        m_obj = m_env->CreatePolyObject(col, 0, info.m_pos, info.m_ang, &params);
        m_obj->Wake();
    }

    if (!m_env || !m_obj)
    {
        return false;
    }

    // Reset states
    m_hasImpacted = false;
    m_impactPos = {};
    m_radius = info.m_radius;
    m_tickCount = 0;

    // Set position and velocity
    {
        Vec3 forward{}, up{};
        Math::AngleVectors(info.m_ang, &forward, nullptr, &up);
        Vec3 vel = forward * info.m_speed;
        Vec3 ang_vel{};

        switch (info.m_type)
        {
        case SimProjType::PIPEBOMB:
        case SimProjType::PIPEBOMB_REMOTE:
        case SimProjType::PIPEBOMB_PRACTICE:
        case SimProjType::CANNONBALL:
            if (!no_vec_up)
            {
                vel += up * 200.0f;
            }
            ang_vel = { 600.0f, -1200.0f, 0.0f };
            break;
        default:
            break;
        }

        if (info.no_spin)
        {
            ang_vel.Zero();
        }

        m_obj->SetPosition(info.m_pos, info.m_ang, true);
        m_obj->SetVelocity(&vel, &ang_vel);
    }

    // Set drag
    {
        float drag = 1.0f;
        Vec3 drag_basis{};
        Vec3 ang_drag_basis{};

        switch (info.m_type)
        {
        case SimProjType::PIPEBOMB:
            drag_basis = { 0.003902f, 0.009962f, 0.009962f };
            ang_drag_basis = { 0.003618f, 0.001514f, 0.001514f };
            break;
        case SimProjType::PIPEBOMB_REMOTE:
        case SimProjType::PIPEBOMB_PRACTICE:
            drag_basis = { 0.007491f, 0.007491f, 0.007306f };
            ang_drag_basis = { 0.002777f, 0.002842f, 0.002812f };
            break;
        case SimProjType::CANNONBALL:
            drag_basis = { 0.020971f, 0.019420f, 0.020971f };
            ang_drag_basis = { 0.012997f, 0.013496f, 0.013714f };
            break;
        default:
            break;
        }

        m_obj->SetDragCoefficient(&drag, &drag);
        m_obj->m_dragBasis = drag_basis;
        m_obj->m_angDragBasis = ang_drag_basis;
    }

    // Set env params
    {
        auto max_vel = 1000000.0f;
        auto max_ang_vel = 1000000.0f;

        switch (info.m_type)
        {
        case SimProjType::PIPEBOMB:
        case SimProjType::PIPEBOMB_REMOTE:
        case SimProjType::PIPEBOMB_PRACTICE:
        case SimProjType::CANNONBALL:
            max_vel = k_flMaxVelocity;
            max_ang_vel = k_flMaxAngularVelocity;
            break;
        default:
            break;
        }

        physics_performanceparams_t params{};
        params.Defaults();
        params.maxVelocity = max_vel;
        params.maxAngularVelocity = max_ang_vel;
        m_env->SetPerformanceSettings(&params);
        m_env->SetAirDensity(2.0f);
        m_env->SetGravity({ 0.0f, 0.0f, -(800.0f * info.m_gravity_mod) });
        m_env->ResetSimulationClock();
    }

    return true;
}

void CProjectileSim::RunTick()
{
    if (!m_env || m_hasImpacted || m_tickCount >= MAX_TICKS)
    {
        return;
    }

    const int substeps = 4;
    float sub_dt = TICK_INTERVAL / static_cast<float>(substeps);

    for (int i = 0; i < substeps; ++i)
    {
        m_env->Simulate(sub_dt);
        m_tickCount++;

        if (!m_hasImpacted)
        {
            Vec3 contact{};
            if (m_obj->GetContactPoint(&contact, nullptr))
            {
                m_impactPos = contact;
                m_hasImpacted = true;
                m_obj->Sleep();
                break;
            }
        }
    }
}

Vec3 CProjectileSim::GetOrigin()
{
    if (!m_obj)
    {
        return {};
    }

    Vec3 out{};
    m_obj->GetPosition(&out, nullptr);
    return out;
}

Vec3 CProjectileSim::GetImpactOrigin() const
{
    return m_hasImpacted ? m_impactPos : Vec3{};
}

bool CProjectileSim::HasImpacted() const
{
    return m_hasImpacted;
}

float CProjectileSim::GetExplosionRadius() const
{
    return m_radius;
}

bool CProjectileSim::CanHitWithSplash(const Vec3& targetPos) const
{
    if (!HasImpacted() || m_radius <= 0.0f)
    {
        return false;
    }

    Vec3 impact = GetImpactOrigin();
    float dist = impact.DistTo(targetPos);
    if (dist > m_radius)
    {
        return false;
    }

    Ray_t ray;
    ray.Init(impact, targetPos);
    trace_t tr;
    CTraceFilterWorldOnly filter;
    I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &tr);

    return (tr.fraction == 1.0f && !tr.startsolid);
}

// Funções de compatibilidade para o aimbot
bool CProjectileSim::SimulateToTime(float flTime, Vec3& outPos, bool bTrace)
{
    if (!m_env || !m_obj) return false;

    int ticks = TIME_TO_TICKS(flTime);
    if (ticks <= 0)
    {
        outPos = GetOrigin();
        return true;
    }

    for (int i = 0; i < ticks && !m_hasImpacted && m_tickCount < MAX_TICKS; ++i)
    {
        RunTick();
    }

    outPos = GetOrigin();
    return true;
}

bool CProjectileSim::TraceToTarget(const Vec3& targetPos, float& flTimeToImpact)
{
    flTimeToImpact = 0.0f;

    if (!m_env || !m_obj) return false;

    Vec3 prev = GetOrigin();
    float maxTime = 5.0f;
    int maxTicks = TIME_TO_TICKS(maxTime);

    for (int tick = 1; tick <= maxTicks; ++tick)
    {
        RunTick();
        Vec3 cur = GetOrigin();

        trace_t tr;
        Ray_t ray;
        ray.Init(prev, cur);
        CTraceFilterWorldOnly filter;
        I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &tr);

        if (tr.fraction < 1.0f || m_hasImpacted)
        {
            flTimeToImpact = TICK_INTERVAL * tick;
            return true;
        }

        if (cur.DistTo(targetPos) < 48.0f)
        {
            flTimeToImpact = TICK_INTERVAL * tick;
            return true;
        }

        prev = cur;
    }

    return false;
}