#include "functions_menu.h"
#include "../Menu.h"
#include <sstream>
#include <iomanip>
#include <windows.h>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include "../../../Features/PlayersList/PlayersList.h"
#include "../../../CFG.h"
#include "../../../Utils/Storage/Storage.h"

// Moved namespace menu static definitions
std::string menu::tauntMessage;
float menu::tauntEndTime;
text_type menu::tauntType;
std::string menu::config_name = "default";
bool menu::editing_config_name[256] = {};
std::vector<std::string> menu::config_files;
int menu::current_config_index = 0;
int menu::item_count = 1;
int menu::item_countx3 = 12;

// ============= TAB E DRAG VARIABLES =============
menu::Tab menu::current_tab = menu::Tab::AIMBOT;
int menu::menu_x = 70;
int menu::menu_y = 30;
bool menu::is_dragging = false;
int menu::drag_offset_x = 0;
int menu::drag_offset_y = 0;

// Estado global para dropdowns
static bool dropdown_open[256] = {};
static int dropdown_scroll[256] = {};
static const int MAX_VISIBLE_ITEMS = 8;

// ============= KEYBIND SYSTEM =============
struct KeybindState {
    int key;
    int mode;  // 0 = Always, 1 = Hold, 2 = Toggle, 3 = Off
    bool popup_open;
};

static KeybindState keybind_states[256] = {};
static const std::vector<std::string> keybind_modes = {
    "Always",
    "Hold",
    "Toggle",
    "Off"
};

// ============= SCROLL SYSTEM =============
namespace menu {
    // Variáveis de scroll por tab
    int scroll_offset_aimbot = 0;
    int scroll_offset_antiaim = 0;
    int scroll_offset_visuals = 0;
    int scroll_offset_skins = 0;
    int scroll_offset_misc = 0;

    // Área visível de conteúdo
    int content_visible_height = 460;
    int total_content_height = 0;

    // ScrollBar
    bool scrollbar_dragging = false;
    int scrollbar_drag_start_y = 0;
    int scrollbar_drag_start_offset = 0;

    int& GetCurrentScrollOffset() {
        switch (current_tab) {
        case Tab::AIMBOT: return scroll_offset_aimbot;
        case Tab::ANTIAIM: return scroll_offset_antiaim;
        case Tab::VISUALS: return scroll_offset_visuals;
        case Tab::SKINS: return scroll_offset_skins;
        case Tab::MISC: return scroll_offset_misc;
        default: return scroll_offset_aimbot;
        }
    }

    void ResetScrollOnTabChange() {
        scrollbar_dragging = false;
    }
}

std::string GetKeyName(int vk) {
    if (vk == 0) return "NONE";
    switch (vk) {
    case VK_LBUTTON: return "MOUSE1";
    case VK_RBUTTON: return "MOUSE2";
    case VK_MBUTTON: return "MOUSE3";
    case VK_XBUTTON1: return "MOUSE4";
    case VK_XBUTTON2: return "MOUSE5";
    case VK_SHIFT: return "SHIFT";
    case VK_CONTROL: return "CTRL";
    case VK_MENU: return "ALT";
    case VK_SPACE: return "SPACE";
    case VK_RETURN: return "ENTER";
    case VK_ESCAPE: return "ESC";
    }
    UINT sc = MapVirtualKeyA(static_cast<UINT>(vk), MAPVK_VK_TO_VSC);
    LONG lparam = (sc << 16);
    wchar_t namew[128] = {};
    if (GetKeyNameTextW(lparam, namew, ARRAYSIZE(namew)) > 0) {
        int needed = WideCharToMultiByte(CP_UTF8, 0, namew, -1, nullptr, 0, nullptr, nullptr);
        if (needed > 0) {
            std::string out(needed, '\0');
            WideCharToMultiByte(CP_UTF8, 0, namew, -1, out.data(), needed, nullptr, nullptr);
            if (!out.empty() && out.back() == '\0') out.pop_back();
            return out;
        }
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "VK_%02X", vk & 0xFF);
    return std::string(buf);
}

Color_t HSVToRGB(float h, float s, float v) {
    h = fmod(h, 360.0f);
    if (h < 0) h += 360.0f;
    int i = static_cast<int>(floor(h / 60.0f)) % 6;
    float f = h / 60.0f - floor(h / 60.0f);
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);
    float r, g, b;
    switch (i) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
    default: r = g = b = 0; break;
    }
    return Color_t(static_cast<unsigned char>(r * 255), static_cast<unsigned char>(g * 255), static_cast<unsigned char>(b * 255), 255);
}

void RGBToHSV(const Color_t& rgb, float& h, float& s, float& v) {
    float r = rgb.r / 255.0f;
    float g = rgb.g / 255.0f;
    float b = rgb.b / 255.0f;
    float max_val = std::max({ r, g, b });
    float min_val = std::min({ r, g, b });
    v = max_val;
    float delta = max_val - min_val;
    s = (max_val == 0.0f) ? 0.0f : delta / max_val;
    if (delta == 0.0f) {
        h = 0.0f;
    }
    else {
        if (max_val == r) {
            h = 60.0f * ((g - b) / delta + (g < b ? 6.0f : 0.0f));
        }
        else if (max_val == g) {
            h = 60.0f * ((b - r) / delta + 2.0f);
        }
        else {
            h = 60.0f * ((r - g) / delta + 4.0f);
        }
    }
}

