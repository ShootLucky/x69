#include "App.h"
#include "../src/SDK/SDK.h"

void CApp::Start()
{
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
        H::Entities->UpdateModelIndexes();
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

const char* ChooseRandomSubText()
{
    // 0 .. 9999  (precisão alta para frases raras)
    int r = rand() % 10000;

    // COMUNS
    if (r < 4500)      return "god i wish i had X69";                // 45%
    else if (r < 6000) return "Try Minecraft";                       // 15%
    else if (r < 7400) return "Don't try Valorant";                  // 14%
    else if (r < 8800) return "medusa.solutions best cheats";        // 14%

    // RARAS
    else if (r < 9500) return "skill issue detected";                // 7%
    else if (r < 9800) return "VAC was never enough";                // 3%

    // RARÍSSIMAS
    else if (r < 9950) return "you weren't supposed to see this";    // 1.5%
    else if (r < 9990) return "Fuck ic0z, drevs, and those pedophiles."; // 0.4%
    else               return "X69 INTERNAL // DEV BUILD";           // 0.1%
}


void CApp::Draw()
{
    if (!bBlackScreen)
        return;

    int w, h;
    I::EngineClient->GetScreenSize(w, h);

    // Sorteia o texto UMA vez
    if (!g_SubTextChosen)
    {
        srand(GetTickCount());
        g_SubText = ChooseRandomSubText();
        g_SubTextChosen = true;
    }

    DWORD current = GetTickCount();
    float totalElapsed = static_cast<float>(current - ulBlackStart) / 1000.0f;

    const float fadeTime = 1.0f;
    const float traceTime = 2.5f;
    const float holdTimeSec = 3.0f;
    const float totalDuration = fadeTime + holdTimeSec + fadeTime;

    if (totalElapsed < fadeTime)
        blackAlpha = (totalElapsed / fadeTime) * 255.0f;
    else if (totalElapsed < fadeTime + holdTimeSec)
        blackAlpha = 255.0f;
    else if (totalElapsed < totalDuration)
    {
        float fadeOutElapsed = totalElapsed - (fadeTime + holdTimeSec);
        blackAlpha = 255.0f - (fadeOutElapsed / fadeTime) * 255.0f;
    }
    else
    {
        bBlackScreen = false;
        g_SubTextChosen = false;
        I::EngineClient->ClientCmd_Unrestricted("play hl1/fvox/activated.wav");
        return;
    }

    int alpha = static_cast<int>(blackAlpha);
    H::Draw->Rect(0, 0, w, h, Color_t(5, 2, 3, alpha));

    int cx = w / 2;
    int cy = h / 2;

    int rectW = w / 3;
    int rectH = h / 6;

    int left = cx - rectW / 2;
    int right = cx + rectW / 2;
    int top = cy - rectH / 2;
    int bottom = cy + rectH / 2;

    const int r = 20;
    float straightW = static_cast<float>(rectW - 2 * r);
    float straightH = static_cast<float>(rectH - 2 * r);
    float arcLen = (3.14159f / 2.0f) * r;

    float segmentCumul[8] = {
        straightW,
        straightW + arcLen,
        straightW + arcLen + straightH,
        straightW + arcLen + straightH + arcLen,
        straightW + arcLen + straightH + arcLen + straightW,
        straightW + arcLen + straightH + arcLen + straightW + arcLen,
        straightW + arcLen + straightH + arcLen + straightW + arcLen + straightH,
        straightW + arcLen + straightH + arcLen + straightW + arcLen + straightH + arcLen
    };

    float perimeter = segmentCumul[7];
    float progress = fmod(totalElapsed / traceTime, 1.0f);

    Color_t lineColor(255, 254, 246, alpha);

    auto getPosition = [&](float dist) -> std::pair<float, float>
        {
            dist = fmod(dist, perimeter);
            int segment = 0;
            for (; segment < 8; ++segment)
                if (dist < segmentCumul[segment]) break;

            float prev = (segment == 0 ? 0.f : segmentCumul[segment - 1]);
            float local = dist - prev;

            float px = 0.f, py = 0.f;

            switch (segment)
            {
            case 0: px = left + r + local; py = top; break;
            case 1:
            {
                float a = (270.f + (local / arcLen) * 90.f) * (3.14159f / 180.f);
                px = right - r + cosf(a) * r;
                py = top + r + sinf(a) * r;
                break;
            }
            case 2: px = right; py = top + r + local; break;
            case 3:
            {
                float a = (local / arcLen) * 90.f * (3.14159f / 180.f);
                px = right - r + cosf(a) * r;
                py = bottom - r + sinf(a) * r;
                break;
            }
            case 4: px = right - r - local; py = bottom; break;
            case 5:
            {
                float a = (90.f + (local / arcLen) * 90.f) * (3.14159f / 180.f);
                px = left + r + cosf(a) * r;
                py = bottom - r + sinf(a) * r;
                break;
            }
            case 6: px = left; py = bottom - r - local; break;
            case 7:
            {
                float a = (180.f + (local / arcLen) * 90.f) * (3.14159f / 180.f);
                px = left + r + cosf(a) * r;
                py = top + r + sinf(a) * r;
                break;
            }
            }
            return { px, py };
        };

    auto drawTrace = [&](float offset)
        {
            float dist = fmod((progress + offset) * perimeter + perimeter, perimeter);

            const int trail_segments = 12;
            const float trail_fade = 0.82f;
            const float seg = 28.f;
            const float step = 0.75f;

            for (int i = 0; i < trail_segments; ++i)
            {
                float td = fmod(dist - i * (seg / 1.5f) + perimeter, perimeter);
                Color_t c = lineColor;
                c.a = (int)(alpha * pow(trail_fade, i));

                for (float d = 0; d < seg; d += step)
                {
                    auto p1 = getPosition(td + d);
                    auto p2 = getPosition(td + d + step);
                    H::Draw->Line((int)p1.first, (int)p1.second, (int)p2.first, (int)p2.second, c);
                }
            }
        };

    drawTrace(0.f);
    drawTrace(0.5f);

    const CFont& font = H::Fonts->Get(EFonts::PIXEL);

    const char* mainText = "X69";
    int visible = (int)(strlen(mainText) * std::min(1.f, totalElapsed / 1.5f));

    char buf[16] = {};
    strncpy_s(buf, mainText, visible);

    H::Draw->String(font, cx, cy - 20, Color_t(255, 254, 246, alpha),
        POS_CENTERX | POS_CENTERY, buf);

    H::Draw->Line(cx - 100, cy, cx + 100, cy, Color_t(255, 254, 246, alpha));

    // 🔽 TEXTO SORTEADO (RARO / RARÍSSIMO)
    H::Draw->String(
        font,
        cx,
        cy + 20,
        Color_t(255, 254, 246, alpha),
        POS_CENTERX | POS_CENTERY,
        g_SubText
    );
}
