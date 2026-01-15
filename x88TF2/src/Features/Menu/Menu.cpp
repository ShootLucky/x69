#include "Menu.h"
#include "../src/Features/Menu/functions_menu/functions_menu.h"
#include <vector>
#include <string>
#include <cstdio>
#include "../src/SDK/SDK.h"
#include "../../Features/PlayersList/PlayersList.h"
#include <functional>
#include <windows.h>

// Added for menu toggle
bool menu::is_open = false;

// Add for hitscan section scroll
namespace menu {
    int scroll_offset_hitscan = 0;
    bool hitscan_scrollbar_dragging = false;
    int hitscan_scrollbar_drag_start_y = 0;
    int hitscan_scrollbar_drag_start_offset = 0;
}

void RenderAimbotTab(int left_x, int right_x, int content_y_start, int section_width, int line_height,
    int section_padding, int item_indent, int gap, Color_t bg_section, Color_t bg_item_selected,
    Color_t border, Color_t blue_title, Color_t blue_accent1, Color_t blue_accent2,
    Color_t text_white, Color_t text_gray, Color_t text_dark) {

    // ============= APLICAR SCROLL OFFSET =============
    int scroll_offset = menu::GetCurrentScrollOffset();
    int content_y = content_y_start - scroll_offset;
    int content_start_y = content_y; // Guardar posição inicial para calcular altura total

    menu::item_count = 0; // Reset item count for keyboard navigation

    // ========== COLUNA ESQUERDA ==========
    // Seção HITSCAN com altura fixa e scroll interno
    int fixed_hitscan_height = 460; // Ajuste se necessário para caber na tela
    H::Draw->Rect(left_x, content_y, section_width, fixed_hitscan_height, bg_section);
    H::Draw->OutlinedRect(left_x, content_y, section_width, fixed_hitscan_height, border);
    H::Draw->Rect(left_x, content_y, section_width, line_height + 4, blue_title);
    H::Draw->String(H::Fonts->Get(EFonts::Menu), left_x + item_indent, content_y + 5, text_white, POS_DEFAULT, "HITSCAN");

    int title_height = line_height + 4;
    int items_visible_height = fixed_hitscan_height - title_height - section_padding * 2; // Ajustado padding
    int items_start_y = content_y + title_height + section_padding;

    int items_height = 500; // Ajuste este valor se necessário após testes (aumente se faltar scroll, diminua se excessivo)

    int max_scroll = std::max(0, items_height - items_visible_height);
    menu::scroll_offset_hitscan = std::clamp(menu::scroll_offset_hitscan, 0, max_scroll);

    // Ativar clipping para a área de itens (apenas dentro da seção HITSCAN)
    I::MatSystemSurface->DisableClipping(false);
    I::MatSystemSurface->SetClippingRect(left_x, items_start_y, left_x + section_width, items_start_y + items_visible_height);

    // Renderizar itens com offset de scroll interno
    int items_y = items_start_y - menu::scroll_offset_hitscan;
    int x_left_val = left_x + item_indent;
    key_selector(x_left_val, &items_y, &CFG::Aimbot_Key, menu::item_count++);
    checkbox(x_left_val, &items_y, "Hitscan", &CFG::Aimbot_Enable, false, 255, menu::item_count++);
    checkbox(x_left_val, &items_y, "Visible Check", &CFG::Aimbot_VisibleCheck, false, 255, menu::item_count++);
    checkbox(x_left_val, &items_y, "Team Check", &CFG::Aimbot_TeamCheck, false, 255, menu::item_count++);
    checkbox(x_left_val, &items_y, "Target Lag Records", &CFG::Aimbot_TargetLagRecords, false, 255, menu::item_count++);
    checkbox(x_left_val, &items_y, "Target Stickies", &CFG::Aimbot_TargetStickies, false, 255, menu::item_count++);
    checkbox(x_left_val, &items_y, "Smooth Auto Shoot", &CFG::Aimbot_SmoothAutoShoot, false, 255, menu::item_count++);
    checkbox(x_left_val, &items_y, "Wait For Headshot", &CFG::Aimbot_WaitForHeadshot, false, 255, menu::item_count++);
    checkbox(x_left_val, &items_y, "Wait For Charge", &CFG::Aimbot_WaitForCharge, false, 255, menu::item_count++);
    checkbox(x_left_val, &items_y, "Minigun Tapfire", &CFG::Aimbot_MinigunTapfire, false, 255, menu::item_count++);
    combo(x_left_val, &items_y, "Aim Type", &CFG::Aimbot_Hitscan_Mode, { "Aimlock", "Silent" }, false, 255, menu::item_count++);
    combo(x_left_val, &items_y, "Sort", &CFG::Aimbot_Hitscan_Sort, { "Distance", "FOV", "Health" }, false, 255, menu::item_count++);
    float_slider(x_left_val, &items_y, "FOV", CFG::Aimbot_FOV, 0.f, 180.f, menu::item_count++);
    float_slider(x_left_val, &items_y, "Smoothing", CFG::Aimbot_Hitscan_Smoothing, 0.f, 20.f, menu::item_count++);
    checkbox(x_left_val, &items_y, "Ignore Invisible", &CFG::Aimbot_Ignore_Invisible, false, 255, menu::item_count++);
    checkbox(x_left_val, &items_y, "Ignore Taunting", &CFG::Aimbot_Ignore_Taunting, false, 255, menu::item_count++);
    checkbox(x_left_val, &items_y, "Active Lag Records", &CFG::Aimbot_ActiveLagRecords, false, 255, menu::item_count++);
    checkbox(x_left_val, &items_y, "Auto Shoot", &CFG::Aimbot_AutoShoot, false, 255, menu::item_count++);
    checkbox(x_left_val, &items_y, "Auto Scope", &CFG::Aimbot_AutoScope, false, 255, menu::item_count++);
    checkbox(x_left_val, &items_y, "Target Players", &CFG::Aimbot_Target_Players, false, 255, menu::item_count++);
    checkbox(x_left_val, &items_y, "Target Buildings", &CFG::Aimbot_Target_Buildings, false, 255, menu::item_count++);
    checkbox(x_left_val, &items_y, "Ignore Invulnerable", &CFG::Aimbot_Ignore_Invulnerable, false, 255, menu::item_count++);

    // Desativar clipping da seção e restaurar o clipping principal do tab
    I::MatSystemSurface->DisableClipping(false);
    I::MatSystemSurface->SetClippingRect(menu::menu_x, menu::menu_y + 70, menu::menu_x + 720, menu::menu_y + 70 + menu::content_visible_height);

    // Desenhar scrollbar interno se necessário
    if (max_scroll > 0) {
        int scrollbar_x = left_x + section_width - 8;
        int scrollbar_y = items_start_y;
        int scrollbar_width = 6;
        int scrollbar_height = items_visible_height;

        // Background da scrollbar
        H::Draw->Rect(scrollbar_x, scrollbar_y, scrollbar_width, scrollbar_height, Color_t(25, 25, 28, 255));

        // Calcular thumb
        float visible_ratio = (float)items_visible_height / (float)items_height;
        int thumb_height = std::max(30, (int)(scrollbar_height * visible_ratio));

        float scroll_ratio = (float)menu::scroll_offset_hitscan / (float)max_scroll;
        int thumb_y = scrollbar_y + (int)((scrollbar_height - thumb_height) * scroll_ratio);

        bool thumb_hover = menu::IsMouseInRect(scrollbar_x - 2, thumb_y, scrollbar_width + 4, thumb_height);
        Color_t thumb_color = thumb_hover || menu::hitscan_scrollbar_dragging ? Color_t(85, 150, 255, 200) : Color_t(65, 65, 70, 180);

        H::Draw->Rect(scrollbar_x, thumb_y, scrollbar_width, thumb_height, thumb_color);

        // Input do scrollbar interno (mouse wheel e drag)
        bool mouse_over_section = menu::IsMouseInRect(left_x, items_start_y, section_width, items_visible_height);

        if (mouse_over_section && !menu::hitscan_scrollbar_dragging) {
            int mouse_wheel = H::Input->GetMouseScroll();
            if (mouse_wheel != 0) {
                menu::scroll_offset_hitscan -= mouse_wheel * 20;
                menu::scroll_offset_hitscan = std::clamp(menu::scroll_offset_hitscan, 0, max_scroll);
            }
        }

        bool left_mouse_down = H::Input->IsDown(VK_LBUTTON);

        if (!menu::hitscan_scrollbar_dragging) {
            if (left_mouse_down && thumb_hover) {
                menu::hitscan_scrollbar_dragging = true;
                menu::hitscan_scrollbar_drag_start_y = H::Input->GetMouseY();
                menu::hitscan_scrollbar_drag_start_offset = menu::scroll_offset_hitscan;
            }
        }
        else {
            if (left_mouse_down) {
                int mouse_y = H::Input->GetMouseY();
                int delta_y = mouse_y - menu::hitscan_scrollbar_drag_start_y;

                float scroll_range = scrollbar_height - thumb_height;
                float scroll_per_pixel = (float)max_scroll / scroll_range;

                menu::scroll_offset_hitscan = menu::hitscan_scrollbar_drag_start_offset + (int)(delta_y * scroll_per_pixel);
                menu::scroll_offset_hitscan = std::clamp(menu::scroll_offset_hitscan, 0, max_scroll);
            }
            else {
                menu::hitscan_scrollbar_dragging = false;
            }
        }
    }

    content_y += fixed_hitscan_height + 10; // Avançar para próxima seção (se houver)

    int left_end_y = content_y;

    // ========== COLUNA DIREITA ==========
    content_y = content_y_start - scroll_offset;
    int x_right_val = right_x + item_indent;

    // Seção HITBOX
    int hitbox_height = 15 * 8 + section_padding * 2 + 4;
    H::Draw->Rect(right_x, content_y, section_width, hitbox_height, bg_section);
    H::Draw->OutlinedRect(right_x, content_y, section_width, hitbox_height, border);
    H::Draw->Rect(right_x, content_y, section_width, line_height + 4, blue_title);
    H::Draw->String(H::Fonts->Get(EFonts::Menu), right_x + item_indent, content_y + 5, text_white, POS_DEFAULT, "HITBOX");
    content_y += line_height + section_padding + 4;

    combo(x_right_val, &content_y, "Hitbox Sort", &CFG::Aimbot_Hitbox_Sort, { "Auto", "Damage", "Accuracy" }, false, 255, menu::item_count++);
    multi_combo(x_right_val, &content_y, "Hitbox Types", &CFG::Aimbot_Hitscan_Hitbox, { "Head", "Body", "Pelvis", "Arms", "Legs" }, 255, menu::item_count++);
    checkbox(x_right_val, &content_y, "Scan Buildings", &CFG::Aimbot_Hitscan_Scan_Buildings, false, 255, menu::item_count++);

    content_y += 10;

    // Seção EXPLOITS
    int exploits_height = 15 * 3 + section_padding * 2 + 4;
    H::Draw->Rect(right_x, content_y, section_width, exploits_height, bg_section);
    H::Draw->OutlinedRect(right_x, content_y, section_width, exploits_height, border);
    H::Draw->Rect(right_x, content_y, section_width, line_height + 4, blue_title);
    H::Draw->String(H::Fonts->Get(EFonts::Menu), right_x + item_indent, content_y + 5, text_white, POS_DEFAULT, "EXPLOITS");
    content_y += line_height + section_padding + 4;

    text(x_right_val, &content_y, "soon...", regular);

    int right_end_y = content_y;

    // ============= CALCULAR ALTURA TOTAL DO CONTEÚDO =============
    menu::total_content_height = std::max(left_end_y, right_end_y) - content_start_y + scroll_offset + 20;
}

