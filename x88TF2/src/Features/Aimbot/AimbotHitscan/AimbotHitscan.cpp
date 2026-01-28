// AimbotHitscan.cpp - CORRIGIDO para o SEU SDK
// Adapted from Amalgam by rei-2

#include "AimbotHitscan.h"
#include "CFG.h"

void CAimbotHitscan::Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    if (!CFG::Aimbot_Active)
        return;

    static bool bToggled = false;
    if (!H::Input->KeybindMethod(CFG::Aimbot_Key, CFG::Aimbot_KeyMode, &bToggled))
        return;

    if (!pLocal || !pWeapon || pLocal->deadflag())
        return;

    const int nWeaponID = pWeapon->GetWeaponID();

    // ========== MINIGUN HANDLING ==========
    if (nWeaponID == TF_WEAPON_MINIGUN)
    {
        pCmd->buttons |= IN_ATTACK2; // Auto rev

        auto pMinigun = pWeapon->As<C_TFMinigun>();
        if (pMinigun)
        {
            int nWeaponState = pMinigun->m_iWeaponState();
            if (nWeaponState != AC_STATE_FIRING && nWeaponState != AC_STATE_SPINNING)
                return;
        }
    }

    // ========== AUTO SCOPE ==========
    if (nWeaponID == TF_WEAPON_SNIPERRIFLE || nWeaponID == TF_WEAPON_SNIPERRIFLE_DECAP)
    {
        bool bScoped = pLocal->InCond(TF_COND_ZOOMED);
        if (CFG::Aimbot_AutoScope && !bScoped)
        {
            pCmd->buttons |= IN_ATTACK2;
            return;
        }
    }

    // ========== CLASSIC CHARGE ==========
    if (nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC)
    {
        if (CFG::Aimbot_Hitscan_Mode) // Se mode != 0
            pCmd->buttons |= IN_ATTACK;
    }

    // ========== GET TARGETS ==========
    std::vector<Target_t> vTargets = GetTargets(pLocal, pWeapon);
    if (vTargets.empty())
        return;

    // ========== PROCESS TARGETS ==========
    for (auto& tTarget : vTargets)
    {
        const int iResult = CanHit(tTarget, pLocal, pWeapon, pCmd);
        if (!iResult)
            continue;

        // Setar globals
        G::nTargetIndex = tTarget.pEntity->entindex();
        G::flAimbotFOV = tTarget.fFOV;

        // Se só pode mirar mas não atirar
        if (iResult == 2)
        {
            Aim(pCmd, tTarget.vAngles, CFG::Aimbot_Hitscan_Mode);
            break;
        }

        // ========== SHOULD FIRE ==========
        if (ShouldFire(pLocal, pWeapon, pCmd, tTarget))
        {
            switch (nWeaponID)
            {
            case TF_WEAPON_SNIPERRIFLE_CLASSIC:
                if (pWeapon->As<C_TFSniperRifle>()->m_flChargedDamage() && pLocal->m_hGroundEntity())
                    pCmd->buttons &= ~IN_ATTACK;
                break;
            default:
                pCmd->buttons |= IN_ATTACK;
                break;
            }

            // Tapfire
            if (CFG::Aimbot_MinigunTapfire && pWeapon->GetWeaponSpread() != 0.f
                && m_vEyePos.DistTo(tTarget.vPos) > 1000.0f)
            {
                const float flTimeSinceLastShot = (pLocal->m_nTickBase() * TICK_INTERVAL) - pWeapon->m_flLastFireTime();
                if (flTimeSinceLastShot <= (pWeapon->GetBulletsPerShot() > 1 ? 0.25f : 1.25f))
                    pCmd->buttons &= ~IN_ATTACK;
            }
        }

        // Backtrack
        if (tTarget.bBacktrack && tTarget.pRecord)
        {
            pCmd->tick_count = TIME_TO_TICKS(tTarget.pRecord->m_flSimTime);
        }

        // Aplicar aim
        Aim(pCmd, tTarget.vAngles, CFG::Aimbot_Hitscan_Mode);
        break;
    }
}

