#include "ProjectileSim.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ========== IMPLEMENTAÇÃO DAS FUNÇÕES ORIGINAIS ==========

bool CProjectileSim::GetInfo(C_TFPlayer* player, C_TFWeaponBase* weapon, const Vec3& angles, ProjectileInfo& out)
{
    if (!player || !weapon)
        return false;

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
            offset = { 0.0f, 0.0f, 0.0f };

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
        auto pPipeLauncher = weapon->As<C_TFPipebombLauncher>();
        if (!pPipeLauncher)
            return false;

        auto charge_begin_time = pPipeLauncher->m_flChargeBeginTime();
        auto charge = cur_time - charge_begin_time;
        auto speed = Math::RemapValClamped(charge, 0.0f,
            SDKUtils::AttribHookValue(4.0f, "stickybomb_charge_rate", weapon), 900.0f, 2400.0f);

        if (charge_begin_time <= 0.0f)
            speed = 900.0f;

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
    return GetInfo(player, weapon, angles, out);
}

bool CProjectileSim::Init(const ProjectileInfo& info, bool no_vec_up)
{
    if (!m_env)
        m_env = I::Physics->CreateEnvironment();

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
    default:
        bbox_min = { -0.5f, -0.5f, -0.5f };
        bbox_max = { 0.5f, 0.5f, 0.5f };
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
        return false;

    m_hasImpacted = false;
    m_impactPos = {};
    m_radius = info.m_radius;
    m_tickCount = 0;
    m_storedTrajectory.clear();

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
                vel += up * 200.0f;
            ang_vel = { 600.0f, -1200.0f, 0.0f };
            break;
        default:
            break;
        }

        if (info.no_spin)
            ang_vel.Zero();

        m_obj->SetPosition(info.m_pos, info.m_ang, true);
        m_obj->SetVelocity(&vel, &ang_vel);
    }

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
        return;

    const int substeps = 4;
    float sub_dt = TICK_INTERVAL / static_cast<float>(substeps);

    for (int i = 0; i < substeps; ++i)
    {
        m_env->Simulate(sub_dt);

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

    TrajectoryPoint point;
    point.position = GetOrigin();
    m_obj->GetVelocity(&point.velocity, nullptr);
    point.time = TICK_INTERVAL * m_tickCount;
    point.tickNum = m_tickCount;
    m_storedTrajectory.push_back(point);

    m_tickCount++;
}

Vec3 CProjectileSim::GetOrigin()
{
    if (!m_obj)
        return {};

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

float CProjectileSim::CalculateSplashFalloff(float distance, float radius) const
{
    if (distance >= radius)
        return 0.0f;

    return 1.0f - (distance / radius);
}

bool CProjectileSim::CheckLineOfSight(const Vec3& from, const Vec3& to, trace_t* outTrace) const
{
    Ray_t ray;
    ray.Init(from, to);
    trace_t tr;
    CTraceFilterWorldOnly filter;
    I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &tr);

    if (outTrace)
        *outTrace = tr;

    return (tr.fraction == 1.0f && !tr.startsolid);
}

bool CProjectileSim::CanHitWithSplash(const Vec3& targetPos) const
{
    if (!HasImpacted() || m_radius <= 0.0f)
        return false;

    Vec3 impact = GetImpactOrigin();
    float dist = impact.DistTo(targetPos);

    if (dist > m_radius)
        return false;

    return CheckLineOfSight(impact, targetPos);
}

float CProjectileSim::CalculateSplashDamage(const Vec3& targetPos, float maxDamage) const
{
    if (!HasImpacted() || m_radius <= 0.0f)
        return 0.0f;

    Vec3 impact = GetImpactOrigin();
    float dist = impact.DistTo(targetPos);

    if (dist > m_radius)
        return 0.0f;

    if (!CheckLineOfSight(impact, targetPos))
        return 0.0f;

    float falloff = CalculateSplashFalloff(dist, m_radius);
    return maxDamage * falloff;
}