void RenderAntiaimTab(int left_x, int right_x, int content_y_start, int section_width, int line_height,
    int section_padding, int item_indent, int gap, Color_t bg_section, Color_t bg_item_selected,
    Color_t border, Color_t blue_title, Color_t blue_accent1, Color_t blue_accent2,
    Color_t text_white, Color_t text_gray, Color_t text_dark) {

    // ============= APLICAR SCROLL OFFSET =============
    int scroll_offset = menu::GetCurrentScrollOffset();
    int content_y = content_y_start - scroll_offset;
    int content_start_y = content_y;

    menu::item_count = 0;

    int section_height = 15 * 8 + section_padding * 2;
    H::Draw->Rect(left_x, content_y, section_width, section_height, bg_section);
    H::Draw->OutlinedRect(left_x, content_y, section_width, section_height, border);
    text(left_x + item_indent, &content_y, "ANTI-AIM", info);
    text(left_x + item_indent, &content_y, "Anti-aim settings will be here...", regular);

    // ============= CALCULAR ALTURA TOTAL =============
    menu::total_content_height = content_y - content_start_y + scroll_offset + 20;
}

void RenderVisualsTab(int left_x, int right_x, int content_y_start, int section_width, int line_height,
    int section_padding, int item_indent, int gap, Color_t bg_section, Color_t bg_item_selected,
    Color_t border, Color_t blue_title, Color_t blue_accent1, Color_t blue_accent2,
    Color_t text_white, Color_t text_gray, Color_t text_dark) {

    // ============= APLICAR SCROLL OFFSET =============
    int scroll_offset = menu::GetCurrentScrollOffset();
    int content_y = content_y_start - scroll_offset;
    int content_start_y = content_y;

    menu::item_count = 0;

    // Left column: ESP Players section
    int players_section_height = 15 * 28 + section_padding * 2 + 4;
    H::Draw->Rect(left_x, content_y, section_width, players_section_height, bg_section);
    H::Draw->OutlinedRect(left_x, content_y, section_width, players_section_height, border);

    H::Draw->Rect(left_x, content_y, section_width, line_height + 4, blue_title);
    H::Draw->String(H::Fonts->Get(EFonts::ESP), left_x + item_indent, content_y + 5, text_white, POS_DEFAULT, "ESP PLAYERS");
    content_y += line_height + section_padding + 4;

    int x_left = left_x + item_indent;

    checkbox(x_left, &content_y, "ESP Master", &CFG::ESP_Enable, false, 255, menu::item_count++);
    checkbox(x_left, &content_y, "Team Check", &CFG::ESP_Team, false, 255, menu::item_count++);
    checkbox(x_left, &content_y, "Box", &CFG::ESP_Box, false, 255, menu::item_count++);
    combo(x_left, &content_y, "Box Style", &CFG::ESP_BoxType, { "2D", "3D", "Corner" }, false, 255, menu::item_count++);
    checkbox(x_left, &content_y, "Name", &CFG::ESP_Name, false, 255, menu::item_count++);
    checkbox(x_left, &content_y, "Health", &CFG::ESP_Health, false, 255, menu::item_count++);
    combo(x_left, &content_y, "Health Type", &CFG::ESP_HealthType, { "Health bar", "Health number", "Number + bar" }, false, 255, menu::item_count++);
    checkbox(x_left, &content_y, "World Pickups", &CFG::ESP_Pickups, false, 255, menu::item_count++);
    checkbox(x_left, &content_y, "Pickups Box", &CFG::ESP_PickupsBox, false, 255, menu::item_count++);
    checkbox(x_left, &content_y, "Pickups Name", &CFG::ESP_PickupsName, false, 255, menu::item_count++);
    checkbox(x_left, &content_y, "Flag ESP", &CFG::ESP_CaptureFlag, false, 255, menu::item_count++);
    checkbox(x_left, &content_y, "Box Capture", &CFG::ESP_BoxCapture, false, 255, menu::item_count++);
    checkbox(x_left, &content_y, "Name Capture", &CFG::ESP_NameCapture, false, 255, menu::item_count++);
    checkbox(x_left, &content_y, "Show Local Player", &CFG::ESP_LocalPlayer, false, 255, menu::item_count++);
    checkbox(x_left, &content_y, "Hide Cloaked Players", &CFG::ESP_HideCloaked, false, 255, menu::item_count++);
    checkbox(x_left, &content_y, "Aimbot FOV", &CFG::Aimbot_DrawFOV, false, 255, menu::item_count++);
    checkbox(x_left, &content_y, "Player Conditions", &CFG::ESP_Conds, false, 255, menu::item_count++);
    checkbox(x_left, &content_y, "ESP Sniper Lines", &CFG::ESP_SniperLines, false, 255, menu::item_count++);

    // ========== COLUNA DIREITA ==========
    // Reset para coluna direita com scroll
    content_y = content_y_start - scroll_offset;

    // Right column: ESP Other section
    int other_section_height = 15 * 28 + section_padding * 2 + 4;
    H::Draw->Rect(right_x, content_y, section_width, other_section_height, bg_section);
    H::Draw->OutlinedRect(right_x, content_y, section_width, other_section_height, border);

    H::Draw->Rect(right_x, content_y, section_width, line_height + 4, blue_title);
    H::Draw->String(H::Fonts->Get(EFonts::Menu), right_x + item_indent, content_y + 5, text_white, POS_DEFAULT, "ESP OTHER");
    content_y += line_height + section_padding + 4;

    int x_center = right_x + item_indent;

    checkbox(x_center, &content_y, "Player Tracers", &CFG::ESP_Tracer, false, 255, menu::item_count++);
    checkbox(x_center, &content_y, "UberCharge Status", &CFG::ESP_Uber, false, 255, menu::item_count++);
    checkbox(x_center, &content_y, "UberCharge Bar", &CFG::ESP_UberBar, false, 255, menu::item_count++);
    checkbox(x_center, &content_y, "Distance Enemy", &CFG::ESP_DistanceEnemy, false, 255, menu::item_count++);
    combo(x_center, &content_y, "Distance Position", &CFG::ESP_DistancePosition, { "Side", "Bottom" }, false, 255, menu::item_count++);
    checkbox(x_center, &content_y, "Buffs", &CFG::ESP_Buffs, false, 255, menu::item_count++);
    checkbox(x_center, &content_y, "Debuffs", &CFG::ESP_Debuffs, false, 255, menu::item_count++);
    checkbox(x_center, &content_y, "Latency (Ping)", &CFG::ESP_Ping, false, 255, menu::item_count++);
    checkbox(x_center, &content_y, "KDR Player", &CFG::ESP_KRDPlayer, false, 255, menu::item_count++);
    checkbox(x_center, &content_y, "Lag Compensation", &CFG::ESP_LagCompensation, false, 255, menu::item_count++);
    checkbox(x_center, &content_y, "ESP Build", &CFG::ESP_Build, false, 255, menu::item_count++);
    checkbox(x_center, &content_y, "ESP Build Only Enemy", &CFG::ESP_BuildOnlyEnemy, false, 255, menu::item_count++);
    checkbox(x_center, &content_y, "Offscreen Indicators", &CFG::ESP_Offscreen, false, 255, menu::item_count++);
    float_slider(x_center, &content_y, "Offscreen Radius", CFG::ESP_Offscreen_Radius, 10.0f, 500.0f, menu::item_count++);
    float_slider(x_center, &content_y, "Offscreen Max Distance", CFG::ESP_Offscreen_MaxDist, 0.0f, 2000.0f, menu::item_count++);
    combo(x_center, &content_y, "Offscreen Style", &CFG::ESP_Offscreen_Style, { "Triangle", "Circle", "Bar" }, false, 255, menu::item_count++);
    checkbox(x_center, &content_y, "Offscreen Filled", &CFG::ESP_Offscreen_Filled, false, 255, menu::item_count++);

    // ============= CALCULAR ALTURA TOTAL =============
    menu::total_content_height = content_y - content_start_y + scroll_offset + 20;
}

