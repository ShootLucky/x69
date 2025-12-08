// AimbotHitscan.cpp
#include "AimbotHitscan.h"
#include "../src/CFG.h"
#include "../../../SDK/Helpers/Entities/Entities.h"
#include "../../../Utils/Math/Math.h"
#include "../../../SDK/Helpers/AimUtils/AimUtils.h"

C_BaseEntity* CAimbotHitscan::m_pLastTarget = nullptr;

int CAimbotHitscan::GetAimHitbox()
{
	if (CFG::Aimbot_Hitbox_Head) return HITBOX_HEAD;
	else if (CFG::Aimbot_Hitbox_Neck) return HITBOX_NECK;
	else if (CFG::Aimbot_Hitbox_Chest) return HITBOX_CHEST;
	else if (CFG::Aimbot_Hitbox_Pelvis) return HITBOX_PELVIS;
	else return HITBOX_HEAD;
}

bool CAimbotHitscan::GetTarget(C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, HitscanTarget_t& outTarget)
{
	const Vec3 vLocalPos = pLocal->GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	m_vecTargets.clear();

	const int nAimHitbox = GetAimHitbox();

	if (CFG::Aimbot_Aimlock && m_pLastTarget)
	{
		auto pPlayer = m_pLastTarget->As<C_TFPlayer>();
		if (pPlayer && !pPlayer->deadflag() && pPlayer->m_iTeamNum() != pLocal->m_iTeamNum())
		{
			Vec3 vPos = pPlayer->GetHitboxPos(nAimHitbox);
			if (H::AimUtils->TraceEntityBullet(pPlayer, vLocalPos, vPos))
			{
				Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
				float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
				if (flFOVTo <= CFG::Aimbot_FOV)
				{
					outTarget.Entity = pPlayer;
					outTarget.Position = vPos;
					outTarget.AngleTo = vAngleTo;
					outTarget.FOVTo = flFOVTo;
					outTarget.DistTo = vLocalPos.DistTo(vPos);
					outTarget.AimedHitbox = nAimHitbox;
					return true;
				}
			}
		}
		m_pLastTarget = nullptr;
	}

	for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PLAYERS_ENEMIES))
	{
		if (!pEntity) continue;

		const auto pPlayer = pEntity->As<C_TFPlayer>();
		if (!pPlayer || pPlayer == pLocal) continue;
		if (pPlayer->deadflag()) continue;

		Vec3 vPos = pPlayer->GetHitboxPos(nAimHitbox);
		if (!H::AimUtils->TraceEntityBullet(pPlayer, vLocalPos, vPos)) continue;

		Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
		float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
		if (flFOVTo > CFG::Aimbot_FOV) continue;

		float flDistTo = vLocalPos.DistTo(vPos);

		m_vecTargets.emplace_back(HitscanTarget_t{ pPlayer, vPos, vAngleTo, flFOVTo, flDistTo, nAimHitbox });
	}

	if (m_vecTargets.empty()) return false;

	std::sort(m_vecTargets.begin(), m_vecTargets.end(), [](const HitscanTarget_t& a, const HitscanTarget_t& b) {
		return a.FOVTo < b.FOVTo;
		});

	outTarget = m_vecTargets.front();
	if (CFG::Aimbot_Aimlock) m_pLastTarget = outTarget.Entity;

	return true;
}

bool CAimbotHitscan::ShouldAim(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
	static bool bToggled = false;
	static bool bWasPressed = false;

	if (!CFG::Aimbot_Enable) return false;

	int iKey = CFG::Aimbot_Key;
	bool bPressed = false;

	if (iKey == 0)
	{
		bPressed = (pCmd->buttons & IN_ATTACK);
	}
	else
	{
		bPressed = (GetAsyncKeyState(iKey) & 0x8000) != 0;
	}

	switch (CFG::Aimbot_KeyMode)
	{
	case 0: // Hold
		return bPressed;
	case 1: // Toggle
	{
		if (bPressed && !bWasPressed)
			bToggled = !bToggled;
		bWasPressed = bPressed;
		return bToggled;
	}
	case 2: // Always
		return true;
	default: return false;
	}
}

void CAimbotHitscan::Aim(CUserCmd* pCmd, C_TFPlayer* pLocal, const Vec3& vAngles)
{
	Vec3 vCurAngles = I::EngineClient->GetViewAngles();
	Vec3 vDelta = vAngles - vCurAngles;
	Math::ClampAngles(vDelta);

	float fSmooth = CFG::Aimbot_Hitscan_Smoothing;
	if (fSmooth < 1.0f) fSmooth = 1.0f;

	float fFactor = 1.0f / fSmooth;

	vDelta *= fFactor;

	Vec3 vNewAngles = vCurAngles + vDelta;
	Math::ClampAngles(vNewAngles);

	I::EngineClient->SetViewAngles(vNewAngles);
	pCmd->viewangles = vNewAngles;
}

bool CAimbotHitscan::ShouldFire(const CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon, const HitscanTarget_t& target)
{
	if (pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN) return false;

	return true;
}

void CAimbotHitscan::HandleFire(CUserCmd* pCmd, C_TFWeaponBase* pWeapon)
{
	pCmd->buttons |= IN_ATTACK;
}

bool CAimbotHitscan::IsFiring(const CUserCmd* pCmd, C_TFWeaponBase* pWeapon)
{
	if (!pCmd || !pWeapon) return false;

	return (pCmd->buttons & IN_ATTACK) && pWeapon->HasPrimaryAmmoForShot();
}

void CAimbotHitscan::Run(CUserCmd* pCmd, C_TFPlayer* pLocal, C_TFWeaponBase* pWeapon)
{
	if (!ShouldAim(pCmd, pLocal, pWeapon)) return;

	HitscanTarget_t target = {};
	if (!GetTarget(pLocal, pWeapon, target)) return;

	Aim(pCmd, pLocal, target.AngleTo);

	if (ShouldFire(pCmd, pLocal, pWeapon, target))
		HandleFire(pCmd, pWeapon);
}