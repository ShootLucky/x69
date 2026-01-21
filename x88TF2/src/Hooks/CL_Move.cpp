#include "../src/SDK/SDK.h"
#include "../src/SDK/Helpers/Entities/Entities.h"
#include "../Features/Exploits/shifting/shifting.h"
#include "../Features/Exploits/nospread/nospread.h"
#include "../CFG.h"
#include "../src/Features/Network/network_fix.h"
#include <windows.h>

MAKE_SIGNATURE(CL_Move, "engine.dll", "40 55 53 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 83 3D", 0x0);
MAKE_SIGNATURE(net_time, "engine.dll", "F2 0F 10 0D ? ? ? ? F2 0F 5C 0D", 0x4);

// ✅ HELPER FUNCTION: Verificar se key está ativa COM ESTADO SEPARADO
// IMPORTANTE: Esta função mantém seu próprio estado para recharge
static bool g_recharge_was_pressed = false;
static bool g_recharge_toggled = false;

bool IsRechargeKeyActive(int key, int key_mode)
{
    if (key <= 0)
        return false;

    bool key_down = (GetAsyncKeyState(key) & 0x8000) != 0;

    switch (key_mode)
    {
    case 0: // Always on
        return true;
    case 1: // Hold on
        return key_down;
    case 2: // Toggle
        if (key_down && !g_recharge_was_pressed)
        {
            g_recharge_toggled = !g_recharge_toggled;
        }
        g_recharge_was_pressed = key_down;
        return g_recharge_toggled;
    case 3: // Hold off
        return !key_down;
    default:
        return false;
    }
}

MAKE_HOOK(CL_Move, Signatures::CL_Move.Get(), void, __fastcall,
    float accumulated_extra_samples, bool bFinalTick)
{
    const auto pLocal = H::Entities->GetLocal();

    if (CFG::ping_reducer)
        g_network_fix->FixInputDelay(bFinalTick);

    g_no_spread->AskForPlayerPerf();

    // ✅ RECHARGE LOGIC - USANDO TECLA CUSTOMIZÁVEL
    if (Shifting::nAvailableTicks < MAX_COMMANDS)
    {
        if (!Shifting::bRecharging && !Shifting::bShifting && !Shifting::bShiftingWarp &&
            H::Entities->GetWeapon() && IsRechargeKeyActive(CFG::shifting_recharge_key, CFG::shifting_recharge_key_mode))
        {
            if (!I::MatSystemSurface->IsCursorVisible() && !I::EngineVGui->IsGameUIVisible())
                Shifting::bRecharging = true;
        }

        if (Shifting::bRecharging)
        {
            Shifting::nAvailableTicks++;
            return;
        }
    }
    else
    {
        Shifting::bRecharging = false;
    }

    auto callOriginal = [&](bool bFinal)
        {
            CALL_ORIGINAL(accumulated_extra_samples, bFinal);
        };

    if (Shifting::bRapidFireWantShift)
    {
        Shifting::bRapidFireWantShift = false;
        Shifting::bShifting = true;

        const int nTicks = 22;
        for (int i = 0; i < nTicks && Shifting::nAvailableTicks > 0; i++)
        {
            callOriginal(i == nTicks - 1);
            Shifting::nAvailableTicks--;
        }

        Shifting::bShifting = false;
        return;
    }

    // ✅ WARP KEY FUNCTIONALITY - USANDO TECLA CUSTOMIZÁVEL
    if (pLocal && !pLocal->deadflag() && !Shifting::bRecharging && !Shifting::bShifting && !Shifting::bShiftingWarp)
    {
        // ✅ Verifica usando a tecla configurável do warp
        if (g_shifting->should_warp() && Shifting::nAvailableTicks > 0 &&
            !I::MatSystemSurface->IsCursorVisible() && !I::EngineVGui->IsGameUIVisible())
        {
            Shifting::bShifting = true;
            Shifting::bShiftingWarp = true;

            const int warpTicks = Shifting::nAvailableTicks;

            for (int i = 0; i < warpTicks; i++)
            {
                callOriginal(i == warpTicks - 1);
                Shifting::nAvailableTicks--;
            }

            Shifting::bShifting = false;
            Shifting::bShiftingWarp = false;
            return;
        }
    }

    callOriginal(bFinalTick);

    if (I::EngineClient->IsInGame())
    {
        // Obter o ponteiro net_time corretamente
        static double* s_net_time = nullptr;
        if (!s_net_time)
        {
            auto addr = Signatures::net_time.Get();
            if (addr)
            {
                // Ler o offset RIP-relative
                int offset = *reinterpret_cast<int*>(addr);
                s_net_time = reinterpret_cast<double*>(addr + offset + 4);
            }
        }

        if (s_net_time)
        {
            const double net_time_value = *s_net_time;

            float cl_cmdrate = I::CVar->FindVar("cl_cmdrate")->GetFloat();

            const float cmd_interval = 1.0f / cl_cmdrate;
            const float max_delta = std::min(I::GlobalVars->interval_per_tick, cmd_interval);
            const float delta = std::clamp((float)(net_time_value - I::ClientState->m_flNextCmdTime), 0.0f, max_delta);

            I::ClientState->m_flNextCmdTime = net_time_value + I::GlobalVars->interval_per_tick;

#ifdef OPTIMAL_CMD_RATE
            I::ClientState->m_flNextCmdTime = net_time_value + I::GlobalVars->interval_per_tick;
#endif
        }
    }
    else
    {
        static double* s_net_time = nullptr;
        if (!s_net_time)
        {
            auto addr = Signatures::net_time.Get();
            if (addr)
            {
                int offset = *reinterpret_cast<int*>(addr);
                s_net_time = reinterpret_cast<double*>(addr + offset + 4);
            }
        }

        if (s_net_time)
        {
            I::ClientState->m_flNextCmdTime = *s_net_time + 0.2f;
        }
    }
}