bool IsMouseInRectHelper(int x, int y, int w, int h) {
    int mouse_x = H::Input->GetMouseX();
    int mouse_y = H::Input->GetMouseY();
    return (mouse_x >= x && mouse_x <= x + w && mouse_y >= y && mouse_y <= y + h);
}

void text(int x, int* y, std::string text_str, text_type type, int alpha, bool /*calc_height*/) {
    if (menu::menu_locked)
        alpha = static_cast<int>(alpha * 0.20f);
    Color_t text_color;
    switch (type) {
    case info: text_color = Color_t(255, 255, 0, alpha); break;
    case regular: text_color = Color_t(255, 255, 255, alpha); break;
    case enabled: text_color = Color_t(0, 0, 255, alpha); break;
    case enabled_green: text_color = Color_t(0, 255, 0, alpha); break;
    case warning: text_color = Color_t(255, 0, 0, alpha); break;
    case extra: text_color = Color_t(120, 255, 255, alpha); break;
    default: text_color = Color_t(255, 255, 255, alpha); break;
    }
    H::Draw->String(H::Fonts->Get(EFonts::Menu), x, *y, text_color, POS_DEFAULT, text_str.c_str());
    *y += 15;
}

void checkbox(int x, int* y, std::string text_str, bool* option, bool special, int alpha, int item_counts, int entindex, PlayerPriority* pri, bool use_return, bool /*calc_height*/) {
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    if (menu::menu_locked)
        alpha = 0;

    int line_height = 16;
    int checkbox_size = 10;
    int checkbox_x = x + 12;
    int checkbox_y = *y + 2;
    int text_x = checkbox_x + checkbox_size + 6;

    Color_t bg_color = Color_t(55, 55, 60, alpha);
    Color_t border_color = *option ? Color_t(0, 150, 255, alpha) : Color_t(65, 65, 70, alpha);

    H::Draw->Rect(checkbox_x, checkbox_y, checkbox_size, checkbox_size, bg_color);
    H::Draw->OutlinedRect(checkbox_x, checkbox_y, checkbox_size, checkbox_size, border_color);

    if (*option) {
        Color_t check_color = special ? Color_t(255, 70, 70, alpha) : Color_t(0, 150, 255, alpha);
        H::Draw->Rect(checkbox_x + 2, checkbox_y + 2, checkbox_size - 4, checkbox_size - 4, check_color);
    }

    Color_t text_color = Color_t(165, 165, 170, alpha);
    H::Draw->String(font, text_x, *y, text_color, POS_DEFAULT, text_str.c_str());

    static bool was_clicked = false;
    bool is_hovering = IsMouseInRectHelper(checkbox_x, checkbox_y, checkbox_size, checkbox_size);
    bool is_clicking = H::Input->IsPressed(VK_LBUTTON);

    if (!menu::menu_locked && is_hovering && is_clicking && !was_clicked) {
        *option = !*option;
        if (entindex > 0 && pri != nullptr) {
            F::Players->Mark(entindex, *pri);
        }
        was_clicked = true;
    }

    if (!is_clicking) {
        was_clicked = false;
    }

    *y += line_height;
}