// ========== GET TARGETS ==========
std::vector<CAimbotHitscan::Target_t> CAimbotHitscan::GetTargets(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    std::vector<Target_t> vTargets;
    vTargets.reserve(32);

    const Vec3 vLocalPos = pLocal->GetShootPos();
    const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

    for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++)
    {
        if (i == pLocal->entindex())
            continue;

        auto pEntity = I::ClientEntityList->GetClientEntity(i)->As<C_TFPlayer>();
        if (!pEntity || !IsValidTarget(pEntity, pLocal))
            continue;

        auto hitboxes = GetActiveHitboxes();

        for (int nHitbox : hitboxes)
        {
            Vec3 vHitboxPos = GetHitboxPos(pEntity, nHitbox);
            if (vHitboxPos.IsZero())
                continue;

            Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vHitboxPos);
            float flFOV = Math::CalcFov(vLocalAngles, vAngleTo);

            if (flFOV > CFG::Aimbot_FOV)
                continue;

            float flDist = vLocalPos.DistTo(vHitboxPos);

            Target_t target{};
            target.pEntity = pEntity;
            target.vPos = vHitboxPos;
            target.vAngles = vAngleTo;
            target.fFOV = flFOV;
            target.fDist = flDist;
            target.nHitbox = nHitbox;
            target.nPriority = GetHitboxPriority(nHitbox, pLocal, pWeapon, pEntity);
            target.bBacktrack = false;
            target.pRecord = nullptr;

            vTargets.push_back(target);
        }
    }

    // Sort
    std::sort(vTargets.begin(), vTargets.end(), [](const Target_t& a, const Target_t& b) {
        if (a.nPriority != b.nPriority)
            return a.nPriority < b.nPriority;
        return a.fFOV < b.fFOV;
        });

    return vTargets;
}

// ========== GET HITBOX PRIORITY ==========
int CAimbotHitscan::GetHitboxPriority(int nHitbox, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, C_BaseEntity* pTarget)
{
    if (nHitbox < 0)
        return -1;

    bool bHeadshot = false;

    // Verificar se é player (não tem IsPlayer(), então checamos diretamente)
    if (pTarget->entindex() >= 1 && pTarget->entindex() <= I::EngineClient->GetMaxClients())
    {
        switch (pWeapon->GetWeaponID())
        {
        case TF_WEAPON_SNIPERRIFLE:
        case TF_WEAPON_SNIPERRIFLE_DECAP:
        case TF_WEAPON_SNIPERRIFLE_CLASSIC:
        {
            // Verificar se pode dar headshot (scoped ou classic charged)
            if (pLocal->InCond(TF_COND_ZOOMED) || CFG::Aimbot_WaitForHeadshot)
                bHeadshot = true;
            break;
        }
        }
    }

    bool bHeadOnly = bHeadshot; // Sem HeadshotOnly cfg

    int iHeadPriority = bHeadOnly || bHeadshot ? 0 : 1;
    int iBodyPriority = bHeadOnly ? -1 : bHeadshot ? 1 : 0;
    int iMiscPriority = bHeadOnly ? -1 : 2;
    int iLimbPriority = bHeadOnly ? -1 : 3;

    switch (nHitbox)
    {
    case AIMBOT_HITBOX_HEAD: return iHeadPriority;
    case AIMBOT_HITBOX_SPINE0:
    case AIMBOT_HITBOX_SPINE1:
    case AIMBOT_HITBOX_SPINE2:
    case AIMBOT_HITBOX_SPINE3: return iBodyPriority;
    case AIMBOT_HITBOX_PELVIS: return iMiscPriority;
    }

    return iLimbPriority;
}

