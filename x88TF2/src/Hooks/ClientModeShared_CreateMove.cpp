#include "../SDK/SDK.h"
#include "../Features/Misc/Misc.h"
#include "../Features/Aimbot/Aimbot.h"
#include "../Features/EnginePrediction/EnginePrediction.h"
#include "../Features/Exploits/nospread/nospread.h"
#include "../Features/Exploits/shifting/shifting.h"

MAKE_HOOK(ClientModeShared_CreateMove, Memory::GetVFunc(I::ClientModeShared, 21), bool, __fastcall,
	CClientModeShared* ecx, float flInputSampleTime, CUserCmd* pCmd)
{
	G::bSilentAngles = false;
	G::bPSilentAngles = false;
	G::bFiring = false;
	G::bRunCmd = false; // ← ADICIONAR ESTA LINHA
	G::CurrentUserCmd = pCmd;

	if (!pCmd || !pCmd->command_number)
	{
		return CALL_ORIGINAL(ecx, flInputSampleTime, pCmd);
	}

	I::Prediction->Update
	(
		I::ClientState->m_nDeltaTick,
		I::ClientState->m_nDeltaTick > 0,
		I::ClientState->last_command_ack,
		I::ClientState->lastoutgoingcommand + I::ClientState->chokedcommands
	);

	if (g_shifting->should_exit_create_move(pCmd))
	{
		return g_shifting->get_shift_silent_angles() ? false : CALL_ORIGINAL(ecx, flInputSampleTime, pCmd);
	}

	if (Shifting::bRecharging)
	{
		if (pCmd->buttons & IN_JUMP)
		{
			pCmd->buttons &= ~IN_JUMP;
		}

		return CALL_ORIGINAL(ecx, flInputSampleTime, pCmd);
	}

	bool* pSendPacket = reinterpret_cast<bool*>(uintptr_t(_AddressOfReturnAddress()) + 0x128);

	// Cache angles and movement for later restoration
	const Vec3 vOldAngles = pCmd->viewangles;
	const float flOldSide = pCmd->sidemove;
	const float flOldForward = pCmd->forwardmove;

	// Cache entity pointers
	auto pLocal = H::Entities->GetLocal();
	auto pWeapon = H::Entities->GetWeapon();

	// Cache weapon capabilities
	if (pLocal && pWeapon)
	{
		G::bCanPrimaryAttack = pWeapon->CanPrimaryAttack(pLocal);
		G::bCanSecondaryAttack = pWeapon->CanSecondaryAttack(pLocal);
		G::bCanHeadshot = pWeapon->CanHeadShot(pLocal);
	}
	else
	{
		G::bCanPrimaryAttack = false;
		G::bCanSecondaryAttack = false;
		G::bCanHeadshot = false;
	}

	// Track ticks since can fire
	{
		static bool bOldCanFire = G::bCanPrimaryAttack;

		if (G::bCanPrimaryAttack != bOldCanFire)
		{
			G::nTicksSinceCanFire = 0;
			bOldCanFire = G::bCanPrimaryAttack;
		}
		else
		{
			if (G::bCanPrimaryAttack)
				G::nTicksSinceCanFire++;
			else
				G::nTicksSinceCanFire = 0;
		}
	}

	// Early exit if no local player
	if (!pLocal)
		return CALL_ORIGINAL(ecx, flInputSampleTime, pCmd);

	F::Misc->Bunnyhop(pCmd);
	F::Misc->AutoStrafe(pCmd);
	F::Misc->AutoRocketJump(pCmd);

	F::EnginePrediction->Start(pCmd);
	{
		{
			if ((pLocal->m_fFlags() & FL_ONGROUND) && !(F::EnginePrediction->flags & FL_ONGROUND))
			{
				*pSendPacket = false;
			}
		}

		// ← MODIFICAÇÃO: Setar flag para fire_projectile
		G::bRunCmd = true;

		// Rodar aimbot (vai rodar PROJECTILE weapons aqui)
		// Hitscan será rodado no fire_projectile
		F::Aimbot->Run(pCmd);
	}
	F::EnginePrediction->End();

	g_no_spread->AdjustAngles(pCmd);

	// Track target consistency
	{
		static int nOldTargetIndex = G::nTargetIndexEarly;

		if (G::nTargetIndexEarly != nOldTargetIndex)
		{
			G::nTicksTargetSame = 0;
			nOldTargetIndex = G::nTargetIndexEarly;
		}
		else
		{
			G::nTicksTargetSame++;
		}

		if (G::nTargetIndexEarly <= 1)
			G::nTicksTargetSame = 0;
	}

	// pSilent
	{
		static bool bWasSet = false;

		if (G::bPSilentAngles)
		{
			*pSendPacket = false;
			bWasSet = true;
		}
		else
		{
			if (bWasSet)
			{
				*pSendPacket = true;
				pCmd->viewangles = vOldAngles;
				pCmd->sidemove = flOldSide;
				pCmd->forwardmove = flOldForward;
				bWasSet = false;
			}
		}
	}

	// Don't choke too much
	if (I::ClientState->chokedcommands > 22)
	{
		*pSendPacket = true;
	}

	g_shifting->run(pCmd, pSendPacket);

	G::nOldButtons = pCmd->buttons;
	G::vUserCmdAngles = pCmd->viewangles;

	return (G::bSilentAngles || G::bPSilentAngles) ? false : CALL_ORIGINAL(ecx, flInputSampleTime, pCmd);
}