void combo(int x, int* y, std::string text, int* option, std::vector<std::string> aliases, bool special, int alpha, int item_counts, bool /*calc_height*/) {
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    if (menu::menu_locked)
        alpha = 0;

    Color_t label_color = Color_t(165, 165, 170, alpha);
    H::Draw->String(font, x + 12, *y, label_color, POS_DEFAULT, text.c_str());
    *y += 16;

    int dropdown_x = x + 12;
    int dropdown_width = 268;
    int dropdown_height = 18;
    int dropdown_y = *y;

    std::string display = aliases.at(*option);

    Color_t bg_color = Color_t(48, 48, 52, alpha);
    Color_t border_color = Color_t(30, 30, 35, alpha);
    Color_t hover_color = Color_t(53, 53, 57, alpha);

    bool is_hovering = IsMouseInRectHelper(dropdown_x, dropdown_y, dropdown_width, dropdown_height);
    Color_t current_bg = is_hovering ? hover_color : bg_color;

    H::Draw->Rect(dropdown_x, dropdown_y, dropdown_width, dropdown_height, current_bg);
    H::Draw->OutlinedRect(dropdown_x, dropdown_y, dropdown_width, dropdown_height, border_color);

    H::Draw->String(font, dropdown_x + 6, dropdown_y + 3, Color_t(190, 190, 195, alpha), POS_DEFAULT, display.c_str());

    std::string arrow = dropdown_open[item_counts] ? "▲" : "▼";
    int arrow_x = dropdown_x + dropdown_width - 16;
    H::Draw->String(font, arrow_x, dropdown_y + 3, Color_t(110, 110, 115, alpha), POS_DEFAULT, arrow.c_str());

    static bool was_clicked = false;
    bool is_clicking = H::Input->IsPressed(VK_LBUTTON);

    if (!menu::menu_locked && is_hovering && is_clicking && !was_clicked) {
        dropdown_open[item_counts] = !dropdown_open[item_counts];

        if (dropdown_open[item_counts]) {
            int total_items = aliases.size();
            int visible_items = std::min(MAX_VISIBLE_ITEMS, total_items);
            int max_scroll = total_items - visible_items;
            if (max_scroll > 0) {
                int target = *option - (visible_items / 2);
                dropdown_scroll[item_counts] = std::clamp(target, 0, max_scroll);
            }
            else {
                dropdown_scroll[item_counts] = 0;
            }
        }

        was_clicked = true;
    }

    if (dropdown_open[item_counts]) {
        int list_y = dropdown_y + dropdown_height;
        int total_items = aliases.size();
        int visible_items = std::min(MAX_VISIBLE_ITEMS, total_items);
        int item_height = 18;
        int list_height = visible_items * item_height;
        bool needs_scroll = total_items > visible_items;

        int list_width = needs_scroll ? dropdown_width - 12 : dropdown_width;
        int item_width = list_width - 2;

        int start_index = dropdown_scroll[item_counts];
        if (needs_scroll) {
            start_index = std::clamp(start_index, 0, total_items - visible_items);
        }
        else {
            start_index = 0;
        }
        dropdown_scroll[item_counts] = start_index;
        int end_index = std::min(start_index + visible_items, total_items);

        H::Draw->Rect(dropdown_x, list_y, dropdown_width, list_height, Color_t(42, 42, 46, alpha));
        H::Draw->OutlinedRect(dropdown_x, list_y, dropdown_width, list_height, border_color);

        for (int i = start_index; i < end_index; ++i) {
            int item_y = list_y + (i - start_index) * item_height;
            bool item_hover = IsMouseInRectHelper(dropdown_x + 1, item_y, item_width, item_height);

            if (item_hover) {
                H::Draw->Rect(dropdown_x + 1, item_y, item_width, item_height, Color_t(58, 58, 62, alpha));
            }

            if (i == *option) {
                H::Draw->Rect(dropdown_x + 1, item_y, item_width, item_height, Color_t(0, 100, 200, alpha / 8));
            }

            Color_t item_color = (i == *option) ? Color_t(0, 150, 255, alpha) : Color_t(180, 180, 185, alpha);
            H::Draw->String(font, dropdown_x + 6, item_y + 3, item_color, POS_DEFAULT, aliases[i].c_str());

            if (!menu::menu_locked && item_hover && is_clicking && !was_clicked) {
                *option = i;
                dropdown_open[item_counts] = false;
                was_clicked = true;
            }
        }

        if (needs_scroll) {
            int scroll_x = dropdown_x + dropdown_width - 12;
            int scroll_y = list_y;
            int scroll_h = list_height;

            H::Draw->Rect(scroll_x, scroll_y, 12, scroll_h, Color_t(35, 35, 40, alpha));

            float ratio = (float)visible_items / total_items;
            int handle_h = (int)(scroll_h * ratio);
            handle_h = std::max(handle_h, 20);

            int max_scroll = total_items - visible_items;
            float fraction = (float)start_index / max_scroll;
            int handle_y = scroll_y + (int)(fraction * (scroll_h - handle_h));

            Color_t handle_color = Color_t(80, 80, 85, alpha);
            bool handle_hover = IsMouseInRectHelper(scroll_x, handle_y, 12, handle_h);
            if (handle_hover) handle_color = Color_t(100, 100, 105, alpha);

            H::Draw->Rect(scroll_x, handle_y, 12, handle_h, handle_color);

            static bool is_dragging_scroll = false;
            static int drag_dropdown = -1;

            bool is_down = H::Input->IsDown(VK_LBUTTON);

            if (!menu::menu_locked && handle_hover && is_clicking && !is_dragging_scroll) {
                is_dragging_scroll = true;
                drag_dropdown = item_counts;
            }

            if (is_dragging_scroll && drag_dropdown == item_counts) {
                if (is_down) {
                    int mouse_y = H::Input->GetMouseY();
                    int relative_y = mouse_y - scroll_y;
                    relative_y = std::clamp(relative_y, 0, scroll_h - handle_h);
                    float new_fraction = (float)relative_y / (scroll_h - handle_h);
                    int new_scroll = (int)(new_fraction * max_scroll);
                    dropdown_scroll[item_counts] = new_scroll;
                }
                else {
                    is_dragging_scroll = false;
                }
            }

            bool track_hover = IsMouseInRectHelper(scroll_x, scroll_y, 12, scroll_h);
            if (!menu::menu_locked && track_hover && is_clicking && !handle_hover) {
                int mouse_y = H::Input->GetMouseY();
                int relative_y = mouse_y - scroll_y;
                float fraction = (float)relative_y / scroll_h;
                int new_scroll = (int)(fraction * max_scroll);
                new_scroll = std::clamp(new_scroll, 0, max_scroll);
                dropdown_scroll[item_counts] = new_scroll;
            }
        }
    }

    if (!is_clicking) {
        was_clicked = false;
    }

    *y += dropdown_height + 6;
}

