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
        m_iLastShotTick = 0;
        m_iTapfireDelay = 0;
        m_bWaitingForRelease = false;
        return;
    }

    if (!pLocal || !pWeapon || pLocal->deadflag())
        return;

    const int nWeaponID = pWeapon->GetWeaponID();
    bool bSilentMode = (CFG::Aimbot_Hitscan_Mode == 1);

    if (CFG::Aimbot_AutoScope)
    {
        if (nWeaponID == TF_WEAPON_SNIPERRIFLE ||
            nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC ||
            nWeaponID == TF_WEAPON_SNIPERRIFLE_DECAP)
        {
            if (!pLocal->IsZoomed())
            {
                pCmd->buttons |= IN_ATTACK2;
                return;
            }
        }
    }

    if (CFG::Aimbot_WaitForCharge)
    {
        if (nWeaponID == TF_WEAPON_SNIPERRIFLE ||
            nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC ||
            nWeaponID == TF_WEAPON_SNIPERRIFLE_DECAP)
        {
            float flCharge = pLocal->m_flChargeMeter();
            if (flCharge < 100.0f)
                return;
        }
    }

    if (CFG::Aimbot_MinigunTapfire && nWeaponID == TF_WEAPON_MINIGUN)
    {
        if (m_iTapfireDelay > 0)
        {
            m_iTapfireDelay--;
            return;
        }
    }

    if (bSilentMode && m_bWaitingForRelease)
    {
        if (pWeapon->m_flNextPrimaryAttack() > I::GlobalVars->curtime)
            return;

        if (pLocal->m_flNextAttack() > I::GlobalVars->curtime)
            return;

        m_bWaitingForRelease = false;
    }

    Target_t target;
    if (!GetTarget(pLocal, pWeapon, target))
    {
        m_iLastTargetIndex = 0;
        m_bTargetLocked = false;
        return;
    }

    Vec3 vOldAngles = pCmd->viewangles;
    float fOldForward = pCmd->forwardmove;
    float fOldSidemove = pCmd->sidemove;

    if (bSilentMode)
    {
        pCmd->viewangles = target.vAngles;
        Math::ClampAngles(pCmd->viewangles);
        G::bPSilentAngles = true;

        // Corrigir movimento
        float yawDelta = pCmd->viewangles.y - vOldAngles.y;
        while (yawDelta > 180.0f) yawDelta -= 360.0f;
        while (yawDelta < -180.0f) yawDelta += 360.0f;

        float yawRad = DEG2RAD(yawDelta);
        float cosYaw = cosf(yawRad);
        float sinYaw = sinf(yawRad);

        pCmd->forwardmove = (cosYaw * fOldForward) - (sinYaw * fOldSidemove);
        pCmd->sidemove = (sinYaw * fOldForward) + (cosYaw * fOldSidemove);
    }
    else
    {
        // Aimlock com smoothing
        float fSmooth = CFG::Aimbot_Hitscan_Smoothing;

        if (fSmooth <= 1.0f)
        {
            // Smoothing = 1: Snap direto, sem interpolação
            pCmd->viewangles = target.vAngles;
            Math::ClampAngles(pCmd->viewangles);
        }
        else
        {
            // Smoothing > 1: Interpolação suave
            Vec3 vCurrentAngles = pCmd->viewangles;
            Vec3 vDelta = target.vAngles - vCurrentAngles;
            Math::ClampAngles(vDelta);

            // Aplicar smoothing
            vDelta.x /= fSmooth;
            vDelta.y /= fSmooth;
            vDelta.z = 0.0f;

            pCmd->viewangles = vCurrentAngles + vDelta;
            Math::ClampAngles(pCmd->viewangles);
        }
    }

    G::nTargetIndex = target.pEntity->entindex();
    G::flAimbotFOV = target.fFOV;

    if (CFG::Aimbot_AutoShoot && target.bCanShoot)
    {
        if (!G::bCanPrimaryAttack || !pWeapon->HasPrimaryAmmoForShot())
            return;

        if (nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC)
            return;

        // VERIFICAR SE A MIRA ESTÁ REALMENTE NO ALVO
        Vec3 vCurrentViewAngles = pCmd->viewangles;
        Vec3 vAngleDelta = target.vAngles - vCurrentViewAngles;
        Math::ClampAngles(vAngleDelta);
        float fAngleDifference = sqrtf((vAngleDelta.x * vAngleDelta.x) + (vAngleDelta.y * vAngleDelta.y));

        // Threshold baseado no modo
        float fAccuracyThreshold;
        if (bSilentMode)
        {
            // Silent aim: sempre preciso (aplicamos os ângulos corretos)
            fAccuracyThreshold = 0.5f;
        }
        else
        {
            // Aimlock: precisa estar próximo ao alvo
            if (CFG::Aimbot_Hitscan_Smoothing <= 1.0f)
                fAccuracyThreshold = 0.5f; // Snap direto, muito preciso
            else if (CFG::Aimbot_Hitscan_Smoothing <= 5.0f)
                fAccuracyThreshold = 1.5f; // Smooth baixo, preciso
            else if (CFG::Aimbot_Hitscan_Smoothing <= 10.0f)
                fAccuracyThreshold = 3.0f; // Smooth médio
            else
                fAccuracyThreshold = 5.0f; // Smooth alto, tolerante
        }

        // NÃO ATIRAR SE NÃO ESTIVER PRECISO
        if (fAngleDifference > fAccuracyThreshold)
            return;

        int iCurrentTick = I::GlobalVars->tickcount;

        if (nWeaponID == TF_WEAPON_MINIGUN)
        {
            if (CFG::Aimbot_MinigunTapfire)
            {
                const float TAPFIRE_DISTANCE = 1000.0f;

                if (target.fDist > TAPFIRE_DISTANCE)
                {
                    if ((iCurrentTick - m_iLastShotTick) < 2)
                        return;

                    pCmd->buttons |= IN_ATTACK;
                    m_iLastShotTick = iCurrentTick;
                    m_iTapfireDelay = 3;
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
            return;
        }

        if (bSilentMode)
        {
            if (m_bWaitingForRelease)
                return;

            if (pWeapon->m_flNextPrimaryAttack() > I::GlobalVars->curtime)
                return;

            if (pLocal->m_flNextAttack() > I::GlobalVars->curtime)
                return;

            pCmd->buttons |= IN_ATTACK;
            m_iLastShotTick = iCurrentTick;
            m_bWaitingForRelease = true;
        }
        else
        {
            pCmd->buttons |= IN_ATTACK;
        }
    }
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

bool CAimbotHitscan::GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, Target_t& outTarget)
{
    Vec3 vLocalPos = pLocal->GetShootPos();
    Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

    std::vector<int> activeHitboxes = GetActiveHitboxes();
    if (activeHitboxes.empty())
        return false;

    // Encontrar todos os alvos válidos dentro do FOV
    std::vector<Target_t> validTargets;

    for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++)
    {
        if (i == pLocal->entindex())
            continue;

        C_TFPlayer* pEntity = I::ClientEntityList->GetClientEntity(i)->As<C_TFPlayer>();
        if (!pEntity || !IsValidTarget(pEntity, pLocal))
            continue;

        Target_t t;
        if (ScanTargetHitboxes(pLocal, pEntity, vLocalPos, vLocalAngles, activeHitboxes, t))
        {
            // RESPEITAR FOV - só adicionar se dentro do FOV
            if (t.fFOV <= CFG::Aimbot_FOV)
            {
                t.bCanShoot = !CFG::Aimbot_WaitForHeadshot || t.nHitbox == 0;
                validTargets.push_back(t);
            }
        }
    }

    if (validTargets.empty())
        return false;

    // Ordenar targets
    std::sort(validTargets.begin(), validTargets.end(), [](const Target_t& a, const Target_t& b) {
        switch (CFG::Aimbot_Hitscan_Sort)
        {
        case 0: return a.fDist < b.fDist;
        case 1: return a.fFOV < b.fFOV;
        case 2: return a.nHealth < b.nHealth;
        default: return a.fFOV < b.fFOV;
        }
        });

    outTarget = validTargets.front();
    return true;
}