// ========== CAN HIT (ADAPTADO) ==========
int CAimbotHitscan::CanHit(Target_t& tTarget, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, CUserCmd* pCmd)
{
    m_vEyePos = pLocal->GetShootPos();

    // GetRange não existe, usar valor fixo
    const float flMaxRange = 8192.0f * 8192.0f; // Squared

    auto pModel = tTarget.pEntity->GetModel();
    if (!pModel) return 0;

    auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
    if (!pHDR) return 0;

    auto pSet = pHDR->pHitboxSet(tTarget.pEntity->As<C_BaseAnimating>()->m_nHitboxSet());
    if (!pSet) return 0;

    // Sem backtrack - apenas current state
    matrix3x4_t aBones[MAXSTUDIOBONES];
    if (!tTarget.pEntity->SetupBones(aBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, tTarget.pEntity->m_flSimulationTime()))
        return 0;

    // Construir lista de hitboxes com prioridade
    std::vector<std::tuple<const mstudiobbox_t*, int, int>> vHitboxes;
    for (int nHitbox = 0; nHitbox < pSet->numhitboxes; nHitbox++)
    {
        int iPriority = GetHitboxPriority(nHitbox, pLocal, pWeapon, tTarget.pEntity);
        if (iPriority == -1)
            continue;

        auto pBox = pSet->pHitbox(nHitbox);
        if (!pBox) continue;

        vHitboxes.emplace_back(pBox, nHitbox, iPriority);
    }

    std::sort(vHitboxes.begin(), vHitboxes.end(), [&](const auto& a, const auto& b) -> bool
        {
            return std::get<2>(a) < std::get<2>(b);
        });

    float flModelScale = tTarget.pEntity->As<C_BaseAnimating>()->m_flModelScale();
    float flBoneScale = 0.5f; // BoneSizeMinimumScale fixo
    float flBoneSubtract = 0.0f; // BoneSizeSubtract fixo

    int iReturn = 0;

    for (auto& [pBox, nHitbox, _] : vHitboxes)
    {
        Vec3 vMins = pBox->bbmin;
        Vec3 vMaxs = pBox->bbmax;
        Vec3 vCheckMins = (vMins + flBoneSubtract / flModelScale) * flBoneScale;
        Vec3 vCheckMaxs = (vMaxs - flBoneSubtract / flModelScale) * flBoneScale;

        Vec3 vOffset;
        {
            Vec3 vOrigin, vCenter;
            Math::VectorTransform({}, aBones[pBox->bone], vOrigin);
            Math::VectorTransform((vMins + vMaxs) / 2, aBones[pBox->bone], vCenter);
            vOffset = vCenter - vOrigin;
        }

        // ========== MULTIPOINT ==========
        std::vector<Vec3> vPoints = { Vec3() };

        // Multipoint simples apenas para cabeça
        if (nHitbox == AIMBOT_HITBOX_HEAD && CFG::Aimbot_Hitscan_Multipoint_Scale > 0.f)
        {
            float flScale = CFG::Aimbot_Hitscan_Multipoint_Scale / 100.f;
            Vec3 vMinsS = (vMins - vMaxs) / 2 * flScale;
            Vec3 vMaxsS = (vMaxs - vMins) / 2 * flScale;

            vPoints = {
                Vec3(),
                Vec3(vMinsS.x, vMinsS.y, vMaxsS.z),
                Vec3(vMaxsS.x, vMinsS.y, vMaxsS.z),
                Vec3(vMinsS.x, vMaxsS.y, vMaxsS.z),
                Vec3(vMaxsS.x, vMaxsS.y, vMaxsS.z),
                Vec3(vMinsS.x, vMinsS.y, vMinsS.z),
                Vec3(vMaxsS.x, vMinsS.y, vMinsS.z),
                Vec3(vMinsS.x, vMaxsS.y, vMinsS.z),
                Vec3(vMaxsS.x, vMaxsS.y, vMinsS.z)
            };
        }

        for (auto& vPoint : vPoints)
        {
            Vec3 vOrigin;
            Math::VectorTransform(vPoint, aBones[pBox->bone], vOrigin);
            vOrigin += vOffset;

            if (m_vEyePos.DistToSqr(vOrigin) > flMaxRange)
                continue;

            Vec3 vAngles;
            bool bChanged = Aim(pCmd->viewangles, Math::CalcAngle(m_vEyePos, vOrigin), vAngles, CFG::Aimbot_Hitscan_Mode);

            Vec3 vForward;
            Math::AngleVectors(vAngles, &vForward);
            float flDist = m_vEyePos.DistTo(vOrigin);

            if (bChanged || H::AimUtils->VisPos(pLocal, tTarget.pEntity, m_vEyePos, vOrigin))
            {
                // Verificar se ray acerta a hitbox (SEM RayToOBB - usar vischeck simples)
                if (!bChanged || H::AimUtils->VisPos(pLocal, tTarget.pEntity, m_vEyePos, m_vEyePos + vForward * flDist))
                {
                    // Pode atirar!
                    tTarget.vPos = vOrigin;
                    tTarget.vAngles = vAngles;
                    tTarget.nAimedHitbox = nHitbox;
                    tTarget.bBacktrack = false;
                    return 1;
                }
                else if (bChanged && H::AimUtils->VisPos(pLocal, tTarget.pEntity, m_vEyePos, vOrigin))
                {
                    // Pode mirar mas não atirar
                    if (iReturn != 2 || Math::CalcFov(pCmd->viewangles, vAngles) < Math::CalcFov(pCmd->viewangles, tTarget.vAngles))
                        tTarget.vAngles = vAngles;
                    iReturn = 2;
                }
            }
        }
    }

    return iReturn;
}