void multi_combo(int x, int* y, std::string text, int* option, std::vector<std::string> aliases, int alpha, int item_counts, bool /*calc_height*/) {
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    if (menu::menu_locked)
        alpha = 0;

    Color_t label_color = Color_t(180, 180, 185, alpha);
    H::Draw->String(font, x + 8, *y, label_color, POS_DEFAULT, text.c_str());
    *y += 18;

    int dropdown_x = x + 8;
    int dropdown_width = 270;
    int dropdown_height = 22;
    int dropdown_y = *y;

    std::string display;
    for (size_t i = 0; i < aliases.size(); ++i) {
        if (*option & (1 << i)) {
            if (!display.empty()) display += ", ";
            display += aliases[i];
        }
    }
    if (display.empty()) display = "None";

    Color_t bg_color = Color_t(50, 50, 55, alpha);
    Color_t border_color = Color_t(35, 35, 40, alpha);
    Color_t hover_color = Color_t(55, 55, 60, alpha);

    bool is_hovering = IsMouseInRectHelper(dropdown_x, dropdown_y, dropdown_width, dropdown_height);
    Color_t current_bg = is_hovering ? hover_color : bg_color;

    H::Draw->Rect(dropdown_x, dropdown_y, dropdown_width, dropdown_height, current_bg);
    H::Draw->OutlinedRect(dropdown_x, dropdown_y, dropdown_width, dropdown_height, border_color);

    if (display.length() > 20) display = display.substr(0, 17) + "...";
    H::Draw->String(font, dropdown_x + 8, dropdown_y + 4, Color_t(200, 200, 205, alpha), POS_DEFAULT, display.c_str());

    std::string arrow = dropdown_open[item_counts] ? "▲" : "▼";
    int arrow_x = dropdown_x + dropdown_width - 20;
    H::Draw->String(font, arrow_x, dropdown_y + 4, Color_t(120, 120, 125, alpha), POS_DEFAULT, arrow.c_str());

    static bool was_clicked = false;
    bool is_clicking = H::Input->IsPressed(VK_LBUTTON);

    if (!menu::menu_locked && is_hovering && is_clicking && !was_clicked) {
        dropdown_open[item_counts] = !dropdown_open[item_counts];

        if (dropdown_open[item_counts]) {
            dropdown_scroll[item_counts] = 0;
        }

        was_clicked = true;
    }

    if (dropdown_open[item_counts]) {
        int list_y = dropdown_y + dropdown_height;
        int total_items = aliases.size();
        int visible_items = std::min(MAX_VISIBLE_ITEMS, total_items);
        int item_height = 20;
        int list_height = visible_items * item_height;
        bool needs_scroll = total_items > visible_items;

        int list_width = needs_scroll ? dropdown_width - 12 : dropdown_width;
        int item_width = list_width - 4;

        int start_index = dropdown_scroll[item_counts];
        if (needs_scroll) {
            start_index = std::clamp(start_index, 0, total_items - visible_items);
        }
        else {
            start_index = 0;
        }
        dropdown_scroll[item_counts] = start_index;
        int end_index = std::min(start_index + visible_items, total_items);

        H::Draw->Rect(dropdown_x, list_y, dropdown_width, list_height, Color_t(45, 45, 50, alpha));
        H::Draw->OutlinedRect(dropdown_x, list_y, dropdown_width, list_height, border_color);

        for (int i = start_index; i < end_index; ++i) {
            int item_y = list_y + (i - start_index) * item_height;
            bool item_hover = IsMouseInRectHelper(dropdown_x + 2, item_y, item_width, item_height);
            bool is_checked = (*option & (1 << i)) != 0;

            if (item_hover) {
                H::Draw->Rect(dropdown_x + 2, item_y + 1, item_width, item_height - 2, Color_t(60, 60, 65, alpha));
            }

            int check_size = 12;
            int check_x = dropdown_x + 6;
            int check_y = item_y + 4;

            Color_t check_bg = Color_t(45, 45, 50, alpha);
            Color_t check_border = is_checked ? Color_t(0, 150, 255, alpha) : Color_t(70, 70, 75, alpha);

            H::Draw->Rect(check_x, check_y, check_size, check_size, check_bg);
            H::Draw->OutlinedRect(check_x, check_y, check_size, check_size, check_border);

            if (is_checked) {
                H::Draw->Rect(check_x + 2, check_y + 2, check_size - 4, check_size - 4, Color_t(0, 150, 255, alpha));
            }

            Color_t item_color = is_checked ? Color_t(0, 150, 255, alpha) : Color_t(190, 190, 195, alpha);
            H::Draw->String(font, check_x + check_size + 6, item_y + 3, item_color, POS_DEFAULT, aliases[i].c_str());

            if (!menu::menu_locked && item_hover && is_clicking && !was_clicked) {
                *option ^= (1 << i);
                was_clicked = true;
            }
        }

        if (needs_scroll) {
            int scroll_x = dropdown_x + dropdown_width - 12;
            int scroll_y = list_y;
            int scroll_h = list_height;

            H::Draw->Rect(scroll_x, scroll_y, 12, scroll_h, Color_t(35, 35, 40, alpha));

            float ratio = (float)visible_items / total_items;
            int handle_h = (int)(scroll_h * ratio);
            handle_h = std::max(handle_h, 20);

            int max_scroll = total_items - visible_items;
            float fraction = (float)start_index / max_scroll;
            int handle_y = scroll_y + (int)(fraction * (scroll_h - handle_h));

            Color_t handle_color = Color_t(80, 80, 85, alpha);
            bool handle_hover = IsMouseInRectHelper(scroll_x, handle_y, 12, handle_h);
            if (handle_hover) handle_color = Color_t(100, 100, 105, alpha);

            H::Draw->Rect(scroll_x, handle_y, 12, handle_h, handle_color);

            static bool is_dragging_scroll = false;
            static int drag_dropdown = -1;

            bool is_down = H::Input->IsDown(VK_LBUTTON);

            if (!menu::menu_locked && handle_hover && is_clicking && !is_dragging_scroll) {
                is_dragging_scroll = true;
                drag_dropdown = item_counts;
            }

            if (is_dragging_scroll && drag_dropdown == item_counts) {
                if (is_down) {
                    int mouse_y = H::Input->GetMouseY();
                    int relative_y = mouse_y - scroll_y;
                    relative_y = std::clamp(relative_y, 0, scroll_h - handle_h);
                    float new_fraction = (float)relative_y / (scroll_h - handle_h);
                    int new_scroll = (int)(new_fraction * max_scroll);
                    dropdown_scroll[item_counts] = new_scroll;
                }
                else {
                    is_dragging_scroll = false;
                }
            }

            bool track_hover = IsMouseInRectHelper(scroll_x, scroll_y, 12, scroll_h);
            if (!menu::menu_locked && track_hover && is_clicking && !handle_hover) {
                int mouse_y = H::Input->GetMouseY();
                int relative_y = mouse_y - scroll_y;
                float fraction = (float)relative_y / scroll_h;
                int new_scroll = (int)(fraction * max_scroll);
                new_scroll = std::clamp(new_scroll, 0, max_scroll);
                dropdown_scroll[item_counts] = new_scroll;
            }
        }
    }

    if (!is_clicking) {
        was_clicked = false;
    }

    *y += dropdown_height + 8;
}

