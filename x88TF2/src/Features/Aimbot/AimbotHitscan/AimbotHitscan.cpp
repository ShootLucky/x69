#include "AimbotHitscan.h"
#include "CFG.h"
#include <array>
#include <algorithm>

const char* HitboxNames[HITBOX_MAX] = {
    "Head",
    "Neck",
    "Lower Neck",
    "Pelvis",
    "Body",
    "Thorax",
    "Chest",
    "Upper Chest",
    "Right Thigh",
    "Left Thigh",
    "Right Calf",
    "Left Calf",
    "Right Foot",
    "Left Foot",
    "Right Hand",
    "Left Hand",
    "Right Upper Arm",
    "Right Forearm",
    "Left Upper Arm",
    "Left Forearm"
};

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

bool CAimbotHitscan::ScanHead(C_TFPlayer* pLocal, HitscanTarget_t& target, const Vec3& vLocalAngles)
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
    matrix3x4_t boneMatrix[128];
    if (!pPlayer->SetupBones(boneMatrix, 128, 0x100, I::GlobalVars->curtime))
        return false;
    const Vec3 vMins = pBox->bbmin;
    const Vec3 vMaxs = pBox->bbmax;
    const float scale = 0.95f;
    const std::array<Vec3, 9> vPoints = {
        Vec3(0.0f, 0.0f, 0.0f),
        Vec3(vMins.x * scale, 0.0f, 0.0f),
        Vec3(vMaxs.x * scale, 0.0f, 0.0f),
        Vec3(0.0f, vMins.y * scale, 0.0f),
        Vec3(0.0f, vMaxs.y * scale, 0.0f),
        Vec3(0.0f, 0.0f, vMins.z * scale),
        Vec3(0.0f, 0.0f, vMaxs.z * scale),
        Vec3(vMins.x * scale * 0.5f, vMins.y * scale * 0.5f, vMins.z * scale * 0.5f),
        Vec3(vMaxs.x * scale * 0.5f, vMaxs.y * scale * 0.5f, vMaxs.z * scale * 0.5f)
    };
    const Vec3 vLocalPos = pLocal->GetShootPos();
    struct PointInfo_t
    {
        Vec3 Position;
        float FOVTo;
        float DistToCenter;
    };
    std::vector<PointInfo_t> vecVisiblePoints = {};
    Vec3 vCenter = {};
    Math::VectorTransform(Vec3(0.0f, 0.0f, 0.0f), boneMatrix[pBox->bone], vCenter);
    for (const auto& vPoint : vPoints)
    {
        Vec3 vTransformed = {};
        Math::VectorTransform(vPoint, boneMatrix[pBox->bone], vTransformed);
        int nHitHitbox = -1;
        if (!H::AimUtils->TraceEntityBullet(pPlayer, vLocalPos, vTransformed, &nHitHitbox))
            continue;
        if (nHitHitbox != HITBOX_HEAD)
            continue;
        const Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vTransformed);
        const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
        const float flDistToCenter = vTransformed.DistTo(vCenter);
        vecVisiblePoints.push_back({ vTransformed, flFOVTo, flDistToCenter });
    }
    if (vecVisiblePoints.empty())
        return false;
    std::sort(vecVisiblePoints.begin(), vecVisiblePoints.end(), [](const PointInfo_t& a, const PointInfo_t& b)
        {
            return a.DistToCenter < b.DistToCenter;
        });
    target.Position = vecVisiblePoints.front().Position;
    target.AngleTo = Math::CalcAngle(vLocalPos, target.Position);
    target.WasMultiPointed = true;
    return true;
}

