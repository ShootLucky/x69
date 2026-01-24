#include "AimbotHitscan.h"
#include "CFG.h"

void CAimbotHitscan::Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    if (!CFG::Aimbot_Active)
        return;

    static bool bToggled = false;
    if (!H::Input->KeybindMethod(CFG::Aimbot_Key, CFG::Aimbot_KeyMode, &bToggled))
    {
        m_iLastTargetIndex = 0;
        m_bTargetLocked = false;
        return;
    }

    if (!pLocal || !pWeapon || pLocal->deadflag())
        return;

    const int nWeaponID = pWeapon->GetWeaponID();
    bool bSilentMode = (CFG::Aimbot_Hitscan_Mode == 1);

    // Minigun auto-rev (CORRIGIDO - baseado Amalgam linha 545)
    if (nWeaponID == TF_WEAPON_MINIGUN)
    {
        auto pMinigun = pWeapon->As<C_TFMinigun>();
        if (pMinigun)
        {
            int iWeaponState = pMinigun->m_iWeaponState();
            // Se não está FIRING ou SPINNING, começar rev
            if (iWeaponState != AC_STATE_FIRING && iWeaponState != AC_STATE_SPINNING)
            {
                pCmd->buttons |= IN_ATTACK2;
                return; // RETURN - não faz mais nada até estar pronto
            }
        }
    }

    // Auto scope
    if (CFG::Aimbot_AutoScope)
    {
        if (nWeaponID == TF_WEAPON_SNIPERRIFLE || nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC || nWeaponID == TF_WEAPON_SNIPERRIFLE_DECAP)
        {
            bool bScoped = pLocal->InCond(TF_COND_ZOOMED);
            if (!bScoped)
            {
                pCmd->buttons |= IN_ATTACK2;
                return;
            }
        }
    }

    // Wait for charge
    if (CFG::Aimbot_WaitForCharge)
    {
        if (nWeaponID == TF_WEAPON_SNIPERRIFLE || nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC || nWeaponID == TF_WEAPON_SNIPERRIFLE_DECAP)
        {
            if (pLocal->m_flChargeMeter() < 100.0f)
                return;
        }
    }

    // Get targets
    auto vTargets = GetTargets(pLocal, pWeapon);
    if (vTargets.empty())
    {
        m_iLastTargetIndex = 0;
        m_bTargetLocked = false;
        return;
    }

    Target_t& target = vTargets.front();

    // Aplicar aim
    Vec3 vOldAngles = pCmd->viewangles;
    float fOldForward = pCmd->forwardmove;
    float fOldSidemove = pCmd->sidemove;

    if (bSilentMode)
    {
        pCmd->viewangles = target.vAngles;
        Math::ClampAngles(pCmd->viewangles);
        G::bPSilentAngles = true;

        // Fix movement (baseado Amalgam SDK::FixMovement)
        float yawDelta = pCmd->viewangles.y - vOldAngles.y;
        while (yawDelta > 180.0f) yawDelta -= 360.0f;
        while (yawDelta < -180.0f) yawDelta += 360.0f;

        float yawRad = DEG2RAD(yawDelta);
        float cosYaw = cosf(yawRad);
        float sinYaw = sinf(yawRad);

        pCmd->forwardmove = cosYaw * fOldForward - sinYaw * fOldSidemove;
        pCmd->sidemove = sinYaw * fOldForward + cosYaw * fOldSidemove;
    }
    else
    {
        float fSmooth = CFG::Aimbot_Hitscan_Smoothing;
        if (fSmooth <= 1.0f)
        {
            pCmd->viewangles = target.vAngles;
        }
        else
        {
            Vec3 vDelta = target.vAngles - pCmd->viewangles;
            Math::ClampAngles(vDelta);
            pCmd->viewangles += vDelta / fSmooth;
        }
        Math::ClampAngles(pCmd->viewangles);
    }

    G::nTargetIndex = target.pEntity->entindex();
    G::flAimbotFOV = target.fFOV;

    // Auto shoot
    if (!CFG::Aimbot_AutoShoot || !target.bCanShoot)
        return;

    if (!G::bCanPrimaryAttack || !pWeapon->HasPrimaryAmmoForShot())
        return;

    if (nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC)
        return;

    // Minigun (baseado Amalgam linha 582)
    if (nWeaponID == TF_WEAPON_MINIGUN)
    {
        if (CFG::Aimbot_MinigunTapfire && target.fDist > 1000.0f)
        {
            float flTimeSinceLastShot = (pLocal->m_nTickBase() * TICK_INTERVAL) - pWeapon->m_flLastFireTime();
            if (flTimeSinceLastShot <= 1.25f)
            {
                pCmd->buttons &= ~IN_ATTACK;
                return;
            }
        }

        pCmd->buttons |= IN_ATTACK;
        return;
    }

    // Verificar precisão
    Vec3 vAngleDelta = target.vAngles - pCmd->viewangles;
    Math::ClampAngles(vAngleDelta);
    float fAngleDiff = sqrtf(vAngleDelta.x * vAngleDelta.x + vAngleDelta.y * vAngleDelta.y);

    bool bScoped = (nWeaponID == TF_WEAPON_SNIPERRIFLE || nWeaponID == TF_WEAPON_SNIPERRIFLE_DECAP) && pLocal->IsZoomed();
    float fThreshold = bSilentMode ? (bScoped ? 0.1f : 0.5f) : (bScoped ? 0.5f : 3.0f);

    if (fAngleDiff > fThreshold)
        return;

    pCmd->buttons |= IN_ATTACK;
}