void RenderSkinsTab(int left_x, int right_x, int content_y_start, int section_width, int line_height,
    int section_padding, int item_indent, int gap, Color_t bg_section, Color_t bg_item_selected,
    Color_t border, Color_t blue_title, Color_t blue_accent1, Color_t blue_accent2,
    Color_t text_white, Color_t text_gray, Color_t text_dark) {

    // ============= APLICAR SCROLL OFFSET =============
    int scroll_offset = menu::GetCurrentScrollOffset();
    int content_y = content_y_start - scroll_offset;
    int content_start_y = content_y;

    menu::item_count = 0;

    int section_height = 15 * 8 + section_padding * 2;
    H::Draw->Rect(left_x, content_y, section_width, section_height, bg_section);
    H::Draw->OutlinedRect(left_x, content_y, section_width, section_height, border);
    text(left_x + item_indent, &content_y, "SKINS", info);
    text(left_x + item_indent, &content_y, "Skins settings will be here...", regular);

    // ============= CALCULAR ALTURA TOTAL =============
    menu::total_content_height = content_y - content_start_y + scroll_offset + 20;
}

void RenderMiscTab(int left_x, int right_x, int content_y_start, int section_width, int line_height,
    int section_padding, int item_indent, int gap, Color_t bg_section, Color_t bg_item_selected,
    Color_t border, Color_t blue_title, Color_t blue_accent1, Color_t blue_accent2,
    Color_t text_white, Color_t text_gray, Color_t text_dark) {

    // ============= APLICAR SCROLL OFFSET =============
    int scroll_offset = menu::GetCurrentScrollOffset();
    int content_y = content_y_start - scroll_offset;
    int content_start_y = content_y;

    menu::item_count = 0;

    int section_height = 15 * 8 + section_padding * 2;
    H::Draw->Rect(left_x, content_y, section_width, section_height, bg_section);
    H::Draw->OutlinedRect(left_x, content_y, section_width, section_height, border);
    text(left_x + item_indent, &content_y, "MISC", info);
    text(left_x + item_indent, &content_y, "Misc settings will be here...", regular);

    // ============= CALCULAR ALTURA TOTAL =============
    menu::total_content_height = content_y - content_start_y + scroll_offset + 20;
}

