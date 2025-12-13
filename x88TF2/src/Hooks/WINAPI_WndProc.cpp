#include "WINAPI_WndProc.h"

#include "../Features/Menu/Menu.h"

namespace Hooks
{
    struct WINAPI_WndProc
    {
        static WNDPROC Original;
        static HWND hwWindow;

        static LRESULT __stdcall Func(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static void Init();
        static void Release();
    };
}

WNDPROC Hooks::WINAPI_WndProc::Original = nullptr;
HWND Hooks::WINAPI_WndProc::hwWindow = nullptr;

LRESULT __stdcall Hooks::WINAPI_WndProc::Func(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (F::Menu->IsOpen() && H::Input->IsGameFocused())
    {
        if (F::Menu->m_bWantTextInput || F::Menu->m_bInKeybind)
        {
            I::InputSystem->ResetInputState();
            return 1;
        }

        if (uMsg >= WM_MOUSEFIRST && WM_MOUSELAST >= uMsg)
            return 1;
    }

    return CallWindowProc(Original, hWnd, uMsg, wParam, lParam);
}

void Hooks::WINAPI_WndProc::Init()
{
    hwWindow = SDKUtils::GetTeamFortressWindow();
    Original = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Func)));
}

void Hooks::WINAPI_WndProc::Release()
{
    SetWindowLongPtr(hwWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Original));
}