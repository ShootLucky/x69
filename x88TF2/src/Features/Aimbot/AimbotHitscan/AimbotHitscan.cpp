#include "AimbotHitscan.h"
#include "CFG.h"
#include <array>

int CAimbotHitscan::GetAimHitbox(C_TFWeaponBase* pWeapon)
{
    switch (CFG::Aimbot_Hitscan_Hitbox)
    {
    case 0: return HITBOX_HEAD;
    case 1: return HITBOX_PELVIS;
    case 2:
    {
        if (pWeapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE_CLASSIC)
            return (pWeapon->As<C_TFSniperRifleClassic>()->m_flChargedDamage() >= 150.0f) ? HITBOX_HEAD : HITBOX_PELVIS;
        return H::AimUtils->IsWeaponCapableOfHeadshot(pWeapon) ? HITBOX_HEAD : HITBOX_PELVIS;
    }
    default: return HITBOX_PELVIS;
    }
}

bool CAimbotHitscan::ScanHead(C_TFPlayer* pLocal, HitscanTarget_t& target)
{
    if (!CFG::Aimbot_Hitscan_Scan_Head)
        return false;
    const auto pPlayer = target.Entity->As<C_TFPlayer>();
    if (!pPlayer)
        return false;
    const auto pModel = pPlayer->GetModel();
    if (!pModel)
        return false;
    const auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
    if (!pHDR)
        return false;
    const auto pSet = pHDR->pHitboxSet(pPlayer->m_nHitboxSet());
    if (!pSet)
        return false;
    const auto pBox = pSet->pHitbox(HITBOX_HEAD);
    if (!pBox)
        return false;
    matrix3x4_t boneMatrix[128] = {};
    if (!pPlayer->SetupBones(boneMatrix, 128, 0x100, I::GlobalVars->curtime))
        return false;
    const Vec3 vMins = pBox->bbmin;
    const Vec3 vMaxs = pBox->bbmax;
    const float scale = 0.8f;
    const std::array<Vec3, 7> vPoints = {
        Vec3(0.0f, 0.0f, 0.0f),
        Vec3(vMins.x * scale, 0.0f, 0.0f),
        Vec3(vMaxs.x * scale, 0.0f, 0.0f),
        Vec3(0.0f, vMins.y * scale, 0.0f),
        Vec3(0.0f, vMaxs.y * scale, 0.0f),
        Vec3(0.0f, 0.0f, vMins.z * scale),
        Vec3(0.0f, 0.0f, vMaxs.z * scale)
    };
    const Vec3 vLocalPos = pLocal->GetShootPos();
    for (const auto& vPoint : vPoints)
    {
        Vec3 vTransformed = {};
        Math::VectorTransform(vPoint, boneMatrix[pBox->bone], vTransformed);
        int nHitHitbox = -1;
        if (!H::AimUtils->TraceEntityBullet(pPlayer, vLocalPos, vTransformed, &nHitHitbox))
            continue;
        if (nHitHitbox != HITBOX_HEAD)
            continue;
        target.Position = vTransformed;
        target.AngleTo = Math::CalcAngle(vLocalPos, vTransformed);
        target.WasMultiPointed = true;
        return true;
    }
    return false;
}