bool CAimbotHitscan::ScanBody(C_TFPlayer* pLocal, HitscanTarget_t& target, const Vec3& vLocalAngles)
{
    const bool bScanningBody = CFG::Aimbot_Hitscan_Scan_Body;
    const bool bScanningArms = CFG::Aimbot_Hitscan_Scan_Arms;
    const bool bScanningLegs = CFG::Aimbot_Hitscan_Scan_Legs;
    if (!bScanningBody && !bScanningArms && !bScanningLegs)
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
    matrix3x4_t boneMatrix[128];
    if (!pPlayer->SetupBones(boneMatrix, 128, 0x100, I::GlobalVars->curtime))
        return false;
    const Vec3 vLocalPos = pLocal->GetShootPos();
    struct PointInfo_t
    {
        Vec3 Position;
        float FOVTo;
        float DistToCenter;
    };
    std::vector<PointInfo_t> vecVisiblePoints = {};
    std::vector<int> vecHitboxes = {};
    if (bScanningBody)
    {
        vecHitboxes.push_back(HITBOX_PELVIS);
        vecHitboxes.push_back(HITBOX_BODY);
        vecHitboxes.push_back(HITBOX_THORAX);
        vecHitboxes.push_back(HITBOX_CHEST);
        vecHitboxes.push_back(HITBOX_UPPER_CHEST);
        vecHitboxes.push_back(HITBOX_LOWER_NECK);
        vecHitboxes.push_back(HITBOX_NECK);
    }
    if (bScanningArms)
    {
        vecHitboxes.push_back(HITBOX_LEFT_UPPER_ARM);
        vecHitboxes.push_back(HITBOX_LEFT_FOREARM);
        vecHitboxes.push_back(HITBOX_RIGHT_UPPER_ARM);
        vecHitboxes.push_back(HITBOX_RIGHT_FOREARM);
    }
    if (bScanningLegs)
    {
        vecHitboxes.push_back(HITBOX_LEFT_THIGH);
        vecHitboxes.push_back(HITBOX_LEFT_CALF);
        vecHitboxes.push_back(HITBOX_RIGHT_THIGH);
        vecHitboxes.push_back(HITBOX_RIGHT_CALF);
    }
    const float scale = 0.95f;
    for (int n : vecHitboxes)
    {
        const auto pBox = pSet->pHitbox(n);
        if (!pBox)
            continue;
        const Vec3 vMins = pBox->bbmin;
        const Vec3 vMaxs = pBox->bbmax;
        const std::array<Vec3, 9> vPoints = {
            Vec3(0.0f, 0.0f, 0.0f),
            Vec3(vMins.x * scale, 0.0f, 0.0f),
            Vec3(vMaxs.x * scale, 0.0f, 0.0f),
            Vec3(0.0f, vMins.y * scale, 0.0f),
            Vec3(0.0f, vMaxs.y * scale, 0.0f),
            Vec3(0.0f, 0.0f, vMins.z * scale),
            Vec3(0.0f, 0.0f, vMaxs.z * scale),
            Vec3(vMins.x * scale * 0.5f, vMins.y * scale * 0.5f, vMins.z * scale * 0.5f),
            Vec3(vMaxs.x * scale * 0.5f, vMaxs.y * scale * 0.5f, vMaxs.z * scale * 0.5f)
        };
        Vec3 vCenter = {};
        Math::VectorTransform(Vec3(0.0f, 0.0f, 0.0f), boneMatrix[pBox->bone], vCenter);
        for (const auto& vPoint : vPoints)
        {
            Vec3 vTransformed = {};
            Math::VectorTransform(vPoint, boneMatrix[pBox->bone], vTransformed);
            int nHitHitbox = -1;
            if (!H::AimUtils->TraceEntityBullet(pPlayer, vLocalPos, vTransformed, &nHitHitbox))
                continue;
            if (nHitHitbox != n)
                continue;
            const Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vTransformed);
            const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
            const float flDistToCenter = vTransformed.DistTo(vCenter);
            vecVisiblePoints.push_back({ vTransformed, flFOVTo, flDistToCenter });
        }
    }
    if (vecVisiblePoints.empty())
        return false;
    std::sort(vecVisiblePoints.begin(), vecVisiblePoints.end(), [](const PointInfo_t& a, const PointInfo_t& b)
        {
            return a.DistToCenter < b.DistToCenter;
        });
    target.Position = vecVisiblePoints.front().Position;
    target.AngleTo = Math::CalcAngle(vLocalPos, target.Position);
    return true;
}

