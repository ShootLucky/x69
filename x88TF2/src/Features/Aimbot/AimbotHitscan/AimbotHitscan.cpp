#include "AimbotHitscan.h"
#include "CFG.h"
#include "../../ESP/ESP.h" // add ESP include to notify aim points and shots

// Adapted from Amalgam by rei-2
// https://github.com/rei-2/Amalgam

void CAimbotHitscan::Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    const int nWeaponID = pWeapon->GetWeaponID();

    // Check if aimbot is active
    if (!CFG::Aimbot_Active)
        return;

    // Check keybind
    static bool bToggled = false;
    if (!H::Input->KeybindMethod(CFG::Aimbot_Key, CFG::Aimbot_KeyMode, &bToggled))
        return;

    if (!pLocal || !pWeapon || pLocal->deadflag())
        return;

    // Minigun handling - EXATAMENTE como no Amalgam
    if (nWeaponID == TF_WEAPON_MINIGUN)
    {
        auto pMinigun = pWeapon->As<C_TFMinigun>();
        if (pMinigun)
        {
            int nState = pMinigun->m_iWeaponState();
            // Se não está girando/atirando, força o spin
            if (nState != AC_STATE_FIRING && nState != AC_STATE_SPINNING)
            {
                pCmd->buttons |= IN_ATTACK2;
                return; // IMPORTANTE: Retorna aqui e espera próximo tick
            }
        }
    }

    // Auto scope - EXATAMENTE como no Amalgam
    if (nWeaponID == TF_WEAPON_SNIPERRIFLE || nWeaponID == TF_WEAPON_SNIPERRIFLE_DECAP)
    {
        bool bScoped = pLocal->InCond(TF_COND_ZOOMED);
        if (CFG::Aimbot_AutoScope && !bScoped)
        {
            pCmd->buttons |= IN_ATTACK2;
            return;
        }
    }

    // Classic sniper - mantém attack pressionado
    if (nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC)
    {
        if (CFG::Aimbot_Hitscan_Mode) // Se é silent
            pCmd->buttons |= IN_ATTACK;
    }

    // Get targets
    std::vector<Target_t> vTargets = GetTargets(pLocal, pWeapon);
    if (vTargets.empty())
        return;

    // Process targets
    for (auto& target : vTargets)
    {
        const auto iResult = CanHit(target, pLocal, pWeapon);
        if (!iResult)
            continue;

        // Inform ESP about where aimbot is currently aiming (dynamic aim point)
        gESP.SetAimbotAimPoint(target.pEntity, target.vPos);

        // Se só pode mirar mas não atirar (result == 2)
        if (iResult == 2)
        {
            Aim(pCmd, target.vAngles);
            break;
        }

        // Target válido - aplicar aim
        G::nTargetIndex = target.pEntity->entindex();
        G::flAimbotFOV = target.fFOV;

        // Aplicar ângulos baseado no modo
        Aim(pCmd, target.vAngles);

        // AUTO SHOOT - EXATAMENTE como no Amalgam
        if (ShouldFire(pLocal, pWeapon, pCmd, target))
        {
            switch (nWeaponID)
            {
            case TF_WEAPON_SNIPERRIFLE_CLASSIC:
                // Classic: solta o botão quando carregado
                if (pWeapon->As<C_TFSniperRifle>() && pWeapon->As<C_TFSniperRifle>()->m_flChargedDamage() > 0.0f)
                    pCmd->buttons &= ~IN_ATTACK;
                break;

            default:
                // Todas as outras armas: pressiona attack
                pCmd->buttons |= IN_ATTACK;
                break;
            }

            // Notify ESP of a shot snapshot (captures bone matrix at shot time to visualize)
            gESP.StoreShotSnapshot(target.pEntity);

            // TAPFIRE - EXATAMENTE como no Amalgam
            if (CFG::Aimbot_MinigunTapfire && pWeapon->GetWeaponSpread() != 0.0f
                && target.fDist > 1000.0f)
            {
                float flTimeSinceShot = (pLocal->m_nTickBase() * TICK_INTERVAL) - pWeapon->m_flLastFireTime();
                float flTapTime = (pWeapon->GetBulletsPerShot() > 1) ? 0.25f : 1.25f;

                if (flTimeSinceShot <= flTapTime)
                    pCmd->buttons &= ~IN_ATTACK;
            }
        }

        // Backtrack
        if (target.bBacktrack && CFG::Aimbot_Hitscan_Target_LagRecords)
        {
            pCmd->tick_count = target.nTickCount;
        }

        break; // Só processa o primeiro target válido
    }
}

// Get targets - simplificado do Amalgam
std::vector<CAimbotHitscan::Target_t> CAimbotHitscan::GetTargets(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    std::vector<Target_t> vTargets;
    vTargets.reserve(32);

    Vec3 vLocalPos = pLocal->GetShootPos();
    Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

    // Scan players
    for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++)
    {
        if (i == pLocal->entindex())
            continue;

        auto pEntity = I::ClientEntityList->GetClientEntity(i)->As<C_TFPlayer>();
        if (!pEntity || !IsValidTarget(pEntity, pLocal))
            continue;

        // Scan hitboxes deste player
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

            Target_t target;
            target.pEntity = pEntity;
            target.vPos = vHitboxPos;
            target.vAngles = vAngleTo;
            target.fFOV = flFOV;
            target.fDist = flDist;
            target.nHitbox = nHitbox;
            target.nPriority = GetHitboxPriority(nHitbox);

            vTargets.push_back(target);
        }
    }

    // Sort por FOV (como Amalgam por padrão)
    std::sort(vTargets.begin(), vTargets.end(), [](const Target_t& a, const Target_t& b) {
        return a.fFOV < b.fFOV;
        });

    return vTargets;
}

