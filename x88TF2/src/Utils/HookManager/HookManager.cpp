#include "HookManager.h"

CHook::CHook(void* pInitFunc)
{
	U::HookManager->AddInit(pInitFunc);
}

void CHookManager::InitializeAllHooks()
{
	MH_Initialize();

	for (const auto& pInit : m_vecHookInits)
		reinterpret_cast<void(__cdecl*)()>(pInit)();

	MH_EnableHook(MH_ALL_HOOKS);
}