void menu::render() {
    // Toggle menu with INSERT
    static bool last_insert = false; bool insert_pressed = H::Input->IsPressed(VK_INSERT); if (insert_pressed && !last_insert) {
        is_open = !is_open;
        // show/hide cursor in surface
        I::MatSystemSurface->SetCursorAlwaysVisible(is_open);

        // tell the menu singleton that we want text/input (this makes WndProc block game input)
        F::Menu->m_bWantTextInput = is_open;

        // always reset input state when toggling to avoid residual mouse deltas / clicks
        I::InputSystem->ResetInputState();

        if (!is_open) {
            // clear any keybind mode when closing to avoid sticky keybind capture
            F::Menu->m_bInKeybind = false;
            // do NOT call SetCursorPos(saved_mouse_x, saved_mouse_y) — avoids camera jumps
        }
    }
    last_insert = insert_pressed;

    if (!is_open) {
        return;
    }

    // Handle drag and tab input
    HandleDrag();
    HandleTabClick();

    int screen[2];
    auto pLocal = H::Entities->GetLocal();
    I::EngineClient->GetScreenSize(screen[0], screen[1]);

    int menu_width = 720;
    int menu_height = 560;

    // Colors
    Color_t bg_main = Color_t(52, 52, 52, 255);
    Color_t bg_section = Color_t(48, 48, 48, 255);
    Color_t bg_item_selected = Color_t(58, 58, 58, 255);
    Color_t bg_header = Color_t(15, 15, 15, 255);
    Color_t blue_title = Color_t(0, 145, 255, 255);
    Color_t blue_accent1 = Color_t(0, 160, 255, 255);
    Color_t blue_accent2 = Color_t(255, 140, 0, 255);
    Color_t text_white = Color_t(255, 255, 255, 255);
    Color_t text_gray = Color_t(170, 170, 170, 255);
    Color_t text_dark = Color_t(110, 110, 110, 255);
    Color_t separator = Color_t(58, 58, 58, 255);
    Color_t border = Color_t(12, 12, 12, 255);

    // Background
    H::Draw->Rect(menu::menu_x, menu::menu_y, menu_width, menu_height, bg_main);
    H::Draw->OutlinedRect(menu::menu_x, menu::menu_y, menu_width, menu_height, border);

    // Header
    int header_height = 48;
    H::Draw->Rect(menu::menu_x, menu::menu_y, menu_width, header_height, bg_header);
    H::Draw->Rect(menu::menu_x, menu::menu_y + header_height - 1, menu_width, 1, separator);

    // Logo
    int logo_x = menu::menu_x + 10;
    int logo_y = menu::menu_y + 8;
    H::Draw->Rect(logo_x, logo_y, 22, 30, blue_title);
    H::Draw->String(H::Fonts->Get(EFonts::Menu), logo_x + 6, logo_y + 6, text_white, POS_DEFAULT, "N");
    H::Draw->String(H::Fonts->Get(EFonts::Menu), logo_x + 30, logo_y + 3, text_white, POS_DEFAULT, "EMESIS");
    H::Draw->String(H::Fonts->Get(EFonts::Menu), logo_x + 30, logo_y + 17, text_dark, POS_DEFAULT, "DEVELOPED BY REIS");

    // ============= TABS WITH ACTIVE INDICATOR =============
    int tab_x_start = menu::menu_x + 320;
    int tab_x = tab_x_start;
    int tab_y = menu::menu_y + 14;
    int tab_spacing = 75;

    // Aimbot tab
    Color_t tab_color = (current_tab == Tab::AIMBOT) ? blue_accent1 : text_gray;
    H::Draw->String(H::Fonts->Get(EFonts::Menu), tab_x, tab_y, tab_color, POS_DEFAULT, "Aimbot");
    if (current_tab == Tab::AIMBOT) {
        H::Draw->Rect(tab_x, tab_y + 18, 48, 2, blue_accent1);
    }

    // Anti-aim tab
    tab_x += tab_spacing;
    tab_color = (current_tab == Tab::ANTIAIM) ? blue_accent1 : text_gray;
    H::Draw->String(H::Fonts->Get(EFonts::Menu), tab_x, tab_y, tab_color, POS_DEFAULT, "TrigerBot");
    if (current_tab == Tab::ANTIAIM) {
        H::Draw->Rect(tab_x, tab_y + 18, 60, 2, blue_accent1);
    }

    // Visuals tab
    tab_x += tab_spacing;
    tab_color = (current_tab == Tab::VISUALS) ? blue_accent1 : text_gray;
    H::Draw->String(H::Fonts->Get(EFonts::Menu), tab_x, tab_y, tab_color, POS_DEFAULT, "Visuals");
    if (current_tab == Tab::VISUALS) {
        H::Draw->Rect(tab_x, tab_y + 18, 50, 2, blue_accent1);
    }

    // Skins tab
    tab_x += tab_spacing;
    tab_color = (current_tab == Tab::SKINS) ? blue_accent1 : text_gray;
    H::Draw->String(H::Fonts->Get(EFonts::Menu), tab_x, tab_y, tab_color, POS_DEFAULT, "Player List");
    if (current_tab == Tab::SKINS) {
        H::Draw->Rect(tab_x, tab_y + 18, 40, 2, blue_accent1);
    }

    // Misc tab
    tab_x += 58;
    tab_color = (current_tab == Tab::MISC) ? blue_accent1 : text_gray;
    H::Draw->String(H::Fonts->Get(EFonts::Menu), tab_x, tab_y, tab_color, POS_DEFAULT, "Misc");
    if (current_tab == Tab::MISC) {
        H::Draw->Rect(tab_x, tab_y + 18, 35, 2, blue_accent1);
    }

    // Layout
    int left_x = menu::menu_x + 18;
    int right_x = menu::menu_x + (menu_width / 2) + 12;
    int content_y_start = menu::menu_y + header_height + 18;
    int line_height = 22;
    int section_padding = 12;
    int item_indent = 12;
    int gap = 14;
    int section_width = (menu_width / 2) - 36;

    // ============= ATIVAR CLIPPING (SCISSOR RECT) =============
    int content_x = menu::menu_x;
    int content_y = menu::menu_y + 70;
    int content_width = 720;
    int content_height = menu::content_visible_height;

    // Ativa clipping e define o retângulo
    I::MatSystemSurface->DisableClipping(false);
    I::MatSystemSurface->SetClippingRect(
        content_x,
        content_y,
        content_x + content_width,
        content_y + content_height
    );

    // ============= RENDER CURRENT TAB CONTENT =============
    switch (current_tab) {
    case Tab::AIMBOT:
        RenderAimbotTab(left_x, right_x, content_y_start, section_width, line_height, section_padding,
            item_indent, gap, bg_section, bg_item_selected, border, blue_title, blue_accent1,
            blue_accent2, text_white, text_gray, text_dark);
        break;
    case Tab::ANTIAIM:
        RenderAntiaimTab(left_x, right_x, content_y_start, section_width, line_height, section_padding,
            item_indent, gap, bg_section, bg_item_selected, border, blue_title, blue_accent1,
            blue_accent2, text_white, text_gray, text_dark);
        break;
    case Tab::VISUALS:
        RenderVisualsTab(left_x, right_x, content_y_start, section_width, line_height, section_padding,
            item_indent, gap, bg_section, bg_item_selected, border, blue_title, blue_accent1,
            blue_accent2, text_white, text_gray, text_dark);
        break;
    case Tab::SKINS:
        RenderSkinsTab(left_x, right_x, content_y_start, section_width, line_height, section_padding,
            item_indent, gap, bg_section, bg_item_selected, border, blue_title, blue_accent1,
            blue_accent2, text_white, text_gray, text_dark);
        break;
    case Tab::MISC:
        RenderMiscTab(left_x, right_x, content_y_start, section_width, line_height, section_padding,
            item_indent, gap, bg_section, bg_item_selected, border, blue_title, blue_accent1,
            blue_accent2, text_white, text_gray, text_dark);
        break;
    }

    // ============= DESATIVAR CLIPPING =============
    I::MatSystemSurface->DisableClipping(true);
}