void int_slider(int x, int* y, std::string text, int& option, int min_value, int max_value, int item_counts, bool /*calc_height*/) {
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    int alpha = menu::menu_locked ? 0 : 255;

    Color_t label_color = Color_t(165, 165, 170, alpha);
    H::Draw->String(font, x + 12, *y, label_color, POS_DEFAULT, text.c_str());
    *y += 16;

    int slider_x = x + 12;
    int slider_width = 268;
    int slider_height = 4;
    int slider_y = *y + 7;

    Color_t bg_color = Color_t(38, 38, 42, alpha);
    Color_t fill_color = Color_t(0, 140, 255, alpha);
    Color_t border_color = Color_t(30, 30, 35, alpha);

    H::Draw->Rect(slider_x, slider_y, slider_width, slider_height, bg_color);
    H::Draw->OutlinedRect(slider_x, slider_y, slider_width, slider_height, border_color);

    float percentage = (float)(option - min_value) / (float)(max_value - min_value);
    int fill_width = (int)(slider_width * percentage);
    if (fill_width > 0) {
        H::Draw->Rect(slider_x, slider_y, fill_width, slider_height, fill_color);
    }

    std::string value_str = std::to_string(option);
    int value_x = slider_x + slider_width + 8;
    H::Draw->String(font, value_x, *y, Color_t(170, 170, 175, alpha), POS_DEFAULT, value_str.c_str());

    static bool is_dragging = false;
    static int drag_item = -1;
    bool is_hovering = IsMouseInRectHelper(slider_x, slider_y - 4, slider_width, slider_height + 8);
    bool is_clicking = H::Input->IsDown(VK_LBUTTON);

    if (!menu::menu_locked && is_hovering && is_clicking && !is_dragging) {
        is_dragging = true;
        drag_item = item_counts;
    }

    if (is_dragging && drag_item == item_counts) {
        if (is_clicking) {
            int mouse_x = H::Input->GetMouseX();
            float new_percentage = (float)(mouse_x - slider_x) / (float)slider_width;
            new_percentage = std::clamp(new_percentage, 0.0f, 1.0f);
            option = min_value + (int)(new_percentage * (max_value - min_value));
        }
        else {
            is_dragging = false;
            drag_item = -1;
        }
    }

    option = std::clamp(option, min_value, max_value);
    *y += 22;
}

