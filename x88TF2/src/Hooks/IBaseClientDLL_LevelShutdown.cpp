#include "../src/SDK/SDK.h"

#include "../Features/Chams/Chams.h"
#include "../Features/Outlines/Outlines.h"
#include "../Features/SeedPred/SeedPred.h"

MAKE_HOOK(IBaseClientDLL_LevelShutdown, Memory::GetVFunc(I::BaseClientDLL, 7), void, __fastcall,
	void* ecx)
{
	CALL_ORIGINAL(ecx);

	H::Entities->ClearCache();
	H::Entities->ClearModelIndexes();

	F::Materials->CleanUp();
	F::Outlines->CleanUp();
	F::SeedPred->Reset();

	G::mapVelFixRecords.clear();

	Shifting::Reset();
}