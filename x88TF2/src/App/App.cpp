#include "App.h"
#include "../src/SDK/SDK.h" // garante que H::Draw e demais singletons estejam declarados

void CApp::Start()
{
    while (!Memory::FindSignature("client.dll", "48 8B 0D ? ? ? ? 48 8B 10 48 8B 19 48 8B C8 FF 92"))
    {
        bUnload = GetAsyncKeyState(VK_F11) & 0x8000;
        if (bUnload)
            return;

        Sleep(500);
    }

    U::Storage->Init("x69");
    U::SignatureManager->InitializeAllSignatures();
    U::InterfaceManager->InitializeAllInterfaces();

    H::Draw->UpdateScreenSize(); // substituído gDraw() por H::Draw
    H::Fonts->Reload();


    if (I::EngineClient->IsInGame() && I::EngineClient->IsConnected())
    {
        H::Entities->UpdateModelIndexes();
    }

    U::HookManager->InitializeAllHooks();

    Config::Load(U::Storage->GetConfigFolder() / "default.json");

    I::EngineClient->ClientCmd_Unrestricted("toggleconsole; clear");
    Sleep(25);
    I::CVar->ConsoleColorPrintf(
        Color_t(0, 150, 255, 255),
        "[x69] This project was made purely for fun and learning.\n"
        "[x69] Not meant for unfair advantage in public matches.\n"
        "[x69] Use responsibly and only for harmless messing around.\n"
        "[x69] Creator: ShootLucky | Contributor: Star.k\n"
    );


    Beep(1000, 250);
    Sleep(100);
    Beep(1000, 250);
}

void CApp::Loop()
{
    while (true)
    {
        bool bShouldUnload = GetAsyncKeyState(VK_F11) & 0x8000 && SDKUtils::IsGameWindowInFocus() || bUnload;
        if (bShouldUnload)
            break;

        Sleep(50);
    }
}

void CApp::Shutdown()
{
    if (!bUnload)
    {
        U::HookManager->FreeAllHooks();

        Sleep(250);
    }
    I::EngineClient->ClientCmd_Unrestricted("clear");
    Sleep(25);
    I::CVar->ConsoleColorPrintf({ 255, 70, 70, 255 }, "ok..... bye :('\n");

    Beep(500, 250);
    Sleep(100);
    Beep(500, 250);
}