bool CAimbotHitscan::IsFiring(CUserCmd* pCmd, C_TFWeaponBase* pWeapon)
{
    if (!pWeapon->HasPrimaryAmmoForShot())
        return false;
    if (pWeapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE_CLASSIC)
        return (G::nOldButtons & IN_ATTACK) && !(pCmd->buttons & IN_ATTACK);
    return (pCmd->buttons & IN_ATTACK) && G::bCanPrimaryAttack;
}

std::vector<CAimbotHitscan::Target_t> CAimbotHitscan::GetTargets(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    std::vector<Target_t> vTargets;
    Vec3 vLocalPos = pLocal->GetShootPos();
    Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

    std::vector<int> hitboxes = GetActiveHitboxes();
    if (hitboxes.empty())
        return vTargets;

    for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++)
    {
        if (i == pLocal->entindex())
            continue;

        C_TFPlayer* pEntity = I::ClientEntityList->GetClientEntity(i)->As<C_TFPlayer>();
        if (!pEntity || !IsValidTarget(pEntity, pLocal))
            continue;

        Target_t t;
        if (ScanTarget(pLocal, pEntity, vLocalPos, vLocalAngles, hitboxes, t))
        {
            if (t.fFOV <= CFG::Aimbot_FOV)
            {
                t.bCanShoot = !CFG::Aimbot_WaitForHeadshot || t.nHitbox == 0;
                vTargets.push_back(t);
            }
        }
    }

    std::sort(vTargets.begin(), vTargets.end(), [](const Target_t& a, const Target_t& b) {
        switch (CFG::Aimbot_Hitscan_Sort)
        {
        case 0: return a.fDist < b.fDist;
        case 1: return a.fFOV < b.fFOV;
        case 2: return a.nHealth < b.nHealth;
        default: return a.fFOV < b.fFOV;
        }
        });

    return vTargets;
}

bool CAimbotHitscan::ScanTarget(C_TFPlayer* pLocal, C_TFPlayer* pTarget, const Vec3& vLocalPos, const Vec3& vLocalAngles, const std::vector<int>& hitboxes, Target_t& out)
{
    Vec3 vPredOffset(0, 0, 0);
    INetChannelInfo* pNetChan = I::EngineClient->GetNetChannelInfo();
    if (pNetChan)
    {
        float flLat = pNetChan->GetLatency(FLOW_OUTGOING) + pNetChan->GetLatency(FLOW_INCOMING);
        float flLerp = I::CVar->FindVar("cl_interp")->GetFloat();
        vPredOffset = pTarget->m_vecVelocity() * (flLat + flLerp);
    }

    float fBestScore = FLT_MAX;
    bool bFound = false;

    // Baseado Amalgam - itera hitboxes em ordem de prioridade
    for (int hitbox : hitboxes)
    {
        Vec3 vHitbox = GetHitboxPos(pTarget, hitbox);
        if (vHitbox.IsZero())
            continue;

        Vec3 vPred = vHitbox + vPredOffset;
        Vec3 vAngles = Math::CalcAngle(vLocalPos, vPred);
        Vec3 vDelta = vLocalAngles - vAngles;
        Math::ClampAngles(vDelta);
        float fFOV = sqrtf(vDelta.x * vDelta.x + vDelta.y * vDelta.y);

        if (fFOV > CFG::Aimbot_FOV)
            continue;

        Vec3 vForward;
        Math::AngleVectors(vAngles, &vForward);
        int nHit = -1;
        if (!H::AimUtils->TraceEntityBullet(pTarget, vLocalPos, vLocalPos + vForward * 8192.0f, &nHit))
            continue;

        // Score: FOV + distância - prioridade
        float fScore = fFOV * 2.0f + vLocalPos.DistTo(vHitbox) * 0.001f - GetHitboxPriority(hitbox);
        if (fScore < fBestScore)
        {
            fBestScore = fScore;
            out.pEntity = pTarget;
            out.vHitboxPos = vPred;
            out.vAngles = vAngles;
            out.fFOV = fFOV;
            out.fDist = vLocalPos.DistTo(vHitbox);
            out.nHealth = pTarget->m_iHealth();
            out.nHitbox = hitbox;
            out.bCanHit = true;
            bFound = true;
        }
    }

    return bFound;
}

