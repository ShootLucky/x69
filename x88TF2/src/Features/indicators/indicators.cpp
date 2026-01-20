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
        int team;

        Spectator_t(const std::string& n, int m, float r, int t)
            : name(n), mode(m), respawnTime(r), animTime(0.f), removing(false), removeTime(0.f), team(t) {
        }
    };

    static std::vector<Spectator_t> g_spectators;
    static constexpr float SPEC_ANIM_DURATION = 0.3f;
    static constexpr float SPEC_FADE_DURATION = 0.25f;

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

static bool DragWindow(float& cfg_x, float& cfg_y, float width, float height, Vector2D& drag, Vector2D& last, Vector2D& cur) {
    if (!show_menu) {
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
        cur.y < y + 20 &&
        H::Input->KeyDown(VK_LBUTTON)) {

        drag.x += cur.x - last.x;
        drag.y += cur.y - last.y;
        isDragging = true;
    }

    static bool was_dragging = false;
    if (was_dragging && !H::Input->KeyDown(VK_LBUTTON)) {
        cfg_x += drag.x;
        cfg_y += drag.y;
        drag.x = 0.f;
        drag.y = 0.f;
    }
    was_dragging = isDragging;

    return isDragging;
}

namespace indicators {

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
            int team = pPlayer->m_iTeamNum();

            // CORREÇÃO: Calcular tempo de respawn corretamente
            if (team == pLocal->m_iTeamNum() && !pPlayer->IsAlive()) {
                if (auto* pr = GetTFPlayerResource()) {
                    float nextRespawn = pr->GetNextRespawnTime(pPlayer->entindex());
                    if (nextRespawn > 0.f) {
                        float remain = nextRespawn - now;
                        if (remain > 0.f) {
                            respawnTime = remain;
                        }
                    }
                }
            }

            present.emplace_back(info.name, mode, respawnTime, team);
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

                // Verificar se já está na lista
                bool already_added = false;
                for (const auto& p : present) {
                    if (p.name == info.name) {
                        already_added = true;
                        break;
                    }
                }

                if (already_added)
                    continue;

                float respawnTime = 0.f;
                int team = pPlayer->m_iTeamNum();

                // CORREÇÃO: Calcular tempo de respawn corretamente
                if (team == pLocal->m_iTeamNum() && !pPlayer->IsAlive()) {
                    if (auto* pr = GetTFPlayerResource()) {
                        float nextRespawn = pr->GetNextRespawnTime(pPlayer->entindex());
                        if (nextRespawn > 0.f) {
                            float remain = nextRespawn - now;
                            if (remain > 0.f) {
                                respawnTime = remain;
                            }
                        }
                    }
                }

