#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <Windows.h>
#include <string>
#include "src/imgui/imgui.h"
#include "src/imgui/imgui_internal.h"

namespace gui
{
    ImFont* menu_font = nullptr;

    ImFont* title_font = nullptr;

    void set_theme()
    {
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
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.69f, 0.27f, 0.95f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.07f, 0.08f, 0.09f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.59f, 0.00f, 1.00f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.59f, 0.00f, 1.00f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.62f, 0.00f, 1.00f, 1.00f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
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
        float box_offset_x = 5.0f;
        float box_offset_y = 3.0f;
        float text_offset_x = 25.0f; 

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
            ImU32 accent_color = IM_COL32(0, 122, 187, (int)(255 * progress));
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
            float t = (mouse_x - (bb.Min.x + 5.0f)) / width;
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

        dl->AddText(ImVec2(pos.x + 5.0f, pos.y), IM_COL32(120, 120, 120, 255), name_str.c_str());

        ImVec2 track_pos = ImVec2(pos.x + 5.0f, pos.y + 15.0f);
        ImVec2 track_end = ImVec2(track_pos.x + width, track_pos.y + track_height);
        dl->AddRectFilled(track_pos, track_end, IM_COL32(60, 60, 60, 255));

        if (actual_t > 0.0f)
        {
            float fill_width = width * actual_t;
            ImVec2 fill_end = ImVec2(track_pos.x + fill_width, track_pos.y + track_height);

            ImU32 accent_top = IM_COL32(0, 122, 187, 255);
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
        ImVec4 col_active = ImColor(11, 163, 248, 255);

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

        ImU32 accent_main = IM_COL32(0, 122, 187, 255);
        ImU32 accent_faded = IM_COL32(0, 122, 187, 70); 

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
}