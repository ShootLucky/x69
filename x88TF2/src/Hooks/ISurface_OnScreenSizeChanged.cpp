#include "../src/SDK/SDK.h"

#include "../Features/Chams/Chams.h"
#include "../Features/Outlines/Outlines.h"

MAKE_HOOK(ISurface_OnScreenSizeChanged, Memory::GetVFunc(I::MatSystemSurface, 111), void, __fastcall,
	ISurface* ecx, int nOldWidth, int OldHeight)
{
	CALL_ORIGINAL(ecx, nOldWidth, OldHeight);

	H::Fonts->Reload();
	H::Draw->UpdateScreenSize();

	F::Materials->CleanUp();
	F::Outlines->CleanUp();
}