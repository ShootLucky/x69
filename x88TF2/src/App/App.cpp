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

void CApp::Draw()
{
    if (!bBlackScreen)
        return;
    int w, h;
    I::EngineClient->GetScreenSize(w, h);
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
        I::EngineClient->ClientCmd_Unrestricted("play hl1/fvox/activated.wav");
        return;
    }
    int alpha = static_cast<int>(blackAlpha);
    H::Draw->Rect(0, 0, w, h, Color_t(30, 30, 30, alpha));
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
    float arcLen = (3.14159f / 2.0f) * static_cast<float>(r);
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
            float prevCumul = (segment == 0 ? 0.0f : segmentCumul[segment - 1]);
            float local = dist - prevCumul;
            float px = 0.0f, py = 0.0f;
            switch (segment)
            {
            case 0: px = static_cast<float>(left + r) + local; py = static_cast<float>(top); break;
            case 1: {
                float centerX = static_cast<float>(right - r);
                float centerY = static_cast<float>(top + r);
                float angleDeg = 270.0f + (local / arcLen) * 90.0f;
                float angleRad = angleDeg * (3.14159f / 180.0f);
                px = centerX + static_cast<float>(r) * cosf(angleRad);
                py = centerY + static_cast<float>(r) * sinf(angleRad);
                break;
            }
            case 2: px = static_cast<float>(right); py = static_cast<float>(top + r) + local; break;
            case 3: {
                float centerX = static_cast<float>(right - r);
                float centerY = static_cast<float>(bottom - r);
                float angleDeg = 0.0f + (local / arcLen) * 90.0f;
                float angleRad = angleDeg * (3.14159f / 180.0f);
                px = centerX + static_cast<float>(r) * cosf(angleRad);
                py = centerY + static_cast<float>(r) * sinf(angleRad);
                break;
            }
            case 4: px = static_cast<float>(right - r) - local; py = static_cast<float>(bottom); break;
            case 5: {
                float centerX = static_cast<float>(left + r);
                float centerY = static_cast<float>(bottom - r);
                float angleDeg = 90.0f + (local / arcLen) * 90.0f;
                float angleRad = angleDeg * (3.14159f / 180.0f);
                px = centerX + static_cast<float>(r) * cosf(angleRad);
                py = centerY + static_cast<float>(r) * sinf(angleRad);
                break;
            }
            case 6: px = static_cast<float>(left); py = static_cast<float>(bottom - r) - local; break;
            case 7: {
                float centerX = static_cast<float>(left + r);
                float centerY = static_cast<float>(top + r);
                float angleDeg = 180.0f + (local / arcLen) * 90.0f;
                float angleRad = angleDeg * (3.14159f / 180.0f);
                px = centerX + static_cast<float>(r) * cosf(angleRad);
                py = centerY + static_cast<float>(r) * sinf(angleRad);
                break;
            }
            }
            return { px, py };
        };
    auto drawTrace = [&](float offset)
        {
            float dist = (progress + offset) * perimeter;
            dist = fmod(dist + perimeter, perimeter);
            const int trail_segments = 12; // mais segmentos = rastro mais contínuo
            const float trail_fade = 0.82f; // fade mais suave
            const float seg = 28.0f;
            const float step_size = 0.75f; // MUITO importante (anti-pixel)
            for (int i = 0; i < trail_segments; ++i)
            {
                float trail_dist = dist - i * (seg / 1.5f);
                trail_dist = fmod(trail_dist + perimeter, perimeter);
                Color_t trailColor = lineColor;
                trailColor.a = static_cast<int>(alpha * pow(trail_fade, i));
                float trail_seg = seg - i * 2.0f;
                if (trail_seg < 5.0f) trail_seg = 5.0f;
                auto drawBlurredLine = [&](int x1, int y1, int x2, int y2, Color_t col)
                    {
                        const int blur_levels = 3;
                        const float blur_fade = 0.5f;
                        for (int b = -blur_levels + 1; b < blur_levels; ++b)
                        {
                            if (b == 0) continue;
                            Color_t blurCol = col;
                            blurCol.a = static_cast<int>(col.a * pow(blur_fade, abs(b)));
                            if (abs(x1 - x2) > abs(y1 - y2))
                                H::Draw->Line(x1, y1 + b, x2, y2 + b, blurCol);
                            else
                                H::Draw->Line(x1 + b, y1, x2 + b, y2, blurCol);
                        }
                        H::Draw->Line(x1, y1, x2, y2, col);
                    };
                for (float d = 0.0f; d < trail_seg; d += step_size)
                {
                    auto p1 = getPosition(trail_dist + d);
                    auto p2 = getPosition(trail_dist + d + step_size);
                    if (d + step_size > trail_seg) p2 = getPosition(trail_dist + trail_seg);
                    int x1 = static_cast<int>(p1.first);
                    int y1 = static_cast<int>(p1.second);
                    int x2 = static_cast<int>(p2.first);
                    int y2 = static_cast<int>(p2.second);
                    drawBlurredLine(x1, y1, x2, y2, trailColor);
                }
            }
        };
    drawTrace(0.00f);
    drawTrace(0.50f);
    const CFont& font = H::Fonts->Get(EFonts::PIXEL);
    const char* fullText = "X69";
    int maxLen = (int)strlen(fullText);
    float typeTime = 1.5f;
    float typeProg = std::min(1.0f, totalElapsed / typeTime);
    int visible = int(maxLen * typeProg);
    char buffer[64] = {};
    strncpy_s(buffer, fullText, visible);
    wchar_t wbuffer[64] = {};
    wsprintfW(wbuffer, L"%hs", buffer);
    int currW = 0, currH = 0;
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wbuffer, currW, currH);
    int textY = cy - 20; // A bit higher up
    H::Draw->String(
        font,
        cx,
        textY,
        Color_t(255, 254, 246, alpha),
        POS_CENTERX | POS_CENTERY,
        buffer
    );
    // Thin line below X69 with expansion animation
    float lineProg = std::min(1.0f, totalElapsed / 1.5f);
    float maxHalfWidth = 100.0f; // Adjust max width as needed
    float halfWidth = maxHalfWidth * lineProg;
    int lineY = textY + (currH / 2) + 5; // Below the text
    H::Draw->Line(
        static_cast<int>(cx - halfWidth),
        lineY,
        static_cast<int>(cx + halfWidth),
        lineY,
        Color_t(255, 254, 246, alpha)
    );
    H::Draw->String(
        H::Fonts->Get(EFonts::PIXEL),
        cx,
        cy + 20,
        Color_t(255, 254, 246, alpha),
        POS_CENTERX | POS_CENTERY,
        "god i wish i had X69"
    );
}