// ========== SHOULD FIRE ==========
bool CAimbotHitscan::ShouldFire(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, CUserCmd* pCmd, const Target_t& tTarget)
{
    if (!CFG::Aimbot_AutoShoot)
        return false;

    if (CFG::Aimbot_WaitForHeadshot)
    {
        switch (pWeapon->GetWeaponID())
        {
        case TF_WEAPON_SNIPERRIFLE:
        case TF_WEAPON_SNIPERRIFLE_DECAP:
            if (!pLocal->InCond(TF_COND_ZOOMED) && pLocal->InCond(TF_COND_AIMING))
                return false;
            break;
        case TF_WEAPON_SNIPERRIFLE_CLASSIC:
            if (!pLocal->InCond(TF_COND_ZOOMED))
                return false;
            break;
        }
    }

    if (CFG::Aimbot_WaitForCharge)
    {
        switch (pWeapon->GetWeaponID())
        {
        case TF_WEAPON_SNIPERRIFLE:
        case TF_WEAPON_SNIPERRIFLE_DECAP:
        case TF_WEAPON_SNIPERRIFLE_CLASSIC:
        {
            auto pSniper = pWeapon->As<C_TFSniperRifle>();
            if (pSniper && pLocal->InCond(TF_COND_AIMING))
            {
                float flCharge = pSniper->m_flChargedDamage();
                if (flCharge < 150.f)
                {
                    int iHealth = tTarget.pEntity->m_iHealth();
                    int iDamage = static_cast<int>(std::ceil(std::max(flCharge, 50.f)));

                    if (iHealth > iDamage)
                        return false;
                }
            }

            return false;
        }
        }
    }

    return true;
}

// ========== AIM OVERLOAD 1 (CALCULAR ÂNGULO) ==========
bool CAimbotHitscan::Aim(Vec3 vCurAngle, Vec3 vToAngle, Vec3& vOut, int iMethod)
{
    Vec3 vPunch = H::Entities->GetLocal() ? H::Entities->GetLocal()->m_vecPunchAngle() : Vec3();

    bool bReturn = false;
    vToAngle -= vPunch;

    switch (iMethod)
    {
    case 0: // Plain
    case 1: // Silent
    case 3: // Locking
        vOut = vToAngle;
        break;
    case 2: // Smooth
    {
        // LerpAngle manual
        Vec3 vDelta = vToAngle - vCurAngle;
        Math::ClampAngles(vDelta);
        float fLerp = CFG::Aimbot_Hitscan_Smoothing / 100.f;
        vOut = vCurAngle + (vDelta * fLerp);
        bReturn = true;
        break;
    }
    }

    Math::ClampAngles(vOut);
    return bReturn;
}