// CanHit - verificação de visibilidade
int CAimbotHitscan::CanHit(Target_t& target, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
    Vec3 vStart = pLocal->GetShootPos();
    Vec3 vEnd = target.vPos;

    // Visibility check básico
    if (!H::AimUtils->VisPos(pLocal, target.pEntity, vStart, vEnd))
    {
        // Try backtrack
        if (CFG::Aimbot_Hitscan_Target_LagRecords)
        {
            // TODO: implementar backtrack completo
            return 0;
        }
        return 0;
    }

    target.bBacktrack = false;
    return 1; // Pode atirar
}

// ShouldFire - EXATAMENTE como no Amalgam
bool CAimbotHitscan::ShouldFire(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, CUserCmd* pCmd, const Target_t& target)
{
    if (!CFG::Aimbot_AutoShoot)
        return false;

    // Wait for headshot (snipers) — target.pEntity já é C_TFPlayer*, não precisa de IsPlayer()
    if (CFG::Aimbot_WaitForHeadshot)
    {
        switch (pWeapon->GetWeaponID())
        {
        case TF_WEAPON_SNIPERRIFLE:
        case TF_WEAPON_SNIPERRIFLE_DECAP:
            // substitui G::CanHeadshot por verificação direta usando a função do weapon
            if (!pWeapon->CanHeadShot(pLocal) && pLocal->InCond(TF_COND_AIMING))
                return false;
            break;
        case TF_WEAPON_SNIPERRIFLE_CLASSIC:
            if (!pWeapon->CanHeadShot(pLocal))
                return false;
            break;
        }
    }

    // Wait for charge (snipers)
    if (CFG::Aimbot_WaitForCharge)
    {
        switch (pWeapon->GetWeaponID())
        {
        case TF_WEAPON_SNIPERRIFLE:
        case TF_WEAPON_SNIPERRIFLE_DECAP:
        case TF_WEAPON_SNIPERRIFLE_CLASSIC:
        {
            auto pSniper = pWeapon->As<C_TFSniperRifle>();
            if (pSniper)
            {
                if (!pLocal->InCond(TF_COND_AIMING))
                    break;

                float flCharge = pSniper->m_flChargedDamage();
                if (flCharge < 150.0f) // Não está full charge
                {
                    // Checa se pode matar com bodyshot
                    int iHealth = target.pEntity->m_iHealth();
                    int iDamage = (int)std::ceil(std::max(flCharge, 50.0f));

                    if (iHealth > iDamage)
                        return false; // Precisa esperar mais carga
                }
            }
            break;
        }
        }
    }

    return true;
}

// Aim - aplicar ângulos EXATAMENTE como no Amalgam
void CAimbotHitscan::Aim(CUserCmd* pCmd, Vec3 vAngle)
{
    Vec3 vOldAngles = pCmd->viewangles;

    switch (CFG::Aimbot_Hitscan_Mode)
    {
    case 0: // Smooth
    {
        float fSmooth = CFG::Aimbot_Hitscan_Smoothing;
        if (fSmooth > 1.0f)
        {
            Vec3 vDelta = vAngle - vOldAngles;
            Math::ClampAngles(vDelta);
            vAngle = vOldAngles + (vDelta / fSmooth);
        }

        pCmd->viewangles = vAngle;
        I::EngineClient->SetViewAngles(vAngle);
        Math::ClampAngles(pCmd->viewangles);
        break;
    }

    case 1: // Silent
    {
        // Fix movement ANTES de alterar viewangles
        Vec3 vMove(pCmd->forwardmove, pCmd->sidemove, 0.0f);
        float fSpeed = vMove.Length2D();

        if (fSpeed > 0.0f)
        {
            Vec3 vMoveAng;
            Math::VectorAngles(vMove, vMoveAng);

            float fYawDelta = vAngle.y - vOldAngles.y;
            vMoveAng.y -= fYawDelta;

            Vec3 vNewMove;
            Math::AngleVectors(vMoveAng, &vNewMove);
            vNewMove *= fSpeed;

            pCmd->forwardmove = vNewMove.x;
            pCmd->sidemove = vNewMove.y;
        }

        pCmd->viewangles = vAngle;
        Math::ClampAngles(pCmd->viewangles);
        G::bPSilentAngles = true;
        break;
    }

    default:
        pCmd->viewangles = vAngle;
        Math::ClampAngles(pCmd->viewangles);
        break;
    }
}

// Helper functions
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

    if (CFG::Aimbot_Hitbox_Arms)
    {
        // Arms hitboxes
        for (int i = 5; i <= 10; i++)
            hitboxes.push_back(i);
    }

    if (CFG::Aimbot_Hitbox_Legs)
    {
        // Legs hitboxes
        for (int i = 11; i <= 16; i++)
            hitboxes.push_back(i);
    }

    return hitboxes;
}

int CAimbotHitscan::GetHitboxPriority(int nHitbox)
{
    // Higher priority = lower number (será atingido primeiro)
    if (nHitbox == AIMBOT_HITBOX_HEAD)
        return 0;

    if (nHitbox >= AIMBOT_HITBOX_PELVIS && nHitbox <= AIMBOT_HITBOX_SPINE3)
        return 1;

    if (nHitbox >= 5 && nHitbox <= 10) // Arms
        return 2;

    if (nHitbox >= 11 && nHitbox <= 16) // Legs
        return 3;

    return 4;
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

    if (CFG::Aimbot_Ignore_Taunting && pEntity->InCond(TF_COND_TAUNTING))
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