#include "App.h"
#include "../src/SDK/SDK.h"
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>

const char* ChooseRandomSubText()
{
    // 0 .. 9999 (precisão alta para frases raras)
    int r = rand() % 10000;

    // COMUNS
    if (r < 4500) return "god i wish i had x69";
    else if (r < 6000) return "totally legit, trust me";
    else if (r < 7400) return "random crits are balanced";
    else if (r < 8800) return "medusa.solutions best cheats";

    // RARAS
    else if (r < 9300) return "skill issue detected";
    else if (r < 9600) return "vac was never enough";
    else if (r < 9750) return "moneybot = fedoraware";
    else if (r < 9850) return "spy backtrack go brr";
    else if (r < 9900) return "lmaobox paste detected";

    // RARÍSSIMAS
    else if (r < 9950) return "hvh mid on badlands";
    else if (r < 9990) return "moneybot >> lmaobox >> paste";
    else if (r < 9995) return "nullcore users fear this... x64";
    else return "x69 internal // dev build";
}


void CApp::Start()
{
    srand(static_cast<unsigned int>(time(NULL))); // Seed rand
    while (!Memory::FindSignature(
        "client.dll",
        "48 8B 0D ? ? ? ? 48 8B 10 48 8B 19 48 8B C8 FF 92"))
    {
        bUnload = GetAsyncKeyState(VK_F11) & 0x8000;
        if (bUnload)
            return;
        Sleep(500);
    }
    U::Storage->Init("x69");
    U::SignatureManager->InitializeAllSignatures();
    U::InterfaceManager->InitializeAllInterfaces();
    H::Draw->UpdateScreenSize();
    H::Fonts->Reload();
    if (I::EngineClient->IsInGame() && I::EngineClient->IsConnected())
    {
        H::Entities->UpdateModelIndexes();
        player_info_t info;
        if (I::EngineClient->GetPlayerInfo(I::EngineClient->GetLocalPlayer(), &info))
        {
            m_PlayerName = info.name;
        }
        else
        {
            m_PlayerName = "I forgot your name";
        }
    }
    else
    {
        m_PlayerName = "I forgot your name";
    }
    if (m_PlayerName == "I forgot your name")
    {
        g_SubText = ChooseRandomSubText();
        g_SubTextChosen = true;
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
    bBlackScreen = true;
    fadingIn = true;
    fadingOut = false;
    blackAlpha = 0.0f;
    ulBlackStart = GetTickCount();
}
void CApp::Loop()
{
    while (true)
    {
        bool bShouldUnload =
            (GetAsyncKeyState(VK_F11) & 0x8000 && SDKUtils::IsGameWindowInFocus()) || bUnload;
        if (bShouldUnload)
            break;
        static DWORD lastCheck = 0;
        if (GetTickCount() - lastCheck > 5000)
        {
            if (I::EngineClient->IsInGame() && I::EngineClient->IsConnected())
            {
                player_info_t info;
                if (I::EngineClient->GetPlayerInfo(I::EngineClient->GetLocalPlayer(), &info))
                {
                    if (m_PlayerName != info.name)
                    {
                        m_PlayerName = info.name;
                    }
                }
                else
                {
                    m_PlayerName = "Why";
                }
            }
            else
            {
                m_PlayerName = "Why";
            }
            lastCheck = GetTickCount();
        }
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
    I::EngineClient->ClientCmd_Unrestricted(
        "play hl1/fvox/deactivated.wav");
    I::CVar->ConsoleColorPrintf(
        { 255, 70, 70, 255 },
        "ok..... bye :('\n");
}
void CApp::Draw()
{
    if (!bBlackScreen)
        return;
    int w, h;
    I::EngineClient->GetScreenSize(w, h);
    DWORD current = GetTickCount();
    float totalElapsed =
        static_cast<float>(current - ulBlackStart) / 1000.0f;
    const float phase1Duration = 1.0f;
    const float phase2Duration = 1.0f;
    const float phase3Duration = 0.7f;
    const float phase4Duration = 0.5f;
    const float pauseDuration = 2.0f;
    const float circleDuration = 1.5f;
    const float nameDuration = 1.0f;
    const float nameDelay = 0.4f;
    const float fadeDuration = 1.0f;
    const float holdDuration = 2.0f; // Novo: mais tempo segurando o quadro final
    const float textEnd =
        phase1Duration +
        phase2Duration +
        phase3Duration +
        phase4Duration;
    const float pauseEnd = textEnd + pauseDuration;
    const float circleStart = pauseEnd;
    const float circleEnd = circleStart + circleDuration;
    const float nameStart = circleStart + nameDelay;
    const float nameEnd = nameStart + nameDuration;
    const float fadeStart = nameEnd + holdDuration;
    const float totalEnd = fadeStart + fadeDuration;
    if (totalElapsed >= totalEnd)
    {
        bBlackScreen = false;
        I::EngineClient->ClientCmd_Unrestricted(
            "play hl1/fvox/activated.wav");
        return;
    }
    // Check for ESC key to skip
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
    {
        bBlackScreen = false;
        I::EngineClient->ClientCmd_Unrestricted(
            "play hl1/fvox/activated.wav");
        return;
    }
    int cx = w / 2;
    int cy = h / 2;
    DWORD splashFont =
        I::MatSystemSurface->CreateFont();
    I::MatSystemSurface->SetFontGlyphSet(
        splashFont,
        "Roboto",
        80,
        600,
        0,
        0,
        FONTFLAG_ANTIALIAS
    );
    // ================= WIPE =================
    if (totalElapsed < phase1Duration)
    {
        float wipeWidth = static_cast<float>(w) * (totalElapsed / phase1Duration);
        const float slant = 150.0f;
        float topRightX = wipeWidth - slant;
        topRightX = std::max(0.0f, topRightX);
        float bottomRightX = wipeWidth + slant;
        bottomRightX = std::min(static_cast<float>(w), bottomRightX);
        I::MatSystemSurface->DrawSetColor(45, 46, 45, 255);
        I::MatSystemSurface->DrawSetTexture(0);
        Vertex_t v[4];
        v[0].m_Position = { 0.0f, 0.0f };
        v[1].m_Position = { topRightX, 0.0f };
        v[2].m_Position = { bottomRightX, static_cast<float>(h) };
        v[3].m_Position = { 0.0f, static_cast<float>(h) };
        I::MatSystemSurface->DrawTexturedPolygon(4, v);
        return;
    }
    // ================= BACKGROUND =================
    int bgAlpha = 255;
    Color_t bgColor = Color_t(45, 46, 45, bgAlpha); // Default dark gray
    if (totalElapsed >= circleEnd)
    {
        bgColor = Color_t(0, 150, 255, bgAlpha);
    }
    if (totalElapsed >= fadeStart)
    {
        float p = std::min(1.0f, (totalElapsed - fadeStart) / fadeDuration);
        bgAlpha = static_cast<int>(255 * (1.0f - p));
        bgColor.a = bgAlpha;
    }
    H::Draw->Rect(
        0, 0, w, h,
        bgColor
    );
    // ================= TEXT (ALINHAMENTO ORIGINAL) =================
    int letsWidth, letsHeight;
    int getWidth, getHeight;
    int x69Width, x69Height;
    I::MatSystemSurface->GetTextSize(
        splashFont, L"Let's", letsWidth, letsHeight);
    I::MatSystemSurface->GetTextSize(
        splashFont, L"Get", getWidth, getHeight);
    I::MatSystemSurface->GetTextSize(
        splashFont, L"X69", x69Width, x69Height);
    const int spacing = 0;
    int totalHeight =
        letsHeight + getHeight + x69Height;
    int letsY =
        cy - totalHeight / 2;
    int getY =
        letsY + letsHeight + spacing;
    int x69Y =
        getY + getHeight + spacing;
    float letsCharWidth =
        letsWidth / 5.0f;
    int tPosInLets =
        static_cast<int>(2 * letsCharWidth);
    int letsX =
        cx - letsWidth / 2;
    int getX =
        letsX + tPosInLets;
    float getCharWidth =
        getWidth / 3.0f;
    int tPosInGet =
        static_cast<int>(2 * getCharWidth);
    int x69X =
        getX + tPosInGet;
    I::MatSystemSurface->DrawSetTextFont(splashFont);
    // LET'S
    if (totalElapsed >= phase1Duration && totalElapsed < circleStart)
    {
        float p =
            std::min(1.0f,
                (totalElapsed - phase1Duration) / phase2Duration);
        int drawX =
            static_cast<int>(
                -letsWidth + (letsX + letsWidth) * p);
        I::MatSystemSurface->DrawSetTextColor(255, 254, 246, 255);
        I::MatSystemSurface->DrawSetTextPos(drawX, letsY);
        I::MatSystemSurface->DrawPrintText(L"Let's", 5);
    }
    // GET
    if (totalElapsed >= phase1Duration + phase2Duration && totalElapsed < circleStart)
    {
        float p =
            std::min(1.0f,
                (totalElapsed - phase1Duration - phase2Duration)
                / phase3Duration);
        int drawY =
            static_cast<int>(h + (getY - h) * p);
        I::MatSystemSurface->DrawSetTextColor(255, 254, 246, 255);
        I::MatSystemSurface->DrawSetTextPos(getX, drawY);
        I::MatSystemSurface->DrawPrintText(L"Get", 3);
    }
    // X69 (AZUL, MESMO ALINHAMENTO)
    if (totalElapsed >=
        phase1Duration + phase2Duration + phase3Duration && totalElapsed < circleStart)
    {
        float p =
            std::min(1.0f,
                (totalElapsed -
                    phase1Duration -
                    phase2Duration -
                    phase3Duration)
                / phase4Duration);
        int drawX =
            static_cast<int>(w + (x69X - w) * p);
        I::MatSystemSurface->DrawSetTextColor(0, 150, 255, 255);
        I::MatSystemSurface->DrawSetTextPos(drawX, x69Y);
        I::MatSystemSurface->DrawPrintText(L"X69", 3);
    }
    // ================= CIRCLE =================
    if (totalElapsed >= circleStart &&
        totalElapsed < circleEnd)
    {
        float p =
            (totalElapsed - circleStart) / circleDuration;
        int maxRadius =
            static_cast<int>(
                sqrtf(static_cast<float>((w / 2) * (w / 2) + (h / 2) * (h / 2))));
        int radius =
            static_cast<int>(p * maxRadius);
        I::MatSystemSurface->DrawSetColor(0, 150, 255, 255);
        I::MatSystemSurface->DrawSetTexture(0);
        const int seg = 64;
        Vertex_t v[seg + 1];
        for (int i = 0; i <= seg; ++i)
        {
            float a =
                (static_cast<float>(i) / seg) * 6.2831853f;
            v[i].m_Position.x = cx + cosf(a) * radius;
            v[i].m_Position.y = cy + sinf(a) * radius;
        }
        I::MatSystemSurface->DrawTexturedPolygon(seg + 1, v);
    }
    // ================= NAME =================
    if (totalElapsed >= nameStart)
    {
        std::wstring name(
            m_PlayerName.begin(),
            m_PlayerName.end());
        float p;
        if (m_PlayerName == "Why")
        {
            p = std::min(1.0f, (totalElapsed - nameStart) / circleDuration);
        }
        else
        {
            p = std::min(1.0f, (totalElapsed - nameStart) / nameDuration);
        }
        int tw, th;
        I::MatSystemSurface->GetTextSize(
            splashFont, name.c_str(), tw, th);
        int nameAlpha = static_cast<int>(255 * p);
        if (totalElapsed >= fadeStart)
        {
            float fade_p = std::min(1.0f, (totalElapsed - fadeStart) / fadeDuration);
            nameAlpha = static_cast<int>(nameAlpha * (1 - fade_p));
        }
        I::MatSystemSurface->DrawSetTextColor(
            255, 254, 246,
            nameAlpha);
        I::MatSystemSurface->DrawSetTextPos(
            cx - tw / 2,
            cy - th / 2);
        I::MatSystemSurface->DrawPrintText(
            name.c_str(),
            name.length());
        // ================= SUBTEXT (below "I forgot your name") =================
        if (g_SubTextChosen && g_SubText)
        {
            DWORD subFont = I::MatSystemSurface->CreateFont();
            I::MatSystemSurface->SetFontGlyphSet(
                subFont,
                "Roboto",
                30,
                500,
                0,
                0,
                FONTFLAG_ANTIALIAS
            );
            std::wstring subText(g_SubText, g_SubText + strlen(g_SubText));
            int sw, sh;
            I::MatSystemSurface->GetTextSize(
                subFont, subText.c_str(), sw, sh);
            int subX = cx - sw / 2;
            int subY = (cy - th / 2) + th + 10; // Abaixo do nome principal
            I::MatSystemSurface->DrawSetTextFont(subFont);
            I::MatSystemSurface->DrawSetTextColor(45, 46, 45,nameAlpha); // Mesmo alpha do nome
            I::MatSystemSurface->DrawSetTextPos(subX, subY);
            I::MatSystemSurface->DrawPrintText(
                subText.c_str(),
                subText.length());
        }
    }
    // ================= SKIP MESSAGE =================
    DWORD skipFont = I::MatSystemSurface->CreateFont();
    I::MatSystemSurface->SetFontGlyphSet(
        skipFont,
        "Tahoma",
        20,
        400,
        0,
        0,
        FONTFLAG_ANTIALIAS
    );
    std::wstring skipText = L"press ESCAPE to skip";
    int skipWidth, skipHeight;
    I::MatSystemSurface->GetTextSize(skipFont, skipText.c_str(), skipWidth, skipHeight);
    int skipX = cx - skipWidth / 2;
    int skipY = h - skipHeight - 20; // 20 pixels from bottom
    float blinkAlpha = 128 + 127 * sinf(totalElapsed * 3.0f); // Blinking speed
    I::MatSystemSurface->DrawSetTextFont(skipFont);
    I::MatSystemSurface->DrawSetTextColor(255, 255, 255, static_cast<int>(blinkAlpha));
    I::MatSystemSurface->DrawSetTextPos(skipX, skipY);
    I::MatSystemSurface->DrawPrintText(skipText.c_str(), skipText.length());
}