Vec3 CProjectileSim::FindOptimalSplashPoint(const Vec3& targetPos, float searchRadius) const
{
    if (!HasImpacted() || m_radius <= 0.0f)
        return GetImpactOrigin();

    Vec3 bestPoint = GetImpactOrigin();
    float bestDamage = CalculateSplashDamage(targetPos);
    float maxDamage = 112.0f;

    const int numTests = 16;
    for (int i = 0; i < numTests; ++i)
    {
        float angle = (2.0f * static_cast<float>(M_PI) * i) / numTests;
        Vec3 offset = {
            cosf(angle) * searchRadius,
            sinf(angle) * searchRadius,
            0.0f
        };

        Vec3 testPoint = targetPos + offset;

        trace_t tr;
        Ray_t ray;
        ray.Init(testPoint + Vec3(0, 0, 50), testPoint - Vec3(0, 0, 100));
        CTraceFilterWorldOnly filter;
        I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &tr);

        if (tr.fraction < 1.0f)
        {
            testPoint = tr.endpos;

            float dist = testPoint.DistTo(targetPos);
            if (dist <= m_radius && CheckLineOfSight(testPoint, targetPos))
            {
                float damage = maxDamage * CalculateSplashFalloff(dist, m_radius);
                if (damage > bestDamage)
                {
                    bestDamage = damage;
                    bestPoint = testPoint;
                }
            }
        }
    }

    return bestPoint;
}

bool CProjectileSim::GetSplashCoverage(const Vec3& impactPoint, std::vector<Vec3>& outCoveredPoints, int resolution) const
{
    outCoveredPoints.clear();

    if (m_radius <= 0.0f)
        return false;

    float stepSize = m_radius / resolution;

    for (float x = -m_radius; x <= m_radius; x += stepSize)
    {
        for (float y = -m_radius; y <= m_radius; y += stepSize)
        {
            for (float z = -m_radius; z <= m_radius; z += stepSize)
            {
                Vec3 offset(x, y, z);
                float dist = offset.Length();

                if (dist <= m_radius)
                {
                    Vec3 testPoint = impactPoint + offset;

                    if (CheckLineOfSight(impactPoint, testPoint))
                    {
                        outCoveredPoints.push_back(testPoint);
                    }
                }
            }
        }
    }

    return !outCoveredPoints.empty();
}

bool CProjectileSim::SolveBallisticArc(
    const Vec3& origin,
    const Vec3& target,
    float speed,
    float gravity,
    float& outAngleLow,
    float& outAngleHigh) const
{
    if (gravity == 0.0f)
        return false;

    Vec3 diff = target - origin;
    float horizDist = sqrtf(diff.x * diff.x + diff.y * diff.y);
    float heightDiff = diff.z;

    float g = 800.0f * gravity;
    float v2 = speed * speed;
    float v4 = v2 * v2;

    float discriminant = v4 - g * (g * horizDist * horizDist + 2.0f * heightDiff * v2);

    if (discriminant < 0.0f)
        return false;

    float sqrtDisc = sqrtf(discriminant);

    outAngleLow = atanf((v2 - sqrtDisc) / (g * horizDist));
    outAngleHigh = atanf((v2 + sqrtDisc) / (g * horizDist));

    return true;
}

bool CProjectileSim::CalculateArcShot(
    const Vec3& startPos,
    const Vec3& targetPos,
    float initialSpeed,
    float gravityMod,
    ArcSolution& outSolution,
    bool preferHighArc)
{
    outSolution.isValid = false;

    if (gravityMod == 0.0f)
    {
        Vec3 dir = (targetPos - startPos).Normalized();
        Vec3 angles;
        Math::VectorAngles(dir, angles);
        outSolution.angles = angles;
        outSolution.timeToTarget = startPos.DistTo(targetPos) / initialSpeed;
        outSolution.apexHeight = 0.0f;
        outSolution.isValid = true;
        outSolution.impactPoint = targetPos;
        return true;
    }

    float angleLow, angleHigh;
    if (!SolveBallisticArc(startPos, targetPos, initialSpeed, gravityMod, angleLow, angleHigh))
        return false;

    float pitchAngle = preferHighArc ? angleHigh : angleLow;

    Vec3 diff = targetPos - startPos;
    float yaw = atan2f(diff.y, diff.x);

    outSolution.angles.x = pitchAngle * (180.0f / static_cast<float>(M_PI));
    outSolution.angles.y = yaw * (180.0f / static_cast<float>(M_PI));
    outSolution.angles.z = 0.0f;

    float vz = initialSpeed * sinf(pitchAngle);
    float g = 800.0f * gravityMod;
    outSolution.timeToTarget = 2.0f * vz / g;
    outSolution.apexHeight = (vz * vz) / (2.0f * g);

    outSolution.impactPoint = targetPos;
    outSolution.isValid = true;

    return true;
}

