#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <Windows.h>
#include <string>
#include "src/imgui/imgui.h"
#include "src/imgui/imgui_internal.h"
#include "src/SDK/SDK.h"
#include "src/CFG.h"
#include "../src/Features/PlayersList/PlayersList.h"
#include "../src/Features/Menu/notification_system/notifs.h"
#include "../src/Features/indicators/indicators.h"
#define STB_IMAGE_IMPLEMENTATION

namespace gui
{
    ImFont* menu_font = nullptr;
    ImFont* indicator_font = nullptr;
    ImFont* title_font = nullptr;
    ImFont* icon_font = nullptr;

    // ===== VARIÁVEIS GLOBAIS PARA TEMA =====
    static ImVec4 g_current_accent_color = ImVec4(0.0f, 122.0f / 255.0f, 187.0f / 255.0f, 1.0f);

    // ===== FUNÇÃO PARA OBTER COR ATUAL DO TEMA =====
    inline ImU32 GetAccentColor(int alpha = 255)
    {
        return IM_COL32(
            (int)(g_current_accent_color.x * 255),
            (int)(g_current_accent_color.y * 255),
            (int)(g_current_accent_color.z * 255),
            alpha
        );
    }

    inline ImU32 GetAccentColorFaded(int alpha = 70)
    {
        return IM_COL32(
            (int)(g_current_accent_color.x * 255),
            (int)(g_current_accent_color.y * 255),
            (int)(g_current_accent_color.z * 255),
            alpha
        );
    }

    void set_theme(ImVec4 accent_color = ImVec4(0.0f, 122.0f / 255.0f, 187.0f / 255.0f, 1.0f))
    {
        // Atualizar cor global
        g_current_accent_color = accent_color;

        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;
        style.FrameBorderSize = 1.0f;
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 0.0f;
        style.ChildRounding = 0.0f;
        style.WindowRounding = 0.0f;
        style.FrameRounding = 3.0f;
        style.WindowPadding = ImVec2(0.f, 8.f);
        style.FramePadding = ImVec2(5.f, 3.f);
        style.WindowTitleAlign = ImVec2(0.50f, 0.50f);
        style.ItemSpacing = ImVec2(0.f, 4.f);
        style.ScrollbarSize = 4.0f;
        style.ScrollbarRounding = 2.0f;


        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.85f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2f, 0.2f, 0.2f, 0.8f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.00f);
        colors[ImGuiCol_CheckMark] = accent_color;
        colors[ImGuiCol_SliderGrab] = accent_color;
        colors[ImGuiCol_SliderGrabActive] = accent_color;
        colors[ImGuiCol_Button] = ImVec4(0.07f, 0.08f, 0.09f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = accent_color;
        colors[ImGuiCol_ButtonActive] = accent_color;
        colors[ImGuiCol_Header] = accent_color;
        colors[ImGuiCol_HeaderHovered] = accent_color;
        colors[ImGuiCol_HeaderActive] = accent_color;
        colors[ImGuiCol_Separator] = accent_color;
        colors[ImGuiCol_SeparatorHovered] = ImVec4(accent_color.x * 0.5f, accent_color.y * 0.5f, accent_color.z * 0.5f, accent_color.w);
        colors[ImGuiCol_SeparatorActive] = ImVec4(accent_color.x * 0.5f, accent_color.y * 0.5f, accent_color.z * 0.5f, accent_color.w);
        colors[ImGuiCol_ResizeGrip] = ImVec4(accent_color.x * 0.5f, accent_color.y * 0.5f, accent_color.z * 0.5f, accent_color.w * 0.2f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(accent_color.x * 0.67f, accent_color.y * 0.67f, accent_color.z * 0.67f, accent_color.w);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(accent_color.x * 0.95f, accent_color.y * 0.95f, accent_color.z * 0.95f, accent_color.w);
        colors[ImGuiCol_TabHovered] = ImVec4(accent_color.x * 0.8f, accent_color.y * 0.8f, accent_color.z * 0.8f, accent_color.w);
        colors[ImGuiCol_Tab] = ImVec4(accent_color.x * 0.58f, accent_color.y * 0.58f, accent_color.z * 0.58f, accent_color.w * 0.86f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(accent_color.x * 0.35f, accent_color.y * 0.35f, accent_color.z * 0.35f, accent_color.w);
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    }

    bool checkbox(const char* label, bool& value)
    {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        if (!ctx) return false;

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();

        ImVec2 box_size(14.0f, 14.0f);
        float box_offset_x = 0.0f;  // Colado na borda esquerda
        float box_offset_y = 3.0f;
        float text_offset_x = 20.0f;  // Reduzido de 25 para 20

        ImVec2 text_size = ImGui::CalcTextSize(label);
        ImVec2 total_size(text_offset_x + text_size.x, ImMax(box_size.y + box_offset_y, text_size.y));

        ImGui::InvisibleButton(label, total_size);
        if (ImGui::IsItemClicked())
            value = !value;

        ImGuiID id = ImGui::GetID(label);
        ImGuiStorage* storage = ImGui::GetStateStorage();
        float progress = storage->GetFloat(id, value ? 1.0f : 0.0f);
        progress += ((value ? 1.0f : 0.0f) - progress) * 0.15f;
        storage->SetFloat(id, progress);

        ImVec2 box_pos = ImVec2(pos.x + box_offset_x, pos.y + box_offset_y);
        ImVec2 box_end = ImVec2(box_pos.x + box_size.x, box_pos.y + box_size.y);

        draw->AddRectFilledMultiColor
        (
            box_pos, box_end,
            IM_COL32(52, 52, 52, 255),
            IM_COL32(52, 52, 52, 255),
            IM_COL32(41, 41, 41, 255),
            IM_COL32(41, 41, 41, 255)
        );

        if (progress > 0.01f)
        {
            ImU32 accent_color = GetAccentColor((int)(255 * progress));
            ImU32 bottom_grad = IM_COL32(25, 25, 25, (int)(255 * progress));

            draw->AddRectFilledMultiColor
            (
                box_pos, box_end,
                accent_color, accent_color,
                bottom_grad, bottom_grad
            );
        }

        draw->AddRect(box_pos, box_end, IM_COL32(15, 15, 15, 150));

        ImU32 text_col = IM_COL32(120, 120, 120, 255);
        draw->AddText(ImVec2(pos.x + text_offset_x, pos.y + 2.0f), text_col, label);

        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + total_size.y + ctx->Style.ItemSpacing.y));

        return value;
    }

    bool slider(const char* label, float* value, float min, float max, const char* suffix = "", float width = 330.0f)
    {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        const ImGuiStyle& style = ctx->Style;
        ImGuiID id = window->GetID(label);

        float track_height = 10.0f;
        float text_height = ImGui::GetFontSize();
        float spacing = 2.0f;

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 total_size(width + 10.0f, 15.0f + track_height);

        ImRect bb(pos, pos + total_size);
        ImGui::ItemSize(total_size, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        bool hovered, held;
        ImGui::ButtonBehavior(bb, id, &hovered, &held);

        if (held)
        {
            float mouse_x = ImGui::GetIO().MousePos.x;
            float t = (mouse_x - bb.Min.x) / width;
            t = ImClamp(t, 0.0f, 1.0f);
            *value = min + t * (max - min);
        }

        float actual_t = ImSaturate((*value - min) / (max - min));
        ImDrawList* dl = window->DrawList;

        std::string name_str = label;
        if (hovered || held)
        {
            name_str += ": " + std::to_string((int)*value) + suffix;
        }

        dl->AddText(ImVec2(pos.x, pos.y), IM_COL32(120, 120, 120, 255), name_str.c_str());

        ImVec2 track_pos = ImVec2(pos.x, pos.y + 15.0f);
        ImVec2 track_end = ImVec2(track_pos.x + width, track_pos.y + track_height);
        dl->AddRectFilled(track_pos, track_end, IM_COL32(60, 60, 60, 255));

        if (actual_t > 0.0f)
        {
            float fill_width = width * actual_t;
            ImVec2 fill_end = ImVec2(track_pos.x + fill_width, track_pos.y + track_height);

            // Usar a cor do tema customizado
            ImU32 accent_top = IM_COL32(
                (int)(g_current_accent_color.x * 255),
                (int)(g_current_accent_color.y * 255),
                (int)(g_current_accent_color.z * 255),
                255
            );
            ImU32 accent_bottom = IM_COL32(25, 25, 25, 255);

            dl->AddRectFilledMultiColor
            (
                track_pos, fill_end,
                accent_top, accent_top,
                accent_bottom, accent_bottom
            );
        }

        dl->AddRect(track_pos, track_end, IM_COL32(15, 15, 15, 155));

        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + total_size.y + style.ItemSpacing.y));

