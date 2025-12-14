#include "../SDK/SDK.h"
#include "../Features/Menu/Menu.h"
#include "../App/App.h" // ajuste o caminho se necessário
#include "../Features/ESP/ESP.h"
#include "../src/Features/Aimbot/AimbotHitscan/AimbotHitscan.h" // Added include for AimbotHitscan
#include "../Features/Radio/Radio.h" // Added include for Radio

MAKE_HOOK(IEngineVGuiInternal_Paint, Memory::GetVFunc(I::EngineVGui, 14), void, __fastcall,
	void* ecx, int mode)
{
	CALL_ORIGINAL(ecx, mode);

	if (mode & PAINT_UIPANELS)
	{
		H::Draw->UpdateW2SMatrix();
		I::MatSystemSurface->StartDrawing();
		{
			gESP.Run();
			F::Radio->Run();
			menu::render();

			App->Draw(); // ✅ CORRETO
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