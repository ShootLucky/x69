#include "App.h"
#include "../src/SDK/SDK.h"
#include "../src/Features/Menu/notification_system/notifs.h"
#include "../src/Features/PlayersList/PlayersList.h"
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <vector>
#include "../src/Features/Aimbot/AimbotHitscan/AimbotHitscan.h"
#include "../src/CFG.h" // Para CFG::Menu_ThemeColor

// ============================================================================
// FUNÇÕES DE EASING
// ============================================================================
float EaseInOutCubic(float t)
{
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

// ============================================================================
// SUBTEXTOS
// ============================================================================
const char* ChooseRandomSubText()
{
    int r = rand() % 10000;
    if (r < 4500) return "premium gaming experience";
    else if (r < 6000) return "totally legit, trust me";
    else if (r < 7400) return "random crits are balanced";
    else if (r < 8800) return "phantom.club best cheats";
    else if (r < 9300) return "skill issue detected";
    else if (r < 9600) return "vac was never enough";
    else if (r < 9750) return "advanced gaming tools";
    else if (r < 9850) return "spy backtrack go brr";
    else if (r < 9900) return "next-gen gaming";
    else if (r < 9950) return "hvh mid on badlands";
    else if (r < 9990) return "phantom >> everything";
    else if (r < 9995) return "premium club access";
    else return "phantom.club // dev build";
}

// ============================================================================
// INICIALIZAÇÃO
// ============================================================================
void CApp::Start()
{
    srand(static_cast<unsigned int>(time(NULL)));
    while (!Memory::FindSignature(
        "client.dll",
        "48 8B 0D ? ? ? ? 48 8B 10 48 8B 19 48 8B C8 FF 92"))
    {
        bUnload = GetAsyncKeyState(VK_F11) & 0x8000;
        if (bUnload)
            return;
        Sleep(500);
    }
    U::Storage->Init("phantom");
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
            m_PlayerName = "Unknown";
        }
    }
    else
    {
        m_PlayerName = "Unknown";
    }
    g_SubText = ChooseRandomSubText();
    g_SubTextChosen = true;
    U::HookManager->InitializeAllHooks();
    F::Players->Parse();
    Config::Load(U::Storage->GetConfigFolder() / "default.json");
    bBlackScreen = true;
    ulBlackStart = GetTickCount();
}

