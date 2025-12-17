#include "Misc.h"
#include <vector>
#include <windows.h>
#include "CFG.h"
#include "../src/SDK/TF2/tf_shareddefs.h"
void CMisc::Bunnyhop(CUserCmd* pCmd)
{
    if (!CFG::Misc_AutoJump)
        return;
    if (const auto pLocal = H::Entities->GetLocal())
    {
        if (pLocal->deadflag() || pLocal->m_nWaterLevel() > static_cast<byte>(WL_Feet))
            return;
        static bool bJumpState = false;
        if (pCmd->buttons & IN_JUMP)
        {
            if (!bJumpState && !(pLocal->m_fFlags() & FL_ONGROUND))
                pCmd->buttons &= ~IN_JUMP;
            else if (bJumpState)
                bJumpState = false;
        }
        else if (!bJumpState)
        {
            bJumpState = true;
        }
    }
}
void CMisc::Thirdperson(CViewSetup* pSetup)
{
    if (!pSetup) return;
    if (!CFG::Misc_ThirdPerson_Enable) return;
    auto pLocal = H::Entities->GetLocal();
    if (!pLocal || pLocal->deadflag()) return;
    bool shouldThird = false;
    static bool toggle = false;
    int key = CFG::Misc_ThirdPerson_Key;
    switch (CFG::Misc_ThirdPerson_KeyMode) {
    case 0: // Hold
        shouldThird = (GetAsyncKeyState(key) & 0x8000) != 0;
        break;
    case 1: // Toggle
        if (GetAsyncKeyState(key) & 1) toggle = !toggle;
        shouldThird = toggle;
        break;
    case 2: // Always
        shouldThird = true;
        break;
    default:
        shouldThird = false;
        break;
    }
    const bool bShouldDoTP = shouldThird
        || pLocal->InCond(TF_COND_TAUNTING)
        || pLocal->InCond(TF_COND_HALLOWEEN_KART)
        || pLocal->InCond(TF_COND_HALLOWEEN_THRILLER)
        || pLocal->InCond(TF_COND_HALLOWEEN_GHOST_MODE)
        || G::bStartedFakeTaunt;
    if (bShouldDoTP) {
        I::Input->CAM_ToThirdPerson();
        pLocal->m_nForceTauntCam() = 1;
        pLocal->UpdateVisibility();
    }
    else {
        I::Input->CAM_ToFirstPerson();
        pLocal->m_nForceTauntCam() = 0;
    }
    pLocal->ThirdPersonSwitch();
    if (bShouldDoTP) {
        Vec3 vForward = {}, vRight = {}, vUp = {};
        Math::AngleVectors(pSetup->angles, &vForward, &vRight, &vUp);
        float clampedBackDist = std::max(0.0f, std::min(300.0f, CFG::Misc_ThirdPerson_Fov));
        float clampedSideOffset = std::max(-100.0f, std::min(100.0f, CFG::Misc_ThirdPerson_SideOffset));
        float clampedUpDist = std::max(-100.0f, std::min(100.0f, CFG::Misc_ThirdPerson_Distance));
        const Vec3 vOffset = (vForward * clampedBackDist)
            - (vRight * clampedSideOffset)
            - (vUp * clampedUpDist);
        const Vec3 vDesiredOrigin = pSetup->origin - vOffset;
        Ray_t ray = {};
        ray.Init(pSetup->origin, vDesiredOrigin, { -10.0f, -10.0f, -10.0f }, { 10.0f, 10.0f, 10.0f });
        CTraceFilterWorldCustom traceFilter = {};
        trace_t trace = {};
        I::EngineTrace->TraceRay(ray, MASK_SOLID, &traceFilter, &trace);
        pSetup->origin -= vOffset * trace.fraction;
        if (trace.fraction < 1.0f) {
            pSetup->origin += trace.plane.normal * 1.0f;
        }
    }
}
void CMisc::AutoRocketJump(CUserCmd* pCmd)
{
    if (!CFG::Misc_AutoRocketJump_Enable)
        return;
    if (!(GetAsyncKeyState(CFG::Misc_AutoRocketJump_Key) & 0x8000))
        return;
    if (const auto pLocal = H::Entities->GetLocal())
    {
        if (pLocal->deadflag() || pLocal->m_iClass() != TF_CLASS_SOLDIER)
            return;
        if (pLocal->InCond(TF_COND_TAUNTING) || pLocal->InCond(TF_COND_HALLOWEEN_GHOST_MODE) || pLocal->InCond(TF_COND_HALLOWEEN_BOMB_HEAD) || pLocal->InCond(TF_COND_HALLOWEEN_KART))
            return;
        const auto pWeapon = H::Entities->GetWeapon();
        if (!pWeapon)
            return;
        if (pWeapon->GetWeaponID() != TF_WEAPON_ROCKETLAUNCHER && pWeapon->GetWeaponID() != TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT)
            return;
        if (pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka)
            return;
        if (!pWeapon->HasPrimaryAmmoForShot())
            return;
        if (pLocal->m_fFlags() & FL_DUCKING || !(pLocal->m_fFlags() & FL_ONGROUND))
            return;
        float pitch = Math::RemapValClamped(I::EngineClient->GetViewAngles().x, -89.0f, 0.0f, 89.0f, 50.0f);
        float yaw = Math::NormalizeAngle(Math::VelocityToAngles(pLocal->m_vecVelocity()).y + 180.0f);
        if (!(pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)))
        {
            yaw = Math::NormalizeAngle(I::EngineClient->GetViewAngles().y + 180.0f);
        }
        pCmd->viewangles.x = pitch;
        pCmd->viewangles.y = yaw;
        pCmd->viewangles.z = (pWeapon->m_iItemDefinitionIndex() != Soldier_m_TheOriginal) ? 90.0f : 0.0f;
        pCmd->buttons |= IN_ATTACK | IN_DUCK | IN_JUMP;
    }
}
void CMisc::AutoStrafe(CUserCmd* pCmd)
{
    if (!CFG::Misc_AutoStrafer_Enable)
        return;
    if (const auto pLocal = H::Entities->GetLocal())
    {
        if (pLocal->deadflag() || (pLocal->m_fFlags() & FL_ONGROUND))
            return;
        if (pLocal->m_nWaterLevel() > static_cast<byte>(WL_Feet) || pLocal->GetMoveType() != MOVETYPE_WALK)
            return;
        if (pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT))
        {
            const float flForwardMove = pCmd->forwardmove;
            const float flSideMove = pCmd->sidemove;
            Vec3 vForward = {}, vRight = {};
            Math::AngleVectors(pCmd->viewangles, &vForward, &vRight, nullptr);
            vForward.z = vRight.z = 0.0f;
            vForward.Normalize();
            vRight.Normalize();
            Vec3 vWishDir = {};
            Math::VectorAngles({ (vForward.x * flForwardMove) + (vRight.x * flSideMove), (vForward.y * flForwardMove) + (vRight.y * flSideMove), 0.0f }, vWishDir);
            Vec3 vCurDir = {};
            Math::VectorAngles(pLocal->m_vecVelocity(), vCurDir);
            const float flDirDelta = Math::NormalizeAngle(vWishDir.y - vCurDir.y);
            const float flTurnScale = Math::RemapValClamped(CFG::Misc_AutoStrafer_Intensity, 0.0f, 1.0f, 0.9f, 1.0f);
            const float flRotation = DEG2RAD((flDirDelta > 0.0f ? -90.0f : 90.f) + (flDirDelta * flTurnScale));
            const float flCosRot = cosf(flRotation);
            const float flSinRot = sinf(flRotation);
            pCmd->forwardmove = (flCosRot * flForwardMove) - (flSinRot * flSideMove);
            pCmd->sidemove = (flSinRot * flForwardMove) + (flCosRot * flSideMove);
        }
        else
        {
            pCmd->forwardmove = 450.0f;
            const float flForwardMove = pCmd->forwardmove;
            const float flSideMove = pCmd->sidemove;
            Vec3 vForward = {}, vRight = {};
            Math::AngleVectors(pCmd->viewangles, &vForward, &vRight, nullptr);
            vForward.z = vRight.z = 0.0f;
            vForward.Normalize();
            vRight.Normalize();
            Vec3 vWishDir = {};
            Math::VectorAngles({ (vForward.x * flForwardMove) + (vRight.x * flSideMove), (vForward.y * flForwardMove) + (vRight.y * flSideMove), 0.0f }, vWishDir);
            Vec3 vCurDir = {};
            Math::VectorAngles(pLocal->m_vecVelocity(), vCurDir);
            const float flDirDelta = Math::NormalizeAngle(vWishDir.y - vCurDir.y);
            const float flTurnScale = Math::RemapValClamped(CFG::Misc_AutoStrafer_Intensity, 0.0f, 1.0f, 0.9f, 1.0f);
            const float flRotation = DEG2RAD((flDirDelta > 0.0f ? -90.0f : 90.f) + (flDirDelta * flTurnScale));
            const float flCosRot = cosf(flRotation);
            const float flSinRot = sinf(flRotation);
            pCmd->forwardmove = (flCosRot * flForwardMove) - (flSinRot * flSideMove);
            pCmd->sidemove = (flSinRot * flForwardMove) + (flCosRot * flSideMove);
        }
    }
}
void CMisc::AntiAFK(CUserCmd* pCmd)
{
    if (!CFG::Misc_AntiAFK_Enable)
        return;
    if (const auto pLocal = H::Entities->GetLocal())
    {
        if (pLocal->deadflag() || pLocal->m_nWaterLevel() > static_cast<byte>(WL_Feet))
            return;
        if (pCmd->buttons & (IN_ATTACK | IN_ATTACK2 | IN_JUMP | IN_DUCK | IN_FORWARD | IN_BACK | IN_USE | IN_LEFT | IN_RIGHT | IN_MOVELEFT | IN_MOVERIGHT | IN_RELOAD | IN_SCORE))
            return;
        if (pCmd->forwardmove != 0.0f || pCmd->sidemove != 0.0f || pCmd->upmove != 0.0f)
            return;
        if (pCmd->mousedx != 0 || pCmd->mousedy != 0)
            return;
        static bool direction = false;
        static int tick_counter = 0;
        if (++tick_counter >= 64) // Approximately every second (assuming ~66 ticks/sec in TF2)
        {
            direction = !direction;
            tick_counter = 0;
        }
        pCmd->sidemove = direction ? 1.0f : -1.0f;
    }
}
float CMisc::GetFakeLatency() const
{
    return CFG::Misc_FakeLatency_Enable ? CFG::Misc_FakeLatencyfloat_Enable : 0.0f;
}
void CMisc::RecordIncomingSequence(CNetChannel* pNetChan)
{
    if (!pNetChan || !CFG::Misc_FakeLatency_Enable) return;
    if (pNetChan->m_nInSequenceNr > m_lastincomingsequencenumber)
    {
        m_lastincomingsequencenumber = pNetChan->m_nInSequenceNr;
        IncomingSequence_t seq;
        seq.inreliablestate = pNetChan->m_nInReliableState;
        seq.sequencenr = pNetChan->m_nInSequenceNr;
        seq.curtime = I::GlobalVars->realtime;
        m_Sequences.push_front(seq);
        if (m_Sequences.size() > 2048)
            m_Sequences.pop_back();
    }
}
void CMisc::AdjustPing(CNetChannel* pNetChan)
{
    if (!pNetChan || !CFG::Misc_FakeLatency_Enable) return;
    for (auto it = m_Sequences.begin(); it != m_Sequences.end(); )
    {
        if (I::GlobalVars->realtime - it->curtime >= GetFakeLatency())
        {
            pNetChan->m_nInReliableState = it->inreliablestate;
            pNetChan->m_nInSequenceNr = it->sequencenr;
            it = m_Sequences.erase(it);
        }
        else
        {
            ++it;
        }
    }
}