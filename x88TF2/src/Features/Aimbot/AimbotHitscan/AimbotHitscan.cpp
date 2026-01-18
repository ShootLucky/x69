#include "AimbotHitscan.h"
#include "CFG.h"
#include <windows.h>  // Adicione isso para GetAsyncKeyState e VK_XBUTTON1 (se não estiver incluído em outro lugar)
#include <array>
#include <algorithm>
#include <cmath>
// Hitbox name mapping for debug
const char* HitboxNames[HITBOX_MAX] = {
    "Head", "Neck", "Lower Neck", "Pelvis", "Body", "Thorax", "Chest", "Upper Chest",
    "Right Thigh", "Left Thigh", "Right Calf", "Left Calf", "Right Foot", "Left Foot",
    "Right Hand", "Left Hand", "Right Upper Arm", "Right Forearm", "Left Upper Arm", "Left Forearm"
};

// ============================================================================
// TARGET ACQUISITION
// ============================================================================

bool CAimbotHitscan::GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, HitscanTarget_t& outTarget)
{
    m_vecTargets.clear();

    const Vec3 vLocalPos = pLocal->GetShootPos();
    const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

    // Get active hitboxes from config
    std::vector<int> activeHitboxes = GetActiveHitboxes();
    if (activeHitboxes.empty())
        return false;

    // Scan players
    for (auto pEntity : H::Entities->GetGroup(EEntGroup::PLAYERS_ENEMIES))
    {
        auto pPlayer = pEntity->As<C_TFPlayer>();
        if (!pPlayer || !ValidateTarget(pPlayer, pLocal, pWeapon))
            continue;

        HitscanTarget_t target;
        target.Entity = pPlayer;

        // Try each hitbox group
        for (int group = 0; group <= 4; group++)
        {
            std::vector<int> groupHitboxes;
            for (int hitbox : activeHitboxes)
            {
                if (GetHitboxGroup(hitbox) == group)
                    groupHitboxes.push_back(hitbox);
            }

            if (groupHitboxes.empty())
                continue;

            if (ScanHitboxGroup(pLocal, pPlayer, target, vLocalAngles, groupHitboxes, group))
            {
                m_vecTargets.push_back(target);
                break;
            }
        }
    }

    // Scan buildings if enabled
    if (CFG::Aimbot_Hitbox_Buildings)
    {
        for (auto pEntity : H::Entities->GetGroup(EEntGroup::BUILDINGS_ENEMIES))
        {
            HitscanTarget_t target;
            target.Entity = pEntity;
            if (ScanBuilding(pLocal, target, vLocalAngles))
            {
                m_vecTargets.push_back(target);
            }
        }
    }

    if (m_vecTargets.empty())
        return false;

    // Sort by priority (using existing config)
    F::AimbotCommon->Sort(m_vecTargets, CFG::Aimbot_Hitscan_Sort);

    outTarget = m_vecTargets.front();
    return true;
}