bool CAimbotHitscan::ScanBody(C_TFPlayer* pLocal, HitscanTarget_t& target)
{
    const bool bScanningBody = CFG::Aimbot_Hitscan_Scan_Body;
    const bool bScaningArms = CFG::Aimbot_Hitscan_Scan_Arms;
    const bool bScanningLegs = CFG::Aimbot_Hitscan_Scan_Legs;
    if (!bScanningBody && !bScaningArms && !bScanningLegs)
        return false;
    const auto pPlayer = target.Entity->As<C_TFPlayer>();
    if (!pPlayer)
        return false;
    matrix3x4_t boneMatrix[128] = {};
    if (!pPlayer->SetupBones(boneMatrix, 128, 0x100, I::GlobalVars->curtime))
        return false;
    const Vec3 vLocalPos = pLocal->GetShootPos();
    for (int n = 1; n < pPlayer->GetNumOfHitboxes(); n++)
    {
        if (n == target.AimedHitbox)
            continue;
        const int nHitboxGroup = pPlayer->GetHitboxGroup(n);
        if (!bScanningBody && (nHitboxGroup == HITGROUP_CHEST || nHitboxGroup == HITGROUP_STOMACH))
            continue;
        if (!bScaningArms && (nHitboxGroup == HITGROUP_LEFTARM || nHitboxGroup == HITGROUP_RIGHTARM))
            continue;
        if (!bScanningLegs && (nHitboxGroup == HITGROUP_LEFTLEG || nHitboxGroup == HITGROUP_RIGHTLEG))
            continue;
        Vec3 vHitbox = pPlayer->GetHitboxPos(n);
        if (!H::AimUtils->TraceEntityBullet(pPlayer, vLocalPos, vHitbox))
            continue;
        target.Position = vHitbox;
        target.AngleTo = Math::CalcAngle(vLocalPos, vHitbox);
        return true;
    }
    return false;
}

bool CAimbotHitscan::ScanBuilding(C_TFPlayer* pLocal, HitscanTarget_t& target)
{
    if (!CFG::Aimbot_Hitscan_Scan_Buildings)
        return false;
    const auto pObject = target.Entity->As<C_BaseObject>();
    if (!pObject)
        return false;
    const Vec3 vLocalPos = pLocal->GetShootPos();
    if (pObject->GetClassId() == ETFClassIds::CObjectSentrygun)
    {
        for (int n = 0; n < pObject->GetNumOfHitboxes(); n++)
        {
            Vec3 vHitbox = pObject->GetHitboxPos(n);
            if (!H::AimUtils->TraceEntityBullet(pObject, vLocalPos, vHitbox))
                continue;
            target.Position = vHitbox;
            target.AngleTo = Math::CalcAngle(vLocalPos, vHitbox);
            return true;
        }
    }
    else
    {
        const Vec3 vMins = pObject->m_vecMins();
        const Vec3 vMaxs = pObject->m_vecMaxs();
        const std::array<Vec3, 6> vPoints = {
            Vec3(vMins.x * 0.9f, ((vMins.y + vMaxs.y) * 0.5f), ((vMins.z + vMaxs.z) * 0.5f)),
            Vec3(vMaxs.x * 0.9f, ((vMins.y + vMaxs.y) * 0.5f), ((vMins.z + vMaxs.z) * 0.5f)),
            Vec3(((vMins.x + vMaxs.x) * 0.5f), vMins.y * 0.9f, ((vMins.z + vMaxs.z) * 0.5f)),
            Vec3(((vMins.x + vMaxs.x) * 0.5f), vMaxs.y * 0.9f, ((vMins.z + vMaxs.z) * 0.5f)),
            Vec3(((vMins.x + vMaxs.x) * 0.5f), ((vMins.y + vMaxs.y) * 0.5f), vMins.z * 0.9f),
            Vec3(((vMins.x + vMaxs.x) * 0.5f), ((vMins.y + vMaxs.y) * 0.5f), vMaxs.z * 0.9f)
        };
        const matrix3x4_t& transform = pObject->RenderableToWorldTransform();
        for (const auto& vPoint : vPoints)
        {
            Vec3 vTransformed = {};
            Math::VectorTransform(vPoint, transform, vTransformed);
            if (!H::AimUtils->TraceEntityBullet(pObject, vLocalPos, vTransformed))
                continue;
            target.Position = vTransformed;
            target.AngleTo = Math::CalcAngle(vLocalPos, vTransformed);
            return true;
        }
    }
    return false;
}

