#include "../src/SDK/SDK.h"

#include "CFG.h"

MAKE_SIGNATURE(CTFPlayer_MaybeDrawRailgunBeam, "client.dll", "48 89 74 24 ? 55 57 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? F3 41 0F 10 58", 0x0);

//Hello I am TF2 developer and today we're going to shitcode

MAKE_HOOK(CTFPlayer_MaybeDrawRailgunBeam, Signatures::CTFPlayer_MaybeDrawRailgunBeam.Get(), void, __fastcall,
	C_TFPlayer* ecx, void* pFilter, C_TFWeaponBase* pWeapon, const Vector& vStartPos, const Vector& vEndPos)
{
	if (CFG::BulletTracer_Type && (CFG::BulletTracer_Type != 1 && CFG::BulletTracer_Type != 2 && CFG::BulletTracer_Type != 3 && CFG::BulletTracer_Type != 4 && CFG::BulletTracer_Type != 5 && CFG::BulletTracer_Type != 6 && CFG::BulletTracer_Type != 7 && CFG::BulletTracer_Type != 8 && CFG::BulletTracer_Type != 9))
	{
		if (const auto pLocal = H::Entities->GetLocal())
		{
			if (ecx == pLocal)
				return;
		}
	}

	CALL_ORIGINAL(ecx, pFilter, pWeapon, vStartPos, vEndPos);
}