bool CAimbotHitscan::ScanBuilding(C_TFPlayer* pLocal, HitscanTarget_t& target, const Vec3& vLocalAngles)
{
    if (!CFG::Aimbot_Hitscan_Scan_Buildings)
        return false;
    const auto pObject = target.Entity->As<C_BaseObject>();
    if (!pObject)
        return false;
    const Vec3 vLocalPos = pLocal->GetShootPos();
    struct PointInfo_t
    {
        Vec3 Position;
        float FOVTo;
        float DistToCenter;
    };
    std::vector<PointInfo_t> vecVisiblePoints = {};
    Vec3 vCenter = pObject->GetAbsOrigin() + (pObject->m_vecMins() + pObject->m_vecMaxs()) * 0.5f;
    if (pObject->GetClassId() == ETFClassIds::CObjectSentrygun)
    {
        for (int n = 0; n < pObject->GetNumOfHitboxes(); n++)
        {
            Vec3 vHitbox = pObject->GetHitboxPos(n);
            if (!H::AimUtils->TraceEntityBullet(pObject, vLocalPos, vHitbox))
                continue;
            const Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vHitbox);
            const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
            const float flDistToCenter = vHitbox.DistTo(vCenter);
            vecVisiblePoints.push_back({ vHitbox, flFOVTo, flDistToCenter });
        }
    }
    else
    {
        const Vec3 vMins = pObject->m_vecMins();
        const Vec3 vMaxs = pObject->m_vecMaxs();
        const std::array<Vec3, 8> vPoints = {
            Vec3(vMins.x * 0.9f, ((vMins.y + vMaxs.y) * 0.5f), ((vMins.z + vMaxs.z) * 0.5f)),
            Vec3(vMaxs.x * 0.9f, ((vMins.y + vMaxs.y) * 0.5f), ((vMins.z + vMaxs.z) * 0.5f)),
            Vec3(((vMins.x + vMaxs.x) * 0.5f), vMins.y * 0.9f, ((vMins.z + vMaxs.z) * 0.5f)),
            Vec3(((vMins.x + vMaxs.x) * 0.5f), vMaxs.y * 0.9f, ((vMins.z + vMaxs.z) * 0.5f)),
            Vec3(((vMins.x + vMaxs.x) * 0.5f), ((vMins.y + vMaxs.y) * 0.5f), vMins.z * 0.9f),
            Vec3(((vMins.x + vMaxs.x) * 0.5f), ((vMins.y + vMaxs.y) * 0.5f), vMaxs.z * 0.9f),
            Vec3(vMins.x * 0.9f, vMins.y * 0.9f, vMins.z * 0.9f),
            Vec3(vMaxs.x * 0.9f, vMaxs.y * 0.9f, vMaxs.z * 0.9f)
        };
        const matrix3x4_t& transform = pObject->RenderableToWorldTransform();
        for (const auto& vPoint : vPoints)
        {
            Vec3 vTransformed = {};
            Math::VectorTransform(vPoint, transform, vTransformed);
            if (!H::AimUtils->TraceEntityBullet(pObject, vLocalPos, vTransformed))
                continue;
            const Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vTransformed);
            const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
            const float flDistToCenter = vTransformed.DistTo(vCenter);
            vecVisiblePoints.push_back({ vTransformed, flFOVTo, flDistToCenter });
        }
    }
    if (vecVisiblePoints.empty())
        return false;
    std::sort(vecVisiblePoints.begin(), vecVisiblePoints.end(), [](const PointInfo_t& a, const PointInfo_t& b)
        {
            return a.DistToCenter < b.DistToCenter;
        });
    target.Position = vecVisiblePoints.front().Position;
    target.AngleTo = Math::CalcAngle(vLocalPos, target.Position);
    return true;
}

