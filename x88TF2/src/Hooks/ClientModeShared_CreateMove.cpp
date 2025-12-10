#include "../SDK/SDK.h"
#include "../Features/Misc/Misc.h"
#include "../Features/Aimbot/Aimbot.h"
#include "../Features/EnginePrediction/EnginePrediction.h"

MAKE_HOOK(ClientModeShared_CreateMove, Memory::GetVFunc(I::ClientModeShared, 21), bool, __fastcall,
	CClientModeShared* ecx, float flInputSampleTime, CUserCmd* pCmd)
{
	G::bSilentAngles = false;
	G::bPSilentAngles = false;
	G::bFiring = false;

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

	bool* pSendPacket = reinterpret_cast<bool*>(uintptr_t(_AddressOfReturnAddress()) + 0x128);

	Vec3 vOldAngles = pCmd->viewangles;
	float flOldSide = pCmd->sidemove;
	float flOldForward = pCmd->forwardmove;

	auto pLocal = H::Entities->GetLocal();
	auto pWeapon = H::Entities->GetWeapon();

	if (pLocal)
	{
		if (pWeapon)
		{
			//TODO?: do we really need to cache these?
			G::bCanPrimaryAttack = pWeapon->CanPrimaryAttack(pLocal);
			G::bCanSecondaryAttack = pWeapon->CanSecondaryAttack(pLocal);
			G::bCanHeadshot = pWeapon->CanHeadShot(pLocal);
		}
	}

	//nTicksSinceCanFire
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

	F::Misc->Bunnyhop(pCmd);

	F::EnginePrediction->Start(pCmd);
	{
		F::Aimbot->Run(pCmd);
	}
	F::EnginePrediction->End();

	//nTicksTargetSame
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

	//pSilent
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

	G::nOldButtons = pCmd->buttons;
	G::vUserCmdAngles = pCmd->viewangles;

	return (G::bSilentAngles || G::bPSilentAngles) ? false : CALL_ORIGINAL(ecx, flInputSampleTime, pCmd);
}