bool CAimbotHitscan::GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, HitscanTarget_t& outTarget)
{
    const Vec3 vLocalPos = pLocal->GetShootPos();
    const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();
    m_vecTargets.clear();
    if (CFG::Aimbot_Target_Players)
    {
        const int nAimHitbox = GetAimHitbox(pWeapon);
        for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PLAYERS_ENEMIES))
        {
            if (!pEntity)
                continue;
            const auto pPlayer = pEntity->As<C_TFPlayer>();
            if (pPlayer->deadflag() || pPlayer->InCond(TF_COND_HALLOWEEN_GHOST_MODE))
                continue;
            if (CFG::Aimbot_Ignore_Friends && pPlayer->IsPlayerOnSteamFriendsList())
                continue;
            if (CFG::Aimbot_Ignore_Invisible && pPlayer->IsInvisible())
                continue;
            if (CFG::Aimbot_Ignore_Invulnerable && pPlayer->IsInvulnerable())
                continue;
            if (CFG::Aimbot_Ignore_Taunting && pPlayer->InCond(TF_COND_TAUNTING))
                continue;
            matrix3x4_t boneMatrix[128] = {};
            if (!pPlayer->SetupBones(boneMatrix, 128, 0x100, I::GlobalVars->curtime))
                continue;
            if (CFG::Aimbot_TargetLagRecords)
            {
                int nRecords = 0;
                if (!F::LagRecords->HasRecords(pPlayer, &nRecords))
                    continue;
                const bool bFakeLatencyActive = F::LagRecords->GetFakeLatency() > 0.0f;
                const int nWeaponID = pWeapon->GetWeaponID();
                const bool bIsSniperRifle = (nWeaponID == TF_WEAPON_SNIPERRIFLE || nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC || nWeaponID == TF_WEAPON_SNIPERRIFLE_DECAP);
                const bool bIsAmbassador = (pWeapon->m_iItemDefinitionIndex() == Spy_m_TheAmbassador || pWeapon->m_iItemDefinitionIndex() == Spy_m_FestiveAmbassador);
                int nStartRecord = 1;
                int nEndRecord = nRecords;
                if (bFakeLatencyActive || (pLocal->m_iClass() == TF_CLASS_SNIPER && bIsSniperRifle) || bIsAmbassador)
                {
                    nStartRecord = std::max(1, nRecords - 5);
                    nEndRecord = nRecords;
                    const int nPriorityRecord = nRecords - 3;
                    if (nPriorityRecord >= 1 && nPriorityRecord < nRecords)
                    {
                        const LagRecord_t* lagRecord = F::LagRecords->GetRecord(pPlayer, nPriorityRecord);
                        if (lagRecord && lagRecord->bValid && lagRecord->SimulationTime > 0.0f)
                        {
                            F::LagRecordMatrixHelper->Set(lagRecord);
                            Vec3 vPos = pPlayer->GetHitboxPos(nAimHitbox);
                            bool bVisible = true;
                            if (CFG::Aimbot_VisibleCheck)
                            {
                                bVisible = H::AimUtils->VisPos(pLocal, pPlayer, vLocalPos, vPos);
                            }
                            float flFOV = Math::CalcFov(vLocalAngles, Math::CalcAngle(vLocalPos, vPos));
                            if (flFOV > CFG::Aimbot_FOV)
                            {
                                F::LagRecordMatrixHelper->Restore();
                                continue;
                            }
                            float flDist = vLocalPos.DistTo(vPos);
                            HitscanTarget_t tLag = {};
                            tLag.Entity = pPlayer;
                            tLag.Position = vPos;
                            tLag.AngleTo = Math::CalcAngle(vLocalPos, vPos);
                            tLag.FOVTo = flFOV;
                            tLag.DistanceTo = flDist;
                            tLag.AimedHitbox = nAimHitbox;
                            tLag.SimulationTime = lagRecord->SimulationTime;
                            tLag.LagRecord = lagRecord;
                            if (bVisible)
                            {
                                m_vecTargets.push_back(tLag);
                            }
                            F::LagRecordMatrixHelper->Restore();
                        }
                    }
                }
                for (int n = nStartRecord; n < nEndRecord; n++)
                {
                    if ((bFakeLatencyActive || (pLocal->m_iClass() == TF_CLASS_SNIPER && bIsSniperRifle) || bIsAmbassador) && n == nRecords - 3)
                        continue;
                    const LagRecord_t* lagRecord = F::LagRecords->GetRecord(pPlayer, n);
                    if (!lagRecord || !lagRecord->bValid || lagRecord->SimulationTime <= 0.0f)
                        continue;
                    F::LagRecordMatrixHelper->Set(lagRecord);
                    Vec3 vPos = pPlayer->GetHitboxPos(nAimHitbox);
                    bool bVisible = true;
                    if (CFG::Aimbot_VisibleCheck)
                    {
                        bVisible = H::AimUtils->VisPos(pLocal, pPlayer, vLocalPos, vPos);
                    }
                    float flFOV = Math::CalcFov(vLocalAngles, Math::CalcAngle(vLocalPos, vPos));
                    if (flFOV > CFG::Aimbot_FOV)
                    {
                        F::LagRecordMatrixHelper->Restore();
                        continue;
                    }
                    float flDist = vLocalPos.DistTo(vPos);
                    HitscanTarget_t tLag = {};
                    tLag.Entity = pPlayer;
                    tLag.Position = vPos;
                    tLag.AngleTo = Math::CalcAngle(vLocalPos, vPos);
                    tLag.FOVTo = flFOV;
                    tLag.DistanceTo = flDist;
                    tLag.AimedHitbox = nAimHitbox;
                    tLag.SimulationTime = lagRecord->SimulationTime;
                    tLag.LagRecord = lagRecord;
                    if (bVisible)
                    {
                        m_vecTargets.push_back(tLag);
                    }
                    F::LagRecordMatrixHelper->Restore();
                }
            }
            const bool bFakeLatencyActive = F::LagRecords->GetFakeLatency() > 0.0f;
            const int nWeaponID = pWeapon->GetWeaponID();
            const bool bIsSniperRifle = (nWeaponID == TF_WEAPON_SNIPERRIFLE || nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC || nWeaponID == TF_WEAPON_SNIPERRIFLE_DECAP);
            const bool bIsAmbassador = (pWeapon->m_iItemDefinitionIndex() == Spy_m_TheAmbassador || pWeapon->m_iItemDefinitionIndex() == Spy_m_FestiveAmbassador);
            if (CFG::Aimbot_TargetLagRecords ? !(bFakeLatencyActive || (pLocal->m_iClass() == TF_CLASS_SNIPER && bIsSniperRifle) || bIsAmbassador) : true)
            {
                Vec3 vPos = pPlayer->GetHitboxPos(nAimHitbox);
                bool bVisible = true;
                if (CFG::Aimbot_VisibleCheck)
                {
                    bVisible = H::AimUtils->VisPos(pLocal, pPlayer, vLocalPos, vPos);
                }
                float flFOV = Math::CalcFov(vLocalAngles, Math::CalcAngle(vLocalPos, vPos));
                if (flFOV > CFG::Aimbot_FOV)
                    continue;
                float flDist = vLocalPos.DistTo(vPos);
                HitscanTarget_t t = {};
                t.Entity = pPlayer;
                t.Position = vPos;
                t.AngleTo = Math::CalcAngle(vLocalPos, vPos);
                t.FOVTo = flFOV;
                t.DistanceTo = flDist;
                t.AimedHitbox = nAimHitbox;
                t.SimulationTime = pPlayer->m_flSimulationTime();
                t.LagRecord = nullptr;
                if (bVisible)
                {
                    m_vecTargets.push_back(t);
                }
            }
        }
    }
    if (CFG::Aimbot_Target_Buildings)
    {
        for (const auto pEntity : H::Entities->GetGroup(EEntGroup::BUILDINGS_ENEMIES))
        {
            if (!pEntity)
                continue;
            const auto pObject = pEntity->As<C_BaseObject>();
            if (pObject->m_bHasSapper() || pObject->m_bPlasmaDisable())
                continue;
            Vec3 vPos = pObject->GetAbsOrigin() + (pObject->m_vecBuildMins() + pObject->m_vecBuildMaxs()) * 0.5f;
            bool bVisible = true;
            if (CFG::Aimbot_VisibleCheck)
            {
                bVisible = H::AimUtils->VisPos(pLocal, pObject, vLocalPos, vPos);
            }
            float flFOV = Math::CalcFov(vLocalAngles, Math::CalcAngle(vLocalPos, vPos));
            if (flFOV > CFG::Aimbot_FOV)
                continue;
            float flDist = vLocalPos.DistTo(vPos);
            HitscanTarget_t t = {};
            t.Entity = pObject;
            t.Position = vPos;
            t.AngleTo = Math::CalcAngle(vLocalPos, vPos);
            t.FOVTo = flFOV;
            t.DistanceTo = flDist;
            t.AimedHitbox = -1;
            t.SimulationTime = -1.0f;
            t.LagRecord = nullptr;
            if (bVisible)
            {
                m_vecTargets.push_back(t);
            }
        }
    }
    F::AimbotCommon->Sort(m_vecTargets, CFG::Aimbot_Hitscan_Sort);
    if (m_vecTargets.empty())
        return false;
    for (auto& t : m_vecTargets)
    {
        if (t.Entity->GetClassId() == ETFClassIds::CTFPlayer)
        {
            if (t.LagRecord)
            {
                F::LagRecordMatrixHelper->Set(t.LagRecord);
            }
            if (ScanHead(pLocal, t) || ScanBody(pLocal, t))
            {
                t.AngleTo = Math::CalcAngle(vLocalPos, t.Position);
                t.FOVTo = Math::CalcFov(vLocalAngles, t.AngleTo);
                t.DistanceTo = vLocalPos.DistTo(t.Position);
            }
            if (t.LagRecord)
            {
                F::LagRecordMatrixHelper->Restore();
            }
        }
        else
        {
            if (ScanBuilding(pLocal, t))
            {
                t.AngleTo = Math::CalcAngle(vLocalPos, t.Position);
                t.FOVTo = Math::CalcFov(vLocalAngles, t.AngleTo);
                t.DistanceTo = vLocalPos.DistTo(t.Position);
            }
        }
    }
    m_vecTargets.erase(std::remove_if(m_vecTargets.begin(), m_vecTargets.end(), [](const HitscanTarget_t& t) {
        return t.FOVTo > CFG::Aimbot_FOV;
        }), m_vecTargets.end());
    F::AimbotCommon->Sort(m_vecTargets, CFG::Aimbot_Hitscan_Sort);
    if (m_vecTargets.empty())
        return false;
    outTarget = m_vecTargets.front();
    return true;
}