std::vector<int> CAimbotHitscan::GetActiveHitboxes()
{
    std::vector<int> h;

    // Ordem de prioridade baseada no Amalgam (head > body > pelvis > arms > legs)
    // HITBOXES CORRETOS do TF2 (baseado Amalgam GetHitboxPriority linha 66-82)
    if (CFG::Aimbot_Hitbox_Head)
        h.push_back(0); // HITBOX_HEAD

    if (CFG::Aimbot_Hitbox_Body)
    {
        // HITBOX_SPINE (corpo) - correto do TF2
        h.push_back(1); // HITBOX_SPINE0 (pelvis superior/corpo baixo)
        h.push_back(2); // HITBOX_SPINE1 (corpo médio)
        h.push_back(3); // HITBOX_SPINE2 (corpo superior)
        h.push_back(4); // HITBOX_SPINE3 (peito/chest)
    }

    if (CFG::Aimbot_Hitbox_Pelvis)
        h.push_back(1); // HITBOX_PELVIS (overlap com spine0)

    if (CFG::Aimbot_Hitbox_Arms)
    {
        for (int i = 5; i <= 10; i++) // Arms no TF2
            h.push_back(i);
    }

    if (CFG::Aimbot_Hitbox_Legs)
    {
        for (int i = 11; i <= 16; i++) // Legs no TF2
            h.push_back(i);
    }

    return h;
}

float CAimbotHitscan::GetHitboxPriority(int hitbox)
{
    // Baseado Amalgam GetHitboxPriority
    if (hitbox == 0) return 100.0f; // HEAD
    if (hitbox >= 1 && hitbox <= 4) return 50.0f; // SPINE/BODY
    if (hitbox >= 5 && hitbox <= 10) return 20.0f; // ARMS
    if (hitbox >= 11 && hitbox <= 16) return 10.0f; // LEGS
    return 5.0f;
}

bool CAimbotHitscan::IsValidTarget(C_TFPlayer* pEntity, C_TFPlayer* pLocal)
{
    if (pEntity->deadflag()) return false;
    if (CFG::Aimbot_TeamCheck && pEntity->m_iTeamNum() == pLocal->m_iTeamNum()) return false;
    if (pEntity->InCond(TF_COND_PHASE) || pEntity->InCond(TF_COND_HALLOWEEN_GHOST_MODE)) return false;
    if (CFG::Aimbot_Ignore_Invulnerable && pEntity->IsInvulnerable()) return false;
    if (CFG::Aimbot_Ignore_Invisible && pEntity->IsInvisible()) return false;
    if (CFG::Aimbot_Ignore_Taunting && pEntity->InCond(TF_COND_TAUNTING)) return false;
    return true;
}

Vec3 CAimbotHitscan::GetHitboxPos(C_TFPlayer* pEntity, int nHitbox)
{
    const model_t* pModel = pEntity->GetModel();
    if (!pModel) return Vec3(0, 0, 0);

    studiohdr_t* pHdr = I::ModelInfoClient->GetStudiomodel(pModel);
    if (!pHdr) return Vec3(0, 0, 0);

    mstudiohitboxset_t* pSet = pHdr->pHitboxSet(pEntity->m_nHitboxSet());
    if (!pSet || nHitbox >= pSet->numhitboxes) return Vec3(0, 0, 0);

    mstudiobbox_t* pBox = pSet->pHitbox(nHitbox);
    if (!pBox) return Vec3(0, 0, 0);

    matrix3x4_t bones[128];
    if (!pEntity->SetupBones(bones, 128, 0x100, I::GlobalVars->curtime)) return Vec3(0, 0, 0);

    Vec3 vMin, vMax;
    Math::VectorTransform(pBox->bbmin, bones[pBox->bone], vMin);
    Math::VectorTransform(pBox->bbmax, bones[pBox->bone], vMax);
    return (vMin + vMax) * 0.5f;
}