bool CProjectileSim::FindArcSolutions(
    const Vec3& startPos,
    const Vec3& targetPos,
    float initialSpeed,
    float gravityMod,
    std::vector<ArcSolution>& outSolutions)
{
    outSolutions.clear();

    ArcSolution lowArc, highArc;

    if (CalculateArcShot(startPos, targetPos, initialSpeed, gravityMod, lowArc, false))
        outSolutions.push_back(lowArc);

    if (CalculateArcShot(startPos, targetPos, initialSpeed, gravityMod, highArc, true))
    {
        if (fabs(highArc.angles.x - lowArc.angles.x) > 0.5f)
            outSolutions.push_back(highArc);
    }

    return !outSolutions.empty();
}

bool CProjectileSim::ArcCanClearObstacles(
    const ArcSolution& arcSolution,
    const Vec3& targetPos,
    float clearanceHeight)
{
    if (!arcSolution.isValid || arcSolution.trajectory.empty())
        return false;

    for (size_t i = 0; i < arcSolution.trajectory.size() - 1; ++i)
    {
        const auto& p1 = arcSolution.trajectory[i];
        const auto& p2 = arcSolution.trajectory[i + 1];

        trace_t tr;
        if (!CheckLineOfSight(p1.position, p2.position, &tr))
        {
            if (tr.endpos.z < targetPos.z + clearanceHeight)
                return false;
        }
    }

    return true;
}

bool CProjectileSim::FindBestArcSplashShot(
    const Vec3& startPos,
    const Vec3& targetPos,
    float initialSpeed,
    float gravityMod,
    ArcSolution& outBestSolution)
{
    std::vector<SplashSurface> surfaces;
    FindNearbyWalls(targetPos, m_radius, surfaces);

    if (surfaces.empty())
    {
        return CalculateArcShot(startPos, targetPos, initialSpeed, gravityMod, outBestSolution, false);
    }

    float bestScore = 0.0f;
    bool foundSolution = false;

    for (const auto& surface : surfaces)
    {
        ArcSolution solution;
        if (CalculateWallSplashShot(startPos, targetPos, surface, solution))
        {
            float score = surface.damageMultiplier;

            if (score > bestScore)
            {
                bestScore = score;
                outBestSolution = solution;
                foundSolution = true;
            }
        }
    }

    return foundSolution;
}

bool CProjectileSim::SimulateFullTrajectory(
    const ProjectileInfo& info,
    std::vector<TrajectoryPoint>& outTrajectory,
    float maxTime)
{
    outTrajectory.clear();

    if (!Init(info))
        return false;

    int maxTicks = TIME_TO_TICKS(maxTime);

    while (m_tickCount < maxTicks && !m_hasImpacted)
    {
        RunTick();
    }

    outTrajectory = m_storedTrajectory;
    return !outTrajectory.empty();
}

bool CProjectileSim::PredictImpactWithLatency(
    const Vec3& targetPos,
    const Vec3& targetVel,
    float latency,
    Vec3& outPredictedPos,
    float& outTimeToImpact)
{
    outPredictedPos = targetPos + (targetVel * latency);
    outTimeToImpact = latency;

    return true;
}

Vec3 CProjectileSim::InterpolateTrajectory(float time) const
{
    if (m_storedTrajectory.empty())
        return {};

    for (size_t i = 0; i < m_storedTrajectory.size() - 1; ++i)
    {
        if (m_storedTrajectory[i].time <= time && m_storedTrajectory[i + 1].time >= time)
        {
            float t = (time - m_storedTrajectory[i].time) /
                (m_storedTrajectory[i + 1].time - m_storedTrajectory[i].time);

            return m_storedTrajectory[i].position * (1.0f - t) +
                m_storedTrajectory[i + 1].position * t;
        }
    }

    return m_storedTrajectory.back().position;
}