bool CAimbotHitscan::ShouldAim(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    static bool bToggled = false;
    static bool bLastDown = false;
    if (CFG::Aimbot_KeyMode == 2) // Always On
        return true;
    bool bDown = (GetAsyncKeyState(CFG::Aimbot_Key) & 0x8000) != 0;
    if (CFG::Aimbot_KeyMode == 1) // Toggle
    {
        if (bDown && !bLastDown)
        {
            bToggled = !bToggled;
        }
        bLastDown = bDown;
        return bToggled;
    }
    return bDown;
}

void CAimbotHitscan::Aim(CUserCmd* pCmd, C_TFPlayer* pLocal, const Vec3& vAngles)
{
    const Vec3 vOldAngles = pCmd->viewangles;
    Vec3 vDelta = vAngles - vOldAngles;
    vDelta.x = Math::NormalizeAngle(vDelta.x);
    vDelta.y = Math::NormalizeAngle(vDelta.y);
    vDelta.z = Math::NormalizeAngle(vDelta.z);
    float fSmoothing = CFG::Aimbot_Hitscan_Smoothing;
    Vec3 vStep;
    if (fSmoothing <= 0.0f)
    {
        vStep = vDelta;
    }
    else
    {
        vStep = vDelta / fSmoothing;
    }
    const float fMaxChange = 30.0f;
    vStep.x = std::clamp(vStep.x, -fMaxChange, fMaxChange);
    vStep.y = std::clamp(vStep.y, -fMaxChange, fMaxChange);
    pCmd->viewangles = vOldAngles + vStep;
    Math::ClampAngles(pCmd->viewangles);
    if (CFG::Aimbot_Hitscan_Mode == 1)
    {
        Vec3 oldAngles = vOldAngles;
        I::EngineClient->SetViewAngles(oldAngles);
        G::bSilentAngles = true;
    }
}

