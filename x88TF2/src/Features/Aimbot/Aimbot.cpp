#include "Aimbot.h"
#include "AimbotHitscan/AimbotHitscan.h"
#include "CFG.h"

// Não declarei mais instância aqui — singleton já é provido por MAKE_SINGLETON_SCOPED
// namespace F { inline CAimbotHitscan AimbotHitscan; }

void CAimbot::RunMain(CUserCmd * pCmd)
{
    G::nTargetIndex = -1;
    G::flAimbotFOV = 0.0f;
    G::nTargetIndexEarly = -1;

    if (!CFG::Aimbot_Enable || I::EngineVGui->IsGameUIVisible() || I::MatSystemSurface->IsCursorVisible() || SDKUtils::BInEndOfMatch())
        return;

    if (Shifting::bRecharging)
        return;

    const auto pLocal = H::Entities->GetLocal();
    const auto pWeapon = H::Entities->GetWeapon();

    if (!pLocal || !pWeapon
        || pLocal->deadflag()
        || pLocal->InCond(TF_COND_TAUNTING) || pLocal->InCond(TF_COND_PHASE)
        || pLocal->InCond(TF_COND_HALLOWEEN_GHOST_MODE)
        || pLocal->InCond(TF_COND_HALLOWEEN_BOMB_HEAD)
        || pLocal->InCond(TF_COND_HALLOWEEN_KART)
        || pLocal->m_bFeignDeathReady() || pLocal->m_flInvisibility() > 0.0f
        || pWeapon->m_iItemDefinitionIndex() == Soldier_m_RocketJumper || pWeapon->m_iItemDefinitionIndex() == Demoman_s_StickyJumper)
        return;

    if (H::AimUtils->GetWeaponType(pWeapon) != EWeaponType::HITSCAN)
        return;

    // Usar acesso ao singleton definido por MAKE_SINGLETON_SCOPED (ponteiro ou objeto conforme macro)
    F::AimbotHitscan->Run(pCmd, pLocal, pWeapon);
}

void CAimbot::Run(CUserCmd* pCmd)
{
    RunMain(pCmd);

    //same-ish code below to see if we are firing manually
    const auto pLocal = H::Entities->GetLocal();
    const auto pWeapon = H::Entities->GetWeapon();

    if (!pLocal || !pWeapon
        || pLocal->deadflag()
        || pLocal->InCond(TF_COND_TAUNTING) || pLocal->InCond(TF_COND_PHASE)
        || pLocal->m_bFeignDeathReady() || pLocal->m_flInvisibility() > 0.0f)
        return;

    const auto nWeaponType = H::AimUtils->GetWeaponType(pWeapon);

    if (nWeaponType != EWeaponType::HITSCAN)
        return;

    if (!G::bFiring)
    {
        G::bFiring = F::AimbotHitscan->IsFiring(pCmd, pWeapon);
    }
}