bool CAimbotHitscan::ScanTargetHitboxes(C_TFPlayer* pLocal, C_TFPlayer* pTarget,
    const Vec3& vLocalPos, const Vec3& vLocalAngles,
    const std::vector<int>& hitboxes, Target_t& outTarget)
{
    Vec3 vPredictionOffset(0, 0, 0);
    INetChannelInfo* pNetChan = I::EngineClient->GetNetChannelInfo();
    if (pNetChan)
    {
        float flLatency = pNetChan->GetLatency(FLOW_OUTGOING) + pNetChan->GetLatency(FLOW_INCOMING);
        float flLerp = I::CVar->FindVar("cl_interp")->GetFloat();
        vPredictionOffset = pTarget->m_vecVelocity() * (flLatency + flLerp);
    }

    // MULTIPOINT AUTOMÁTICO
    std::vector<std::pair<int, Vec3>> hitboxPoints;

    for (int hitbox : hitboxes)
    {
        Vec3 vHitboxPos = GetHitboxPosition(pTarget, hitbox);
        if (vHitboxPos.IsZero())
            continue;

        // Ponto central
        hitboxPoints.push_back({ hitbox, vHitboxPos });

        // Multipoint para head e body
        if (hitbox == 0) // Head
        {
            // Top, Left, Right da cabeça
            Vec3 vTop = vHitboxPos; vTop.z += 2.0f;
            Vec3 vLeft = vHitboxPos; vLeft.x -= 3.0f;
            Vec3 vRight = vHitboxPos; vRight.x += 3.0f;

            hitboxPoints.push_back({ hitbox, vTop });
            hitboxPoints.push_back({ hitbox, vLeft });
            hitboxPoints.push_back({ hitbox, vRight });
        }
        else if (hitbox >= 4 && hitbox <= 7) // Body
        {
            // Left, Right do corpo
            Vec3 vLeft = vHitboxPos; vLeft.x -= 5.0f;
            Vec3 vRight = vHitboxPos; vRight.x += 5.0f;

            hitboxPoints.push_back({ hitbox, vLeft });
            hitboxPoints.push_back({ hitbox, vRight });
        }
    }

    // Testar todos os pontos
    float fBestScore = FLT_MAX;
    bool bFoundHitbox = false;

    for (const auto& [hitbox, vPoint] : hitboxPoints)
    {
        Vec3 vPredictedPos = vPoint + vPredictionOffset;
        float fFOV = CalculateFOV(vLocalAngles, vPredictedPos, vLocalPos);

        // Quick FOV check
        if (fFOV > CFG::Aimbot_FOV)
            continue;

        Vec3 vAngles = Math::CalcAngle(vLocalPos, vPredictedPos);
        Vec3 vForward;
        Math::AngleVectors(vAngles, &vForward);
        Vec3 vTraceEnd = vLocalPos + (vForward * 8192.0f);

        int nHitHitbox = -1;
        if (!H::AimUtils->TraceEntityBullet(pTarget, vLocalPos, vTraceEnd, &nHitHitbox))
            continue;

        // Score: priorizar hitbox visível
        float fScore = CalculateHitboxScore(hitbox, fFOV, vLocalPos.DistTo(vPoint));

        if (fScore < fBestScore)
        {
            fBestScore = fScore;
            outTarget.pEntity = pTarget;
            outTarget.vHitboxPos = vPredictedPos;
            outTarget.vAngles = vAngles;
            outTarget.fFOV = fFOV;
            outTarget.fDist = vLocalPos.DistTo(vPoint);
            outTarget.nHealth = pTarget->m_iHealth();
            outTarget.nHitbox = hitbox;
            outTarget.bCanHit = true;
            bFoundHitbox = true;
        }
    }

    return bFoundHitbox;
}