void float_slider(int x, int* y, std::string text, float& option, float min_value, float max_value, int item_counts, bool /*calc_height*/) {
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    int alpha = menu::menu_locked ? 0 : 255;

    Color_t label_color = Color_t(165, 165, 170, alpha);
    H::Draw->String(font, x + 12, *y, label_color, POS_DEFAULT, text.c_str());
    *y += 16;

    int slider_x = x + 12;
    int slider_width = 268;
    int slider_height = 8;
    int slider_y = *y + 6;

    Color_t bg_main = Color_t(55, 55, 55, alpha);
    Color_t bg_shadow = Color_t(40, 40, 42, alpha);
    Color_t fill_top = Color_t(0, 110, 210, alpha);
    Color_t fill_middle = Color_t(0, 100, 190, alpha);
    Color_t fill_bottom = Color_t(0, 90, 180, alpha);

    H::Draw->Rect(slider_x, slider_y, slider_width, slider_height, bg_main);
    H::Draw->Line(slider_x, slider_y, slider_x + slider_width, slider_y, bg_shadow);

    float percentage = (option - min_value) / (max_value - min_value);
    percentage = std::clamp(percentage, 0.0f, 1.0f);
    int fill_width = (int)(slider_width * percentage);

    if (fill_width > 0) {
        if (slider_height >= 6) {
            int third = slider_height / 3;
            H::Draw->Rect(slider_x, slider_y, fill_width, third, fill_top);
            H::Draw->Rect(slider_x, slider_y + third, fill_width, third, fill_middle);
            H::Draw->Rect(slider_x, slider_y + (third * 2), fill_width,
                slider_height - (third * 2), fill_bottom);
        }
        else {
            H::Draw->Rect(slider_x, slider_y, fill_width, slider_height, fill_middle);
            H::Draw->Line(slider_x, slider_y, slider_x + fill_width, slider_y, fill_top);
            H::Draw->Line(slider_x, slider_y + slider_height - 1,
                slider_x + fill_width, slider_y + slider_height - 1, fill_bottom);
        }

        Color_t highlight = Color_t(60, 180, 255, static_cast<int>(alpha * 0.5f));
        H::Draw->Line(slider_x, slider_y, slider_x + fill_width, slider_y, highlight);
    }

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << option;
    std::string value_str = ss.str();

    int value_x = slider_x + slider_width + 8;
    Color_t value_color = Color_t(170, 170, 175, alpha);
    H::Draw->String(font, value_x, *y, value_color, POS_DEFAULT, value_str.c_str());

    static bool is_dragging = false;
    static int drag_item = -1;

    bool is_hovering = IsMouseInRectHelper(slider_x, slider_y - 4, slider_width, slider_height + 8);
    bool is_clicking = H::Input->IsDown(VK_LBUTTON);

    if (!menu::menu_locked && is_hovering && is_clicking && !is_dragging) {
        is_dragging = true;
        drag_item = item_counts;
    }

    if (is_dragging && drag_item == item_counts) {
        if (is_clicking) {
            int mouse_x = H::Input->GetMouseX();
            float new_percentage = (float)(mouse_x - slider_x) / (float)slider_width;
            new_percentage = std::clamp(new_percentage, 0.0f, 1.0f);
            option = min_value + new_percentage * (max_value - min_value);
        }
        else {
            is_dragging = false;
            drag_item = -1;
        }
    }

    option = std::clamp(option, min_value, max_value);
    *y += 22;
}