        return held;
    }

    bool TabButton(const char* label, int& active_var, int value)
    {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        if (!ctx) return false;

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImVec2 text_size = ImGui::CalcTextSize(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();

        ImVec2 size(text_size.x + 10.0f, text_size.y + 5.0f);
        ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

        ImGui::InvisibleButton(label, size);
        bool is_active = (active_var == value);
        bool hovered = ImGui::IsItemHovered();

        if (ImGui::IsItemClicked())
            active_var = value;

        ImGuiID id = ImGui::GetID(label);
        ImGuiStorage* storage = ImGui::GetStateStorage();
        float progress = storage->GetFloat(id, is_active ? 1.0f : 0.0f);
        progress += ((is_active ? 1.0f : 0.0f) - progress) * 0.15f;
        storage->SetFloat(id, progress);

        ImVec4 col_inactive = ImColor(157, 148, 140, 255);
        // Usar a cor do tema customizado
        ImVec4 col_active = ImVec4(
            g_current_accent_color.x,
            g_current_accent_color.y,
            g_current_accent_color.z,
            1.0f
        );

        ImU32 text_col = ImGui::GetColorU32(ImLerp(col_inactive, col_active, progress));

        ImDrawList* draw = ImGui::GetWindowDrawList();

        ImVec2 text_pos
        (
            pos.x + (size.x - text_size.x) * 0.5f,
            pos.y + (size.y - text_size.y) * 0.5f
        );

        draw->AddText(text_pos, text_col, label);

        if (hovered && !is_active)
        {
            draw->AddText(text_pos, IM_COL32(255, 255, 255, 30), label);
        }

        return is_active;
    }

    float g_GroupBoxContentOffsetX = 0.0f;
    float g_GroupBoxContentOffsetY = 0.0f;

    bool begin_group(const char* label, ImVec2 size, float content_offset_x = 0.0f, float content_offset_y = 0.0f)
    {
        g_GroupBoxContentOffsetX = content_offset_x;
        g_GroupBoxContentOffsetY = content_offset_y;

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGui::BeginGroup();

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 end = ImVec2(pos.x + size.x, pos.y + size.y);

        ImU32 bg_color = IM_COL32(30, 30, 30, 255);
        ImU32 border_color = IM_COL32(16, 16, 16, 255);
        ImU32 text_color = IM_COL32(220, 220, 220, 255);
        ImU32 text_shadow = IM_COL32(10, 10, 10, 150);

        ImU32 accent_main = GetAccentColor(255);
        ImU32 accent_faded = GetAccentColorFaded(70);

        ImGui::Dummy(size);
        draw->AddRectFilled(pos, end, bg_color);


        draw->AddRectFilledMultiColor(
            pos,
            ImVec2(pos.x + size.x, pos.y + 15),
            accent_faded,
            accent_faded,
            accent_main,
            accent_main
        );

        ImVec2 text_pos = ImVec2(pos.x + 5, pos.y + 2);
        draw->AddText(ImVec2(text_pos.x + 1, text_pos.y + 1), text_shadow, label);
        draw->AddText(text_pos, text_color, label);

        draw->AddRect(pos, end, border_color);

        float header_height = 15.0f;
        ImGui::SetCursorScreenPos(ImVec2(
            pos.x + 5.0f + content_offset_x,
            pos.y + header_height + 5.0f + content_offset_y
        ));

        ImGui::Indent(5.0f + content_offset_x);
        return true;
    }

    void end_group()
    {
        ImGui::Unindent(5.0f + g_GroupBoxContentOffsetX);
        ImGui::EndGroup();
    }

    // Variáveis para controlar o scroll group
    static ImVec2 g_ScrollGroupPos;
    static ImVec2 g_ScrollGroupSize;

    bool begin_group_scrollable(const char* label, ImVec2 size, float content_offset_x = 0.0f, float content_offset_y = 0.0f)
    {
        g_GroupBoxContentOffsetX = content_offset_x;
        g_GroupBoxContentOffsetY = content_offset_y;

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGui::BeginGroup();

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 end = ImVec2(pos.x + size.x, pos.y + size.y);

        g_ScrollGroupPos = pos;
        g_ScrollGroupSize = size;

        ImU32 bg_color = IM_COL32(30, 30, 30, 255);
        ImU32 border_color = IM_COL32(16, 16, 16, 255);
        ImU32 text_color = IM_COL32(220, 220, 220, 255);
        ImU32 text_shadow = IM_COL32(10, 10, 10, 150);

        ImU32 accent_main = GetAccentColor(255);
        ImU32 accent_faded = GetAccentColorFaded(70);

        ImGui::Dummy(size);
        draw->AddRectFilled(pos, end, bg_color);

        draw->AddRectFilledMultiColor(
            pos,
            ImVec2(pos.x + size.x, pos.y + 15),
            accent_faded,
            accent_faded,
            accent_main,
            accent_main
        );

        ImVec2 text_pos = ImVec2(pos.x + 5, pos.y + 2);
        draw->AddText(ImVec2(text_pos.x + 1, text_pos.y + 1), text_shadow, label);
        draw->AddText(text_pos, text_color, label);

        draw->AddRect(pos, end, border_color);

        float header_height = 15.0f;
        // Posição do child: mais próximo da borda esquerda
        ImVec2 child_pos = ImVec2(pos.x + 8.0f, pos.y + header_height + 5.0f);
        // Tamanho do child: vai até quase a borda direita (deixa espaço apenas para o scrollbar de 4px)
        ImVec2 child_size = ImVec2(size.x - 12.0f, size.y - header_height - 10.0f);

        ImGui::SetCursorScreenPos(child_pos);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::BeginChild(label, child_size, false, ImGuiWindowFlags_NoBackground);
        ImGui::PopStyleVar();

        return true;
    }

    void end_group_scrollable()
    {
        ImGui::EndChild();
        ImGui::EndGroup();
    }

    bool hotkey(const char* label, int* key, float width = 70.0f, float height = 15.0f)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiID id = window->GetID(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size(width, height);
        ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

        ImGui::ItemSize(size);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

        static bool capturing = false;
        static int* target_key = nullptr;

        if (pressed) {
            capturing = true;
            target_key = key;
        }

        bool is_active = (capturing && key == target_key);

        ImGuiStorage* storage = ImGui::GetStateStorage();
        float progress = storage->GetFloat(id, 0.0f);
        float target = is_active ? 1.0f : 0.0f;
        progress += (target - progress) * 0.12f;
        progress = ImClamp(progress, 0.0f, 1.0f);
        storage->SetFloat(id, progress);

        if (is_active) {
            for (int vk = 1; vk <= 255; vk++) {
                if (GetAsyncKeyState(vk) & 0x8000) {
                    if (vk == VK_LBUTTON && pressed) continue;

                    if (vk == VK_ESCAPE) {
                        *key = 0;
                    }
                    else {
                        *key = vk;
                    }

                    capturing = false;
                    target_key = nullptr;
                    break;
                }
            }
        }

        ImDrawList* draw = window->DrawList;

        ImU32 bg_color = hovered ? IM_COL32(45, 45, 45, 255) : IM_COL32(22, 24, 26, 255);
        draw->AddRectFilled(bb.Min, bb.Max, bg_color, 0.0f);

        ImU32 border_color = hovered ? IM_COL32(80, 80, 80, 255) : IM_COL32(50, 50, 50, 255);
        draw->AddRect(bb.Min, bb.Max, border_color, 0.0f);

        if (progress > 0.01f) {
            draw->AddRectFilled(
                ImVec2(bb.Min.x, bb.Max.y - 2.0f),
                bb.Max,
                IM_COL32(176, 70, 242, (int)(255 * progress))
            );
        }

        char buf[16];
        const char* key_name = "...";

        if (!is_active) {
            int vk = *key;
            if (vk == 0) {
                key_name = "Unbound";
            }
            else {
                switch (vk) {
                case VK_LBUTTON: key_name = "M1"; break;
                case VK_RBUTTON: key_name = "M2"; break;
                case VK_MBUTTON: key_name = "M3"; break;
                case VK_XBUTTON1: key_name = "M4"; break;
                case VK_XBUTTON2: key_name = "M5"; break;
                case VK_SHIFT: key_name = "Shift"; break;
                case VK_CONTROL: key_name = "Ctrl"; break;
                case VK_MENU: key_name = "Alt"; break;
                case VK_SPACE: key_name = "Space"; break;
                case VK_RETURN: key_name = "Enter"; break;
                default:
                    if ((vk >= '0' && vk <= '9') || (vk >= 'A' && vk <= 'Z')) {
                        sprintf(buf, "%c", (char)vk);
                        key_name = buf;
                    }
                    else {
                        key_name = "Unknown";
                    }
                    break;
                }
            }
        }

        ImVec2 text_size = ImGui::CalcTextSize(key_name);
        ImVec2 text_pos = ImVec2(bb.Min.x + (size.x - text_size.x) * 0.5f, bb.Min.y + (size.y - text_size.y) * 0.5f);

        ImVec4 inactive_text(0.70f, 0.70f, 0.70f, 0.80f);
        ImVec4 active_text(0.95f, 0.95f, 0.95f, 1.00f);
        ImU32 text_col = ImGui::GetColorU32(ImLerp(inactive_text, active_text, progress));

        draw->AddText(text_pos, text_col, key_name);

        return pressed;
    }

    bool Spinner(const char* label, float radius, int thickness, const ImU32& color) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size((radius) * 2, (radius + style.FramePadding.y) * 2);

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

        const float time = (float)ImGui::GetTime();
        const float start_angle = time * IM_PI * 2.0f;

        const int num_segments = 20;
        const float arc_length = IM_PI * 0.66f;

        const float pulse = 0.5f + 0.5f * ImSin(time * IM_PI * 2.0f);

        window->DrawList->PathClear();
        for (int i = 0; i <= num_segments; i++) {
            const float angle = start_angle + ((float)i / (float)num_segments) * arc_length;
            window->DrawList->PathLineTo(ImVec2(
                centre.x + ImCos(angle) * radius,
                centre.y + ImSin(angle) * radius
            ));
        }

        window->DrawList->PathStroke(color, false, thickness);

        return true;
    }

    struct ComboState {
        bool is_open = false;
        int stored_id = -1;
        float animation = 0.0f;
        int hovered_item = -1;
    };

    static std::map<ImGuiID, ComboState> combo_states;

    void DrawArrow(ImDrawList* draw, ImVec2 pos, bool down, ImU32 color)
    {
        if (down) {
            draw->AddLine(ImVec2(pos.x, pos.y), ImVec2(pos.x + 4, pos.y + 4), color);
            draw->AddLine(ImVec2(pos.x + 8, pos.y), ImVec2(pos.x + 4, pos.y + 4), color);
        }
        else {
            draw->AddLine(ImVec2(pos.x, pos.y + 4), ImVec2(pos.x + 4, pos.y), color);
            draw->AddLine(ImVec2(pos.x + 8, pos.y + 4), ImVec2(pos.x + 4, pos.y), color);
        }
    }

    bool combo(const char* label, int* current_item, const char* const items[], int items_count, float width = 350.0f)
    {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        const ImGuiStyle& style = ctx->Style;
        ImGuiID id = window->GetID(label);

        ComboState& state = combo_states[id];

        ImVec2 pos = window->DC.CursorPos;
        float combo_height = 23.0f;
        float label_height = 15.0f;

        ImVec2 label_pos = ImVec2(pos.x, pos.y);
        window->DrawList->AddText(label_pos, IM_COL32(120, 120, 120, 255), label);

        ImVec2 combo_pos = ImVec2(pos.x, pos.y + label_height);
        ImVec2 combo_size = ImVec2(width, combo_height);
        ImRect combo_bb(combo_pos, ImVec2(combo_pos.x + combo_size.x, combo_pos.y + combo_size.y));

        ImGui::SetCursorScreenPos(combo_pos);
        ImGui::InvisibleButton("##combo_button", combo_size);
        bool combo_clicked = ImGui::IsItemClicked();
        bool combo_hovered = ImGui::IsItemHovered();

        if (combo_clicked) {
            state.is_open = !state.is_open;
            state.stored_id = state.is_open ? id : -1;
        }

        if (state.is_open && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            state.is_open = false;
            state.stored_id = -1;
        }

        float target_anim = state.is_open ? 1.0f : 0.0f;
        state.animation += (target_anim - state.animation) * 0.15f;

        ImDrawList* draw = window->DrawList;

        draw->AddRectFilledMultiColor(
            combo_bb.Min, combo_bb.Max,
            IM_COL32(41, 41, 41, 255),
            IM_COL32(41, 41, 41, 255),
            IM_COL32(49, 49, 49, 255),
            IM_COL32(49, 49, 49, 255)
        );

        draw->AddRect(combo_bb.Min, combo_bb.Max, IM_COL32(15, 15, 15, 155));

        const char* preview_text = state.is_open ?
            "Press ESC to close" :
            (*current_item >= 0 && *current_item < items_count) ? items[*current_item] : "Select...";

        ImVec2 text_pos = ImVec2(combo_pos.x + 8.0f, combo_pos.y + 5.0f);
        draw->AddText(text_pos, IM_COL32(120, 120, 120, 255), preview_text);

        ImVec2 arrow_pos = ImVec2(combo_pos.x + width - 18.0f, combo_pos.y + 9.0f);
        DrawArrow(draw, arrow_pos, !state.is_open, IM_COL32(170, 170, 170, 255));

        bool value_changed = false;

        if (state.animation > 0.01f) {
            float dropdown_y = combo_pos.y + combo_height + 2.0f;
            float item_height = 23.0f;
            float dropdown_height = (item_height * items_count + 4.0f) * state.animation;

            ImVec2 dropdown_pos = ImVec2(combo_pos.x, dropdown_y);
            ImVec2 dropdown_size = ImVec2(width, dropdown_height);
            ImRect dropdown_bb(dropdown_pos, ImVec2(dropdown_pos.x + dropdown_size.x, dropdown_pos.y + dropdown_size.y));

            draw->AddRectFilled(dropdown_bb.Min, dropdown_bb.Max, IM_COL32(35, 35, 35, 255));
            draw->AddRect(dropdown_bb.Min, dropdown_bb.Max, IM_COL32(15, 15, 15, 155));

            if (state.animation > 0.95f) {
                for (int i = 0; i < items_count; i++) {
                    ImVec2 item_pos = ImVec2(dropdown_pos.x + 1.0f, dropdown_pos.y + 2.0f + (i * item_height));
                    ImVec2 item_size = ImVec2(width - 2.0f, item_height - 1.0f);
                    ImRect item_bb(item_pos, ImVec2(item_pos.x + item_size.x, item_pos.y + item_size.y));

                    bool item_hovered = ImGui::IsMouseHoveringRect(item_bb.Min, item_bb.Max);
                    bool is_selected = (*current_item == i);

                    if (is_selected) {
                        draw->AddRectFilled(item_bb.Min, item_bb.Max, IM_COL32(40, 40, 40, 255));
                    }

                    if (item_hovered && !is_selected) {
                        draw->AddRectFilled(item_bb.Min, item_bb.Max, IM_COL32(45, 45, 45, 100));
                    }

                    ImU32 text_color = (item_hovered || is_selected) ?
                        IM_COL32(
                            (int)(g_current_accent_color.x * 255),
                            (int)(g_current_accent_color.y * 255),
                            (int)(g_current_accent_color.z * 255),
                            255
                        ) : IM_COL32(120, 120, 120, 255);

                    ImVec2 item_text_pos = ImVec2(item_pos.x + 8.0f, item_pos.y + 4.0f);
                    draw->AddText(item_text_pos, text_color, items[i]);

                    if (item_hovered && ImGui::IsMouseClicked(0)) {
                        *current_item = i;
                        state.is_open = false;
                        state.stored_id = -1;
                        value_changed = true;
                    }
                }
            }

            if (state.is_open && ImGui::IsMouseClicked(0)) {
                ImVec2 mouse_pos = ImGui::GetMousePos();
                if (!combo_bb.Contains(mouse_pos) && !dropdown_bb.Contains(mouse_pos)) {
                    state.is_open = false;
                    state.stored_id = -1;
                }
            }
        }

        float total_height = label_height + combo_height + style.ItemSpacing.y;
        if (state.animation > 0.01f) {
            total_height += (23.0f * items_count + 4.0f) * state.animation + 2.0f;
        }

        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + total_height));

        return value_changed;
    }

    bool combo(const char* label, int* current_item, const std::vector<std::string>& items, float width = 350.0f)
    {
        std::vector<const char*> items_cstr;
        items_cstr.reserve(items.size());
        for (const auto& item : items) {
            items_cstr.push_back(item.c_str());
        }
        return combo(label, current_item, items_cstr.data(), (int)items_cstr.size(), width);
    }

    struct MultiComboItem {
        std::string name;
        bool* value;

        MultiComboItem(std::string n, bool* v) : name(n), value(v) {}
    };

    struct MultiComboState {
        bool is_open = false;
        int stored_id = -1;
        float animation = 0.0f;
        std::vector<MultiComboItem> items;
    };

    static std::map<ImGuiID, MultiComboState> multi_combo_states;

    bool multi_combo(const char* label, std::vector<MultiComboItem>& items, float width = 350.0f)
    {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        const ImGuiStyle& style = ctx->Style;
        ImGuiID id = window->GetID(label);

        MultiComboState& state = multi_combo_states[id];

        ImVec2 pos = window->DC.CursorPos;
        float combo_height = 23.0f;
        float label_height = 15.0f;

        std::string preview_text = "";
        int selected_count = 0;

        for (size_t i = 0; i < items.size(); i++) {
            if (*items[i].value) {
                if (!preview_text.empty())
                    preview_text += ", ";
                preview_text += items[i].name;
                selected_count++;
            }
        }

        if (preview_text.empty())
            preview_text = "None";

        ImVec2 text_size = ImGui::CalcTextSize(preview_text.c_str());
        if (text_size.x > width - 30.0f) {
            preview_text = preview_text.substr(0, 10) + "...";
        }

        ImVec2 label_pos = ImVec2(pos.x, pos.y);
        window->DrawList->AddText(label_pos, IM_COL32(120, 120, 120, 255), label);

        ImVec2 combo_pos = ImVec2(pos.x, pos.y + label_height);
        ImVec2 combo_size = ImVec2(width, combo_height);
        ImRect combo_bb(combo_pos, ImVec2(combo_pos.x + combo_size.x, combo_pos.y + combo_size.y));

        ImGui::SetCursorScreenPos(combo_pos);
        ImGui::InvisibleButton("##multicombo_button", combo_size);
        bool combo_clicked = ImGui::IsItemClicked();
        bool combo_hovered = ImGui::IsItemHovered();

        if (combo_clicked) {
            state.is_open = !state.is_open;
            state.stored_id = state.is_open ? id : -1;
        }

        if (state.is_open && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            state.is_open = false;
            state.stored_id = -1;
        }

        float target_anim = state.is_open ? 1.0f : 0.0f;
        state.animation += (target_anim - state.animation) * 0.15f;

        ImDrawList* draw = window->DrawList;

        draw->AddRectFilledMultiColor(
            combo_bb.Min, combo_bb.Max,
            IM_COL32(41, 41, 41, 255),
            IM_COL32(41, 41, 41, 255),
            IM_COL32(49, 49, 49, 255),
            IM_COL32(49, 49, 49, 255)
        );

        draw->AddRect(combo_bb.Min, combo_bb.Max, IM_COL32(15, 15, 15, 155));

        const char* display_text = state.is_open ? "Press ESC to close" : preview_text.c_str();
        ImVec2 text_pos = ImVec2(combo_pos.x + 8.0f, combo_pos.y + 5.0f);
        draw->AddText(text_pos, IM_COL32(120, 120, 120, 255), display_text);

        ImVec2 arrow_pos = ImVec2(combo_pos.x + width - 18.0f, combo_pos.y + 9.0f);
        DrawArrow(draw, arrow_pos, !state.is_open, IM_COL32(170, 170, 170, 255));

        bool value_changed = false;

        if (state.animation > 0.01f) {
            float dropdown_y = combo_pos.y + combo_height + 2.0f;
            float item_height = 23.0f;
            int items_count = (int)items.size();
            float dropdown_height = (item_height * items_count + 4.0f) * state.animation;

            ImVec2 dropdown_pos = ImVec2(combo_pos.x, dropdown_y);
            ImVec2 dropdown_size = ImVec2(width, dropdown_height);
            ImRect dropdown_bb(dropdown_pos, ImVec2(dropdown_pos.x + dropdown_size.x, dropdown_pos.y + dropdown_size.y));

            draw->AddRectFilled(dropdown_bb.Min, dropdown_bb.Max, IM_COL32(35, 35, 35, 255));
            draw->AddRect(dropdown_bb.Min, dropdown_bb.Max, IM_COL32(15, 15, 15, 155));

            if (state.animation > 0.95f) {
                for (int i = 0; i < items_count; i++) {
                    ImVec2 item_pos = ImVec2(dropdown_pos.x + 1.0f, dropdown_pos.y + 2.0f + (i * item_height));
                    ImVec2 item_size = ImVec2(width - 2.0f, item_height - 1.0f);
                    ImRect item_bb(item_pos, ImVec2(item_pos.x + item_size.x, item_pos.y + item_size.y));

                    bool item_hovered = ImGui::IsMouseHoveringRect(item_bb.Min, item_bb.Max);
                    bool is_selected = *items[i].value;

                    if (is_selected) {
                        draw->AddRectFilled(item_bb.Min, item_bb.Max, IM_COL32(40, 40, 40, 255));
                    }

                    if (item_hovered && !is_selected) {
                        draw->AddRectFilled(item_bb.Min, item_bb.Max, IM_COL32(45, 45, 45, 100));
                    }

                    ImU32 text_color = (item_hovered || is_selected) ?
                        IM_COL32(
                            (int)(g_current_accent_color.x * 255),
                            (int)(g_current_accent_color.y * 255),
                            (int)(g_current_accent_color.z * 255),
                            255
                        ) : IM_COL32(120, 120, 120, 255);

                    ImVec2 item_text_pos = ImVec2(item_pos.x + 8.0f, item_pos.y + 4.0f);
                    draw->AddText(item_text_pos, text_color, items[i].name.c_str());

                    if (item_hovered && ImGui::IsMouseClicked(0)) {
                        *items[i].value = !*items[i].value;
                        value_changed = true;
                    }
                }
            }

            if (state.is_open && ImGui::IsMouseClicked(0)) {
                ImVec2 mouse_pos = ImGui::GetMousePos();
                if (!combo_bb.Contains(mouse_pos) && !dropdown_bb.Contains(mouse_pos)) {
                    state.is_open = false;
                    state.stored_id = -1;
                }
            }
        }

        float total_height = label_height + combo_height + style.ItemSpacing.y;
        if (state.animation > 0.01f) {
            total_height += (23.0f * items.size() + 4.0f) * state.animation + 2.0f;
        }

        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + total_height));

        return value_changed;
    }

    std::string ToLower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return str;
    }

    struct ListboxState {
        int scroll_pos = 0;
        size_t last_temp_size = 0;
    };

    static std::map<ImGuiID, ListboxState> listbox_states;

    bool listbox(const char* label, int* current_item, const std::vector<std::string>& items, int visible_items,
        float width = 350.0f, float height = 200.0f, std::string* filter = nullptr) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiID id = ImGui::GetID(label);
        ListboxState& state = listbox_states[id];

        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
        ImVec2 label_size = ImGui::CalcTextSize(label);

        if (label[0] != '#') {
            window->DrawList->AddText(ImVec2(cursor_pos.x + 5.0f, cursor_pos.y + 4.0f),
                IM_COL32(120, 120, 120, 255), label);
        }

        ImVec2 box_pos = ImVec2(cursor_pos.x + 5.0f, cursor_pos.y + 18.0f);
        ImVec2 box_size = ImVec2(width, height);
        ImRect box_rect(box_pos, box_pos + box_size);

        ImGui::Dummy(ImVec2(width, 18.0f + height + 5.0f));

        bool changed = false;

        ImRect wheel_rect(ImVec2(box_pos.x + 6.0f, box_pos.y),
            ImVec2(box_pos.x + 126.0f, box_pos.y + height));
        bool wheel_hovered = ImGui::IsMouseHoveringRect(wheel_rect.Min, wheel_rect.Max);

        if (wheel_hovered && ImGui::GetIO().MouseWheel != 0.0f) {
            state.scroll_pos -= static_cast<int>(ImGui::GetIO().MouseWheel);
        }

        std::vector<std::pair<std::string, int>> temp;
        if (filter && !filter->empty()) {
            if (std::islower((*filter)[0])) {
                (*filter)[0] = std::toupper((*filter)[0]);
            }
            std::string l_filter = ToLower(*filter);
            for (size_t i = 0; i < items.size(); ++i) {
                std::string l_item = ToLower(items[i]);
                if (l_item.find(l_filter) != std::string::npos) {
                    temp.emplace_back(items[i], static_cast<int>(i));
                }
            }
        }
        else {
            for (size_t i = 0; i < items.size(); ++i) {
                temp.emplace_back(items[i], static_cast<int>(i));
            }
        }

        if (temp.size() != state.last_temp_size) {
            state.scroll_pos = 0;
            state.last_temp_size = temp.size();
        }

        int max_scroll = std::max(0, static_cast<int>(temp.size()) - visible_items);
        state.scroll_pos = ImClamp(state.scroll_pos, 0, max_scroll);

        ImDrawList* draw = window->DrawList;
        draw->AddRectFilled(box_pos, box_pos + box_size, IM_COL32(45, 45, 45, 255));
        draw->AddRect(box_pos, box_pos + box_size, IM_COL32(15, 15, 15, 150));

        if (!temp.empty()) {
            float item_height = 22.0f;
            int drawn = 0;
            for (size_t i = state.scroll_pos; i < temp.size() && drawn < visible_items; ++i, ++drawn) {
                ImVec2 item_pos = ImVec2(box_pos.x + 12.0f, box_pos.y + 6.0f + drawn * item_height);
                ImRect item_rect(item_pos, item_pos + ImVec2(120.0f, 15.0f));

                bool item_hovered = ImGui::IsMouseHoveringRect(item_rect.Min, item_rect.Max);
                bool selected = (*current_item == temp[i].second);

                if (selected) {
                    draw->AddRectFilled(ImVec2(box_pos.x, box_pos.y + drawn * item_height),
                        ImVec2(box_pos.x + width, box_pos.y + (drawn + 1) * item_height),
                        IM_COL32(60, 60, 60, 255));
                }

                // Usar a cor do tema customizado
                ImU32 text_col = (item_hovered || selected) ?
                    IM_COL32(
                        (int)(g_current_accent_color.x * 255),
                        (int)(g_current_accent_color.y * 255),
                        (int)(g_current_accent_color.z * 255),
                        255
                    ) : IM_COL32(120, 120, 120, 255);

                draw->AddText(item_pos, text_col, temp[i].first.c_str());

                if (item_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    *current_item = temp[i].second;
                    changed = true;
                }
            }
        }

        return changed;
    }

    struct KeybindState {
        bool binding = false;
        std::string name = "[ NONE ]";
        bool list_open = false;
        float anim = 0.0f;
        float anim_text = 0.0f;
    };

    static std::map<ImGuiID, KeybindState> keybind_states;

    const char* keybind_keys[254] = {
        "NONE", "M1", "M2", "BRK", "M3", "M4", "M5",
        "NONE", "Bspc", "Tab", "NONE", "NONE", "NONE", "enter", "NONE", "NONE", "Shift",
        "Ctrl", "Alt", "pau", "Caps", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE",
        "esc", "NONE", "NONE", "NONE", "NONE", "space", "Pgup", "Pgdown", "End", "Home", "Left",
        "up", "right", "down", "NONE", "prnt", "NONE", "prtscr", "ins", "del", "NONE", "0", "1",
        "2", "3", "4", "5", "6", "7", "8", "9", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE",
        "NONE", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U",
        "V", "W", "X", "Y", "Z", "Lftwin", "Rghtwin", "NONE", "NONE", "NONE", "Num0", "Num1",
        "Num2", "Num3", "Num4", "Num5", "Num6", "Num7", "Num8", "Num9", "*", "+", "_", "-", ".", "/", "F1", "F2", "F3",
        "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "f13", "f14", "f15", "f16", "F17", "F18", "F19", "F20",
        "f21",
        "f22", "f23", "f24", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE",
        "num lock", "scroll lock", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE",
        "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "lshft", "rshft", "lctrl",
        "rctrl", "lmenu", "rmenu", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE",
        "NONE", "NONE", "NONE", "ntrk", "ptrk", "stop", "play", "NONE", "NONE",
        "NONE", "NONE", "NONE", "NONE", ";", "+", ",", "-", ".", "/?", "~", "NONE", "NONE",
        "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE",
        "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE",
        "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "{", "\\|", "}", "'\"", "NONE",
        "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE",
        "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE",
        "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE",
        "NONE", "NONE"
    };

    bool keybind(const char* label, int* value, int* bind_type) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;
        ImGuiID id = window->GetID(label);
        KeybindState& s = keybind_states[id];
        char buffer[128];
        bool is_good = false;
        if (s.binding) {
            s.name = "[ ... ]";
        }
        else {
            if (*value >= 0 && *value < 254 && keybind_keys[*value] != nullptr) {
                s.name = "[ " + std::string(keybind_keys[*value]) + " ]";
                is_good = true;
            }
            else {
                if (GetKeyNameTextA(*value << 16, buffer, 127)) {
                    s.name = "[ " + std::string(buffer) + " ]";
                    is_good = true;
                }
            }
            if (!is_good) {
                s.name = "[ NONE ]";
            }
        }
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 text_size = ImGui::CalcTextSize(s.name.c_str());
        ImVec2 text_pos(pos.x - text_size.x, pos.y);
        ImDrawList* draw = window->DrawList;
        draw->AddText(text_pos, IM_COL32(120, 120, 120, 255), s.name.c_str());
        ImGui::SetCursorScreenPos(text_pos);
        ImGui::InvisibleButton("##keybind_btn", ImVec2(text_size.x, text_size.y));
        bool changed = false;
        if (ImGui::IsItemClicked(0)) {
            s.binding = true;
        }
        if (s.binding) {
            for (int i = 0; i < 255; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    if (i == VK_LBUTTON) continue;
                    if (i == VK_ESCAPE) {
                        *value = -1;
                    }
                    else {
                        *value = i;
                    }
                    s.binding = false;
                    changed = true;
                    break;
                }
            }
        }
        if (ImGui::IsItemClicked(1)) {
            s.list_open = !s.list_open;
        }
        if (s.list_open) {
            s.anim = std::min(100.0f, s.anim + 10.0f);
            s.anim_text = std::min(255.0f, s.anim_text + 15.0f);
        }
        else {
            s.anim = std::max(0.0f, s.anim - 10.0f);
            s.anim_text = std::max(0.0f, s.anim_text - 15.0f);
        }
        if (s.anim > 0.01f) {
            ImDrawList* fg_draw = ImGui::GetForegroundDrawList();
            std::vector<std::string> items = { "Always", "Hold on", "Toggle", "Hold off" };
            float item_height = 18.0f;
            float list_width = 89.0f;
            float list_height = (item_height * items.size() + 4.0f) * (s.anim / 100.0f);
            ImVec2 list_pos(pos.x + 4.0f, pos.y);
            ImRect list_rect(list_pos, ImVec2(list_pos.x + list_width, list_pos.y + list_height));
            fg_draw->AddRectFilled(ImVec2(list_pos.x + 1.0f, list_pos.y + 1.0f),
                ImVec2(list_pos.x + list_width - 1.0f, list_pos.y + (item_height * items.size() + 2.0f) * (s.anim / 100.0f)),
                IM_COL32(60, 60, 60, 255));
            fg_draw->AddRect(list_pos, ImVec2(list_pos.x + list_width, list_pos.y + list_height), IM_COL32(15, 15, 15, 155));
            if (s.anim >= 100.0f) {
                for (size_t i = 0; i < items.size(); i++) {
                    ImVec2 item_pos(list_pos.x + 5.0f, list_pos.y + 4.0f + i * item_height);
                    ImRect item_rect(item_pos, ImVec2(item_pos.x + 88.0f, item_pos.y + 15.0f));
                    bool item_hovered = ImGui::IsMouseHoveringRect(item_rect.Min, item_rect.Max);
                    bool item_selected = (*bind_type == static_cast<int>(i));
                    ImU32 text_color = (item_hovered || item_selected) ?
                        IM_COL32(
                            (int)(g_current_accent_color.x * 255),
                            (int)(g_current_accent_color.y * 255),
                            (int)(g_current_accent_color.z * 255),
                            255
                        ) : IM_COL32(120, 120, 120, 255);
                    fg_draw->AddText(ImVec2(list_pos.x + 24.0f, list_pos.y + 4.0f + i * item_height), text_color, items[i].c_str());
                    if (item_hovered && ImGui::IsMouseClicked(0)) {
                        *bind_type = static_cast<int>(i);
                        s.list_open = false;
                        changed = true;
                    }
                }
            }
            if (s.list_open && s.anim >= 100.0f && ImGui::IsMouseClicked(0) &&
                !ImGui::IsMouseHoveringRect(ImVec2(pos.x + 20.0f, pos.y + 4.0f), ImVec2(pos.x + 20.0f + 90.0f, pos.y + 4.0f + items.size() * 18.0f))) {
                s.list_open = false;
            }
        }
        ImGui::SetCursorScreenPos(pos);
        return changed;
    }

    // ===== ESTRUTURAS RGB E HSV =====
    struct RGB { double r, g, b; };
    struct HSV { double h, s, v; };

    // ===== FUNÇÕES DE CONVERSÃO =====
    inline HSV rgb_to_hsv(const RGB& In)
    {
        HSV m_Result;
        double Min = (In.r < In.g) ? In.r : In.g;
        Min = (Min < In.b) ? Min : In.b;

        double Max = (In.r > In.g) ? In.r : In.g;
        Max = (Max > In.b) ? Max : In.b;

        m_Result.v = Max;
        double Delta = Max - Min;

        if (Delta < 0.0001) {
            m_Result.s = 0;
            m_Result.h = 0;
            return m_Result;
        }

        if (Max > 0) {
            m_Result.s = (Delta / Max);
        }
        else {
            m_Result.s = 0;
            m_Result.h = 0;
            return m_Result;
        }

        if (In.r >= Max) {
            m_Result.h = (In.g - In.b) / Delta;
        }
        else if (In.g >= Max) {
            m_Result.h = 2.0 + (In.b - In.r) / Delta;
        }
        else {
            m_Result.h = 4.0 + (In.r - In.g) / Delta;
        }

        m_Result.h *= 60.0;
        if (m_Result.h < 0) m_Result.h += 360.0;

        return m_Result;
    }

    inline RGB hsv_to_rgb(const HSV& In)
    {
        RGB m_Result;

        if (In.s <= 0.0) {
            m_Result.r = In.v;
            m_Result.g = In.v;
            m_Result.b = In.v;
            return m_Result;
        }

        double HH = (In.h >= 360.0 ? 0.0 : In.h) / 60.0;
        long i = (long)HH;
        double FF = HH - i;
        double P = In.v * (1.0 - In.s);
        double Q = In.v * (1.0 - (In.s * FF));
        double T = In.v * (1.0 - (In.s * (1.0 - FF)));

        switch (i) {
        case 0: m_Result.r = In.v; m_Result.g = T; m_Result.b = P; break;
        case 1: m_Result.r = Q; m_Result.g = In.v; m_Result.b = P; break;
        case 2: m_Result.r = P; m_Result.g = In.v; m_Result.b = T; break;
        case 3: m_Result.r = P; m_Result.g = Q; m_Result.b = In.v; break;
        case 4: m_Result.r = T; m_Result.g = P; m_Result.b = In.v; break;
        case 5:
        default: m_Result.r = In.v; m_Result.g = P; m_Result.b = Q; break;
        }

        return m_Result;
    }

    // ===== ESTADO DO COLOR PICKER =====
    struct ColorPickerState {
        bool is_open = false;
        float hue = 0.0f;
        ImVec2 sv_cursor = ImVec2(0, 0);
        float alpha = 1.0f;
        int fade_alpha = 0;
    };

    static std::map<ImGuiID, ColorPickerState> g_color_picker_states;

    // ===== CORES DO ARCO-ÍRIS =====
    static Color_t rainbow_colors[7] = {
        Color_t(255, 0, 0, 255),     // Vermelho
        Color_t(255, 255, 0, 255),   // Amarelo
        Color_t(0, 255, 0, 255),     // Verde
        Color_t(0, 255, 255, 255),   // Ciano
        Color_t(0, 0, 255, 255),     // Azul
        Color_t(255, 0, 255, 255),   // Magenta
        Color_t(255, 0, 0, 255)      // Vermelho
    };

    // ===== FUNÇÃO COLOR PICKER =====
    bool color_picker(const char* label, Color_t* color, bool show_alpha = true)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiID id = window->GetID(label);
        ColorPickerState& state = g_color_picker_states[id];

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* draw = window->DrawList;

        float box_size = 14.0f;
        draw->AddRectFilled(pos, ImVec2(pos.x + box_size, pos.y + box_size),
            IM_COL32(color->r, color->g, color->b, color->a));
        draw->AddRect(pos, ImVec2(pos.x + box_size, pos.y + box_size),
            IM_COL32(15, 15, 15, 155));

        ImGui::SetCursorScreenPos(pos);
        ImGui::InvisibleButton(label, ImVec2(box_size, box_size));

        bool value_changed = false;

        if (ImGui::IsItemClicked()) {
            state.is_open = !state.is_open;

            if (state.is_open) {
                HSV hsv = rgb_to_hsv({ color->r / 255.0, color->g / 255.0, color->b / 255.0 });
                state.hue = (float)(hsv.h / 360.0);
                state.sv_cursor.x = (float)(hsv.s * 148.0);
                state.sv_cursor.y = (float)((1.0 - hsv.v) * 148.0);
                state.alpha = color->a / 255.0f;
            }
        }

        if (state.is_open) {
            state.fade_alpha += 25;
            if (state.fade_alpha > 255) state.fade_alpha = 255;
        }
        else {
            state.fade_alpha -= 25;
            if (state.fade_alpha < 0) state.fade_alpha = 0;
        }

        if (state.fade_alpha > 0) {
            ImVec2 picker_pos = ImVec2(pos.x + box_size + 5, pos.y);
            float picker_width = show_alpha ? 198.0f : 180.0f;
            float picker_height = 168.0f;

            if (state.is_open) {
                ImGui::SetNextWindowPos(picker_pos);
                ImGui::SetNextWindowSize(ImVec2(picker_width, picker_height));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));

                ImGui::Begin("##colorpicker_blocker", nullptr,
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);

                ImGui::InvisibleButton("##blocker", ImVec2(picker_width, picker_height));

                ImGui::End();
                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);
            }

            draw = ImGui::GetForegroundDrawList();

            draw->AddRectFilled(picker_pos,
                ImVec2(picker_pos.x + picker_width, picker_pos.y + picker_height),
                IM_COL32(35, 35, 35, state.fade_alpha));
            draw->AddRect(picker_pos,
                ImVec2(picker_pos.x + picker_width, picker_pos.y + picker_height),
                IM_COL32(15, 15, 15, state.fade_alpha));

            ImVec2 sv_pos = ImVec2(picker_pos.x + 6, picker_pos.y + 6);
            float sv_size = 150.0f;

            RGB hue_color = hsv_to_rgb({ state.hue * 360.0, 1.0, 1.0 });
            ImU32 hue_col = IM_COL32(
                (int)(hue_color.r * 255),
                (int)(hue_color.g * 255),
                (int)(hue_color.b * 255),
                state.fade_alpha
            );

            draw->AddRectFilledMultiColor(
                sv_pos, ImVec2(sv_pos.x + sv_size, sv_pos.y + sv_size),
                IM_COL32(255, 255, 255, state.fade_alpha),
                IM_COL32(255, 255, 255, state.fade_alpha),
                hue_col,
                hue_col
            );

            draw->AddRectFilledMultiColor(
                sv_pos, ImVec2(sv_pos.x + sv_size, sv_pos.y + sv_size),
                IM_COL32(0, 0, 0, 0),
                IM_COL32(0, 0, 0, 0),
                IM_COL32(0, 0, 0, state.fade_alpha),
                IM_COL32(0, 0, 0, state.fade_alpha)
            );

            draw->AddRect(sv_pos, ImVec2(sv_pos.x + sv_size, sv_pos.y + sv_size),
                IM_COL32(15, 15, 15, state.fade_alpha));

            float cursor_x = sv_pos.x + state.sv_cursor.x + 1;
            float cursor_y = sv_pos.y + state.sv_cursor.y + 1;
            draw->AddCircle(ImVec2(cursor_x, cursor_y), 5.0f, IM_COL32(0, 0, 0, state.fade_alpha), 16, 2.0f);
            draw->AddCircle(ImVec2(cursor_x, cursor_y), 5.0f, IM_COL32(255, 255, 255, state.fade_alpha), 16, 1.5f);

            ImVec2 hue_pos = ImVec2(sv_pos.x + sv_size + 6, sv_pos.y);
            float hue_width = 12.0f;
            float hue_height = 150.0f;

            for (int i = 0; i < 6; i++) {
                float segment_height = hue_height / 6.0f;
                draw->AddRectFilledMultiColor(
                    ImVec2(hue_pos.x, hue_pos.y + (segment_height * i)),
                    ImVec2(hue_pos.x + hue_width, hue_pos.y + (segment_height * (i + 1))),
                    IM_COL32(rainbow_colors[i].r, rainbow_colors[i].g, rainbow_colors[i].b, state.fade_alpha),
                    IM_COL32(rainbow_colors[i].r, rainbow_colors[i].g, rainbow_colors[i].b, state.fade_alpha),
                    IM_COL32(rainbow_colors[i + 1].r, rainbow_colors[i + 1].g, rainbow_colors[i + 1].b, state.fade_alpha),
                    IM_COL32(rainbow_colors[i + 1].r, rainbow_colors[i + 1].g, rainbow_colors[i + 1].b, state.fade_alpha)
                );
            }

            draw->AddRect(hue_pos, ImVec2(hue_pos.x + hue_width, hue_pos.y + hue_height),
                IM_COL32(15, 15, 15, state.fade_alpha));

            float hue_indicator_y = hue_pos.y + (hue_height * state.hue);
            draw->AddLine(
                ImVec2(hue_pos.x - 1, hue_indicator_y),
                ImVec2(hue_pos.x + hue_width + 1, hue_indicator_y),
                IM_COL32(0, 0, 0, state.fade_alpha), 3.0f
            );
            draw->AddLine(
                ImVec2(hue_pos.x - 1, hue_indicator_y),
                ImVec2(hue_pos.x + hue_width + 1, hue_indicator_y),
                IM_COL32(255, 255, 255, state.fade_alpha), 1.5f
            );

            if (show_alpha) {
                ImVec2 alpha_pos = ImVec2(hue_pos.x + hue_width + 6, sv_pos.y);
                float alpha_width = 12.0f;
                float alpha_height = 150.0f;

                for (int y = 0; y < 15; y++) {
                    for (int x = 0; x < 2; x++) {
                        ImU32 checker_col = ((x + y) % 2 == 0) ?
                            IM_COL32(200, 200, 200, state.fade_alpha) :
                            IM_COL32(140, 140, 140, state.fade_alpha);
                        draw->AddRectFilled(
                            ImVec2(alpha_pos.x + (x * 6), alpha_pos.y + (y * 10)),
                            ImVec2(alpha_pos.x + (x * 6) + 6, alpha_pos.y + (y * 10) + 10),
                            checker_col
                        );
                    }
                }

                draw->AddRectFilledMultiColor(
                    alpha_pos,
                    ImVec2(alpha_pos.x + alpha_width, alpha_pos.y + alpha_height),
                    IM_COL32(color->r, color->g, color->b, 0),
                    IM_COL32(color->r, color->g, color->b, 0),
                    IM_COL32(color->r, color->g, color->b, state.fade_alpha),
                    IM_COL32(color->r, color->g, color->b, state.fade_alpha)
                );

                draw->AddRect(alpha_pos, ImVec2(alpha_pos.x + alpha_width, alpha_pos.y + alpha_height),
                    IM_COL32(15, 15, 15, state.fade_alpha));

                float alpha_indicator_y = alpha_pos.y + (alpha_height * state.alpha);
                draw->AddLine(
                    ImVec2(alpha_pos.x - 1, alpha_indicator_y),
                    ImVec2(alpha_pos.x + alpha_width + 1, alpha_indicator_y),
                    IM_COL32(0, 0, 0, state.fade_alpha), 3.0f
                );
                draw->AddLine(
                    ImVec2(alpha_pos.x - 1, alpha_indicator_y),
                    ImVec2(alpha_pos.x + alpha_width + 1, alpha_indicator_y),
                    IM_COL32(255, 255, 255, state.fade_alpha), 1.5f
                );
            }

            if (state.is_open) {
                ImVec2 mouse = ImGui::GetMousePos();
                bool mouse_down = ImGui::IsMouseDown(0);

                bool mouse_in_picker = (mouse.x >= picker_pos.x && mouse.x <= picker_pos.x + picker_width &&
                    mouse.y >= picker_pos.y && mouse.y <= picker_pos.y + picker_height);

                if (mouse_down && mouse_in_picker) {
                    if (mouse.x >= sv_pos.x && mouse.x <= sv_pos.x + sv_size &&
                        mouse.y >= sv_pos.y && mouse.y <= sv_pos.y + sv_size)
                    {
                        state.sv_cursor.x = ImClamp(mouse.x - sv_pos.x - 1, 0.0f, 148.0f);
                        state.sv_cursor.y = ImClamp(mouse.y - sv_pos.y - 1, 0.0f, 148.0f);
                        value_changed = true;
                    }
                    else if (mouse.x >= hue_pos.x && mouse.x <= hue_pos.x + hue_width &&
                        mouse.y >= hue_pos.y && mouse.y <= hue_pos.y + hue_height)
                    {
                        state.hue = ImClamp((mouse.y - hue_pos.y) / hue_height, 0.0f, 1.0f);
                        value_changed = true;
                    }
                    else if (show_alpha) {
                        ImVec2 alpha_pos = ImVec2(hue_pos.x + hue_width + 6, sv_pos.y);
                        float alpha_width = 12.0f;
                        float alpha_height = 150.0f;

                        if (mouse.x >= alpha_pos.x && mouse.x <= alpha_pos.x + alpha_width &&
                            mouse.y >= alpha_pos.y && mouse.y <= alpha_pos.y + alpha_height)
                        {
                            state.alpha = ImClamp((mouse.y - alpha_pos.y) / alpha_height, 0.0f, 1.0f);
                            value_changed = true;
                        }
                    }

                    if (value_changed) {
                        float sat = state.sv_cursor.x / 148.0f;
                        float val = 1.0f - (state.sv_cursor.y / 148.0f);
                        RGB new_color = hsv_to_rgb({ state.hue * 360.0, sat, val });

                        color->r = (unsigned char)(new_color.r * 255.0);
                        color->g = (unsigned char)(new_color.g * 255.0);
                        color->b = (unsigned char)(new_color.b * 255.0);
                        color->a = (unsigned char)(state.alpha * 255.0);
                    }
                }

                if (ImGui::IsMouseClicked(0)) {
                    bool clicked_inside =
                        mouse.x >= picker_pos.x && mouse.x <= picker_pos.x + picker_width &&
                        mouse.y >= picker_pos.y && mouse.y <= picker_pos.y + picker_height;

                    bool clicked_preview =
                        mouse.x >= pos.x && mouse.x <= pos.x + box_size &&
                        mouse.y >= pos.y && mouse.y <= pos.y + box_size;

                    if (!clicked_inside && !clicked_preview) {
                        state.is_open = false;
                    }
                }
            }
        }

        ImGui::SetCursorScreenPos(ImVec2(pos.x + box_size + 2, pos.y));
        return value_changed;
    }

    bool checkbox_color(const char* label, bool& value, Color_t* color, bool show_alpha = false)
    {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        if (!ctx) return false;

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();

        ImVec2 box_size(14.0f, 14.0f);
        float box_offset_x = 0.0f;
        float box_offset_y = 3.0f;
        float text_offset_x = 20.0f;

        ImVec2 text_size = ImGui::CalcTextSize(label);

        // Tamanho total só do checkbox (sem o color picker)
        ImVec2 checkbox_size(text_offset_x + text_size.x, ImMax(box_size.y + box_offset_y, text_size.y));

        // ✅ CORREÇÃO: Usar ItemAdd ao invés de InvisibleButton para não bloquear cliques
        ImGuiID id = ImGui::GetID(label);
        ImRect checkbox_bb(pos, ImVec2(pos.x + checkbox_size.x, pos.y + checkbox_size.y));
        ImGui::ItemSize(checkbox_bb);
        if (!ImGui::ItemAdd(checkbox_bb, id))
            return false;

        // ✅ Detectar clique apenas na área do checkbox (não na linha toda)
        bool hovered = ImGui::IsMouseHoveringRect(checkbox_bb.Min, checkbox_bb.Max);
        if (hovered && ImGui::IsMouseClicked(0))
            value = !value;
        ImGuiStorage* storage = ImGui::GetStateStorage();
        float progress = storage->GetFloat(id, value ? 1.0f : 0.0f);
        progress += ((value ? 1.0f : 0.0f) - progress) * 0.15f;
        storage->SetFloat(id, progress);

        ImVec2 box_pos = ImVec2(pos.x + box_offset_x, pos.y + box_offset_y);
        ImVec2 box_end = ImVec2(box_pos.x + box_size.x, box_pos.y + box_size.y);

        draw->AddRectFilledMultiColor(
            box_pos, box_end,
            IM_COL32(52, 52, 52, 255),
            IM_COL32(52, 52, 52, 255),
            IM_COL32(41, 41, 41, 255),
            IM_COL32(41, 41, 41, 255)
        );

        if (progress > 0.01f)
        {
            ImU32 accent_color = GetAccentColor((int)(255 * progress));
            ImU32 bottom_grad = IM_COL32(25, 25, 25, (int)(255 * progress));

            draw->AddRectFilledMultiColor(
                box_pos, box_end,
                accent_color, accent_color,
                bottom_grad, bottom_grad
            );
        }

        draw->AddRect(box_pos, box_end, IM_COL32(15, 15, 15, 150));

        ImU32 text_col = IM_COL32(120, 120, 120, 255);
        draw->AddText(ImVec2(pos.x + text_offset_x, pos.y + 2.0f), text_col, label);

        // ✅ Color picker na posição 350 (SameLine)
        ImVec2 color_picker_pos = ImVec2(pos.x + 340.0f, pos.y + 2.0f);
        ImGui::SetCursorScreenPos(color_picker_pos);

        // ✅ Criar ID único para o color picker baseado no label
        char picker_id[64];
        snprintf(picker_id, sizeof(picker_id), "##color_%s", label);
        gui::color_picker(picker_id, color, show_alpha);

        // Avança o cursor para a próxima linha
        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + checkbox_size.y + ctx->Style.ItemSpacing.y));

        return value;
    }

    bool button(const char* label, ImVec2 size = ImVec2(360, 32))
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext* ctx = ImGui::GetCurrentContext();
        const ImGuiStyle& style = ctx->Style;
        ImGuiID id = window->GetID(label);

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

        // Renderizar o botão
        ImDrawList* draw = window->DrawList;

        // Gradiente de fundo (top = mais claro, bottom = mais escuro)
        ImU32 color_top = hovered ? IM_COL32(85, 85, 85, 255) : IM_COL32(72, 72, 72, 255);
        ImU32 color_bottom = hovered ? IM_COL32(95, 95, 95, 255) : IM_COL32(107, 107, 107, 255);

        draw->AddRectFilledMultiColor(
            bb.Min, bb.Max,
            color_bottom, color_bottom,
            color_top, color_top
        );

        // Borda
        draw->AddRect(bb.Min, bb.Max, IM_COL32(15, 15, 15, 155));

        // Texto centralizado com sombra
        ImVec2 text_size = ImGui::CalcTextSize(label);
        ImVec2 text_pos(
            pos.x + (size.x - text_size.x) * 0.5f,
            pos.y + (size.y - text_size.y) * 0.5f
        );

        // Sombra do texto
        draw->AddText(ImVec2(text_pos.x + 1, text_pos.y + 1), IM_COL32(10, 10, 10, 235), label);

        // Texto principal
        ImU32 text_color = hovered ? IM_COL32(220, 220, 220, 255) : IM_COL32(180, 180, 180, 255);
        draw->AddText(text_pos, text_color, label);

        return pressed;
    }
}