std::vector<int> CAimbotHitscan::GetActiveHitboxes()
{
    std::vector<int> hitboxes;

    // Ordem de prioridade: Head > Body > Pelvis > Arms > Legs
    if (CFG::Aimbot_Hitbox_Head)
        hitboxes.push_back(0);

    if (CFG::Aimbot_Hitbox_Body)
    {
        hitboxes.push_back(7); // Upper chest (mais visível)
        hitboxes.push_back(6); // Chest
        hitboxes.push_back(5); // Thorax
        hitboxes.push_back(4); // Body
    }

    if (CFG::Aimbot_Hitbox_Pelvis)
        hitboxes.push_back(3);

    if (CFG::Aimbot_Hitbox_Arms)
    {
        hitboxes.push_back(14);
        hitboxes.push_back(15);
        hitboxes.push_back(16);
        hitboxes.push_back(17);
        hitboxes.push_back(18);
        hitboxes.push_back(19);
    }

    if (CFG::Aimbot_Hitbox_Legs)
    {
        hitboxes.push_back(8);
        hitboxes.push_back(9);
        hitboxes.push_back(10);
        hitboxes.push_back(11);
        hitboxes.push_back(12);
        hitboxes.push_back(13);
    }

    return hitboxes;
}

float CAimbotHitscan::CalculateHitboxScore(int hitbox, float fov, float distance)
{
    float fPriority = 1.0f;

    // Prioridade baseada em hitbox
    if (hitbox == 0)
        fPriority = 100.0f; // Head - máxima prioridade
    else if (hitbox >= 4 && hitbox <= 7)
        fPriority = 50.0f; // Body - alta prioridade
    else if (hitbox == 3)
        fPriority = 30.0f; // Pelvis
    else if (hitbox >= 14 && hitbox <= 19)
        fPriority = 20.0f; // Arms
    else
        fPriority = 10.0f; // Legs

    // Score: menor = melhor
    // Prioriza: hitbox > FOV > distance
    return (fov * 2.0f) + (distance * 0.001f) - fPriority;
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

Vec3 CAimbotHitscan::GetHitboxPosition(C_TFPlayer* pEntity, int nHitbox)
{
    const model_t* pModel = pEntity->GetModel();
    if (!pModel) return Vec3(0, 0, 0);

    studiohdr_t* pStudioHdr = I::ModelInfoClient->GetStudiomodel(pModel);
    if (!pStudioHdr) return Vec3(0, 0, 0);

    mstudiohitboxset_t* pHitboxSet = pStudioHdr->pHitboxSet(pEntity->m_nHitboxSet());
    if (!pHitboxSet || nHitbox >= pHitboxSet->numhitboxes) return Vec3(0, 0, 0);

    mstudiobbox_t* pHitbox = pHitboxSet->pHitbox(nHitbox);
    if (!pHitbox) return Vec3(0, 0, 0);

    matrix3x4_t boneMatrix[128];
    if (!pEntity->SetupBones(boneMatrix, 128, 0x100, I::GlobalVars->curtime)) return Vec3(0, 0, 0);

    Vec3 vMin, vMax;
    Math::VectorTransform(pHitbox->bbmin, boneMatrix[pHitbox->bone], vMin);
    Math::VectorTransform(pHitbox->bbmax, boneMatrix[pHitbox->bone], vMax);
    return (vMin + vMax) * 0.5f;
}

float CAimbotHitscan::CalculateFOV(const Vec3& vLocalAngles, const Vec3& vTargetPos, const Vec3& vLocalPos)
{
    Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vTargetPos);
    Vec3 vDelta = vLocalAngles - vAngleTo;
    Math::ClampAngles(vDelta);
    return sqrtf((vDelta.x * vDelta.x) + (vDelta.y * vDelta.y));
}