bool CAimbotHitscan::GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, HitscanTarget_t& outTarget)
{
    const Vec3 vLocalPos = pLocal->GetShootPos();
    Vec3 vLocalAngles;
    I::EngineClient->GetViewAngles(vLocalAngles);
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
            matrix3x4_t boneMatrix[128];
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
                    nStartRecord = 1;
                    nEndRecord = nRecords;
                }
                for (int n = nStartRecord; n <= nEndRecord; n++)
                {
                    const LagRecord_t* lagRecord = F::LagRecords->GetRecord(pPlayer, n);
                    if (!lagRecord || !lagRecord->Player || lagRecord->SimulationTime <= 0.0f)
                        continue;
                    F::LagRecordMatrixHelper->Set(lagRecord);
                    Vec3 vPos = pPlayer->GetHitboxPos(nAimHitbox);
                    float flFOV = Math::CalcFov(vLocalAngles, Math::CalcAngle(vLocalPos, vPos));
                    if (flFOV > CFG::Aimbot_FOV)
                    {
                        F::LagRecordMatrixHelper->Restore();
                        continue;
                    }
                    float flDist = vLocalPos.DistTo(vPos);
                    HitscanTarget_t t = {};
                    t.Entity = pPlayer;
                    t.Position = vPos;
                    t.AngleTo = Math::CalcAngle(vLocalPos, vPos);
                    t.FOVTo = flFOV;
                    t.DistanceTo = flDist;
                    t.AimedHitbox = nAimHitbox;
                    t.SimulationTime = lagRecord->SimulationTime;
                    t.LagRecord = lagRecord;
                    bool bHasVisiblePoint = false;
                    if (nAimHitbox == HITBOX_HEAD)
                    {
                        bHasVisiblePoint = ScanHead(pLocal, t, vLocalAngles);
                    }
                    else
                    {
                        bHasVisiblePoint = ScanBody(pLocal, t, vLocalAngles);
                    }
                    if (!bHasVisiblePoint)
                    {
                        int nHitHitbox = -1;
                        bHasVisiblePoint = H::AimUtils->TraceEntityBullet(pPlayer, vLocalPos, vPos, &nHitHitbox);
                        if (nAimHitbox == HITBOX_HEAD && nHitHitbox != HITBOX_HEAD)
                            bHasVisiblePoint = false;
                    }
                    bool bShouldAdd = !CFG::Aimbot_VisibleCheck || bHasVisiblePoint;
                    if (bShouldAdd)
                    {
                        t.AngleTo = Math::CalcAngle(vLocalPos, t.Position);
                        t.FOVTo = Math::CalcFov(vLocalAngles, t.AngleTo);
                        t.DistanceTo = vLocalPos.DistTo(t.Position);
                        m_vecTargets.push_back(t);
                    }
                    F::LagRecordMatrixHelper->Restore();
                }
            }
            else
            {
                Vec3 vPos = pPlayer->GetHitboxPos(nAimHitbox);
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
                bool bHasVisiblePoint = false;
                if (nAimHitbox == HITBOX_HEAD)
                {
                    bHasVisiblePoint = ScanHead(pLocal, t, vLocalAngles);
                }
                else
                {
                    bHasVisiblePoint = ScanBody(pLocal, t, vLocalAngles);
                }
                if (!bHasVisiblePoint)
                {
                    int nHitHitbox = -1;
                    bHasVisiblePoint = H::AimUtils->TraceEntityBullet(pPlayer, vLocalPos, vPos, &nHitHitbox);
                    if (nAimHitbox == HITBOX_HEAD && nHitHitbox != HITBOX_HEAD)
                        bHasVisiblePoint = false;
                }
                bool bShouldAdd = !CFG::Aimbot_VisibleCheck || bHasVisiblePoint;
                if (bShouldAdd)
                {
                    t.AngleTo = Math::CalcAngle(vLocalPos, t.Position);
                    t.FOVTo = Math::CalcFov(vLocalAngles, t.AngleTo);
                    t.DistanceTo = vLocalPos.DistTo(t.Position);
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
            bool bHasVisiblePoint = ScanBuilding(pLocal, t, vLocalAngles);
            if (!bHasVisiblePoint)
            {
                bHasVisiblePoint = H::AimUtils->TraceEntityBullet(pObject, vLocalPos, vPos);
            }
            bool bShouldAdd = !CFG::Aimbot_VisibleCheck || bHasVisiblePoint;
            if (bShouldAdd)
            {
                t.AngleTo = Math::CalcAngle(vLocalPos, t.Position);
                t.FOVTo = Math::CalcFov(vLocalAngles, t.AngleTo);
                t.DistanceTo = vLocalPos.DistTo(t.Position);
                m_vecTargets.push_back(t);
            }
        }
    }
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
    Vec3 vOldAngles = pCmd->viewangles;
    Vec3 vDelta = vAngles - vOldAngles;
    vDelta.x = Math::NormalizeAngle(vDelta.x);
    vDelta.y = Math::NormalizeAngle(vDelta.y);
    vDelta.z = Math::NormalizeAngle(vDelta.z);
    float fSmoothing = CFG::Aimbot_Hitscan_Smoothing;
    if (CFG::Aimbot_Hitscan_Mode == 1) fSmoothing = 1.0f; // Slight smoothing for silent
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
    if (CFG::Aimbot_Hitscan_Mode != 1)
    {
        vStep.x = std::clamp(vStep.x, -fMaxChange, fMaxChange);
        vStep.y = std::clamp(vStep.y, -fMaxChange, fMaxChange);
    }
    pCmd->viewangles = vOldAngles + vStep;
    Math::ClampAngles(pCmd->viewangles);
    if (CFG::Aimbot_Hitscan_Mode == 1)
    {
        float forward = pCmd->forwardmove;
        float side = pCmd->sidemove;
        float up = pCmd->upmove;
        float yaw_rad = DEG2RAD(pCmd->viewangles.y - vOldAngles.y);
        pCmd->forwardmove = cos(yaw_rad) * forward + cos(yaw_rad + (PI / 2)) * side;
        pCmd->sidemove = sin(yaw_rad) * forward + sin(yaw_rad + (PI / 2)) * side;
        I::EngineClient->SetViewAngles(vOldAngles);
        G::bSilentAngles = true;
    }
}

