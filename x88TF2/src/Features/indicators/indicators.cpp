#include <Windows.h>
#include <string>
#include <algorithm>
#include "../src/imgui/imgui.h"
#include "../src/imgui/imgui_internal.h"
#include "../src/SDK/SDK.h"
#include "../src/CFG.h"
#include "indicators.h"
#include "../../App/App.h"

extern bool show_menu;

namespace indicators {
    bool local_player_death = false;
    bool player_died[64] = {};

    // Estrutura para spectators
    struct Spectator_t {
        std::string name;
        int mode;
        float respawnTime;
        float animTime;
        bool removing;
        float removeTime;

        Spectator_t(const std::string& n, int m, float r)
            : name(n), mode(m), respawnTime(r), animTime(0.f), removing(false), removeTime(0.f) {
        }
    };

    static std::vector<Spectator_t> g_spectators;
    static constexpr float SPEC_ANIM_DURATION = 0.5f;
    static constexpr float SPEC_FADE_DURATION = 0.35f;

    // Variáveis de drag separadas para cada painel
    static Vector2D drag_indicators{}, last_indicators{}, cur_indicators{};
    static Vector2D drag_keybinds{}, last_keybinds{}, cur_keybinds{};
    static Vector2D drag_watermark{}, last_watermark{}, cur_watermark{};
    static Vector2D drag_spectators{}, last_spectators{}, cur_spectators{};
}

const float ind_speed = 0.1f;

static float lerp(float t, float a, float b) {
    return (1 - t) * a + t * b;
}

static float clampf(float v, float a, float b) {
    return v < a ? a : (v > b ? b : v);
}

static void get_mouse(Vector2D& last, Vector2D& cur) {
    last = cur;
    cur = Vector2D(H::Input->GetMouseX(), H::Input->GetMouseY());
}

// Helper para drag com salvamento automático - SÓ FUNCIONA COM MENU ABERTO
static bool DragWindow(float& cfg_x, float& cfg_y, float width, float height, Vector2D& drag, Vector2D& last, Vector2D& cur) {
    // CRÍTICO: Só permite drag se o menu estiver aberto
    if (!show_menu) {
        // Se o menu fechou enquanto estava arrastando, salvar posição
        static bool was_dragging[4] = { false, false, false, false };
        static int drag_index = 0;

        if (was_dragging[drag_index]) {
            cfg_x += drag.x;
            cfg_y += drag.y;
            drag.x = 0.f;
            drag.y = 0.f;
            was_dragging[drag_index] = false;
        }
        return false;
    }

    get_mouse(last, cur);

    float x = cfg_x + drag.x;
    float y = cfg_y + drag.y;

    bool isDragging = false;

    if (cur.x > x && cur.y > y &&
        cur.x < x + width &&
        cur.y < y + 20 && // área do header
        H::Input->IsDown(VK_LBUTTON)) {

        drag.x += cur.x - last.x;
        drag.y += cur.y - last.y;
        isDragging = true;
    }

    // Se soltou o mouse, salva a posição final
    static bool was_dragging = false;
    if (was_dragging && !H::Input->IsDown(VK_LBUTTON)) {
        cfg_x += drag.x;
        cfg_y += drag.y;
        drag.x = 0.f;
        drag.y = 0.f;
    }
    was_dragging = isDragging;

    return isDragging;
}

namespace indicators {

    // Funções helper para spectators
    static int GetModePriority(int mode) {
        if (mode == OBS_MODE_IN_EYE) return 0;
        if (mode == OBS_MODE_CHASE) return 1;
        if (mode == OBS_MODE_DEATHCAM) return 2;
        if (mode == OBS_MODE_FREEZECAM) return 3;
        return 4;
    }

    static const char* GetModeString(int mode) {
        switch (mode) {
        case OBS_MODE_IN_EYE: return "1ST";
        case OBS_MODE_CHASE: return "3RD";
        case OBS_MODE_DEATHCAM: return "DEATH";
        case OBS_MODE_FREEZECAM: return "FREEZE";
        default: return "UNK";
        }
    }

