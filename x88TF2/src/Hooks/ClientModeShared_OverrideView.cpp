#include "../src/SDK/SDK.h"
#include "CFG.h"
#include "../Features/ESP/ESP.h"
#include "../src/Features/Misc/Misc.h"
MAKE_HOOK(ClientModeShared_OverrideView, Memory::GetVFunc(I::ClientModeShared, 16), void, __fastcall,
	CClientModeShared* ecx, CViewSetup* pSetup)
{
	if (!pSetup)
	{
		return;
	}
	CALL_ORIGINAL(ecx, pSetup);
	gESP.CustomFOV(pSetup);
	F::Misc->Thirdperson(pSetup);
}