// ========== AIM OVERLOAD 2 (APLICAR AO COMANDO) ==========
void CAimbotHitscan::Aim(CUserCmd* pCmd, Vec3& vAngle, int iMethod)
{
    switch (iMethod)
    {
    case 0: // Plain
        if (G::bCanPrimaryAttack)
        {
            pCmd->viewangles = vAngle;
            I::EngineClient->SetViewAngles(vAngle);
        }
        break;

    case 2: // Smooth
        pCmd->viewangles = vAngle;
        I::EngineClient->SetViewAngles(vAngle);
        break;

    case 1: // Silent
        if (G::bCanPrimaryAttack)
        {
            H::AimUtils->FixMovement(pCmd, vAngle);
            pCmd->viewangles = vAngle;
            G::bPSilentAngles = true;
        }
        break;

    case 3: // Locking
        H::AimUtils->FixMovement(pCmd, vAngle);
        pCmd->viewangles = vAngle;
        G::bPSilentAngles = true;
        break;
    }
}

// ========== HELPER FUNCTIONS ==========

std::vector<int> CAimbotHitscan::GetActiveHitboxes()
{
    std::vector<int> hitboxes;

    if (CFG::Aimbot_Hitbox_Head)
        hitboxes.push_back(AIMBOT_HITBOX_HEAD);

    if (CFG::Aimbot_Hitbox_Body)
    {
        hitboxes.push_back(AIMBOT_HITBOX_PELVIS);
        hitboxes.push_back(AIMBOT_HITBOX_SPINE0);
        hitboxes.push_back(AIMBOT_HITBOX_SPINE1);
        hitboxes.push_back(AIMBOT_HITBOX_SPINE2);
        hitboxes.push_back(AIMBOT_HITBOX_SPINE3);
    }

    return hitboxes;
}

bool CAimbotHitscan::IsValidTarget(C_TFPlayer* pEntity, C_TFPlayer* pLocal)
{
    if (pEntity->deadflag())
        return false;

    if (CFG::Aimbot_TeamCheck && pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
        return false;

    if (pEntity->InCond(TF_COND_PHASE) || pEntity->InCond(TF_COND_HALLOWEEN_GHOST_MODE))
        return false;

    if (CFG::Aimbot_Ignore_Invulnerable && pEntity->IsInvulnerable())
        return false;

    if (CFG::Aimbot_Ignore_Invisible && pEntity->IsInvisible())
        return false;

    return true;
}

Vec3 CAimbotHitscan::GetHitboxPos(C_TFPlayer* pEntity, int nHitbox)
{
    const model_t* pModel = pEntity->GetModel();
    if (!pModel)
        return Vec3(0, 0, 0);

    studiohdr_t* pHdr = I::ModelInfoClient->GetStudiomodel(pModel);
    if (!pHdr)
        return Vec3(0, 0, 0);

    mstudiohitboxset_t* pSet = pHdr->pHitboxSet(pEntity->m_nHitboxSet());
    if (!pSet || nHitbox >= pSet->numhitboxes)
        return Vec3(0, 0, 0);

    mstudiobbox_t* pBox = pSet->pHitbox(nHitbox);
    if (!pBox)
        return Vec3(0, 0, 0);

    matrix3x4_t bones[128];
    if (!pEntity->SetupBones(bones, 128, 0x100, I::GlobalVars->curtime))
        return Vec3(0, 0, 0);

    Vec3 vMin, vMax;
    Math::VectorTransform(pBox->bbmin, bones[pBox->bone], vMin);
    Math::VectorTransform(pBox->bbmax, bones[pBox->bone], vMax);

    return (vMin + vMax) * 0.5f;
}

bool CAimbotHitscan::IsFiring(CUserCmd* pCmd, C_TFWeaponBase* pWeapon)
{
    if (!pWeapon->HasPrimaryAmmoForShot())
        return false;

    if (pWeapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE_CLASSIC)
        return (G::nOldButtons & IN_ATTACK) && !(pCmd->buttons & IN_ATTACK);

    return (pCmd->buttons & IN_ATTACK) && G::bCanPrimaryAttack;
}