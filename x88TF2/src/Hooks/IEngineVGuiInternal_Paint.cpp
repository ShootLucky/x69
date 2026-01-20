#include "../SDK/SDK.h"
#include "../Features/Menu/Menu.h"
#include "../App/App.h" // ajuste o caminho se necessário
#include "../Features/ESP/ESP.h"
// Added include for AimbotHitscan
#include "../Features/SeedPred/SeedPred.h"
#include "../src/Features/Menu/notification_system/notifs.h"
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
		//	menu::render();
			F::SeedPred->Paint();
			App->Draw(); // ✅ CORRETO
		}
		I::MatSystemSurface->FinishDrawing();
	}
}