bool CAimbotHitscan::ValidateTarget(C_TFPlayer* pEntity, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    if (!pEntity || pEntity == pLocal)
        return false;

    if (pEntity->deadflag() || pEntity->IsDormant())
        return false;

    // Team check
    if (CFG::Aimbot_TeamCheck && pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
        return false;

    // Invulnerability check
    if (CFG::Aimbot_Ignore_Invulnerable)
    {
        if (pEntity->InCond(TF_COND_INVULNERABLE) ||
            pEntity->InCond(TF_COND_INVULNERABLE_WEARINGOFF))
            return false;
    }

    // Cloaked spy check
    if (CFG::Aimbot_Ignore_Invisible && pEntity->InCond(TF_COND_STEALTHED))
        return false;

    // Taunting check
    if (CFG::Aimbot_Ignore_Taunting && pEntity->InCond(TF_COND_TAUNTING))
        return false;

    // Friends check
    if (CFG::Aimbot_Ignore_Friends)
    {
        // Implement friend check if you have a friends system
    }

    return true;
}

// ============================================================================
// AIMING
// ============================================================================

bool CAimbotHitscan::ShouldAim(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    // Always aim if auto-shoot is on
    if (CFG::Aimbot_AutoShoot)
        return true;

    // Check aim key using input system (default to hold mode since no keymode combo)
    bool bKeyActive = (CFG::Aimbot_Key == 0) || H::Input->IsDown(CFG::Aimbot_Key);
    if (!bKeyActive)
        return false;

    // Check if attack button is pressed (for manual shooting while aiming)
    return (pCmd->buttons & IN_ATTACK) != 0;
}

void CAimbotHitscan::Aim(CUserCmd* pCmd, C_TFPlayer* pLocal, const Vec3& vTargetAngles)
{
    Vec3 vAimAngles = vTargetAngles;

    // Apply smoothing
    if (CFG::Aimbot_Hitscan_Smoothing > 0.0f)
    {
        vAimAngles = CalculateSmoothAngles(pCmd->viewangles, vTargetAngles, CFG::Aimbot_Hitscan_Smoothing);
    }

    // Clamp angles
    Math::ClampAngles(vAimAngles);

    // Apply based on mode
    switch (CFG::Aimbot_Hitscan_Mode)
    {
    case 0: // Aimlock
        pCmd->viewangles = vAimAngles;
        I::EngineClient->SetViewAngles(pCmd->viewangles);
        break;

    case 1: // Silent
        pCmd->viewangles = vAimAngles;
        G::bPSilentAngles = true;
        break;
    }
}

Vec3 CAimbotHitscan::CalculateSmoothAngles(const Vec3& vCurrentAngles, const Vec3& vTargetAngles, float smoothing)
{
    Vec3 vDelta = vTargetAngles - vCurrentAngles;
    Math::ClampAngles(vDelta);

    float factor = 1.0f / (smoothing + 1.0f);

    return vCurrentAngles + (vDelta * factor);
}

// ============================================================================
// HITCHANCE VERIFICATION
// ============================================================================

bool CAimbotHitscan::VerifyHitchance(C_TFPlayer* pLocal, const CUserCmd* pCmd, const HitscanTarget_t& target)
{
    // If hitchance config doesn't exist, skip verification
    // You can add this to CFG.h: CFGVAR(Aimbot_Hitchance, 0.0f);
    // For now, just return true
    return true;

    /* Uncomment when you add Aimbot_Hitchance to CFG.h
    if (CFG::Aimbot_Hitchance <= 0.0f)
        return true;

    auto pWeapon = H::Entities->GetWeapon();
    if (!pWeapon)
        return false;

    // Weapons that don't need hitchance verification
    int weaponID = pWeapon->GetWeaponID();
    if (weaponID == TF_WEAPON_SNIPERRIFLE ||
        weaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC ||
        weaponID == TF_WEAPON_SNIPERRIFLE_DECAP)
        return true;

    // Perform trace tests
    const int numTests = 256;
    int hits = 0;

    Vec3 vForward, vRight, vUp;
    Math::AngleVectors(target.AngleTo, &vForward, &vRight, &vUp);

    for (int i = 0; i < numTests; i++)
    {
        // Get weapon spread
        Vec3 vSpread;
        pWeapon->GetSpreadAngles(vSpread);

        // Apply spread
        float x = SDKUtils::RandomFloat(-0.5f, 0.5f) + SDKUtils::RandomFloat(-0.5f, 0.5f);
        float y = SDKUtils::RandomFloat(-0.5f, 0.5f) + SDKUtils::RandomFloat(-0.5f, 0.5f);

        Vec3 vDir = vForward + (vRight * x * vSpread.x) + (vUp * y * vSpread.y);
        vDir.Normalize();

        Vec3 vStart = pLocal->GetShootPos();
        Vec3 vEnd = vStart + (vDir * 8192.0f);

        // Trace using AimUtils
        CGameTrace trace;
        CTraceFilterHitscan filter;
        filter.m_pIgnore = pLocal;
        H::AimUtils->Trace(vStart, vEnd, MASK_SHOT, &filter, &trace);

        if (trace.m_pEnt && trace.m_pEnt == target.Entity)
            hits++;
    }

    float hitchance = (static_cast<float>(hits) / numTests) * 100.0f;
    return hitchance >= CFG::Aimbot_Hitchance;
    */
}

// ============================================================================
// BUILDING SCANNING
// ============================================================================

bool CAimbotHitscan::ScanBuilding(C_TFPlayer* pLocal, HitscanTarget_t& target, const Vec3& vLocalAngles)
{
    if (!target.Entity)
        return false;

    // Buildings don't use hitboxes, aim at center
    Vec3 vBuildingPos = target.Entity->GetAbsOrigin();

    // Add height offset for better aim
    vBuildingPos.z += 30.0f;

    Vec3 vLocalPos = pLocal->GetShootPos();

    Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vBuildingPos);
    float fov = Math::CalcFov(vLocalAngles, vAngleTo);

    if (fov > CFG::Aimbot_FOV)
        return false;

    // Visibility check
    if (CFG::Aimbot_VisibleCheck)
    {
        if (!H::AimUtils->VisPos(pLocal, target.Entity, vLocalPos, vBuildingPos))
            return false;
    }

    target.Position = vBuildingPos;
    target.AngleTo = vAngleTo;
    target.FOVTo = fov;
    target.DistanceTo = vLocalPos.DistTo(vBuildingPos);
    target.AimedHitbox = -1;
    target.SimulationTime = target.Entity->m_flSimulationTime();

    return true;
}
// ============================================================================
// HITBOX CONFIGURATION SYSTEM
// ============================================================================
std::vector<int> CAimbotHitscan::GetActiveHitboxes()
{
    std::vector<int> hitboxes;
    // Check multi_combo flags for each hitbox type
    if (CFG::Aimbot_Hitscan_Hitbox & HITBOX_TYPE_HEAD)
    {
        hitboxes.push_back(HITBOX_HEAD);
    }
    if (CFG::Aimbot_Hitscan_Hitbox & HITBOX_TYPE_BODY)
    {
        hitboxes.push_back(HITBOX_UPPER_CHEST);
        hitboxes.push_back(HITBOX_CHEST);
        hitboxes.push_back(HITBOX_THORAX);
        hitboxes.push_back(HITBOX_BODY);
        hitboxes.push_back(HITBOX_NECK);
        hitboxes.push_back(HITBOX_LOWER_NECK);
    }
    if (CFG::Aimbot_Hitscan_Hitbox & HITBOX_TYPE_PELVIS)
    {
        hitboxes.push_back(HITBOX_PELVIS);
    }
    if (CFG::Aimbot_Hitscan_Hitbox & HITBOX_TYPE_ARMS)
    {
        hitboxes.push_back(HITBOX_LEFT_UPPER_ARM);
        hitboxes.push_back(HITBOX_LEFT_FOREARM);
        hitboxes.push_back(HITBOX_RIGHT_UPPER_ARM);
        hitboxes.push_back(HITBOX_RIGHT_FOREARM);
        hitboxes.push_back(HITBOX_LEFT_HAND);
        hitboxes.push_back(HITBOX_RIGHT_HAND);
    }
    if (CFG::Aimbot_Hitscan_Hitbox & HITBOX_TYPE_LEGS)
    {
        hitboxes.push_back(HITBOX_LEFT_THIGH);
        hitboxes.push_back(HITBOX_LEFT_CALF);
        hitboxes.push_back(HITBOX_RIGHT_THIGH);
        hitboxes.push_back(HITBOX_RIGHT_CALF);
        hitboxes.push_back(HITBOX_LEFT_FOOT);
        hitboxes.push_back(HITBOX_RIGHT_FOOT);
    }
    return hitboxes;
}
int CAimbotHitscan::GetHitboxGroup(int hitbox)
{
    if (hitbox == HITBOX_HEAD)
        return 0;
    if (hitbox == HITBOX_PELVIS)
        return 2;
    if (hitbox >= HITBOX_NECK && hitbox <= HITBOX_UPPER_CHEST)
        return 1;

    // ✅ VALIDAÇÃO EXPLÍCITA - Sem pressupostos sobre ordem
    if ((hitbox == HITBOX_RIGHT_HAND || hitbox == HITBOX_LEFT_HAND ||
        hitbox == HITBOX_RIGHT_UPPER_ARM || hitbox == HITBOX_RIGHT_FOREARM ||
        hitbox == HITBOX_LEFT_UPPER_ARM || hitbox == HITBOX_LEFT_FOREARM))
        return 3;

    if ((hitbox == HITBOX_RIGHT_THIGH || hitbox == HITBOX_LEFT_THIGH ||
        hitbox == HITBOX_RIGHT_CALF || hitbox == HITBOX_LEFT_CALF ||
        hitbox == HITBOX_RIGHT_FOOT || hitbox == HITBOX_LEFT_FOOT))
        return 4;

    return -1;
}
bool CAimbotHitscan::IsHitboxEnabled(int hitbox)
{
    int group = GetHitboxGroup(hitbox);
    switch (group)
    {
    case 0: return (CFG::Aimbot_Hitscan_Hitbox & HITBOX_TYPE_HEAD) != 0;
    case 1: return (CFG::Aimbot_Hitscan_Hitbox & HITBOX_TYPE_BODY) != 0;
    case 2: return (CFG::Aimbot_Hitscan_Hitbox & HITBOX_TYPE_PELVIS) != 0;
    case 3: return (CFG::Aimbot_Hitscan_Hitbox & HITBOX_TYPE_ARMS) != 0;
    case 4: return (CFG::Aimbot_Hitscan_Hitbox & HITBOX_TYPE_LEGS) != 0;
    default: return false;
    }
}
float CAimbotHitscan::CalculateHitboxPriority(int hitbox, C_TFWeaponBase* pWeapon, float distance)
{
    float priority = 0.0f;
    // Base priority by hitbox
    if (hitbox == HITBOX_HEAD)
        priority = 100.0f; // Highest priority for headshots
    else if (hitbox == HITBOX_PELVIS)
        priority = 80.0f;
    else if (hitbox >= HITBOX_NECK && hitbox <= HITBOX_UPPER_CHEST)
        priority = 90.0f; // Body shots
    else if ((hitbox == HITBOX_RIGHT_THIGH || hitbox == HITBOX_LEFT_THIGH ||
        hitbox == HITBOX_RIGHT_CALF || hitbox == HITBOX_LEFT_CALF ||
        hitbox == HITBOX_RIGHT_FOOT || hitbox == HITBOX_LEFT_FOOT))
        priority = 50.0f;
    else if ((hitbox == HITBOX_RIGHT_HAND || hitbox == HITBOX_LEFT_HAND ||
        hitbox == HITBOX_RIGHT_UPPER_ARM || hitbox == HITBOX_RIGHT_FOREARM ||
        hitbox == HITBOX_LEFT_UPPER_ARM || hitbox == HITBOX_LEFT_FOREARM))
        priority = 40.0f;
    // Weapon-specific adjustments
    int weaponID = pWeapon->GetWeaponID();
    bool isSniper = (weaponID == TF_WEAPON_SNIPERRIFLE ||
        weaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC ||
        weaponID == TF_WEAPON_SNIPERRIFLE_DECAP);
    if (isSniper && hitbox == HITBOX_HEAD)
        priority += 50.0f; // Heavily prioritize headshots for snipers
    // Distance penalty for smaller hitboxes
    if (hitbox == HITBOX_HEAD && distance > 1000.0f)
        priority -= 20.0f;
    return priority;
}
// ============================================================================
// MULTIPOINT GENERATION
// ============================================================================
std::vector<Vec3> CAimbotHitscan::GenerateMultipoints(const mstudiobbox_t* pBox, const matrix3x4_t& boneMatrix)
{
    std::vector<Vec3> points;

    // ✅ VALIDAÇÃO DEFENSIVA
    if (!pBox)
        return points;  // Return empty vector

    const Vec3 vMins = pBox->bbmin;
    const Vec3 vMaxs = pBox->bbmax;
    const Vec3 vCenter = (vMins + vMaxs) * 0.5f;
    // Scale factor for safety margin
    const float scale = 0.92f;
    // Center point
    Vec3 vTransformed;
    Math::VectorTransform(vCenter, boneMatrix, vTransformed);
    points.push_back(vTransformed);
    // Generate more points for better coverage
    const std::array<Vec3, 26> offsets = {
        // Cardinal directions
        Vec3(vMins.x * scale, 0.0f, 0.0f),
        Vec3(vMaxs.x * scale, 0.0f, 0.0f),
        Vec3(0.0f, vMins.y * scale, 0.0f),
        Vec3(0.0f, vMaxs.y * scale, 0.0f),
        Vec3(0.0f, 0.0f, vMins.z * scale),
        Vec3(0.0f, 0.0f, vMaxs.z * scale),
        // Diagonal corners
        Vec3(vMins.x * scale, vMins.y * scale, vMins.z * scale),
        Vec3(vMaxs.x * scale, vMaxs.y * scale, vMaxs.z * scale),
        Vec3(vMins.x * scale, vMaxs.y * scale, vMins.z * scale),
        Vec3(vMaxs.x * scale, vMins.y * scale, vMins.z * scale),
        Vec3(vMins.x * scale, vMins.y * scale, vMaxs.z * scale),
        Vec3(vMaxs.x * scale, vMaxs.y * scale, vMins.z * scale),
        Vec3(vMins.x * scale, vMaxs.y * scale, vMaxs.z * scale),
        Vec3(vMaxs.x * scale, vMins.y * scale, vMins.z * scale),
        // Mid-edges
        Vec3(vMins.x * scale * 0.5f, 0.0f, 0.0f),
        Vec3(vMaxs.x * scale * 0.5f, 0.0f, 0.0f),
        Vec3(0.0f, vMins.y * scale * 0.5f, 0.0f),
        Vec3(0.0f, vMaxs.y * scale * 0.5f, 0.0f),
        Vec3(0.0f, 0.0f, vMins.z * scale * 0.5f),
        Vec3(0.0f, 0.0f, vMaxs.z * scale * 0.5f),
        // Additional interpolated points
        Vec3(vMins.x * scale * 0.7f, vMins.y * scale * 0.7f, 0.0f),
        Vec3(vMaxs.x * scale * 0.7f, vMaxs.y * scale * 0.7f, 0.0f),
        Vec3(vMins.x * scale * 0.7f, 0.0f, vMins.z * scale * 0.7f),
        Vec3(vMaxs.x * scale * 0.7f, 0.0f, vMaxs.z * scale * 0.7f),
        Vec3(0.0f, vMins.y * scale * 0.7f, vMins.z * scale * 0.7f),
        Vec3(0.0f, vMaxs.y * scale * 0.7f, vMaxs.z * scale * 0.7f)
    };
    for (const auto& offset : offsets)
    {
        Math::VectorTransform(vCenter + offset, boneMatrix, vTransformed);
        points.push_back(vTransformed);
    }
    return points;
}
// ============================================================================
// HITBOX SCANNING
// ============================================================================
bool CAimbotHitscan::ScanHitboxGroup(C_TFPlayer* pLocal, C_TFPlayer* pTarget, HitscanTarget_t& target,
    const Vec3& vLocalAngles, const std::vector<int>& hitboxes, int group)
{
    if (!pTarget)
        return false;
    const auto pModel = pTarget->GetModel();
    if (!pModel)
        return false;
    const auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
    if (!pHDR)
        return false;
    const auto pSet = pHDR->pHitboxSet(pTarget->m_nHitboxSet());
    if (!pSet)
        return false;
    matrix3x4_t boneMatrix[128];
    if (!pTarget->SetupBones(boneMatrix, 128, 0x100, I::GlobalVars->curtime))
        return false;
    const Vec3 vLocalPos = pLocal->GetShootPos();
    Vec3 vForward;
    Math::AngleVectors(vLocalAngles, &vForward);
    const Vec3 vTraceStart = pLocal->GetShootPos();
    const Vec3 vTraceEnd = vTraceStart + (vForward * 8192.0f);
    // Set lag record if needed
    if (target.LagRecord)
        F::LagRecordMatrixHelper->Set(target.LagRecord);
    // Verify we'll hit
    int nHitHitbox = -1;
    bool bHits = H::AimUtils->TraceEntityBullet(pTarget, vTraceStart, vTraceEnd, &nHitHitbox);
    // For headshot weapons, verify we hit the head
    if (target.AimedHitbox == HITBOX_HEAD && nHitHitbox != HITBOX_HEAD)
        bHits = false;
    // Restore lag record
    if (target.LagRecord)
        F::LagRecordMatrixHelper->Restore();
    return bHits;
}
bool CAimbotHitscan::ShouldFire(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const HitscanTarget_t& target)
{
    if (!CFG::Aimbot_AutoShoot)
        return false;
    if (!G::bCanPrimaryAttack)
        return false;
    const int nWeaponID = pWeapon->GetWeaponID();
    const bool bIsSniper = (nWeaponID == TF_WEAPON_SNIPERRIFLE ||
        nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC ||
        nWeaponID == TF_WEAPON_SNIPERRIFLE_DECAP);
    // Sniper-specific checks
    if (bIsSniper)
    {
        // Wait for headshot capability
        if (CFG::Aimbot_WaitForHeadshot && target.AimedHitbox == HITBOX_HEAD)
        {
            if (!G::bCanHeadshot)
                return false;
        }
        // Wait for charge
        if (CFG::Aimbot_WaitForCharge)
        {
            float flCharge = pWeapon->As<C_TFSniperRifle>()->m_flChargedDamage();
            if (flCharge < 50.0f)
                return false;
        }
    }
    // Verify hitchance
    if (!VerifyHitchance(pLocal, pCmd, target))
        return false;
    return true;
}
void CAimbotHitscan::HandleFire(CUserCmd* pCmd, C_TFWeaponBase* pWeapon)
{
    if (!pWeapon->HasPrimaryAmmoForShot())
        return;
    pCmd->buttons |= IN_ATTACK;
    // Minigun tapfire
    if (CFG::Aimbot_MinigunTapfire && pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN)
    {
        static int nTapfireTicks = 0;
        nTapfireTicks++;
        if (nTapfireTicks % 2 == 0)
            pCmd->buttons &= ~IN_ATTACK;
    }
}
bool CAimbotHitscan::IsFiring(CUserCmd* pCmd, C_TFWeaponBase* pWeapon)
{
    if (!pWeapon->HasPrimaryAmmoForShot())
        return false;
    const int nWeaponID = pWeapon->GetWeaponID();
    // Classic sniper releases on button release
    if (nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC)
        return (G::nOldButtons & IN_ATTACK) && !(pCmd->buttons & IN_ATTACK);
    // Normal weapons fire on attack press
    return (pCmd->buttons & IN_ATTACK) && G::bCanPrimaryAttack;
}
// ============================================================================
// MAIN LOOP
// ============================================================================
void CAimbotHitscan::Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    m_bActive = false;
    m_LastTarget = {};
    if (!CFG::Aimbot_Active)  // Alterado de Aimbot_Enable para Aimbot_Active para compatibilidade com o menu
        return;
    // Don't run during shifts
    if (Shifting::bShifting && !Shifting::bShiftingWarp)
        return;
    // Check for excluded weapons
    int weaponID = pWeapon->GetWeaponID();
    const std::vector<int> excludedWeapons = {
        TF_WEAPON_COMPOUND_BOW,
        TF_WEAPON_PIPEBOMBLAUNCHER,
        TF_WEAPON_CANNON,
        TF_WEAPON_GRENADELAUNCHER,
        TF_WEAPON_SYRINGEGUN_MEDIC,
        TF_WEAPON_SHOTGUN_BUILDING_RESCUE,
        TF_WEAPON_FLAMETHROWER,
        TF_WEAPON_FLAME_BALL,
        TF_WEAPON_FLAREGUN,
        TF_WEAPON_CROSSBOW,
        TF_WEAPON_ROCKETLAUNCHER,
        TF_WEAPON_PARTICLE_CANNON,
        TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT,
        TF_WEAPON_FLAREGUN_REVENGE
    };
    if (std::find(excludedWeapons.begin(), excludedWeapons.end(), weaponID) != excludedWeapons.end())
        return;
    // Get target
    HitscanTarget_t target;
    if (!GetTarget(pLocal, pWeapon, target) || !target.Entity)
        return;
    G::nTargetIndexEarly = target.Entity->entindex();
    // Check if we should aim
    if (!ShouldAim(pCmd, pLocal, pWeapon))
        return;
    // FOV check
    if (target.FOVTo > CFG::Aimbot_FOV)
        return;
    G::nTargetIndex = target.Entity->entindex();
    // Auto scope for snipers
    const bool bIsSniper = (weaponID == TF_WEAPON_SNIPERRIFLE ||
        weaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC ||
        weaponID == TF_WEAPON_SNIPERRIFLE_DECAP);
    if (bIsSniper && CFG::Aimbot_AutoScope && !pLocal->IsZoomed() && G::bCanPrimaryAttack)
    {
        pCmd->buttons |= IN_ATTACK2;
        return;
    }
    // Aim at target
    Aim(pCmd, pLocal, target.AngleTo);
    // Handle shooting
    if (CFG::Aimbot_AutoShoot)
    {
        // Special handling for Classic sniper
        if (weaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC)
        {
            auto pSniper = pWeapon->As<C_TFSniperRifleClassic>();
            float flCharge = pSniper->m_flChargedDamage();
            float flRequired = (target.AimedHitbox == HITBOX_HEAD) ? 150.0f : 0.1f;
            if (CFG::Aimbot_WaitForCharge)
                flRequired = std::max(flRequired, 50.0f);
            if (flCharge < flRequired)
            {
                pCmd->buttons |= IN_ATTACK; // Charging
            }
            else if (ShouldFire(pCmd, pLocal, pWeapon, target))
            {
                pCmd->buttons &= ~IN_ATTACK; // Release to fire
            }
        }
        else
        {
            if (ShouldFire(pCmd, pLocal, pWeapon, target))
            {
                HandleFire(pCmd, pWeapon);
            }
        }
    }
    // Handle tick count for lag compensation
    const bool bIsFiring = IsFiring(pCmd, pWeapon);
    G::bFiring = bIsFiring;
    if (bIsFiring && target.SimulationTime > 0.0f)
    {
        pCmd->tick_count = TIME_TO_TICKS(target.SimulationTime + SDKUtils::GetLerp() + F::LagRecords->GetFakeLatency());
    }
    m_bActive = true;
    m_LastTarget = target;
}
// ============================================================================
// DEBUG VISUALIZATION
// ============================================================================
void CAimbotHitscan::DrawDebug()
{
    int x = 10;
    int y = 200;
    Color_t white{ 255, 255, 255, 255 };
    Color_t red{ 255, 0, 0, 255 };
    Color_t green{ 0, 255, 0, 255 };
    HFont font = H::Fonts->Get(EFonts::ESP).m_dwFont;
    H::Draw->TextF(x, y, font, m_bActive ? green : red,
        ALIGN_DEFAULT, "Aimbot: %s", m_bActive ? "ACTIVE" : "Inactive");
    y += 15;
    if (!m_bActive)
        return;
    H::Draw->TextF(x, y, font, white,
        ALIGN_DEFAULT, "Mode: %s", CFG::Aimbot_Hitscan_Mode == 1 ? "Silent" : "Aimlock");
    y += 15;
    H::Draw->TextF(x, y, font, white,
        ALIGN_DEFAULT, "Smoothing: %.1f", CFG::Aimbot_Hitscan_Smoothing);
    y += 15;
    if (m_LastTarget.Entity)
    {
        if (m_LastTarget.Entity->GetClassId() == ETFClassIds::CTFPlayer)
        {
            auto pPlayer = m_LastTarget.Entity->As<C_TFPlayer>();
            player_info_t info{};
            if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &info))
            {
                H::Draw->TextF(x, y, font, white,
                    ALIGN_DEFAULT, "Target: %s", info.name);
                y += 15;
            }
        }
        else
        {
            H::Draw->Text(x, y, font, white,
                ALIGN_DEFAULT, "Target: Building");
            y += 15;
        }
        if (m_LastTarget.AimedHitbox >= 0 && m_LastTarget.AimedHitbox < HITBOX_MAX)
        {
            H::Draw->TextF(x, y, font, white,
                ALIGN_DEFAULT, "Hitbox: %s", HitboxNames[m_LastTarget.AimedHitbox]);
            y += 15;
        }
        H::Draw->TextF(x, y, font, white,
            ALIGN_DEFAULT, "FOV: %.2f", m_LastTarget.FOVTo);
        y += 15;
        H::Draw->TextF(x, y, font, white,
            ALIGN_DEFAULT, "Multipoint: %s", m_LastTarget.WasMultiPointed ? "Yes" : "No");
        y += 15;
        // Draw crosshair on target
        Vec3 screen;
        if (H::Draw->W2S(m_LastTarget.Position, screen))
        {
            H::Draw->Line(screen.x - 10, screen.y, screen.x + 10, screen.y, red);
            H::Draw->Line(screen.x, screen.y - 10, screen.x, screen.y + 10, red);
            H::Draw->Circle(screen.x, screen.y, 5, 16, red);
        }
    }
}