void CApp::Loop()
{
    while (true)
    {
        if (!alive)
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
}

// ============================================================================
// ANIMAÇÃO SPLASH SCREEN
// ============================================================================
void CApp::Draw()
{
    if (!bBlackScreen)
        return;

    int w = H::Draw->GetScreenW();
    int h = H::Draw->GetScreenH();

    DWORD current = GetTickCount();
    float totalElapsed = static_cast<float>(current - ulBlackStart) / 1000.0f;

    // Timings
    const float flowDuration = 5.0f;
    const float fadeOutDuration = 0.8f;
    const float fadeStart = flowDuration;
    const float totalEnd = fadeStart + fadeOutDuration;

    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000 || totalElapsed >= totalEnd)
    {
        bBlackScreen = false;
        return;
    }

    int cx = w / 2;
    int cy = h / 2;

    // Overall alpha for fade out to transparent
    int overallAlpha = 255;
    if (totalElapsed >= fadeStart)
    {
        float fadeT = (totalElapsed - fadeStart) / fadeOutDuration;
        fadeT = EaseInOutCubic(fadeT);
        overallAlpha = static_cast<int>(255 * (1.0f - fadeT));
    }

    // Background preto
    Color_t bgColor = Color_t(0, 0, 0, overallAlpha);
    H::Draw->RectFilled(0, 0, w, h, bgColor);

    // Fontes
    DWORD mainFont = I::MatSystemSurface->CreateFont();
    I::MatSystemSurface->SetFontGlyphSet(mainFont, "verdanab", 72, 700, 0, 0, FONTFLAG_ANTIALIAS);

    DWORD subFont = I::MatSystemSurface->CreateFont();
    I::MatSystemSurface->SetFontGlyphSet(subFont, "verdanab", 24, 400, 0, 0, FONTFLAG_ANTIALIAS);

    DWORD skipFont = I::MatSystemSurface->CreateFont();
    I::MatSystemSurface->SetFontGlyphSet(skipFont, "verdanab", 16, 400, 0, 0, FONTFLAG_ANTIALIAS);

    // Textos
    std::wstring mainText = L"Phantom.club";
    std::wstring subText(g_SubText, g_SubText + strlen(g_SubText));

    int mainW, mainH, subW, subH;
    I::MatSystemSurface->GetTextSize(mainFont, mainText.c_str(), mainW, mainH);
    I::MatSystemSurface->GetTextSize(subFont, subText.c_str(), subW, subH);

    int mainX = cx - mainW / 2;
    int mainY = cy - mainH / 2 - 20;
    int subX = cx - subW / 2;
    int subY = mainY + mainH + 15;

    // Progresso do preenchimento uniforme (água subindo em todas as letras ao mesmo tempo)
    float flowProgress = totalElapsed / flowDuration;
    if (flowProgress > 1.0f) flowProgress = 1.0f;
    float fillProgress = EaseInOutCubic(flowProgress);

    // Parâmetros para o efeito de neblina/smoke
    const float amp = 0.05f;
    const float transition_width = 0.15f;
    const float waveFreq1 = 0.05f;
    const float waveFreq2 = 0.08f;
    const float waveSpeed1 = 3.0f;
    const float waveSpeed2 = 4.5f;

    // Cor do tema
    Color_t themeColor = CFG::Menu_ThemeColor;

    // ================= TEXTO PRINCIPAL =================
    int currentX = mainX;
    for (size_t i = 0; i < mainText.length(); i++)
    {
        wchar_t ch[2] = { mainText[i], 0 };
        int charW, charH;
        I::MatSystemSurface->GetTextSize(mainFont, ch, charW, charH);

        // Texto base preto
        H::Draw->Text(currentX, mainY, mainFont, Color_t(0, 0, 0, overallAlpha), ALIGN_DEFAULT, ch);

        const int stripHeight = 2;
        const int stripWidth = 2;

        for (int x_strip = 0; x_strip < charW; x_strip += stripWidth)
        {
            int this_w = std::min(stripWidth, charW - x_strip);
            float x = static_cast<float>(currentX + x_strip + this_w / 2);
            float phase1 = waveSpeed1 * totalElapsed + 2.0f * PI * waveFreq1 * x;
            float phase2 = waveSpeed2 * totalElapsed + 2.0f * PI * waveFreq2 * x;
            float norm_wave = sinf(phase1) + 0.5f * sinf(phase2 + PI / 2.0f);
            float local_fillProgress = fillProgress + amp * norm_wave;
            local_fillProgress = std::clamp(local_fillProgress, 0.0f, 1.0f);

            for (int strip = 0; strip < charH; strip += stripHeight)
            {
                int clipTop = mainY + charH - (strip + stripHeight);
                int clipBottom = mainY + charH - strip;
                if (clipTop < mainY) clipTop = mainY;

                float y_from_bottom = static_cast<float>(strip + stripHeight / 2);
                float relative_y = y_from_bottom / static_cast<float>(charH);

                float norm_dist = (relative_y - (local_fillProgress - transition_width / 2.0f)) / transition_width;
                norm_dist = std::clamp(norm_dist, 0.0f, 1.0f);
                float alpha_mod = 1.0f - EaseInOutCubic(norm_dist);

                if (alpha_mod < 0.01f) continue;

                float t = static_cast<float>(strip) / static_cast<float>(charH);
                t = EaseInOutCubic(t);

                int r = static_cast<int>(themeColor.r * (0.5f + 0.5f * t));
                int g = static_cast<int>(themeColor.g * (0.5f + 0.5f * t));
                int b = static_cast<int>(themeColor.b * (0.5f + 0.5f * t));
                int a = static_cast<int>(themeColor.a * alpha_mod * (overallAlpha / 255.0f));

                H::Draw->PushClipRect(currentX + x_strip, clipTop, this_w, clipBottom - clipTop);
                H::Draw->Text(currentX, mainY, mainFont, Color_t(r, g, b, a), ALIGN_DEFAULT, ch);
                H::Draw->PopClipRect();
            }
        }

        currentX += charW;
    }

    // ================= SUBTEXTO =================
    currentX = subX;
    for (size_t i = 0; i < subText.length(); i++)
    {
        wchar_t ch[2] = { subText[i], 0 };
        int charW, charH;
        I::MatSystemSurface->GetTextSize(subFont, ch, charW, charH);

        // Texto base preto
        H::Draw->Text(currentX, subY, subFont, Color_t(0, 0, 0, overallAlpha), ALIGN_DEFAULT, ch);

        const int stripHeight = 2;
        const int stripWidth = 2;

        for (int x_strip = 0; x_strip < charW; x_strip += stripWidth)
        {
            int this_w = std::min(stripWidth, charW - x_strip);
            float x = static_cast<float>(currentX + x_strip + this_w / 2);
            float phase1 = waveSpeed1 * totalElapsed + 2.0f * PI * waveFreq1 * x;
            float phase2 = waveSpeed2 * totalElapsed + 2.0f * PI * waveFreq2 * x;
            float norm_wave = sinf(phase1) + 0.5f * sinf(phase2 + PI / 2.0f);
            float local_fillProgress = fillProgress + amp * norm_wave;
            local_fillProgress = std::clamp(local_fillProgress, 0.0f, 1.0f);

            for (int strip = 0; strip < charH; strip += stripHeight)
            {
                int clipTop = subY + charH - (strip + stripHeight);
                int clipBottom = subY + charH - strip;
                if (clipTop < subY) clipTop = subY;

                float y_from_bottom = static_cast<float>(strip + stripHeight / 2);
                float relative_y = y_from_bottom / static_cast<float>(charH);

                float norm_dist = (relative_y - (local_fillProgress - transition_width / 2.0f)) / transition_width;
                norm_dist = std::clamp(norm_dist, 0.0f, 1.0f);
                float alpha_mod = 1.0f - EaseInOutCubic(norm_dist);

                if (alpha_mod < 0.01f) continue;

                float t = static_cast<float>(strip) / static_cast<float>(charH);
                t = EaseInOutCubic(t);

                int r = static_cast<int>((themeColor.r * 0.5f + 90) * (0.5f + 0.5f * t));
                int g = static_cast<int>((themeColor.g * 0.5f + 90) * (0.5f + 0.5f * t));
                int b = static_cast<int>((themeColor.b * 0.5f + 90) * (0.5f + 0.5f * t));
                int a = static_cast<int>(200 * alpha_mod * (overallAlpha / 255.0f));

                H::Draw->PushClipRect(currentX + x_strip, clipTop, this_w, clipBottom - clipTop);
                H::Draw->Text(currentX, subY, subFont, Color_t(r, g, b, a), ALIGN_DEFAULT, ch);
                H::Draw->PopClipRect();
            }
        }

        currentX += charW;
    }

    // Skip message
    if (totalElapsed < fadeStart)
    {
        std::wstring skipText = L"press ESC to skip";
        int skipW, skipH;
        I::MatSystemSurface->GetTextSize(skipFont, skipText.c_str(), skipW, skipH);

        float pulse = (sinf(totalElapsed * 3.0f) + 1.0f) / 2.0f;
        int skipAlpha = static_cast<int>((80 + 80 * pulse) * (overallAlpha / 255.0f));

        H::Draw->Text(cx - skipW / 2, h - skipH - 30, skipFont, Color_t(themeColor.r, themeColor.g, themeColor.b, skipAlpha), ALIGN_DEFAULT, skipText.c_str());
    }
}