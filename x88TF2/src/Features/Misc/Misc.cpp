#include "Misc.h"
#include <vector>
#include <windows.h>
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