void key_selector(int x, int* y, int* key, int item_counts, bool /*calc_height*/) {
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    int alpha = menu::menu_locked ? 0 : 255;

    std::string text_str = "Keybind";

    Color_t label_color = Color_t(165, 165, 170, alpha);
    H::Draw->String(font, x + 12, *y, label_color, POS_DEFAULT, text_str.c_str());

    static bool waiting_for_key[256] = {};
    KeybindState& state = keybind_states[item_counts];

    if (key) state.key = *key;

    int button_width = 70;
    int button_height = 18;
    int button_x = x + 200;
    int button_y = *y - 1;

    std::string display_text;
    if (waiting_for_key[item_counts]) {
        display_text = "...";
    }
    else {
        display_text = "[ " + GetKeyName(state.key) + " ]";
    }

    Color_t btn_text = Color_t(160, 160, 165, alpha);
    Color_t btn_text_waiting = Color_t(100, 200, 255, alpha);

    bool is_hovering = IsMouseInRectHelper(button_x, button_y, button_width, button_height);

    // SEM quadrado de fundo - apenas texto
    int text_width = 0, text_height = 0;
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, display_text.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);

    int text_x = button_x;
    int text_y = button_y + 2;

    Color_t final_text_color = waiting_for_key[item_counts] ? btn_text_waiting : btn_text;
    H::Draw->String(font, text_x, text_y, final_text_color, POS_DEFAULT, display_text.c_str());

    // SEM indicador de modo

    static int popup_open_for_item = -1;

    bool right_mouse_down = H::Input->IsDown(VK_RBUTTON);
    static bool right_mouse_was_down = false;

    if (!menu::menu_locked && is_hovering && right_mouse_down && !right_mouse_was_down) {
        if (popup_open_for_item == item_counts) {
            popup_open_for_item = -1;
        }
        else {
            popup_open_for_item = item_counts;
        }
        right_mouse_was_down = true;
    }

    if (!right_mouse_down) {
        right_mouse_was_down = false;
    }

    if (popup_open_for_item == item_counts) {
        int popup_width = 100;
        int popup_height = keybind_modes.size() * 20 + 4;
        int popup_x = button_x + button_width + 5;
        int popup_y = button_y;

        Color_t popup_bg = Color_t(40, 40, 43, 250);
        Color_t popup_border = Color_t(60, 60, 65, alpha);

        H::Draw->Rect(popup_x, popup_y, popup_width, popup_height, popup_bg);
        H::Draw->OutlinedRect(popup_x, popup_y, popup_width, popup_height, popup_border);

        bool left_click = H::Input->IsPressed(VK_LBUTTON);
        static bool left_was_clicked = false;

        for (size_t i = 0; i < keybind_modes.size(); i++) {
            int item_y = popup_y + 2 + (i * 20);
            int item_height = 20;

            bool item_hover = IsMouseInRectHelper(popup_x, item_y, popup_width, item_height);

            if (item_hover) {
                Color_t hover_bg = Color_t(55, 120, 200, 150);
                H::Draw->Rect(popup_x, item_y, popup_width, item_height, hover_bg);
            }

            if (state.mode == (int)i) {
                Color_t check_color = Color_t(100, 200, 255, alpha);
                H::Draw->String(font, popup_x + 5, item_y + 3, check_color, POS_DEFAULT, ">");
            }

            Color_t mode_text_color = Color_t(180, 180, 185, alpha);
            H::Draw->String(font, popup_x + 20, item_y + 3, mode_text_color, POS_DEFAULT,
                keybind_modes[i].c_str());

            if (item_hover && left_click && !left_was_clicked) {
                state.mode = (int)i;
                popup_open_for_item = -1;
                left_was_clicked = true;
            }
        }

        if (!left_click) {
            left_was_clicked = false;
        }
    }

    static bool was_clicked = false;
    bool left_mouse_down = H::Input->IsPressed(VK_LBUTTON);

    if (!menu::menu_locked && is_hovering && left_mouse_down && !was_clicked && popup_open_for_item != item_counts) {
        waiting_for_key[item_counts] = true;
        was_clicked = true;
    }

    if (!left_mouse_down) {
        was_clicked = false;
    }

    if (waiting_for_key[item_counts] && !menu::menu_locked) {
        for (int vk = 1; vk <= 254; ++vk) {
            if ((GetAsyncKeyState(vk) & 1) && vk != VK_LBUTTON && vk != VK_RBUTTON) {
                state.key = vk;
                if (key) *key = vk;
                waiting_for_key[item_counts] = false;
                break;
            }
        }
        if (GetAsyncKeyState(VK_ESCAPE) & 1) {
            waiting_for_key[item_counts] = false;
        }
    }

    *y += 22;
}

int GetKeybindMode(int item_id) {
    if (item_id >= 0 && item_id < 256) {
        return keybind_states[item_id].mode;
    }
    return 0;
}

bool IsKeybindActive(int item_id, int key_value) {
    if (item_id < 0 || item_id >= 256) return false;

    KeybindState& state = keybind_states[item_id];
    int mode = state.mode;

    switch (mode) {
    case 0:
        return true;

    case 1:
        return GetAsyncKeyState(key_value) & 0x8000;

    case 2: {
        static bool toggle_states[256] = {};
        static bool key_was_pressed[256] = {};

        bool is_pressed = GetAsyncKeyState(key_value) & 0x8000;
        if (is_pressed && !key_was_pressed[item_id]) {
            toggle_states[item_id] = !toggle_states[item_id];
        }
        key_was_pressed[item_id] = is_pressed;
        return toggle_states[item_id];
    }

    case 3:
        return false;

    default:
        return false;
    }
}

void text_input(int x, int* y, std::string text_label, std::string& input_str, int item_counts, bool /*calc_height*/) {
    *y += 25;
}

void button(int x, int* y, std::string text, std::function<void()> action, int item_counts, bool /*calc_height*/) {
    *y += 25;
}

std::vector<std::string> RefreshConfigFiles() {
    std::vector<std::string> files;
    return files;
}

void LoadSelectedConfig() {}
void CreateConfig() {}
void SaveConfig() {}
void LoadConfig() {}

namespace menu {
    bool IsMouseInRect(int x, int y, int w, int h) {
        int mouse_x = H::Input->GetMouseX();
        int mouse_y = H::Input->GetMouseY();
        return (mouse_x >= x && mouse_x <= x + w && mouse_y >= y && mouse_y <= y + h);
    }