bool CAimbotHitscan::ShouldFire(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const HitscanTarget_t& target)
{
    if (!CFG::Aimbot_AutoShoot)
        return false;
    if (!G::bCanPrimaryAttack)
        return false;
    const int nWeaponID = pWeapon->GetWeaponID();
    const bool bIsSniperRifle = (nWeaponID == TF_WEAPON_SNIPERRIFLE || nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC || nWeaponID == TF_WEAPON_SNIPERRIFLE_DECAP);
    if (bIsSniperRifle)
    {
        if (CFG::Aimbot_WaitForHeadshot && target.AimedHitbox == HITBOX_HEAD)
        {
            if (!G::bCanHeadshot)
                return false;
        }
        if (CFG::Aimbot_WaitForCharge)
        {
            float flCharge = pWeapon->As<C_TFSniperRifle>()->m_flChargedDamage();
            if (flCharge < 50.0f)
                return false;
        }
    }
    if (target.Entity->GetClassId() != ETFClassIds::CTFPlayer)
        return true;
    const auto pPlayer = target.Entity->As<C_TFPlayer>();
    if (CFG::Aimbot_SmoothAutoShoot && CFG::Aimbot_Hitscan_Mode != 0)
    {
        Vec3 vForward = {};
        Math::AngleVectors(pCmd->viewangles, &vForward);
        const Vec3 vTraceStart = pLocal->GetShootPos();
        const Vec3 vTraceEnd = vTraceStart + (vForward * 8192.0f);
        if (target.LagRecord)
        {
            F::LagRecordMatrixHelper->Set(target.LagRecord);
        }
        int nHitHitbox = -1;
        bool bHits = H::AimUtils->TraceEntityBullet(pPlayer, vTraceStart, vTraceEnd, &nHitHitbox);
        if (target.AimedHitbox == HITBOX_HEAD && nHitHitbox != HITBOX_HEAD)
            bHits = false;
        if (target.LagRecord)
        {
            F::LagRecordMatrixHelper->Restore();
        }
        if (!bHits)
            return false;
    }
    return true;
}

void CAimbotHitscan::HandleFire(CUserCmd* pCmd, C_TFWeaponBase* pWeapon)
{
    if (!pWeapon->HasPrimaryAmmoForShot())
        return;
    pCmd->buttons |= IN_ATTACK;
}