    static bool UpdateSpectators() {
        auto pLocal = H::Entities->GetLocal();
        if (!pLocal)
            return false;

        std::vector<Spectator_t> present;
        float now = static_cast<float>(Plat_FloatTime());

        // Coletar espectadores atuais
        for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PLAYERS_OBSERVER)) {
            auto pPlayer = pEntity->As<C_TFPlayer>();
            if (!pPlayer)
                continue;

            auto pTarget = pPlayer->m_hObserverTarget().Get();
            if (pTarget != pLocal)
                continue;

            int mode = pPlayer->m_iObserverMode();
            if (mode != OBS_MODE_IN_EYE && mode != OBS_MODE_CHASE &&
                mode != OBS_MODE_DEATHCAM && mode != OBS_MODE_FREEZECAM)
                continue;

            player_info_t info{};
            if (!I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &info))
                continue;

            float respawnTime = 0.f;
            if (pPlayer->m_iTeamNum() == pLocal->m_iTeamNum() && !pPlayer->IsAlive()) {
                if (auto* pr = GetTFPlayerResource()) {
                    float nextRespawn = pr->GetNextRespawnTime(pPlayer->entindex());
                    float remain = nextRespawn - now;
                    if (remain > 0.f) respawnTime = remain;
                }
            }

            present.emplace_back(info.name, mode, respawnTime);
        }

        // Se local está espectando alguém, mostrar outros espectadores desse alvo
        auto pObserverTarget = pLocal->m_hObserverTarget().Get();
        if (pObserverTarget && pObserverTarget != pLocal) {
            for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PLAYERS_OBSERVER)) {
                auto pPlayer = pEntity->As<C_TFPlayer>();
                if (!pPlayer || pPlayer == pLocal)
                    continue;

                if (pPlayer->m_hObserverTarget().Get() != pObserverTarget)
                    continue;

                int mode = pPlayer->m_iObserverMode();
                if (mode != OBS_MODE_IN_EYE && mode != OBS_MODE_CHASE &&
                    mode != OBS_MODE_DEATHCAM && mode != OBS_MODE_FREEZECAM)
                    continue;

                player_info_t info{};
                if (!I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &info))
                    continue;

                float respawnTime = 0.f;
                if (pPlayer->m_iTeamNum() == pLocal->m_iTeamNum() && !pPlayer->IsAlive()) {
                    if (auto* pr = GetTFPlayerResource()) {
                        float nextRespawn = pr->GetNextRespawnTime(pPlayer->entindex());
                        float remain = nextRespawn - now;
                        if (remain > 0.f) respawnTime = remain;
                    }
                }

                present.emplace_back(info.name, mode, respawnTime);
            }
        }

        // Merge com lista anterior
        std::vector<Spectator_t> merged;

        for (auto& p : present) {
            bool found = false;
            for (auto& old : g_spectators) {
                if (old.name == p.name) {
                    p.animTime = old.removing ? now : old.animTime;
                    p.removing = false;
                    p.removeTime = 0.f;
                    found = true;
                    break;
                }
            }
            if (!found) {
                p.animTime = now;
            }
            merged.push_back(p);
        }

        // Marcar removidos
        for (auto& old : g_spectators) {
            bool stillPresent = false;
            for (const auto& m : merged) {
                if (m.name == old.name) {
                    stillPresent = true;
                    break;
                }
            }
            if (!stillPresent) {
                if (!old.removing) {
                    old.removing = true;
                    old.removeTime = now;
                }
                merged.push_back(old);
            }
        }

        // Ordenar
        std::sort(merged.begin(), merged.end(), [](const Spectator_t& a, const Spectator_t& b) {
            if (a.removing != b.removing) return !a.removing;
            if (a.removing && b.removing) return a.removeTime < b.removeTime;
            int pa = GetModePriority(a.mode);
            int pb = GetModePriority(b.mode);
            if (pa != pb) return pa < pb;
            return a.name < b.name;
            });

        g_spectators = std::move(merged);
        return !g_spectators.empty();
    }

    void indicator() {
        // Verificar se está habilitado
        if (!CFG::Indicators_Enable)
            return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;

        auto local = H::Entities->GetLocal();
        if (!local)
            return;

        bool alive = local->IsAlive();
        float velocity = alive ? local->m_vecVelocity().Length2D() : 0.f;

        // Usar posição da config com drag próprio
        DragWindow(CFG::Indicators_Pos_X, CFG::Indicators_Pos_Y, 210, 20, drag_indicators, last_indicators, cur_indicators);

        float x = CFG::Indicators_Pos_X + drag_indicators.x;
        float y = CFG::Indicators_Pos_Y + drag_indicators.y;

        // Calculate height
        static bool enabled = true;
        int amount = 0;
        if (enabled) {
            amount = 5;
        }
        int height = (5 + amount) * 11 + 12;

        // Background
        dl->AddRectFilled(ImVec2(x, y), ImVec2(x + 210, y + height), IM_COL32(35, 35, 35, 150));

        // Header
        dl->AddRectFilled(ImVec2(x, y + 1), ImVec2(x + 210, y + 18), IM_COL32(20, 20, 20, 255));

        // Accent line
        Color_t accent = CFG::Menu_ModifyTheme ? CFG::Menu_ThemeColor : Color_t(255, 0, 0, 255);
        dl->AddRectFilled(ImVec2(x, y + 1), ImVec2(x + 210, y + 2), IM_COL32(accent.r, accent.g, accent.b, accent.a));

        // Title
        std::string indicators = "indicators";
        std::transform(indicators.begin(), indicators.end(), indicators.begin(), ::toupper);

        ImGui::PushFont(gui::indicator_font);
        ImVec2 ts = ImGui::CalcTextSize(indicators.c_str());
        ImGui::PopFont();

        int centerX = x + (210 - ts.x) / 2;
        dl->AddText(gui::indicator_font, 12.0f, ImVec2(centerX, y + 4), IM_COL32(255, 255, 255, 255), indicators.c_str());

        // Border
        dl->AddRect(ImVec2(x, y), ImVec2(x + 210, y + height), IM_COL32(15, 15, 15, 255));

        auto add_indicator = [&](int idx, const char* name, float progress) {
            float oy = y + 19 + idx * 14;

            std::string indicator_name = name;
            std::transform(indicator_name.begin(), indicator_name.end(), indicator_name.begin(), ::toupper);

            // Text
            dl->AddText(gui::indicator_font, 12.0f, ImVec2(x + 7, oy), IM_COL32(255, 255, 255, 255), indicator_name.c_str());

            // Bar background
            dl->AddRectFilled(ImVec2(x + 200 - 116 - 10, oy + 2), ImVec2(x + 200 - 116 - 10 + 130, oy + 10),
                IM_COL32(20, 20, 20, 100));

            // Bar gradient fill
            float bar_width = progress * 130;
            ImVec2 bar_start(x + 200 - 116 - 10, oy + 2);
            ImVec2 bar_end(x + 200 - 116 - 10 + bar_width, oy + 10);

            ImU32 col_start = IM_COL32(35, 35, 35, 150);
            ImU32 col_end = IM_COL32(accent.r, accent.g, accent.b, accent.a);

            dl->AddRectFilledMultiColor(bar_start, bar_end, col_start, col_end, col_end, col_start);
            };

        // Fake Yaw (LBY)
        {
            static float value = 0.f;
            float change = 0.5f;
            float new_value = clampf(change, 0.f, 1.f);
            value = lerp(ind_speed, value, new_value);

            add_indicator(0, "fake yaw", value);
        }

        // Fakelag
        {
            static float value = 0.f;

            if (CFG::AntiAim_FakeLag_Enable) {
                float new_value = 1.f;
                value = lerp(ind_speed, value, new_value);
            }
            else {
                value = lerp(ind_speed, value, 0.f);
            }

            add_indicator(1, "fakelag", value);
        }

        // Exploit (Doubletap)
        {
            static float value = 0.f;
            int shift_amm = 0;
            float new_value = clampf((float)shift_amm / 14.f, 0.f, 1.f);
            value = lerp(ind_speed, value, new_value);

            add_indicator(2, "exploit", value);
        }

        // Inaccuracy
        {
            static float value = 0.f;
            float new_value = 0.f;
            value = lerp(ind_speed, value, new_value);

            add_indicator(3, "inaccuracy", value);
        }

        // Velocity
        {
            static float value = 0.f;
            float new_value = clampf(velocity / 300.f, 0.f, 1.f);
            value = lerp(ind_speed, value, new_value);

            add_indicator(4, "velocity", value);
        }
    }

    void keybind() {
        // Verificar se está habilitado
        if (!CFG::Indicators_Keybinds_Enable)
            return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;

        // Usar posição da config com drag próprio
        DragWindow(CFG::Keybinds_Pos_X, CFG::Keybinds_Pos_Y, 210, 20, drag_keybinds, last_keybinds, cur_keybinds);

        float x = CFG::Keybinds_Pos_X + drag_keybinds.x;
        float y = CFG::Keybinds_Pos_Y + drag_keybinds.y;

        struct key_binds_t {
            std::string text;
            std::string mode;
        };
        std::vector<key_binds_t> keys{};

        auto translate_mode = [](int style) -> std::string {
            switch (style) {
            case 0: return "active";
            case 1: return "hold";
            case 2: return "toggled";
            case 3: return "force off";
            default: return "unknown";
            }
            };

        int pixel = keys.empty() ? 18 : 18 + (14 * keys.size()) + 1;

        // Background
        dl->AddRectFilled(ImVec2(x, y), ImVec2(x + 210, y + pixel), IM_COL32(35, 35, 35, 150));

        // Header
        dl->AddRectFilled(ImVec2(x, y + 1), ImVec2(x + 210, y + 18), IM_COL32(20, 20, 20, 255));

        // Accent line
        Color_t accent = CFG::Menu_ModifyTheme ? CFG::Menu_ThemeColor : Color_t(255, 0, 0, 255);
        dl->AddRectFilled(ImVec2(x, y + 1), ImVec2(x + 210, y + 2), IM_COL32(accent.r, accent.g, accent.b, accent.a));

        // Title
        std::string indicators2 = "keybinds";
        std::transform(indicators2.begin(), indicators2.end(), indicators2.begin(), ::toupper);

        ImGui::PushFont(gui::indicator_font);
        ImVec2 ts = ImGui::CalcTextSize(indicators2.c_str());
        ImGui::PopFont();

        int centerX = x + (210 - ts.x) / 2;
        dl->AddText(gui::indicator_font, 12.0f, ImVec2(centerX, y + 4), IM_COL32(255, 255, 255, 255), indicators2.c_str());

        // Border
        dl->AddRect(ImVec2(x, y), ImVec2(x + 210, y + pixel), IM_COL32(15, 15, 15, 255));

        // Render keybinds
        for (size_t i = 0; i < keys.size(); ++i) {
            auto& indicator = keys[i];

            std::string mode_upper = indicator.mode;
            std::transform(mode_upper.begin(), mode_upper.end(), mode_upper.begin(), ::toupper);

            ImGui::PushFont(gui::indicator_font);
            ImVec2 mode_size = ImGui::CalcTextSize(mode_upper.c_str());
            ImGui::PopFont();

            float render_y = y + pixel - 13;
            int text_render_pos = x + 216 - mode_size.x;

            dl->AddText(gui::indicator_font, 12.0f, ImVec2(x + 5, render_y), IM_COL32(255, 255, 255, 255), indicator.text.c_str());
            dl->AddText(gui::indicator_font, 12.0f, ImVec2(text_render_pos, render_y - 3), IM_COL32(255, 255, 255, 255), mode_upper.c_str());
        }
    }

    void watermark()
    {
        // Verificar se está habilitado
        if (!CFG::Indicators_Watermark_Enable)
            return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;

        // Player name
        player_info_t info{};
        std::string name = "UNKNOWN";

        if (I::EngineClient->IsInGame() && I::EngineClient->GetLocalPlayer() > 0)
        {
            int local = I::EngineClient->GetLocalPlayer();
            if (I::EngineClient->GetPlayerInfo(local, &info))
                name = info.name;
        }

        // FPS
        static float fps_smooth = 0.f;
        static float fps_last_update = 0.f;

        float realtime = Plat_FloatTime();
        float fps_real = 1.f / std::max(I::GlobalVars->absoluteframetime, 0.0001f);

        if (realtime - fps_last_update > 0.3f)
        {
            fps_smooth = fps_smooth * 0.75f + fps_real * 0.25f;
            fps_last_update = realtime;
        }

        int fps = (int)(fps_smooth + 0.5f);
        char fps_buf[16];
        sprintf_s(fps_buf, "%03d", fps);

        // Ping
        int ping = int(SDKUtils::GetLatency() * 1000.f);

        // Time
        SYSTEMTIME st;
        GetLocalTime(&st);
        char time_buf[32];
        sprintf_s(time_buf, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);

        // Build text
        std::string text = "X69.TECHNOLOGY";

        if (CFG::Watermark_ShowName)
            text += " | " + name;

        if (CFG::Watermark_ShowFPS)
        {
            text += " | FPS: ";
            text += fps_buf;
        }

        if (CFG::Watermark_ShowPing)
            text += " | PING: " + std::to_string(ping);

        if (CFG::Watermark_ShowTime)
            text += " | TIME: " + std::string(time_buf);

        std::transform(text.begin(), text.end(), text.begin(), ::toupper);

        ImGui::PushFont(gui::indicator_font);
        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        ImGui::PopFont();

        float paddingX = 11.f;
        float paddingY = 6.f;

        float width = textSize.x + paddingX * 2.f;
        float height = textSize.y + paddingY * 2.f;

        // Drag com variáveis próprias
        DragWindow(CFG::Watermark_Pos_X, CFG::Watermark_Pos_Y, width, height, drag_watermark, last_watermark, cur_watermark);

        float x = CFG::Watermark_Pos_X + drag_watermark.x;
        float y = CFG::Watermark_Pos_Y + drag_watermark.y;

        // Background
        dl->AddRectFilled(
            ImVec2(x, y),
            ImVec2(x + width, y + height),
            IM_COL32(35, 35, 35, 180)
        );

        // Accent line
        Color_t accent = CFG::Menu_ModifyTheme ? CFG::Menu_ThemeColor : Color_t(255, 0, 0, 255);
        dl->AddRectFilled(
            ImVec2(x, y),
            ImVec2(x + width, y + 2),
            IM_COL32(accent.r, accent.g, accent.b, accent.a)
        );

        // Text
        dl->AddText(
            gui::indicator_font,
            12.f,
            ImVec2(x + paddingX, y + paddingY),
            IM_COL32(255, 255, 255, 255),
            text.c_str()
        );

        // Border
        dl->AddRect(
            ImVec2(x, y),
            ImVec2(x + width, y + height),
            IM_COL32(15, 15, 15, 220)
        );
    }

    void spectator_list() {
        // Verificar se está habilitado
        if (!CFG::Visual_Spectatorlist)
            return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;

        // Atualizar lista
        UpdateSpectators();

        float now = static_cast<float>(Plat_FloatTime());

        // Remover itens que terminaram fade out
        g_spectators.erase(
            std::remove_if(g_spectators.begin(), g_spectators.end(), [now](const Spectator_t& s) {
                return s.removing && (now - s.removeTime >= SPEC_FADE_DURATION);
                }),
            g_spectators.end()
        );

        int itemCount = static_cast<int>(g_spectators.size());
        int height = itemCount > 0 ? 18 + (itemCount * 14) + 1 : 18;

        // Drag com variáveis próprias
        DragWindow(CFG::Spectators_Pos_X, CFG::Spectators_Pos_Y, 210, height, drag_spectators, last_spectators, cur_spectators);

        float x = CFG::Spectators_Pos_X + drag_spectators.x;
        float y = CFG::Spectators_Pos_Y + drag_spectators.y;

        // Background
        dl->AddRectFilled(ImVec2(x, y), ImVec2(x + 210, y + height), IM_COL32(35, 35, 35, 150));

        // Header
        dl->AddRectFilled(ImVec2(x, y + 1), ImVec2(x + 210, y + 18), IM_COL32(20, 20, 20, 255));

        // Accent line
        Color_t accent = CFG::Menu_ModifyTheme ? CFG::Menu_ThemeColor : Color_t(255, 0, 0, 255);
        dl->AddRectFilled(ImVec2(x, y + 1), ImVec2(x + 210, y + 2), IM_COL32(accent.r, accent.g, accent.b, accent.a));

        // Title
        std::string title = "spectators";
        std::transform(title.begin(), title.end(), title.begin(), ::toupper);

        ImGui::PushFont(gui::indicator_font);
        ImVec2 ts = ImGui::CalcTextSize(title.c_str());
        ImGui::PopFont();

        int centerX = x + (210 - ts.x) / 2;
        dl->AddText(gui::indicator_font, 12.0f, ImVec2(centerX, y + 4), IM_COL32(255, 255, 255, 255), title.c_str());

        // Border
        dl->AddRect(ImVec2(x, y), ImVec2(x + 210, y + height), IM_COL32(15, 15, 15, 255));

        // Render spectators
        for (size_t i = 0; i < g_spectators.size(); ++i) {
            const auto& spec = g_spectators[i];
            float itemY = y + 19 + i * 14;

            // Calcular alpha
            unsigned char alpha = 255;
            if (spec.removing) {
                float elapsed = now - spec.removeTime;
                float t = std::max(0.f, std::min(1.f, 1.f - (elapsed / SPEC_FADE_DURATION)));
                alpha = static_cast<unsigned char>(255.f * t);
            }
            else if (spec.animTime > 0.f) {
                float elapsed = now - spec.animTime;
                if (elapsed < SPEC_ANIM_DURATION) {
                    float t = std::max(0.f, std::min(1.f, elapsed / SPEC_ANIM_DURATION));
                    alpha = static_cast<unsigned char>(255.f * t);
                }
            }

            // Texto
            char text[256];
            if (spec.respawnTime > 0.f) {
                snprintf(text, sizeof(text), "[%s] %s (%.1fs)",
                    GetModeString(spec.mode), spec.name.c_str(), spec.respawnTime);
            }
            else {
                snprintf(text, sizeof(text), "[%s] %s",
                    GetModeString(spec.mode), spec.name.c_str());
            }

            dl->AddText(gui::indicator_font, 12.0f, ImVec2(x + 5, itemY),
                IM_COL32(255, 255, 255, alpha), text);
        }
    }

    void Run() {
        if (CFG::Misc_Clean_Screenshot && I::EngineClient->IsTakingScreenshot())
            return;

        if (!show_menu && (I::EngineVGui->IsGameUIVisible() || SDKUtils::BInEndOfMatch()))
            return;

        watermark();
        keybind();
        indicator();
        spectator_list();
    }
}