    void HandleDrag() {
        static bool left_mouse_down_last_frame = false;
        bool left_mouse_down = H::Input->IsDown(VK_LBUTTON);

        int mouse_x = H::Input->GetMouseX();
        int mouse_y = H::Input->GetMouseY();

        int menu_width = 720;
        int header_height = 48;

        bool in_header = IsMouseInRect(menu::menu_x, menu::menu_y, menu_width, header_height);

        if (menu::current_tab != menu::Tab::AIMBOT) {
            if (!left_mouse_down) {
                menu::is_dragging = false;
            }
            left_mouse_down_last_frame = left_mouse_down;
            return;
        }

        if (left_mouse_down && !left_mouse_down_last_frame && in_header) {
            menu::is_dragging = true;
            menu::drag_offset_x = mouse_x - menu::menu_x;
            menu::drag_offset_y = mouse_y - menu::menu_y;
        }

        if (menu::is_dragging && left_mouse_down) {
            menu::menu_x = mouse_x - menu::drag_offset_x;
            menu::menu_y = mouse_y - menu::drag_offset_y;

            int screen[2];
            I::EngineClient->GetScreenSize(screen[0], screen[1]);
            int menu_height = 560;

            if (menu::menu_x < 0) menu::menu_x = 0;
            if (menu::menu_y < 0) menu::menu_y = 0;
            if (menu::menu_x + menu_width > screen[0]) menu::menu_x = screen[0] - menu_width;
            if (menu::menu_y + menu_height > screen[1]) menu::menu_y = screen[1] - menu_height;
        }

        if (!left_mouse_down) {
            menu::is_dragging = false;
        }

        left_mouse_down_last_frame = left_mouse_down;
    }

    void HandleTabClick() {
        static bool left_mouse_clicked = false;
        bool left_mouse_down = H::Input->IsPressed(VK_LBUTTON);

        if (left_mouse_down && !left_mouse_clicked) {
            int tab_x = menu_x + 320;
            int tab_y = menu_y + 14;
            int tab_width = 60;
            int tab_height = 20;
            int tab_spacing = 75;

            Tab old_tab = current_tab;

            if (IsMouseInRect(tab_x, tab_y, tab_width, tab_height)) {
                current_tab = Tab::AIMBOT;
            }

            tab_x += tab_spacing;
            if (IsMouseInRect(tab_x, tab_y, tab_width, tab_height)) {
                current_tab = Tab::ANTIAIM;
            }

            tab_x += tab_spacing;
            if (IsMouseInRect(tab_x, tab_y, tab_width, tab_height)) {
                current_tab = Tab::VISUALS;
            }

            tab_x += tab_spacing;
            if (IsMouseInRect(tab_x, tab_y, tab_width, tab_height)) {
                current_tab = Tab::SKINS;
            }

            tab_x += 58;
            if (IsMouseInRect(tab_x, tab_y, tab_width, tab_height)) {
                current_tab = Tab::MISC;
            }

            if (old_tab != current_tab) {
                ResetScrollOnTabChange();
            }

            left_mouse_clicked = true;
        }

        if (!left_mouse_down) {
            left_mouse_clicked = false;
        }
    }

    void HandleScroll() {
        int& scroll_offset = GetCurrentScrollOffset();

        int content_x = menu_x;
        int content_y = menu_y + 70;
        int content_width = 720;
        int content_height = content_visible_height;

        bool mouse_over_content = IsMouseInRect(content_x, content_y, content_width, content_height);

        if (mouse_over_content && !scrollbar_dragging) {
            int mouse_wheel = H::Input->GetMouseScroll();
            if (mouse_wheel != 0) {
                scroll_offset -= mouse_wheel * 20;
            }
        }

        int max_scroll = std::max(0, total_content_height - content_visible_height);

        scroll_offset = std::clamp(scroll_offset, 0, max_scroll);

        if (max_scroll > 0) {
            int scrollbar_x = content_x + content_width - 8;
            int scrollbar_y = content_y;
            int scrollbar_width = 6;
            int scrollbar_height = content_height;

            H::Draw->Rect(scrollbar_x, scrollbar_y, scrollbar_width, scrollbar_height,
                Color_t(25, 25, 28, 255));

            float visible_ratio = (float)content_visible_height / (float)total_content_height;
            int thumb_height = std::max(30, (int)(scrollbar_height * visible_ratio));

            float scroll_ratio = (float)scroll_offset / (float)max_scroll;
            int thumb_y = scrollbar_y + (int)((scrollbar_height - thumb_height) * scroll_ratio);

            bool thumb_hover = IsMouseInRect(scrollbar_x - 2, thumb_y, scrollbar_width + 4, thumb_height);
            Color_t thumb_color = thumb_hover || scrollbar_dragging ?
                Color_t(85, 150, 255, 200) :
                Color_t(65, 65, 70, 180);

            H::Draw->Rect(scrollbar_x, thumb_y, scrollbar_width, thumb_height, thumb_color);

            bool left_mouse_down = H::Input->IsDown(VK_LBUTTON);

            if (!scrollbar_dragging) {
                if (left_mouse_down && thumb_hover) {
                    scrollbar_dragging = true;
                    scrollbar_drag_start_y = H::Input->GetMouseY();
                    scrollbar_drag_start_offset = scroll_offset;
                }
            }
            else {
                if (left_mouse_down) {
                    int mouse_y = H::Input->GetMouseY();
                    int delta_y = mouse_y - scrollbar_drag_start_y;

                    float scroll_range = scrollbar_height - thumb_height;
                    float scroll_per_pixel = (float)max_scroll / scroll_range;

                    scroll_offset = scrollbar_drag_start_offset + (int)(delta_y * scroll_per_pixel);
                    scroll_offset = std::clamp(scroll_offset, 0, max_scroll);
                }
                else {
                    scrollbar_dragging = false;
                }
            }
        }
    }
}