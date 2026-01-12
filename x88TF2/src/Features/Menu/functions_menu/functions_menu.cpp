// functions_menu.cpp
#include "functions_menu.h"
#include "../Menu.h"  // For namespace menu (e.g., menu_locked, config_files, etc.)
#include <sstream>
#include <iomanip>
#include <windows.h>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include "../../../Features/PlayersList/PlayersList.h"  // For PlayerPriority if needed
#include "../../../CFG.h"  // Assuming Config::Save/Load are here
#include "../../../Utils/Storage/Storage.h"  // For U::Storage

std::string GetKeyName(int vk) {
    if (vk == 0) return "IN_ATTACK";
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

void text(int x, int* y, std::string text_str, text_type type, int alpha) {
    // No ">" for text labels
    if (menu::menu_locked)
        alpha = static_cast<int>(alpha * 0.20f);  // Keep partial transparency for labels if needed
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

void checkbox(int x, int* y, std::string text_str, bool* option, bool special, int alpha, int item_counts, int entindex, PlayerPriority* pri, bool use_return) {
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    if (!menu::menu_locked && item_counts == menu::item_count) {
        H::Draw->String(font, x - 25, *y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    if (menu::menu_locked)
        alpha = 0;  // Hide interactive elements when locked
    int text_width = 0, text_height = 0;
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text_str.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, alpha), POS_DEFAULT, text_str.c_str());
    if (item_counts == menu::item_count && !menu::menu_locked) {
        if (use_return ? (GetAsyncKeyState(VK_RETURN) & 1) : (GetAsyncKeyState(VK_LEFT) & 1 || GetAsyncKeyState(VK_RIGHT) & 1)) {
            *option = !*option;
            if (entindex > 0 && pri != nullptr) {
                F::Players->Mark(entindex, *pri);
            }
        }
    }
    int value_x = x + text_width + 10;
    H::Draw->String(font, value_x, *y, *option ? (special ? Color_t(255, 10, 10, alpha) : Color_t(31, 144, 217, alpha)) : Color_t(255, 255, 255, alpha), POS_DEFAULT, (*option ? "ON" : "OFF"));
    *y += 15;
}

void combo(int x, int* y, std::string text, int* option, std::vector<std::string> aliases, bool special, int alpha, int item_counts) {
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    if (!menu::menu_locked && item_counts == menu::item_count) {
        H::Draw->String(font, x - 25, *y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    if (menu::menu_locked)
        alpha = 0;
    int text_width = 0, text_height = 0;
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, alpha), POS_DEFAULT, text.c_str());
    static bool editing_combo[256] = {};
    static int pending_option[256] = {};
    if (item_counts == menu::item_count && !menu::menu_locked) {
        if (GetAsyncKeyState(VK_RETURN) & 1) {
            if (!editing_combo[item_counts]) {
                pending_option[item_counts] = *option;
            }
            else {
                *option = pending_option[item_counts];
            }
            editing_combo[item_counts] = !editing_combo[item_counts];
        }
        if (editing_combo[item_counts]) {
            if (GetAsyncKeyState(VK_RIGHT) & 1)
                pending_option[item_counts] = (pending_option[item_counts] + 1) % static_cast<int>(aliases.size());
            if (GetAsyncKeyState(VK_LEFT) & 1)
                pending_option[item_counts] = (pending_option[item_counts] - 1 + static_cast<int>(aliases.size())) % static_cast<int>(aliases.size());
        }
    }
    int display_value = editing_combo[item_counts] ? pending_option[item_counts] : *option;
    Color_t display_color = editing_combo[item_counts] ? Color_t(255, 255, 255, alpha) : Color_t(31, 144, 217, alpha);
    int value_x = x + text_width + 10;
    std::string display = "< " + aliases.at(display_value) + " >";
    H::Draw->String(font, value_x, *y, display_color, POS_DEFAULT, display.c_str());
    *y += 15;
}

void multi_combo(int x, int* y, std::string text, int* option, std::vector<std::string> aliases, int alpha, int item_counts) {
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    if (!menu::menu_locked && item_counts == menu::item_count) {
        H::Draw->String(font, x - 25, *y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    if (menu::menu_locked)
        alpha = 0;
    int text_width = 0, text_height = 0;
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, alpha), POS_DEFAULT, text.c_str());
    static bool editing_multi[256] = {};
    static int current_index[256] = {};
    if (item_counts == menu::item_count && !menu::menu_locked) {
        if (GetAsyncKeyState(VK_RETURN) & 1) {
            if (!editing_multi[item_counts]) {
                editing_multi[item_counts] = true;
            }
            else {
                int bit = 1 << current_index[item_counts];
                *option ^= bit;
            }
        }
        if (editing_multi[item_counts]) {
            if (GetAsyncKeyState(VK_LEFT) & 1) {
                current_index[item_counts] = (current_index[item_counts] - 1 + (int)aliases.size()) % (int)aliases.size();
            }
            if (GetAsyncKeyState(VK_RIGHT) & 1) {
                current_index[item_counts] = (current_index[item_counts] + 1) % (int)aliases.size();
            }
            if (GetAsyncKeyState(VK_ESCAPE) & 1) {
                editing_multi[item_counts] = false;
            }
        }
    }
    std::string display;
    Color_t display_color = Color_t(31, 144, 217, alpha);
    if (editing_multi[item_counts]) {
        int idx = current_index[item_counts];
        display = "< " + aliases.at(idx) + " >";
        display_color = (*option & (1 << idx)) ? Color_t(0, 255, 0, alpha) : Color_t(255, 0, 0, alpha);
    }
    else {
        for (size_t i = 0; i < aliases.size(); ++i) {
            if (*option & (1 << i)) {
                if (!display.empty()) display += ", ";
                display += aliases[i];
            }
        }
        if (display.empty()) display = "None";
        display = "< " + display + " >";
    }
    int value_x = x + text_width + 10;
    H::Draw->String(font, value_x, *y, display_color, POS_DEFAULT, display.c_str());
    *y += 15;
}

void int_slider(int x, int* y, std::string text, int& option, int min_value, int max_value, int item_counts) {
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    if (!menu::menu_locked && item_counts == menu::item_count) {
        H::Draw->String(font, x - 25, *y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    int alpha = menu::menu_locked ? 0 : 255;
    int text_width = 0, text_height = 0;
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, alpha), POS_DEFAULT, text.c_str());
    if (item_counts == menu::item_count && !menu::menu_locked) {
        if (GetAsyncKeyState(VK_LEFT) & 1)
            option -= 1;
        else if (GetAsyncKeyState(VK_RIGHT) & 1)
            option += 1;
    }
    option = std::clamp(option, min_value, max_value);
    std::string value_str = (std::stringstream{} << std::setprecision(3) << option).str();
    int value_x = x + text_width + 10;
    H::Draw->String(font, value_x, *y, Color_t(255, 255, 255, alpha), POS_DEFAULT, value_str.c_str());
    *y += 15;
}

void float_slider(int x, int* y, std::string text, float& option, float min_value, float max_value, int item_counts) {
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    if (!menu::menu_locked && item_counts == menu::item_count) {
        H::Draw->String(font, x - 25, *y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    int alpha = menu::menu_locked ? 0 : 255;
    int text_width = 0, text_height = 0;
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, alpha), POS_DEFAULT, text.c_str());
    if (item_counts == menu::item_count && !menu::menu_locked) {
        if (GetAsyncKeyState(VK_LEFT) & 1)
            option -= 1.00f;
        else if (GetAsyncKeyState(VK_RIGHT) & 1)
            option += 1.00f;
    }
    option = std::clamp(option, min_value, max_value);
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << option;
    std::string value_str = ss.str();
    int value_x = x + text_width + 10;
    H::Draw->String(font, value_x, *y, Color_t(255, 255, 255, alpha), POS_DEFAULT, value_str.c_str());
    *y += 15;
}

void key_selector(int x, int* y, int* key, int item_counts) {
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    if (!menu::menu_locked && item_counts == menu::item_count) {
        H::Draw->String(font, x - 25, *y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    int alpha = menu::menu_locked ? 0 : 255;
    std::string text_str = "Keybind";
    int text_width = 0, text_height = 0;
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text_str.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, alpha), POS_DEFAULT, text_str.c_str());
    static bool waiting_for_key[256] = {};
    if (item_counts == menu::item_count && !menu::menu_locked) {
        if ((GetAsyncKeyState(VK_RETURN) & 1) || (GetAsyncKeyState(VK_RIGHT) & 1)) {
            waiting_for_key[item_counts] = true;
        }
    }
    std::string display = waiting_for_key[item_counts] ? "Press any key..." : GetKeyName(*key);
    std::string value_str = "[" + display + "]";
    int value_x = x + text_width + 10;
    H::Draw->String(font, value_x, *y, Color_t(180, 240, 255, alpha), POS_DEFAULT, value_str.c_str());
    if (waiting_for_key[item_counts] && !menu::menu_locked) {
        for (int vk = 1; vk <= 254; ++vk) {
            if ((GetAsyncKeyState(vk) & 1) && vk != VK_RETURN) {
                *key = vk;
                waiting_for_key[item_counts] = false;
                break;
            }
        }
    }
    *y += 15;
}

void text_input(int x, int* y, std::string text_label, std::string& input_str, int item_counts) {
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    if (!menu::menu_locked && item_counts == menu::item_count) {
        H::Draw->String(font, x - 25, *y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    int alpha = menu::menu_locked ? 0 : 255;
    int text_width = 0, text_height = 0;
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text_label.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, alpha), POS_DEFAULT, text_label.c_str());
    if (item_counts == menu::item_count && !menu::menu_locked) {
        if (GetAsyncKeyState(VK_RETURN) & 1) {
            menu::editing_config_name[item_counts] = !menu::editing_config_name[item_counts];
        }
    }
    std::string display = menu::editing_config_name[item_counts] ? input_str + "*" : input_str;
    int value_x = x + text_width + 10;
    H::Draw->String(font, value_x, *y, Color_t(180, 240, 255, alpha), POS_DEFAULT, display.c_str());
    if (menu::editing_config_name[item_counts] && !menu::menu_locked) {
        for (int vk = 'A'; vk <= 'Z'; ++vk) {
            if (GetAsyncKeyState(vk) & 1) {
                input_str += static_cast<char>(vk);
            }
        }
        for (int vk = '0'; vk <= '9'; ++vk) {
            if (GetAsyncKeyState(vk) & 1) {
                input_str += static_cast<char>(vk);
            }
        }
        if (GetAsyncKeyState(VK_BACK) & 1 && !input_str.empty()) {
            input_str.pop_back();
        }
        if (GetAsyncKeyState(VK_SPACE) & 1) {
            input_str += ' ';
        }
    }
    *y += 15;
}

void button(int x, int* y, std::string text, std::function<void()> action, int item_counts) {
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    if (!menu::menu_locked && item_counts == menu::item_count) {
        H::Draw->String(font, x - 25, *y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    int alpha = menu::menu_locked ? 0 : 255;
    int text_width = 0, text_height = 0;
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, alpha), POS_DEFAULT, text.c_str());
    if (item_counts == menu::item_count && !menu::menu_locked) {
        if (GetAsyncKeyState(VK_RETURN) & 1) {
            action();
        }
    }
    *y += 15;
}

std::vector<std::string> RefreshConfigFiles() {
    std::vector<std::string> files;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(U::Storage->GetConfigFolder())) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                files.push_back(entry.path().stem().string());
            }
        }
        std::sort(files.begin(), files.end());
    }
    catch (...) {
        // Ignore errors
    }
    return files;
}

