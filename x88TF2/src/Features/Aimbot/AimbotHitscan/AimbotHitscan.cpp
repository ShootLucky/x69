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
    std::vector<ScanPoint_t> validPoints;
    // Scan all hitboxes in this group
    for (int hitboxId : hitboxes)
    {
        const auto pBox = pSet->pHitbox(hitboxId);
        if (!pBox)
            continue;

        // ✅ VALIDAÇÃO CRÍTICA - Evita acesso fora dos limites
        if (pBox->bone < 0 || pBox->bone >= 128)
        {
            continue;  // Skip este hitbox se o bone for inválido
        }

        std::vector<Vec3> points = GenerateMultipoints(pBox, boneMatrix[pBox->bone]);
        Vec3 vBoxCenter;
        Math::VectorTransform((pBox->bbmin + pBox->bbmax) * 0.5f, boneMatrix[pBox->bone], vBoxCenter);
        // Test each point
        for (const Vec3& vPoint : points)
        {
            int nHitHitbox = -1;
            if (!H::AimUtils->TraceEntityBullet(pTarget, vLocalPos, vPoint, &nHitHitbox))
                continue;
            // Verify we hit the correct hitbox
            if (nHitHitbox != hitboxId)
                continue;
            // Calculate quality metrics
            const Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPoint);
            const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
            const float flDistToCenter = vPoint.DistTo(vBoxCenter);
            const float flDistToLocal = vPoint.DistTo(vLocalPos);
            // Quality score (lower is better)
            float quality = flFOVTo + (flDistToCenter * 0.1f);
            validPoints.push_back({
                vPoint,
                flFOVTo,
                flDistToCenter,
                hitboxId,
                quality
                });
        }
    }
    if (validPoints.empty())
        return false;
    // Sort by hitbox sort method
    if (CFG::Aimbot_Hitbox_Sort == 0) // Auto
    {
        // Use quality score
        std::sort(validPoints.begin(), validPoints.end(),
            [](const ScanPoint_t& a, const ScanPoint_t& b) {
                return a.quality < b.quality;
            });
    }
    else if (CFG::Aimbot_Hitbox_Sort == 1) // Damage
    {
        // Prioritize head, then body, then pelvis
        std::sort(validPoints.begin(), validPoints.end(),
            [this](const ScanPoint_t& a, const ScanPoint_t& b) {
                int priorityA = (a.Hitbox == HITBOX_HEAD) ? 3 : ((a.Hitbox >= HITBOX_NECK && a.Hitbox <= HITBOX_UPPER_CHEST) ? 2 : 1);
                int priorityB = (b.Hitbox == HITBOX_HEAD) ? 3 : ((b.Hitbox >= HITBOX_NECK && b.Hitbox <= HITBOX_UPPER_CHEST) ? 2 : 1);
                if (priorityA != priorityB)
                    return priorityA > priorityB;
                return a.quality < b.quality; // Tie-breaker
            });
    }
    // Pegar o melhor ponto
    const auto& best = validPoints.front();
    target.Position = best.Position;
    target.AimedHitbox = best.Hitbox;
    target.FOVTo = best.FOVTo;
    target.HitboxGroup = group;
    target.WasMultiPointed = (validPoints.size() > 1);
    target.Accuracy = 1.0f - (best.FOVTo / CFG::Aimbot_FOV); // Exemplo
    target.Damage = 0.0f; // TODO: Calcular dano se possível
    target.AngleTo = Math::CalcAngle(vLocalPos, target.Position);
    return true;
}
bool CAimbotHitscan::ScanBuilding(C_TFPlayer* pLocal, HitscanTarget_t& target, const Vec3& vLocalAngles)
{
    // Implementação para buildings, se necessário (originalmente vazia?)
    return false; // Placeholder
}
// ============================================================================
// TARGET SELECTION
// ============================================================================
bool CAimbotHitscan::GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, HitscanTarget_t& outTarget)
{
    m_vecTargets.clear();
    // Loop por entidades
    for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++) {
        auto pClientEntity = I::ClientEntityList->GetClientEntity(i);
        if (!pClientEntity) continue;
        auto pEntity = pClientEntity->As<C_TFPlayer>();
        if (!pEntity || pEntity == pLocal || !ValidateTarget(pEntity, pLocal, pWeapon)) continue;
        HitscanTarget_t tempTarget;
        tempTarget.Entity = pEntity;
        tempTarget.Position = pEntity->GetAbsOrigin(); // Default
        // Scan groups, etc. (implemente o loop por groups e chame ScanHitboxGroup)
        // Exemplo simplificado
        std::vector<int> hitboxes = GetActiveHitboxes();
        if (ScanHitboxGroup(pLocal, pEntity, tempTarget, I::EngineClient->GetViewAngles(), hitboxes, 0)) {
            m_vecTargets.push_back(tempTarget);
        }
    }
    if (m_vecTargets.empty()) return false;
    // Sort targets por FOV ou distance
    std::sort(m_vecTargets.begin(), m_vecTargets.end(), [](const HitscanTarget_t& a, const HitscanTarget_t& b) {
        return a.FOVTo < b.FOVTo; // Exemplo: closest FOV
        });
    outTarget = m_vecTargets.front();
    return true;
}
bool CAimbotHitscan::ValidateTarget(C_TFPlayer* pEntity, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    // Implemente validações: vivo, inimigo, visível, etc.
    if (pEntity->deadflag() || pEntity->m_iTeamNum() == pLocal->m_iTeamNum() && CFG::Aimbot_TeamCheck) return false;
    // Adicione mais (ignore cloaked, etc.)
    return true;
}
// ============================================================================
// AIMING
// ============================================================================
bool CAimbotHitscan::ShouldAim(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    if (CFG::Aimbot_Key != 0)
    {
        if (!(GetAsyncKeyState(CFG::Aimbot_Key) & 0x8000))
            return false;
    }
    return true;
}
Vec3 CAimbotHitscan::CalculateSmoothAngles(const Vec3& vCurrentAngles, const Vec3& vTargetAngles, float smoothing)
{
    // Calculate angle delta
    Vec3 vDelta = vTargetAngles - vCurrentAngles;
    vDelta.x = Math::NormalizeAngle(vDelta.x);
    vDelta.y = Math::NormalizeAngle(vDelta.y);
    vDelta.z = 0.0f;
    // No smoothing = instant lock
    if (smoothing <= 0.0f)
        return vTargetAngles;
    // Apply smoothing
    Vec3 vSmoothed = vDelta / smoothing;
    // Clamp to prevent jittering
    const float fMaxChange = 30.0f;
    vSmoothed.x = std::clamp(vSmoothed.x, -fMaxChange, fMaxChange);
    vSmoothed.y = std::clamp(vSmoothed.y, -fMaxChange, fMaxChange);
    Vec3 vResult = vCurrentAngles + vSmoothed;
    Math::ClampAngles(vResult);
    return vResult;
}
void CAimbotHitscan::Aim(CUserCmd* pCmd, C_TFPlayer* pLocal, const Vec3& vTargetAngles)
{
    const Vec3 vOldAngles = pCmd->viewangles;
    float fSmoothing = CFG::Aimbot_Hitscan_Smoothing;
    // Silent aim uses minimal smoothing
    if (CFG::Aimbot_Hitscan_Mode == 1)
        fSmoothing = 0.0f;
    // Calculate new angles with smoothing
    Vec3 vNewAngles = CalculateSmoothAngles(vOldAngles, vTargetAngles, fSmoothing);
    // Apply to cmd
    pCmd->viewangles = vNewAngles;
    Math::ClampAngles(pCmd->viewangles);
    // Silent aim specific handling
    if (CFG::Aimbot_Hitscan_Mode == 1)
    {
        // Fix movement for silent aim
        float forward = pCmd->forwardmove;
        float side = pCmd->sidemove;
        float yaw_rad = DEG2RAD(pCmd->viewangles.y - vOldAngles.y);
        float cos_yaw = cos(yaw_rad);
        float sin_yaw = sin(yaw_rad);
        pCmd->forwardmove = cos_yaw * forward - sin_yaw * side;
        pCmd->sidemove = sin_yaw * forward + cos_yaw * side;
        // Set view angles back for perfect silent aim
        Vec3 vOldAnglesCopy = vOldAngles; // Create non-const copy
        I::EngineClient->SetViewAngles(vOldAnglesCopy);
        G::bSilentAngles = true;
    }
}
// ============================================================================
// SHOOTING
// ============================================================================
bool CAimbotHitscan::VerifyHitchance(C_TFPlayer* pLocal, const CUserCmd* pCmd, const HitscanTarget_t& target)
{
    if (!CFG::Aimbot_SmoothAutoShoot)
        return true;
    if (target.Entity->GetClassId() != ETFClassIds::CTFPlayer)
        return true;
    auto pPlayer = target.Entity->As<C_TFPlayer>();
    // Calculate where we're actually aiming
    Vec3 vForward;
    Math::AngleVectors(pCmd->viewangles, &vForward);
    const Vec3 vTraceStart = pLocal->GetShootPos();
    const Vec3 vTraceEnd = vTraceStart + (vForward * 8192.0f);
    // Set lag record if needed
    if (target.LagRecord)
        F::LagRecordMatrixHelper->Set(target.LagRecord);
    // Verify we'll hit
    int nHitHitbox = -1;
    bool bHits = H::AimUtils->TraceEntityBullet(pPlayer, vTraceStart, vTraceEnd, &nHitHitbox);
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
    if (!CFG::Aimbot_Enable)
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
    H::Draw->String(H::Fonts->Get(EFonts::ESP), x, y, m_bActive ? green : red,
        POS_DEFAULT, "Aimbot: %s", m_bActive ? "ACTIVE" : "Inactive");
    y += 15;
    if (!m_bActive)
        return;
    H::Draw->String(H::Fonts->Get(EFonts::ESP), x, y, white,
        POS_DEFAULT, "Mode: %s", CFG::Aimbot_Hitscan_Mode == 1 ? "Silent" : "Aimlock");
    y += 15;
    H::Draw->String(H::Fonts->Get(EFonts::ESP), x, y, white,
        POS_DEFAULT, "Smoothing: %.1f", CFG::Aimbot_Hitscan_Smoothing);
    y += 15;
    if (m_LastTarget.Entity)
    {
        if (m_LastTarget.Entity->GetClassId() == ETFClassIds::CTFPlayer)
        {
            auto pPlayer = m_LastTarget.Entity->As<C_TFPlayer>();
            player_info_t info{};
            if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &info))
            {
                H::Draw->String(H::Fonts->Get(EFonts::ESP), x, y, white,
                    POS_DEFAULT, "Target: %s", info.name);
                y += 15;
            }
        }
        else
        {
            H::Draw->String(H::Fonts->Get(EFonts::ESP), x, y, white,
                POS_DEFAULT, "Target: Building");
            y += 15;
        }
        if (m_LastTarget.AimedHitbox >= 0 && m_LastTarget.AimedHitbox < HITBOX_MAX)
        {
            H::Draw->String(H::Fonts->Get(EFonts::ESP), x, y, white,
                POS_DEFAULT, "Hitbox: %s", HitboxNames[m_LastTarget.AimedHitbox]);
            y += 15;
        }
        H::Draw->String(H::Fonts->Get(EFonts::ESP), x, y, white,
            POS_DEFAULT, "FOV: %.2f", m_LastTarget.FOVTo);
        y += 15;
        H::Draw->String(H::Fonts->Get(EFonts::ESP), x, y, white,
            POS_DEFAULT, "Multipoint: %s", m_LastTarget.WasMultiPointed ? "Yes" : "No");
        y += 15;
        // Draw crosshair on target
        Vec3 screen;
        if (H::Draw->W2S(m_LastTarget.Position, screen))
        {
            H::Draw->Line(screen.x - 10, screen.y, screen.x + 10, screen.y, red);
            H::Draw->Line(screen.x, screen.y - 10, screen.x, screen.y + 10, red);
            H::Draw->OutlinedCircle(screen.x, screen.y, 5, 16, red);
        }
    }
}