                present.emplace_back(info.name, mode, respawnTime, team);
            }
        }

        // Merge com lista anterior - CORREÇÃO: animação suave
        std::vector<Spectator_t> merged;

        for (auto& p : present) {
            bool found = false;
            for (auto& old : g_spectators) {
                if (old.name == p.name && !old.removing) {
                    p.animTime = old.animTime;
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
            if (!stillPresent && !old.removing) {
                old.removing = true;
                old.removeTime = now;
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
        if (!CFG::Indicators_Enable)
            return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;

        auto local = H::Entities->GetLocal();
        if (!local)
            return;

        bool alive = local->IsAlive();
        float velocity = alive ? local->m_vecVelocity().Length2D() : 0.f;

        DragWindow(CFG::Indicators_Pos_X, CFG::Indicators_Pos_Y, 210, 20, drag_indicators, last_indicators, cur_indicators);

        float x = CFG::Indicators_Pos_X + drag_indicators.x;
        float y = CFG::Indicators_Pos_Y + drag_indicators.y;

        // Contar indicadores ativos
        int amount = 0;
        if (CFG::Indicators_Show_FakeLatency && CFG::Misc_FakeLatency_Enable) amount++;
        if (CFG::Indicators_Show_RealLatency) amount++;
        if (CFG::Indicators_Show_ScoreboardLatency) amount++;
        if (CFG::Indicators_Show_Inaccuracy) amount++;
        if (CFG::Indicators_Show_Velocity) amount++;

        int height = 18 + (amount * 12) + 2;

        // Background
        dl->AddRectFilled(ImVec2(x, y), ImVec2(x + 210, y + height), IM_COL32(35, 35, 35, 150));

        // Header
        dl->AddRectFilled(ImVec2(x, y + 1), ImVec2(x + 210, y + 18), IM_COL32(20, 20, 20, 255));

        // Accent line - CORREÇÃO: usar cor do tema
        Color_t accent = CFG::Menu_ModifyTheme ? CFG::Menu_ThemeColor : Color_t(0, 122, 187, 255);
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

        auto add_indicator = [&](int idx, const char* name, float progress, float value = 0.f, bool show_number = false) {
            float oy = y + 19 + idx * 12;

            std::string indicator_name = name;
            std::transform(indicator_name.begin(), indicator_name.end(), indicator_name.begin(), ::toupper);

            // Text
            dl->AddText(gui::indicator_font, 12.0f, ImVec2(x + 7, oy - 1), IM_COL32(255, 255, 255, 255), indicator_name.c_str());

            // Verificar modo de exibição
            if (CFG::Indicators_Display_Mode == 0) {
                // Modo 0: Apenas barras
                dl->AddRectFilled(ImVec2(x + 200 - 116 - 10, oy + 1), ImVec2(x + 200 - 116 - 10 + 130, oy + 9),
                    IM_COL32(20, 20, 20, 100));

                float bar_width = progress * 130;
                ImVec2 bar_start(x + 200 - 116 - 10, oy + 1);
                ImVec2 bar_end(x + 200 - 116 - 10 + bar_width, oy + 9);

                ImU32 col_start = IM_COL32(35, 35, 35, 150);
                ImU32 col_end = IM_COL32(accent.r, accent.g, accent.b, accent.a);

                dl->AddRectFilledMultiColor(bar_start, bar_end, col_start, col_end, col_end, col_start);
            }
            else if (CFG::Indicators_Display_Mode == 1) {
                // Modo 1: Apenas números
                if (show_number) {
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%.0f", value);
                    ImVec2 num_size = ImGui::CalcTextSize(buf);
                    dl->AddText(gui::indicator_font, 12.0f, ImVec2(x + 200 - num_size.x - 5, oy - 1),
                        IM_COL32(accent.r, accent.g, accent.b, 255), buf);
                }
            }
            else if (CFG::Indicators_Display_Mode == 2) {
                // Modo 2: Barras + Números
                dl->AddRectFilled(ImVec2(x + 200 - 116 - 10, oy + 1), ImVec2(x + 200 - 116 - 10 + 130, oy + 9),
                    IM_COL32(20, 20, 20, 100));

                float bar_width = progress * 130;
                ImVec2 bar_start(x + 200 - 116 - 10, oy + 1);
                ImVec2 bar_end(x + 200 - 116 - 10 + bar_width, oy + 9);

                ImU32 col_start = IM_COL32(35, 35, 35, 150);
                ImU32 col_end = IM_COL32(accent.r, accent.g, accent.b, accent.a);

                dl->AddRectFilledMultiColor(bar_start, bar_end, col_start, col_end, col_end, col_start);

                // Número acima da barra
                if (show_number) {
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%.0f", value);
                    ImVec2 num_size = ImGui::CalcTextSize(buf);
                    dl->AddText(gui::indicator_font, 10.0f, ImVec2(x + 200 - num_size.x - 5, oy - 10),
                        IM_COL32(255, 255, 255, 200), buf);
                }
            }
            };

        int current_idx = 0;

        // Fake Latency - CORREÇÃO: só mostrar quando realmente ativado
        if (CFG::Indicators_Show_FakeLatency && CFG::Misc_FakeLatency_Enable) {
            static float value = 0.f;
            float fake_latency = CFG::Misc_FakeLatencyfloat_Enable;
            float new_value = clampf(fake_latency / 200.f, 0.f, 1.f);
            value = lerp(ind_speed, value, new_value);
            add_indicator(current_idx++, "fake latency", value, fake_latency, true);
        }

        // Real Latency
        if (CFG::Indicators_Show_RealLatency) {
            static float value = 0.f;
            float real_latency = SDKUtils::GetLatency() * 1000.f;
            float new_value = clampf(real_latency / 200.f, 0.f, 1.f);
            value = lerp(ind_speed, value, new_value);
            add_indicator(current_idx++, "real latency", value, real_latency, true);
        }

        // Scoreboard Latency
        if (CFG::Indicators_Show_ScoreboardLatency) {
            static float value = 0.f;
            float sb_latency = 0.f;

            if (I::EngineClient->IsInGame() && local) {
                player_info_t info{};
                if (I::EngineClient->GetPlayerInfo(local->entindex(), &info)) {
                    if (auto* pr = GetTFPlayerResource()) {
                        sb_latency = pr->GetPing(local->entindex());
                    }
                }
            }

            float new_value = clampf(sb_latency / 200.f, 0.f, 1.f);
            value = lerp(ind_speed, value, new_value);
            add_indicator(current_idx++, "scoreboard", value, sb_latency, true);
        }

        // Inaccuracy - CORREÇÃO: fazer funcionar
        if (CFG::Indicators_Show_Inaccuracy) {
            static float value = 0.f;
            float inaccuracy = 0.f;

            if (alive && local->m_hActiveWeapon().Get()) {
                auto weapon = local->m_hActiveWeapon().Get()->As<C_TFWeaponBase>();
                if (weapon) {
                    // Calcular spread baseado no tempo desde último tiro e movimento
                    float spread = weapon->GetWeaponSpread();
                    inaccuracy = spread * 100.f;
                }
            }

            float new_value = clampf(inaccuracy, 0.f, 1.f);
            value = lerp(ind_speed, value, new_value);
            add_indicator(current_idx++, "inaccuracy", value, inaccuracy * 100.f, true);
        }

        // Velocity
        if (CFG::Indicators_Show_Velocity) {
            static float value = 0.f;
            float new_value = clampf(velocity / 520.f, 0.f, 1.f);
            value = lerp(ind_speed, value, new_value);
            add_indicator(current_idx++, "velocity", value, velocity, true);
        }
    }

    void keybind() {
        if (!CFG::Indicators_Keybinds_Enable)
            return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;

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

        // Accent line - CORREÇÃO: usar cor do tema
        Color_t accent = CFG::Menu_ModifyTheme ? CFG::Menu_ThemeColor : Color_t(0, 122, 187, 255);
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
        if (!CFG::Indicators_Watermark_Enable)
            return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;

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
        std::string text = "PHANTOM.CLUB";

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

        DragWindow(CFG::Watermark_Pos_X, CFG::Watermark_Pos_Y, width, height, drag_watermark, last_watermark, cur_watermark);

        float x = CFG::Watermark_Pos_X + drag_watermark.x;
        float y = CFG::Watermark_Pos_Y + drag_watermark.y;

        // Background
        dl->AddRectFilled(
            ImVec2(x, y),
            ImVec2(x + width, y + height),
            IM_COL32(35, 35, 35, 180)
        );

        // Accent line - CORREÇÃO: usar cor do tema
        Color_t accent = CFG::Menu_ModifyTheme ? CFG::Menu_ThemeColor : Color_t(0, 122, 187, 255);
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
        if (!CFG::Visual_Spectatorlist)
            return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;

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

        DragWindow(CFG::Spectators_Pos_X, CFG::Spectators_Pos_Y, 210, height, drag_spectators, last_spectators, cur_spectators);

        float x = CFG::Spectators_Pos_X + drag_spectators.x;
        float y = CFG::Spectators_Pos_Y + drag_spectators.y;

        // Background
        dl->AddRectFilled(ImVec2(x, y), ImVec2(x + 210, y + height), IM_COL32(35, 35, 35, 150));

        // Header
        dl->AddRectFilled(ImVec2(x, y + 1), ImVec2(x + 210, y + 18), IM_COL32(20, 20, 20, 255));

        // Accent line - CORREÇÃO: usar cor do tema
        Color_t accent = CFG::Menu_ModifyTheme ? CFG::Menu_ThemeColor : Color_t(0, 122, 187, 255);
        dl->AddRectFilled(ImVec2(x, y + 1), ImVec2(x + 210, y + 2), IM_COL32(accent.r, accent.g, accent.b, accent.a));

        // Title
        std::string title = "Spectators";
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

            // CORREÇÃO: Calcular alpha baseado no tempo de existência (fade in) e remoção (fade out)
            unsigned char alpha = 255;

            if (spec.removing) {
                float elapsed = now - spec.removeTime;
                float t = 1.0f - (elapsed / SPEC_FADE_DURATION);
                t = std::max(0.f, std::min(1.f, t));
                alpha = static_cast<unsigned char>(255.f * t);
            }
            else {
                float elapsed = now - spec.animTime;
                if (elapsed < SPEC_ANIM_DURATION) {
                    float t = elapsed / SPEC_ANIM_DURATION;
                    t = std::max(0.f, std::min(1.f, t));
                    alpha = static_cast<unsigned char>(255.f * t);
                }
            }

            // CORREÇÃO: Mostrar tempo de respawn para teammates
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