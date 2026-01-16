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

        ImVec2 label_pos = ImVec2(pos.x + 5.0f, pos.y);
        window->DrawList->AddText(label_pos, IM_COL32(120, 120, 120, 255), label);

        ImVec2 combo_pos = ImVec2(pos.x + 5.0f, pos.y + label_height);
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
                        IM_COL32(0, 122, 187, 255) :
                        IM_COL32(120, 120, 120, 255);

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

        ImVec2 label_pos = ImVec2(pos.x + 5.0f, pos.y);
        window->DrawList->AddText(label_pos, IM_COL32(120, 120, 120, 255), label);

        ImVec2 combo_pos = ImVec2(pos.x + 5.0f, pos.y + label_height);
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
                        IM_COL32(0, 122, 187, 255) :
                        IM_COL32(120, 120, 120, 255);

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

                ImU32 text_col = (item_hovered || selected) ? IM_COL32(0, 122, 187, 255)
                    : IM_COL32(120, 120, 120, 255);
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
                    ImU32 text_color = (item_hovered || item_selected) ? IM_COL32(0, 122, 187, 255) : IM_COL32(120, 120, 120, 255);
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

    struct RGB { double r, g, b; };
    struct HSV { double h, s, v; };

    HSV rgb_to_hsv(const RGB& In) {
        HSV m_Result;
        double Min, Max, Delta;

        Min = In.r < In.g ? In.r : In.g;
        Min = Min < In.b ? Min : In.b;

        Max = In.r > In.g ? In.r : In.g;
        Max = Max > In.b ? Max : In.b;

        m_Result.v = Max;
        Delta = Max - Min;

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
            m_Result.h = NAN;
            return m_Result;
        }

        if (In.r >= Max) {
            m_Result.h = (In.g - In.b) / Delta;
        }
        else if (In.g >= Max) {
            m_Result.h = 2 + (In.b - In.r) / Delta;
        }
        else {
            m_Result.h = 4 + (In.r - In.g) / Delta;
        }

        m_Result.h *= 60;
        if (m_Result.h < 0) m_Result.h += 360;

        return m_Result;
    }

    RGB hsv_to_rgb(const HSV& In) {
        RGB m_Result;
        double HH, P, Q, T, FF;
        long i;

        if (In.s <= 0) {
            m_Result.r = In.v;
            m_Result.g = In.v;
            m_Result.b = In.v;
            return m_Result;
        }

        HH = (In.h >= 360 ? 0 : In.h) / 60;
        i = (long)HH;
        FF = HH - i;

        P = In.v * (1 - In.s);
        Q = In.v * (1 - (In.s * FF));
        T = In.v * (1 - (In.s * (1 - FF)));

        switch (i) {
        case 0: m_Result.r = In.v; m_Result.g = T; m_Result.b = P; break;
        case 1: m_Result.r = Q; m_Result.g = In.v; m_Result.b = P; break;
        case 2: m_Result.r = P; m_Result.g = In.v; m_Result.b = T; break;
        case 3: m_Result.r = P; m_Result.g = Q; m_Result.b = In.v; break;
        case 4: m_Result.r = T; m_Result.g = P; m_Result.b = In.v; break;
        case 5: default: m_Result.r = In.v; m_Result.g = P; m_Result.b = Q; break;
        }

        return m_Result;
    }

    struct Color {
        int r, g, b, a;
        Color(int _r = 255, int _g = 255, int _b = 255, int _a = 255) : r(_r), g(_g), b(_b), a(_a) {}
        ImU32 ToU32() const { return IM_COL32(r, g, b, a); }
        ImVec4 ToVec4() const { return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f); }
        void FromHSV(float h, float s, float v, float alpha = 1.0f) {
            RGB rgb = hsv_to_rgb({ h * 360.0, s, v });
            r = static_cast<int>(rgb.r * 255.0f);
            g = static_cast<int>(rgb.g * 255.0f);
            b = static_cast<int>(rgb.b * 255.0f);
            a = static_cast<int>(alpha * 255.0f);
        }
    };

    static ImU32 bar_colors[7] = {
        IM_COL32(255, 0, 0, 255),
        IM_COL32(255, 255, 0, 255),
        IM_COL32(0, 255, 0, 255),
        IM_COL32(0, 255, 255, 255),
        IM_COL32(0, 0, 255, 255),
        IM_COL32(255, 0, 255, 255),
        IM_COL32(255, 0, 0, 255)
    };

    struct ColorPickerState {
        bool open = false;
        float hue = 0.0f;
        ImVec2 cursor_pos = ImVec2(0, 0);
        float alpha = 1.0f;
        float anim_alpha = 0.0f;
    };

    static std::map<ImGuiID, ColorPickerState> color_picker_states;

    bool color_picker(const char* label, Color* color, bool show_alpha = true) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiID id = window->GetID(label);
        ColorPickerState& state = color_picker_states[id];

        ImVec2 pos = ImGui::GetCursorScreenPos();

        ImDrawList* draw = window->DrawList;
        draw->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + 16.0f, pos.y + 8.0f), color->ToU32());
        draw->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + 16.0f, pos.y + 8.0f), IM_COL32(15, 15, 15, 155));

        ImGui::SetCursorScreenPos(pos);
        ImGui::InvisibleButton("##color_picker_preview", ImVec2(16.0f, 8.0f));

        bool changed = false;

        if (ImGui::IsItemClicked()) {
            state.open = !state.open;
            if (state.open) {
                HSV hsv = rgb_to_hsv({ color->r / 255.0, color->g / 255.0, color->b / 255.0 });
                state.hue = static_cast<float>(hsv.h / 360.0);
                state.cursor_pos.x = static_cast<float>(hsv.s * 150.0f);
                state.cursor_pos.y = 150.0f - static_cast<float>(hsv.v * 150.0f);
                state.alpha = color->a / 255.0f;
            }
        }

        float target_alpha = state.open ? 255.0f : 0.0f;
        state.anim_alpha += (target_alpha - state.anim_alpha) * 0.1f;

        if (state.anim_alpha > 0.01f) {
            ImU32 alpha_u = static_cast<ImU32>(state.anim_alpha);

            ImDrawList* fg_draw = ImGui::GetForegroundDrawList();

            fg_draw->AddRectFilled(ImVec2(pos.x + 23.0f, pos.y), ImVec2(pos.x + 203.0f, pos.y + 180.0f), IM_COL32(35, 35, 35, alpha_u));
            fg_draw->AddRect(ImVec2(pos.x + 23.0f, pos.y), ImVec2(pos.x + 203.0f, pos.y + 180.0f), IM_COL32(15, 15, 15, alpha_u));

            fg_draw->AddRect(ImVec2(pos.x + 29.0f, pos.y + 6.0f), ImVec2(pos.x + 180.0f, pos.y + 162.0f), IM_COL32(15, 15, 15, alpha_u));

            RGB hue_rgb = hsv_to_rgb({ state.hue * 360.0, 1.0, 1.0 });
            fg_draw->AddRectFilledMultiColor(ImVec2(pos.x + 30.0f, pos.y + 7.0f), ImVec2(pos.x + 180.0f, pos.y + 163.0f),
                IM_COL32(255, 255, 255, alpha_u), IM_COL32(static_cast<int>(hue_rgb.r * 255), static_cast<int>(hue_rgb.g * 255), static_cast<int>(hue_rgb.b * 255), alpha_u),
                IM_COL32(255, 255, 255, alpha_u), IM_COL32(static_cast<int>(hue_rgb.r * 255), static_cast<int>(hue_rgb.g * 255), static_cast<int>(hue_rgb.b * 255), alpha_u));

            fg_draw->AddRectFilledMultiColor(ImVec2(pos.x + 30.0f, pos.y + 7.0f), ImVec2(pos.x + 180.0f, pos.y + 163.0f),
                IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0),
                IM_COL32(0, 0, 0, 255), IM_COL32(0, 0, 0, 255));

            fg_draw->AddRectFilled(ImVec2(pos.x + 30.0f + state.cursor_pos.x - 2.0f, pos.y + 7.0f + state.cursor_pos.y - 2.0f),
                ImVec2(pos.x + 30.0f + state.cursor_pos.x + 2.0f, pos.y + 7.0f + state.cursor_pos.y + 2.0f),
                IM_COL32(130, 130, 170, alpha_u));

            fg_draw->AddRect(ImVec2(pos.x + 30.0f, pos.y + 167.0f), ImVec2(pos.x + 180.0f, pos.y + 175.0f), IM_COL32(15, 15, 15, alpha_u));

            for (int i = 0; i < 6; ++i) {
                fg_draw->AddRectFilledMultiColor(ImVec2(pos.x + 30.0f + i * 25.0f, pos.y + 167.0f), ImVec2(pos.x + 30.0f + (i + 1) * 25.0f, pos.y + 175.0f),
                    bar_colors[i], bar_colors[i + 1],
                    bar_colors[i], bar_colors[i + 1]);
            }

            fg_draw->AddRectFilled(ImVec2(pos.x + 30.0f + (150.0f * state.hue), pos.y + 166.0f),
                ImVec2(pos.x + 33.0f + (150.0f * state.hue), pos.y + 177.0f),
                IM_COL32(154, 152, 155, alpha_u));
            fg_draw->AddRect(ImVec2(pos.x + 29.0f + (150.0f * state.hue), pos.y + 165.0f),
                ImVec2(pos.x + 33.0f + (150.0f * state.hue), pos.y + 177.0f),
                IM_COL32(15, 15, 15, alpha_u));

            if (show_alpha) {
                fg_draw->AddRect(ImVec2(pos.x + 187.0f, pos.y + 7.0f), ImVec2(pos.x + 195.0f, pos.y + 162.0f), IM_COL32(15, 15, 15, alpha_u));
                fg_draw->AddRectFilledMultiColor(ImVec2(pos.x + 187.0f, pos.y + 7.0f), ImVec2(pos.x + 195.0f, pos.y + 162.0f),
                    IM_COL32(color->r, color->g, color->b, 0), IM_COL32(color->r, color->g, color->b, 0),
                    IM_COL32(color->r, color->g, color->b, 255), IM_COL32(color->r, color->g, color->b, 255));

                fg_draw->AddRectFilled(ImVec2(pos.x + 186.0f, pos.y + 7.0f + (155.0f * state.alpha)),
                    ImVec2(pos.x + 197.0f, pos.y + 10.0f + (155.0f * state.alpha)),
                    IM_COL32(153, 153, 159, alpha_u));
                fg_draw->AddRect(ImVec2(pos.x + 185.0f, pos.y + 6.0f + (155.0f * state.alpha)),
                    ImVec2(pos.x + 197.0f, pos.y + 10.0f + (155.0f * state.alpha)),
                    IM_COL32(15, 15, 15, alpha_u));

                fg_draw->AddRectFilled(ImVec2(pos.x + 187.0f, pos.y + 7.0f + (155.0f * state.alpha)),
                    ImVec2(pos.x + 195.0f, pos.y + 8.0f + (155.0f * state.alpha)),
                    IM_COL32(153, 153, 159, alpha_u));
                fg_draw->AddRectFilled(ImVec2(pos.x + 187.0f, pos.y + 6.0f + (155.0f * state.alpha)),
                    ImVec2(pos.x + 195.0f, pos.y + 7.0f + (155.0f * state.alpha)),
                    IM_COL32(153, 153, 159, alpha_u));
            }

            if (state.open) {
                ImRect palette_rect(ImVec2(pos.x + 30.0f, pos.y + 6.0f), ImVec2(pos.x + 180.0f, pos.y + 162.0f));
                if (ImGui::IsMouseHoveringRect(palette_rect.Min, palette_rect.Max) && ImGui::IsMouseDown(0)) {
                    state.cursor_pos = ImGui::GetMousePos() - palette_rect.Min;
                    state.cursor_pos.x = ImClamp(state.cursor_pos.x, 0.0f, 150.0f);
                    state.cursor_pos.y = ImClamp(state.cursor_pos.y, 0.0f, 150.0f);
                }

                ImRect hue_rect(ImVec2(pos.x + 30.0f, pos.y + 167.0f), ImVec2(pos.x + 180.0f, pos.y + 175.0f));
                if (ImGui::IsMouseHoveringRect(hue_rect.Min, hue_rect.Max) && ImGui::IsMouseDown(0)) {
                    state.hue = ImClamp((ImGui::GetMousePos().x - hue_rect.Min.x) / 150.0f, 0.0f, 1.0f);
                }

                if (show_alpha) {
                    ImRect alpha_rect(ImVec2(pos.x + 187.0f, pos.y + 7.0f), ImVec2(pos.x + 195.0f, pos.y + 162.0f));
                    if (ImGui::IsMouseHoveringRect(alpha_rect.Min, alpha_rect.Max) && ImGui::IsMouseDown(0)) {
                        state.alpha = ImClamp((ImGui::GetMousePos().y - alpha_rect.Min.y) / 155.0f, 0.0f, 1.0f);
                    }
                }

                float sat = state.cursor_pos.x / 150.0f;
                float val = 1.0f - (state.cursor_pos.y / 150.0f);
                color->FromHSV(state.hue, sat, val, state.alpha);

                changed = true;

                ImRect full_rect(ImVec2(pos.x + 23.0f, pos.y), ImVec2(pos.x + 203.0f, pos.y + (show_alpha ? 180.0f : 164.0f)));
                if (ImGui::IsMouseClicked(0) && !ImGui::IsMouseHoveringRect(full_rect.Min, full_rect.Max)) {
                    state.open = false;
                }
            }
        }

        ImGui::SetCursorScreenPos(pos);

        return changed;
    }
}