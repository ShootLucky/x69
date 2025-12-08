#include "../SDK/SDK.h"
#include "../Features/Menu/Menu.h"
#include "../Features/ESP/ESP.h"

MAKE_HOOK(IEngineVGuiInternal_Paint, Memory::GetVFunc(I::EngineVGui, 14), void, __fastcall,
	void* ecx, int mode)
{
	CALL_ORIGINAL(ecx, mode);

	if (mode & PAINT_UIPANELS)
	{
		// usa singleton do Draw fornecido pelo projeto
		H::Draw->UpdateW2SMatrix();

		I::MatSystemSurface->StartDrawing();
		{
			// ESP global definido em ESP.h/ESP.cpp
			gESP.Run();

			// desenha o menu (ou outra UI)
			menu::render();
		}
		I::MatSystemSurface->FinishDrawing();
	}
}

MAKE_HOOK(ISurface_OnScreenSizeChanged, Memory::GetVFunc(I::MatSystemSurface, 111u), void, __fastcall, int nOldWidth, int nOldHeight)
{
	CALL_ORIGINAL(nOldWidth, nOldHeight);
	H::Fonts->Reload();
	H::Draw->UpdateScreenSize();
}