bool CAimbotHitscan::IsFiring(CUserCmd* pCmd, C_TFWeaponBase* pWeapon)
{
    if (!pWeapon->HasPrimaryAmmoForShot())
        return false;
    const int nWeaponID = pWeapon->GetWeaponID();
    if (nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC)
        return (G::nOldButtons & IN_ATTACK) && !(pCmd->buttons & IN_ATTACK);
    return (pCmd->buttons & IN_ATTACK) && G::bCanPrimaryAttack;
}

void CAimbotHitscan::Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    m_bActive = false;
    m_LastTarget = {};
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
            const bool bIsSniperRifle = (weaponID == TF_WEAPON_SNIPERRIFLE || weaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC || weaponID == TF_WEAPON_SNIPERRIFLE_DECAP);
            if (bIsSniperRifle && CFG::Aimbot_AutoScope && !pLocal->IsZoomed() && G::bCanPrimaryAttack)
            {
                pCmd->buttons |= IN_ATTACK2;
                return;
            }
            Aim(pCmd, pLocal, target.AngleTo);
            if (CFG::Aimbot_AutoShoot)
            {
                if (weaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC)
                {
                    auto pSniper = pWeapon->As<C_TFSniperRifleClassic>();
                    float flCharge = pSniper->m_flChargedDamage();
                    float flRequired = (target.AimedHitbox == HITBOX_HEAD) ? 150.0f : 0.1f;
                    if (CFG::Aimbot_WaitForCharge)
                        flRequired = std::max(flRequired, 50.0f);
                    if (flCharge < flRequired)
                    {
                        pCmd->buttons |= IN_ATTACK;
                    }
                    else if (ShouldFire(pCmd, pLocal, pWeapon, target))
                    {
                        pCmd->buttons &= ~IN_ATTACK;
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
            const bool bIsFiring = IsFiring(pCmd, pWeapon);
            G::bFiring = bIsFiring;
            if (bIsFiring)
            {
                pCmd->tick_count = TIME_TO_TICKS(target.SimulationTime + SDKUtils::GetLerp() + F::LagRecords->GetFakeLatency());
            }
            m_bActive = true;
            m_LastTarget = target;
            m_bAutoShoot = CFG::Aimbot_AutoShoot;
        }
    }
}

void CAimbotHitscan::DrawDebug()
{
    int x = 10;
    int y = 200;
    Color_t white{ 255, 255, 255, 255 };
    H::Draw->String(H::Fonts->Get(EFonts::ESP), x, y, white, POS_DEFAULT, "Aimbot: %s", m_bActive ? "Active" : "Inactive");
    y += 15;
    H::Draw->String(H::Fonts->Get(EFonts::ESP), x, y, white, POS_DEFAULT, "Auto Shoot: %s", m_bAutoShoot ? "On" : "Off");
    y += 15;
    if (m_LastTarget.Entity)
    {
        if (m_LastTarget.Entity->GetClassId() == ETFClassIds::CTFPlayer)
        {
            auto pPlayer = m_LastTarget.Entity->As<C_TFPlayer>();
            player_info_t info{};
            if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &info))
            {
                H::Draw->String(H::Fonts->Get(EFonts::ESP), x, y, white, POS_DEFAULT, "Target: %s", info.name);
                y += 15;
            }
        }
        else
        {
            H::Draw->String(H::Fonts->Get(EFonts::ESP), x, y, white, POS_DEFAULT, "Target: Building");
            y += 15;
        }
        if (m_LastTarget.AimedHitbox >= 0 && m_LastTarget.AimedHitbox < HITBOX_MAX)
        {
            H::Draw->String(H::Fonts->Get(EFonts::ESP), x, y, white, POS_DEFAULT, "Hitbox: %s", HitboxNames[m_LastTarget.AimedHitbox]);
            y += 15;
        }
        Vec3 screen;
        if (H::Draw->W2S(m_LastTarget.Position, screen))
        {
            H::Draw->Line(screen.x - 5, screen.y - 5, screen.x + 5, screen.y + 5, Color_t(255, 0, 0, 255));
            H::Draw->Line(screen.x + 5, screen.y - 5, screen.x - 5, screen.y + 5, Color_t(255, 0, 0, 255));
        }
    }
}