bool CProjectileSim::TraceTrajectory(
    const std::vector<TrajectoryPoint>& trajectory,
    Vec3& outImpactPoint,
    float& outImpactTime,
    bool stopAtPlayers)
{
    for (size_t i = 0; i < trajectory.size() - 1; ++i)
    {
        trace_t tr;
        if (!CheckLineOfSight(trajectory[i].position, trajectory[i + 1].position, &tr))
        {
            outImpactPoint = tr.endpos;
            outImpactTime = trajectory[i].time;
            return true;
        }
    }

    return false;
}

bool CProjectileSim::FindNearbyWalls(
    const Vec3& targetPos,
    float searchRadius,
    std::vector<SplashSurface>& outSurfaces) const
{
    outSurfaces.clear();

    const int numRays = 16;
    const float angleStep = (2.0f * static_cast<float>(M_PI)) / numRays;

    for (int i = 0; i < numRays; ++i)
    {
        float angle = angleStep * i;
        Vec3 dir = {
            cosf(angle),
            sinf(angle),
            0.0f
        };

        Vec3 endPos = targetPos + (dir * searchRadius);

        trace_t tr;
        Ray_t ray;
        ray.Init(targetPos, endPos);
        CTraceFilterWorldOnly filter;
        I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &tr);

        if (tr.fraction < 1.0f && !tr.startsolid)
        {
            SplashSurface surface;
            surface.position = tr.endpos;
            surface.normal = tr.plane.normal;
            surface.distance = targetPos.DistTo(tr.endpos);
            surface.damageMultiplier = 1.0f - (surface.distance / searchRadius);

            outSurfaces.push_back(surface);
        }
    }

    return !outSurfaces.empty();
}

bool CProjectileSim::CalculateWallSplashShot(
    const Vec3& startPos,
    const Vec3& targetPos,
    const SplashSurface& wall,
    ArcSolution& outSolution)
{
    Vec3 toTarget = targetPos - wall.position;
    Vec3 reflected = toTarget - wall.normal * 2.0f * toTarget.Dot(wall.normal);
    Vec3 impactPoint = wall.position + reflected.Normalized() * 10.0f;

    return CalculateArcShot(startPos, impactPoint, 1100.0f, 0.0f, outSolution, false);
}

bool CProjectileSim::TrajectoryIntersectsWorld(
    const std::vector<TrajectoryPoint>& trajectory,
    int startIdx,
    int endIdx,
    Vec3& outIntersection) const
{
    if (startIdx < 0 || endIdx >= static_cast<int>(trajectory.size()))
        return false;

    for (int i = startIdx; i < endIdx; ++i)
    {
        trace_t tr;
        if (!CheckLineOfSight(trajectory[i].position, trajectory[i + 1].position, &tr))
        {
            outIntersection = tr.endpos;
            return true;
        }
    }

    return false;
}

Vec3 CProjectileSim::CalculateImpactNormal(const Vec3& impactPoint) const
{
    Vec3 bestNormal(0, 0, 1);
    float bestDot = -1.0f;

    const Vec3 directions[] = {
        {1, 0, 0}, {-1, 0, 0},
        {0, 1, 0}, {0, -1, 0},
        {0, 0, 1}, {0, 0, -1}
    };

    for (const auto& dir : directions)
    {
        trace_t tr;
        Ray_t ray;
        ray.Init(impactPoint + dir * 5.0f, impactPoint - dir * 5.0f);
        CTraceFilterWorldOnly filter;
        I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &tr);

        if (tr.fraction < 1.0f)
        {
            float dot = tr.plane.normal.Dot(dir);
            if (dot > bestDot)
            {
                bestDot = dot;
                bestNormal = tr.plane.normal;
            }
        }
    }

    return bestNormal;
}

bool CProjectileSim::SimulateToTime(float flTime, Vec3& outPos, bool bTrace)
{
    if (!m_env || !m_obj)
        return false;

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

    if (!m_env || !m_obj)
        return false;

    Vec3 prev = GetOrigin();
    float maxTime = 5.0f;
    int maxTicks = TIME_TO_TICKS(maxTime);

    for (int tick = 1; tick <= maxTicks; ++tick)
    {
        RunTick();
        Vec3 cur = GetOrigin();

        trace_t tr;
        if (!CheckLineOfSight(prev, cur, &tr))
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