bool CAimbotHitscan::ShouldFire(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const HitscanTarget_t& target)
{
    if (!CFG::Aimbot_AutoShoot)
        return false;
    if (!G::bCanPrimaryAttack)
        return false;
    if (target.Entity->GetClassId() != ETFClassIds::CTFPlayer)
        return true;
    const auto pPlayer = target.Entity->As<C_TFPlayer>();
    int nHealth = pPlayer->m_iHealth();
    if (CFG::Aimbot_WaitForHeadshot && H::AimUtils->IsWeaponCapableOfHeadshot(pWeapon))
    {
        if (!G::bCanHeadshot)
            return false;
    }
    if (CFG::Aimbot_WaitForCharge && pWeapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE) {
        auto sniper = reinterpret_cast<C_TFSniperRifle*>(pWeapon);
        if (sniper->m_flChargedDamage() < 50.0f)
            return false;
    }
    if (CFG::Aimbot_SmoothAutoShoot && CFG::Aimbot_Hitscan_Mode == 2)
    {
        Vec3 vForward = {};
        Math::AngleVectors(pCmd->viewangles, &vForward);
        const Vec3 vTraceStart = pLocal->GetShootPos();
        const Vec3 vTraceEnd = vTraceStart + (vForward * 8192.0f);
        if (target.Entity->GetClassId() == ETFClassIds::CTFPlayer)
        {
            const auto pPlayer = target.Entity->As<C_TFPlayer>();
            if (!target.LagRecord)
            {
                int nHitHitbox = -1;
                if (!H::AimUtils->TraceEntityBullet(pPlayer, vTraceStart, vTraceEnd, &nHitHitbox))
                    return false;
                if (target.AimedHitbox == HITBOX_HEAD)
                {
                    if (nHitHitbox != HITBOX_HEAD)
                        return false;
                    Vec3 vMins = {}, vMaxs = {}, vCenter = {};
                    matrix3x4_t matrix = {};
                    pPlayer->GetHitboxInfo(nHitHitbox, &vCenter, &vMins, &vMaxs, &matrix);
                    vMins *= 0.8f;
                    vMaxs *= 0.8f;
                    if (!Math::RayToOBB(vTraceStart, vForward, vCenter, vMins, vMaxs, matrix))
                        return false;
                }
            }
            else
            {
                F::LagRecordMatrixHelper->Set(target.LagRecord);
                int nHitHitbox = -1;
                if (!H::AimUtils->TraceEntityBullet(pPlayer, vTraceStart, vTraceEnd, &nHitHitbox))
                {
                    F::LagRecordMatrixHelper->Restore();
                    return false;
                }
                if (target.AimedHitbox == HITBOX_HEAD)
                {
                    if (nHitHitbox != HITBOX_HEAD)
                    {
                        F::LagRecordMatrixHelper->Restore();
                        return false;
                    }
                    Vec3 vMins = {}, vMaxs = {}, vCenter = {};
                    SDKUtils::GetHitboxInfoFromMatrix(pPlayer, nHitHitbox, const_cast<matrix3x4_t*>(target.LagRecord->BoneMatrix), &vCenter, &vMins, &vMaxs);
                    vMins *= 0.8f;
                    vMaxs *= 0.8f;
                    if (!Math::RayToOBB(vTraceStart, vForward, vCenter, vMins, vMaxs, *target.LagRecord->BoneMatrix))
                    {
                        F::LagRecordMatrixHelper->Restore();
                        return false;
                    }
                }
                F::LagRecordMatrixHelper->Restore();
            }
        }
        else
        {
            if (!H::AimUtils->TraceEntityBullet(target.Entity, vTraceStart, vTraceEnd, nullptr))
                return false;
        }
    }
    return true;
}

