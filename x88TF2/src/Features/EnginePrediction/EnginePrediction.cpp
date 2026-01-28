#include "EnginePrediction.h"

int CEnginePrediction::GetTickbase(CUserCmd* pCmd, C_TFPlayer* pLocal)
{
	static int nTick = 0;
	static CUserCmd* pLastCmd = nullptr;

	if (pCmd)
	{
		if (!pLastCmd || pLastCmd->hasbeenpredicted)
			nTick = pLocal->m_nTickBase();
		else
			nTick++;

		pLastCmd = pCmd;
	}

	return nTick;
}

void CEnginePrediction::Start(CUserCmd* pCmd)
{
	if (!I::MoveHelper || !pCmd)
		return;

	auto pLocal = H::Entities->GetLocal();
	if (!pLocal || !pLocal->IsAlive())
		return;

	// Clear move data
	memset(&m_MoveData, 0, sizeof(CMoveData));

	// Store current flags
	flags = pLocal->m_fFlags();

	// Store old values for restoration
	m_fOldCurrentTime = I::GlobalVars->curtime;
	m_fOldFrameTime = I::GlobalVars->frametime;
	m_nOldTickCount = I::GlobalVars->tickcount;
	m_nOldTickBase = pLocal->m_nTickBase();
	m_bOldIsFirstPrediction = I::Prediction->m_bFirstTimePredicted;
	m_bOldInPrediction = I::Prediction->m_bInPrediction;
	m_vecOldVelocity = pLocal->m_vecVelocity();

	// Handle minigun special case - prevents prediction issues with heavy's minigun
	if (auto pWeapon = H::Entities->GetWeapon())
	{
		if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN)
		{
			// Cast to C_TFMinigun to access m_iWeaponState
			auto pMinigun = reinterpret_cast<C_TFMinigun*>(pWeapon);
			if (pMinigun)
			{
				pMinigun->m_iWeaponState() = 2; // AC_STATE_FIRING
				pWeapon->m_flNextPrimaryAttack() = 0.0f;
				pWeapon->m_flNextSecondaryAttack() = 0.0f;
			}
		}
	}

	// Set host for MoveHelper
	I::MoveHelper->SetHost(pLocal);

	// Set current command
	pLocal->SetCurrentCommand(pCmd);

	// Set random seed for weapon spread
	*SDKUtils::RandomSeed() = MD5_PseudoRandom(pCmd->command_number) & INT_MAX;

	// Calculate current server tick
	const int nServerTicks = GetTickbase(pCmd, pLocal);

	// Update global vars
	I::GlobalVars->curtime = TICKS_TO_TIME(nServerTicks);
	I::GlobalVars->frametime = TICK_INTERVAL;
	I::GlobalVars->tickcount = nServerTicks;

	// Update prediction state
	I::Prediction->m_bFirstTimePredicted = false;
	I::Prediction->m_bInPrediction = true;
	m_bInPrediction = true;

	// Set view angles
	I::Prediction->SetLocalViewAngles(pCmd->viewangles);

	// Set running prediction flag
	G::bRunningPrediction = true;

	// Run prediction through manual process
	if (I::GameMovement && I::Prediction)
	{
		I::GameMovement->StartTrackPredictionErrors(pLocal);

		I::Prediction->SetupMove(pLocal, pCmd, I::MoveHelper, &m_MoveData);
		I::GameMovement->ProcessMovement(pLocal, &m_MoveData);
		I::Prediction->FinishMove(pLocal, pCmd, &m_MoveData);

		I::GameMovement->FinishTrackPredictionErrors(pLocal);
	}

	// Clear running prediction flag
	G::bRunningPrediction = false;

	// Reset MoveHelper host
	I::MoveHelper->SetHost(nullptr);

	// Restore tickbase and velocity (important: don't let them persist)
	pLocal->m_nTickBase() = m_nOldTickBase;
	pLocal->m_vecVelocity() = m_vecOldVelocity;

	// Restore prediction state
	I::Prediction->m_bInPrediction = m_bOldInPrediction;
	I::Prediction->m_bFirstTimePredicted = m_bOldIsFirstPrediction;
}

void CEnginePrediction::End()
{
	auto pLocal = H::Entities->GetLocal();
	if (!pLocal || !pLocal->IsAlive())
		return;

	// Clear current command
	pLocal->SetCurrentCommand(nullptr);

	// Restore global vars
	I::GlobalVars->curtime = m_fOldCurrentTime;
	I::GlobalVars->frametime = m_fOldFrameTime;
	I::GlobalVars->tickcount = m_nOldTickCount;

	// Reset random seed
	*SDKUtils::RandomSeed() = -1;

	// Update prediction state
	m_bInPrediction = false;
}