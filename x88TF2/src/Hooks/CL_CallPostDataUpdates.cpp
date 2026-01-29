#include "../src/Features/SkinChanger/SkinChanger.h" // For g_SkinChanger
#include <cstdint> // For std::uintptr_t
#include <intrin.h> // For _ReturnAddress() in x64

#ifndef _WIN64
// 32-bit (x86) path
MAKE_SIGNATURE(CL_CallPostDataUpdates, "engine.dll", "55 8B EC 8B 45 ? 53 33 DB", 0);

MAKE_HOOK(CL_CallPostDataUpdates, Signatures::CL_CallPostDataUpdates.Get(), void, __cdecl, void* u)
{
    g_SkinChanger.ApplySkins();
    CALL_ORIGINAL(u);
}

#else
// 64-bit (x64) path
MAKE_SIGNATURE(ClientDLL_FrameStageNotify, "engine.dll", "4C 8B DC 56 48 83 EC", 0);
MAKE_SIGNATURE(CL_ProcessPacketEntitites_FSN_Call, "engine.dll", "E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B 01 FF 50 ? 44 8B F0", 5); // Return address of FSN(3) call

MAKE_HOOK(ClientDLL_FrameStageNotify, Signatures::ClientDLL_FrameStageNotify.Get(), void, __fastcall, int stage)
{
    /*static bool done = false;
    if (I::EngineClient->IsInGame() && !done)
    {
        g_Netvars.DumpTables();
        done = true;
    }*/
    if (std::uintptr_t(_ReturnAddress()) == Signatures::CL_ProcessPacketEntitites_FSN_Call.Get())
        g_SkinChanger.ApplySkins();
    CALL_ORIGINAL(stage);
}
#endif