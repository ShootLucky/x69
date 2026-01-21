#include "../src/SDK/SDK.h"

#include "../Features/Chams/Chams.h"
#include "../Features/Outlines/Outlines.h"
#include "../Features/Exploits/nospread/nospread.h"
#include "../Features/Exploits/shifting/shifting.h"

MAKE_HOOK(IBaseClientDLL_LevelShutdown, Memory::GetVFunc(I::BaseClientDLL, 7), void, __fastcall,
	void* ecx)
{
	CALL_ORIGINAL(ecx);

	H::Entities->ClearCache();
	H::Entities->ClearModelIndexes();

	F::Materials->CleanUp();
	F::Outlines->CleanUp();
	g_no_spread->Reset();
	Shifting::Reset();

	G::mapVelFixRecords.clear();
}