void LoadSelectedConfig() {
    if (menu::config_files.empty() || menu::current_config_index < 0 || menu::current_config_index >= static_cast<int>(menu::config_files.size())) return;
    std::string selected_name = menu::config_files[menu::current_config_index];
    std::filesystem::path config_path = U::Storage->GetConfigFolder() / (selected_name + ".json");
    Config::Load(config_path);
}

void CreateConfig() {
    if (menu::config_name.empty()) return;
    std::filesystem::path config_path = U::Storage->GetConfigFolder() / (menu::config_name + ".json");
    Config::Save(config_path);
    menu::config_files = RefreshConfigFiles();
    auto it = std::find(menu::config_files.begin(), menu::config_files.end(), menu::config_name);
    if (it != menu::config_files.end()) {
        menu::current_config_index = static_cast<int>(std::distance(menu::config_files.begin(), it));
    }
    menu::config_name.clear();
}

void SaveConfig() {
    if (menu::config_files.empty() || menu::current_config_index < 0 || menu::current_config_index >= static_cast<int>(menu::config_files.size())) return;
    std::string selected_name = menu::config_files[menu::current_config_index];
    std::filesystem::path config_path = U::Storage->GetConfigFolder() / (selected_name + ".json");
    Config::Save(config_path);
}

void LoadConfig() {
    LoadSelectedConfig();
}