void CAimbotHitscan::HandleFire(CUserCmd* pCmd, C_TFWeaponBase* pWeapon)
{
    if (!pWeapon->HasPrimaryAmmoForShot())
        return;
    if (pWeapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE_CLASSIC)
    {
        if (G::nOldButtons & IN_ATTACK)
        {
            pCmd->buttons &= ~IN_ATTACK;
        }
        else
        {
            pCmd->buttons |= IN_ATTACK;
        }
    }
    else
    {
        pCmd->buttons |= IN_ATTACK;
    }
}

bool CAimbotHitscan::IsFiring(CUserCmd* pCmd, C_TFWeaponBase* pWeapon)
{
    if (!pWeapon->HasPrimaryAmmoForShot())
        return false;
    if (pWeapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE_CLASSIC)
        return !(pCmd->buttons & IN_ATTACK) && (G::nOldButtons & IN_ATTACK);
    return (pCmd->buttons & IN_ATTACK) && G::bCanPrimaryAttack;
}

void CAimbotHitscan::Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    if (!CFG::Aimbot_Enable)
        return;
    if (Shifting::bShifting && !Shifting::bShiftingWarp)
        return;
    int weaponID = pWeapon->GetWeaponID();
    if (weaponID == TF_WEAPON_COMPOUND_BOW || weaponID == TF_WEAPON_PIPEBOMBLAUNCHER || weaponID == TF_WEAPON_CANNON || weaponID == TF_WEAPON_GRENADELAUNCHER || weaponID == TF_WEAPON_SYRINGEGUN_MEDIC || weaponID == TF_WEAPON_SHOTGUN_BUILDING_RESCUE || weaponID == TF_WEAPON_FLAMETHROWER || weaponID == TF_WEAPON_FLAME_BALL || weaponID == TF_WEAPON_FLAREGUN || weaponID == TF_WEAPON_CROSSBOW || weaponID == TF_WEAPON_ROCKETLAUNCHER || weaponID == TF_WEAPON_PARTICLE_CANNON || weaponID == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT || weaponID == TF_WEAPON_FLAREGUN_REVENGE)
        return;
    HitscanTarget_t target = {};
    if (GetTarget(pLocal, pWeapon, target) && target.Entity)
    {
        G::nTargetIndexEarly = target.Entity->entindex();
        const bool bShouldAim = ShouldAim(pCmd, pLocal, pWeapon);
        if (bShouldAim)
        {
            if (target.FOVTo > CFG::Aimbot_FOV)
                return;
            G::nTargetIndex = target.Entity->entindex();
            if (CFG::Aimbot_AutoScope
                && !pLocal->IsZoomed() && pLocal->m_iClass() == TF_CLASS_SNIPER && pWeapon->GetSlot() == WEAPON_SLOT_PRIMARY && G::bCanPrimaryAttack)
            {
                pCmd->buttons |= IN_ATTACK2;
                return;
            }
            Aim(pCmd, pLocal, target.AngleTo);
            if (CFG::Aimbot_AutoShoot && pWeapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE_CLASSIC)
                pCmd->buttons |= IN_ATTACK;
            if (ShouldFire(pCmd, pLocal, pWeapon, target))
            {
                HandleFire(pCmd, pWeapon);
            }
            const bool bIsFiring = IsFiring(pCmd, pWeapon);
            G::bFiring = bIsFiring;
            if (bIsFiring)
            {
                if (CFG::Misc_AccuracyImprovements)
                {
                    if (target.Entity->GetClassId() == ETFClassIds::CTFPlayer)
                    {
                        pCmd->tick_count = TIME_TO_TICKS(target.SimulationTime + SDKUtils::GetLerp());
                    }
                }
                else
                {
                    if (target.LagRecord)
                    {
                        pCmd->tick_count = TIME_TO_TICKS(target.SimulationTime + SDKUtils::GetLerp());
                    }
                }
            }
        }
    }
}