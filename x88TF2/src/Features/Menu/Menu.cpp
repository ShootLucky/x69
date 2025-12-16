#include "Menu.h"
#include <sstream>
#include <iomanip> // For std::setprecision
#include <windows.h> // For MultiByteToWideChar, MapVirtualKey, GetKeyNameText
#include <cstring> // For strcmp (ensure SDK doesn't conflict)
#include "../src/SDK/Helpers/Draw/Draw.h" // adicionada para usar H::Draw
#include <algorithm> // For std::clamp
#include <vector>
#include <string>
#include <filesystem> // For directory iteration
#include "../src/SDK/SDK.h" // Assuming this includes necessary SDK headers for IGameEventListener2 and related
#include "../../Features/PlayersList/PlayersList.h" // Added for Playerlist

namespace menu {
    static int item_count = 1;
    static int item_countx3 = 12;
}

// Helper para obter nome legível de uma virtual-key
static std::string GetKeyName(int vk)
{
    if (vk == 0) return "IN_ATTACK";
    // nomes comuns
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
    // tenta usar MapVirtualKey + GetKeyNameText
    UINT sc = MapVirtualKeyA(static_cast<UINT>(vk), MAPVK_VK_TO_VSC);
    LONG lparam = (sc << 16);
    wchar_t namew[128] = {};
    if (GetKeyNameTextW(lparam, namew, ARRAYSIZE(namew)) > 0) {
        int needed = WideCharToMultiByte(CP_UTF8, 0, namew, -1, nullptr, 0, nullptr, nullptr);
        if (needed > 0) {
            std::string out(needed, '\0');
            WideCharToMultiByte(CP_UTF8, 0, namew, -1, out.data(), needed, nullptr, nullptr);
            // remove terminal null
            if (!out.empty() && out.back() == '\0') out.pop_back();
            return out;
        }
    }
    // fallback hex
    char buf[16];
    snprintf(buf, sizeof(buf), "VK_%02X", vk & 0xFF);
    return std::string(buf);
}
void text(int x, int* y, std::string text, text_type type, int alpha = 255) {
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
    H::Draw->String(H::Fonts->Get(EFonts::Menu), x, *y, text_color, POS_DEFAULT, text.c_str());
    *y += 15;
}
void checkbox(int x, int* y, std::string text, bool* option, bool special = false, int alpha = 255, int item_counts = 1, int entindex = 0, PlayerPriority* pri = nullptr, bool use_return = false) {
    if (menu::menu_locked)
        alpha = static_cast<int>(alpha * 0.f);
    int text_width = 0, text_height = 0;
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, 255), POS_DEFAULT, text.c_str());
    if (item_counts == menu::item_count && !menu::menu_locked) {
        if (use_return ? (GetAsyncKeyState(VK_RETURN) & 1) : (GetAsyncKeyState(VK_LEFT) & 1 || GetAsyncKeyState(VK_RIGHT) & 1)) {
            *option = !*option;
            if (entindex > 0 && pri != nullptr) {
                F::Players->Mark(entindex, *pri);
            }
        }
    }
    int value_x = x + text_width + 10; // Dynamic position: after the text width + padding
    H::Draw->String(font, value_x, *y, *option ? (special ? Color_t(255, 10, 10, 255) : Color_t(31, 144, 217, 255)) : Color_t(255, 255, 255, 255), POS_DEFAULT, (*option ? "ON" : "OFF"));
    *y += 15;
}
void combo(int x, int* y, std::string text, int* option, std::vector<std::string> aliases, bool special = false, int alpha = 255, int item_counts = 1) {
    if (menu::menu_locked)
        alpha = static_cast<int>(alpha * 0.f);
    int text_width = 0, text_height = 0;
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, 255), POS_DEFAULT, text.c_str());
    static bool editing_combo[256] = {}; // Support multiple combos
    static int pending_option[256] = {}; // Pending selection for each combo
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
            if ((GetAsyncKeyState(VK_RIGHT) & 1))
                pending_option[item_counts] = (pending_option[item_counts] + 1) % static_cast<int>(aliases.size());
            if ((GetAsyncKeyState(VK_LEFT) & 1))
                pending_option[item_counts] = (pending_option[item_counts] - 1 + static_cast<int>(aliases.size())) % static_cast<int>(aliases.size());
        }
    }
    int display_value = editing_combo[item_counts] ? pending_option[item_counts] : *option;
    Color_t display_color = editing_combo[item_counts] ? Color_t(255, 255, 255, 255) : Color_t(31, 144, 217, 255);
    int value_x = x + text_width + 10; // Dynamic position: after the text width + padding
    std::string display = "< " + aliases.at(display_value) + " >";
    H::Draw->String(font, value_x, *y, display_color, POS_DEFAULT, display.c_str());
    *y += 15;
}
void multi_combo(int x, int* y, std::string text, int* option, std::vector<std::string> aliases, int alpha = 255, int item_counts = 1) {
    if (menu::menu_locked)
        alpha = static_cast<int>(alpha * 0.f);
    int text_width = 0, text_height = 0;
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, 255), POS_DEFAULT, text.c_str());
    static bool editing_multi[256] = {};
    static int pending_bitmask[256] = {};
    static int pending_index[256] = {};
    if (item_counts == menu::item_count && !menu::menu_locked) {
        if (GetAsyncKeyState(VK_RETURN) & 1) {
            if (!editing_multi[item_counts]) {
                pending_bitmask[item_counts] = *option;
                pending_index[item_counts] = 0;
                editing_multi[item_counts] = true;
            }
            else {
                pending_bitmask[item_counts] ^= (1 << pending_index[item_counts]);
            }
        }
        if (editing_multi[item_counts]) {
            if (GetAsyncKeyState(VK_LEFT) & 1)
                pending_index[item_counts] = (pending_index[item_counts] - 1 + static_cast<int>(aliases.size())) % static_cast<int>(aliases.size());
            if (GetAsyncKeyState(VK_RIGHT) & 1)
                pending_index[item_counts] = (pending_index[item_counts] + 1) % static_cast<int>(aliases.size());
            if (GetAsyncKeyState(VK_ESCAPE) & 1) {
                editing_multi[item_counts] = false;
                *option = pending_bitmask[item_counts];
            }
        }
    }
    std::string display;
    Color_t display_color = Color_t(31, 144, 217, 255);
    if (editing_multi[item_counts]) {
        int idx = pending_index[item_counts];
        display = "< " + aliases.at(idx) + " >";
        display_color = (pending_bitmask[item_counts] & (1 << idx)) ? Color_t(0, 255, 0, 255) : Color_t(255, 0, 0, 255);
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
void int_slider(int x, int* y, std::string text, int& option, int min_value = 0, int max_value = 0, int item_counts = 1) {
    int alpha = 255;
    if (menu::menu_locked)
        alpha = static_cast<int>(alpha * 0.f);
    int text_width = 0, text_height = 0;
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, 255), POS_DEFAULT, text.c_str());
    if (item_counts == menu::item_count && !menu::menu_locked) {
        if (GetAsyncKeyState(VK_LEFT) & 1)
            option -= 1;
        else if (GetAsyncKeyState(VK_RIGHT) & 1)
            option += 1;
    }
    option = std::clamp(option, min_value, max_value);
    std::string value_str = (std::stringstream{} << std::setprecision(3) << option).str();
    int value_x = x + text_width + 10; // Dynamic position: after the text width + padding
    H::Draw->String(font, value_x, *y, Color_t(255, 255, 255, 255), POS_DEFAULT, value_str.c_str());
    *y += 15;
}
// Implementação simples do float_slider
void float_slider(int x, int* y, std::string text, float& option, float min_value = 0.f, float max_value = 0.f, int item_counts = 1) {
    int alpha = 255;
    if (menu::menu_locked)
        alpha = static_cast<int>(alpha * 0.f);
    int text_width = 0, text_height = 0;
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, 255), POS_DEFAULT, text.c_str());
    if (item_counts == menu::item_count && !menu::menu_locked) {
        if (GetAsyncKeyState(VK_LEFT) & 1)
            option -= 1.0f;
        else if (GetAsyncKeyState(VK_RIGHT) & 1)
            option += 1.f;
    }
    option = std::clamp(option, min_value, max_value);
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << option;
    std::string value_str = ss.str();
    int value_x = x + text_width + 10;
    H::Draw->String(font, value_x, *y, Color_t(255, 255, 255, 255), POS_DEFAULT, value_str.c_str());
    *y += 15;
}
void key_selector(int x, int* y, int* key, int item_counts = 1) {
    int alpha = 255;
    if (menu::menu_locked)
        alpha = static_cast<int>(alpha * 0.f);
    std::string text = "Keybind";
    int text_width = 0, text_height = 0;
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, 255), POS_DEFAULT, text.c_str());
    static bool waiting_for_key[256] = {}; // Support multiple
    if (item_counts == menu::item_count && !menu::menu_locked) {
        if ((GetAsyncKeyState(VK_RETURN) & 1) || (GetAsyncKeyState(VK_RIGHT) & 1)) {
            waiting_for_key[item_counts] = true;
        }
    }
    std::string display = waiting_for_key[item_counts] ? "Press any key..." : GetKeyName(*key);
    std::string value_str = "[" + display + "]";
    int value_x = x + text_width + 10;
    H::Draw->String(font, value_x, *y, Color_t(180, 240, 255, 255), POS_DEFAULT, value_str.c_str());
    if (waiting_for_key[item_counts] && !menu::menu_locked) {
        for (int vk = 1; vk <= 254; ++vk) {
            if ((GetAsyncKeyState(vk) & 1) && vk != VK_RETURN) { // Detect press event, ignore ENTER
                *key = vk;
                waiting_for_key[item_counts] = false;
                break;
            }
        }
    }
    *y += 15;
}
// Simple text input function (basic, allows alphanumeric input)
namespace menu {
    static std::string config_name = "default"; // Default config name
    static bool editing_config_name[256] = {}; // Support multiple
}
void text_input(int x, int* y, std::string text_label, std::string& input_str, int item_counts = 1) {
    int alpha = 255;
    if (menu::menu_locked)
        alpha = static_cast<int>(alpha * 0.f);
    int text_width = 0, text_height = 0;
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text_label.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, 255), POS_DEFAULT, text_label.c_str());
    if (item_counts == menu::item_count && !menu::menu_locked) {
        if (GetAsyncKeyState(VK_RETURN) & 1) {
            menu::editing_config_name[item_counts] = !menu::editing_config_name[item_counts];
        }
    }
    std::string display = menu::editing_config_name[item_counts] ? input_str + "*" : input_str;
    int value_x = x + text_width + 10;
    H::Draw->String(font, value_x, *y, Color_t(180, 240, 255, 255), POS_DEFAULT, display.c_str());
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
// Button function without ON/OFF display - draws text and triggers on RIGHT key press
void button(int x, int* y, std::string text, void (*action)(), int item_counts = 1) {
    int alpha = 255;
    if (menu::menu_locked)
        alpha = static_cast<int>(alpha * 0.f);
    int text_width = 0, text_height = 0;
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, 255), POS_DEFAULT, text.c_str());
    if (item_counts == menu::item_count && !menu::menu_locked) {
        if (GetAsyncKeyState(VK_RIGHT) & 1) {
            action();
        }
    }
    *y += 15;
}
// Function to refresh available config files
static std::vector<std::string> RefreshConfigFiles() {
    std::vector<std::string> files;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(U::Storage->GetConfigFolder())) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                files.push_back(entry.path().stem().string());
            }
        }
        std::sort(files.begin(), files.end()); // Sort alphabetically
    }
    catch (...) {
        // Ignore errors
    }
    return files;
}
// Config functions implementation
namespace menu {
    static std::vector<std::string> config_files;
    static int current_config_index = 0;
}
// Config functions implementation
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
    // Refresh and select the new config
    menu::config_files = ::RefreshConfigFiles();
    auto it = std::find(menu::config_files.begin(), menu::config_files.end(), menu::config_name);
    if (it != menu::config_files.end()) {
        menu::current_config_index = static_cast<int>(std::distance(menu::config_files.begin(), it));
    }
    menu::config_name.clear(); // Clear after create
}
void SaveConfig() {
    if (menu::config_files.empty() || menu::current_config_index < 0 || menu::current_config_index >= static_cast<int>(menu::config_files.size())) return;
    std::string selected_name = menu::config_files[menu::current_config_index];
    std::filesystem::path config_path = U::Storage->GetConfigFolder() / (selected_name + ".json");
    Config::Save(config_path);
}
void LoadConfig() {
    LoadSelectedConfig(); // Load the selected one
}
void menu::render() {
    int screen[2];
    auto pLocal = H::Entities->GetLocal();
    I::EngineClient->GetScreenSize(screen[0], screen[1]);
    std::stringstream address_string;
    address_string << I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
    int x = 240, x2 = 390;
    int y = 10;
    H::Draw->String(H::Fonts->Get(EFonts::Menu), 5, 40, Color_t(255, 150, 150, 255), POS_DEFAULT, "x69 V1");
    // Improved Steam name fetching with lobby detection
    static std::string player_name = "LocalPlayer";
    int local_index = I::EngineClient->GetLocalPlayer();
    bool in_lobby = !I::EngineClient->IsInGame() || local_index <= 0;
    std::string full_hello;
    if (in_lobby) {
        player_name = "LocalPlayer";
        full_hello = "Hello LocalPlayer, hope you're well!";
    }
    else if (local_index > 0) {
        player_info_t info{};
        if (I::EngineClient->GetPlayerInfo(local_index, &info)) {
            // Em TF2 só pega nome real quando ESTÁ em jogo
            if (info.name && std::strlen(info.name) > 0) {
                player_name = info.name;
                full_hello = std::string("Hello ") + player_name;
            }
            else {
                full_hello = "Hello UnknownUser";
            }
        }
        else {
            full_hello = "Hello LocalPlayer";
        }
    }
    else {
        full_hello = "Hello LocalPlayer, hope you're well!";
    }
    if (player_name.empty())
        player_name = "LocalPlayer";
    // Animation for hello message on first render (injection)
    static bool first_time = true;
    static std::string animated_hello = "";
    static float anim_start_time = 0.f;
    if (first_time) {
        if (anim_start_time == 0.f) {
            anim_start_time = I::GlobalVars->curtime;
        }
        float elapsed = I::GlobalVars->curtime - anim_start_time;
        size_t char_count = static_cast<size_t>(elapsed * 10.f); // Typing speed: ~10 chars/sec
        if (char_count >= full_hello.length()) {
            animated_hello = full_hello;
            first_time = false;
        }
        else {
            animated_hello = full_hello.substr(0, char_count) + "*";
        }
        text(x, &y, animated_hello, text_type::info);
    }
    else {
        text(x, &y, full_hello, text_type::info);
    }
    if (GetAsyncKeyState(VK_HOME) & 1)
        menu_locked = !menu_locked;
    // If menu_locked, only show hello and version, make rest transparent (skip drawing)
    if (menu_locked) {
        return; // Skip the rest, only hello is already drawn
    }
    int menu_start_y = 40; // Start menu below the hello texts to avoid overlap
    static bool showHitboxOptions = false;
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    if (!menu::menu_locked)
    {
        if (CFG::CurrentSection != 2) {
            if (GetAsyncKeyState(VK_DOWN) & 1) menu::item_count += 1;
            if (GetAsyncKeyState(VK_UP) & 1) menu::item_count -= 1;
        }
    }
    y = menu_start_y;
    int current_item = 1;
    if (!menu_locked && menu::item_count == current_item) {
        H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    combo(x, &y, "Section", &CFG::CurrentSection, std::vector<std::string>{ "Legit", "Visual", "Playerlist", "Misc" }, false, 255, current_item++);
    int max_items = current_item - 1;
    if (CFG::CurrentSection == 0) {
        // LEGIT / AIMBOT: divided into columns - Aimbot on left, Project Aimbot on center, Aimbot Combat on right
        int x_left = 150;
        int x_center = 350;
        int x_right = 550;
        int start_y = y;
        y = start_y;
        text(x_left, &y, "Aimbot Keybind", regular);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        key_selector(x_left, &y, &CFG::Aimbot_Key, current_item++);
        text(x_left, &y, "Key Mode", regular);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_left, &y, "Key Mode", &CFG::Aimbot_KeyMode, std::vector<std::string>{"Hold", "Toggle", "Always On"}, false, 255, current_item++);
        y += 15;
        start_y = y;
        y = start_y;
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Hitscan", &CFG::Aimbot_Enable, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Visible Check", &CFG::Aimbot_VisibleCheck, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Team Check", &CFG::Aimbot_TeamCheck, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Target Lag Records", &CFG::Aimbot_TargetLagRecords, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Target Stickies", &CFG::Aimbot_TargetStickies, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Smooth Auto Shoot", &CFG::Aimbot_SmoothAutoShoot, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Wait For Headshot", &CFG::Aimbot_WaitForHeadshot, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Wait For Charge", &CFG::Aimbot_WaitForCharge, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Minigun Tapfire", &CFG::Aimbot_MinigunTapfire, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_left, &y, "Aim Type", &CFG::Aimbot_Hitscan_Mode, std::vector<std::string>{ "Aimlock", "Silent" }, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_left, &y, "Hitbox Sort", &CFG::Aimbot_Hitbox_Sort, std::vector<std::string>{ "Auto", "Damage", "Accuracy" }, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_left, &y, "Hitbox Type", &CFG::Aimbot_Hitscan_Hitbox, std::vector<std::string>{ "Head", "Pelvis", "Auto" }, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_left, &y, "Sort", &CFG::Aimbot_Hitscan_Sort, std::vector<std::string>{ "Distance", "FOV", "Health" }, false, 255, current_item++);
        static bool showScanOptions = false;
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Scan options", &showScanOptions, false, 255, current_item++);
        if (showScanOptions) {
            if (!menu_locked && menu::item_count == current_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Scan Head", &CFG::Aimbot_Hitscan_Scan_Head, false, 255, current_item++);
            if (!menu_locked && menu::item_count == current_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Scan Body", &CFG::Aimbot_Hitscan_Scan_Body, false, 255, current_item++);
            if (!menu_locked && menu::item_count == current_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Scan Arms", &CFG::Aimbot_Hitscan_Scan_Arms, false, 255, current_item++);
            if (!menu_locked && menu::item_count == current_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Scan Legs", &CFG::Aimbot_Hitscan_Scan_Legs, false, 255, current_item++);
            if (!menu_locked && menu::item_count == current_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Scan Buildings", &CFG::Aimbot_Hitscan_Scan_Buildings, false, 255, current_item++);
        }
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_left, &y, "FOV", CFG::Aimbot_FOV, 0.f, 180.f, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_left, &y, "Smoothing", CFG::Aimbot_Hitscan_Smoothing, 0.f, 20.f, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Ignore Invisible", &CFG::Aimbot_Ignore_Invisible, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Ignore Taunting", &CFG::Aimbot_Ignore_Taunting, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Active Shoot", &CFG::Aimbot_ActiveShoot, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Active Lag Records", &CFG::Aimbot_ActiveLagRecords, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Auto Shoot", &CFG::Aimbot_AutoShoot, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Auto Scope", &CFG::Aimbot_AutoScope, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Target Players", &CFG::Aimbot_Target_Players, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Target Buildings", &CFG::Aimbot_Target_Buildings, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Ignore Friends", &CFG::Aimbot_Ignore_Friends, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Ignore Invulnerable", &CFG::Aimbot_Ignore_Invulnerable, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Whitelist Teammates", &CFG::Aimbot_WhitelistTeammates, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        int_slider(x_left, &y, "Baim After Shots", CFG::Aimbot_BaimAfterShots, 0, 20, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_left, &y, "Baim After Health", CFG::Aimbot_BaimAfterHealth, 0.f, 1.f, current_item++);
        // Project Aimbot (center column)
        y = start_y;
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center, &y, "Projectile Aimbot", &CFG::Aimbot_Projectile_Enable, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center, &y, "No Spread", &CFG::Aimbot_Projectile_NoSpread, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center, &y, "Auto Double Donk", &CFG::Aimbot_Projectile_AutoDoubleDonk, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center, &y, "Advanced Air Strafe", &CFG::Aimbot_Projectile_AdvancedAirStrafe, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center, &y, "Ground Strafe Prediction", &CFG::Aimbot_Projectile_GroundStrafePrediction, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center, &y, "BBox Multipoint", &CFG::Aimbot_Projectile_BBox_Multipoint, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center, &y, "Rocket Splash Preferred", &CFG::Aimbot_Projectile_RocketSplashPoint, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_center, &y, "Aim Type", &CFG::Aimbot_Projectile_Mode, std::vector<std::string>{ "Aimlock", "Silent" }, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_center, &y, "Aim Position", &CFG::Aimbot_Projectile_AimPosition, std::vector<std::string>{ "Auto" }, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_center, &y, "Sort", &CFG::Aimbot_Projectile_Sort, std::vector<std::string>{ "Auto", "Distance", "FOV" }, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_center, &y, "Method", &CFG::Aimbot_Projectile_PredictionMethod, std::vector<std::string>{ "Full Acceleration", "Velocity" }, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        int_slider(x_center, &y, "Ticks Predict", CFG::Aimbot_Projectile_TicksPredict, 0, 128, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center, &y, "Team Check", &CFG::Aimbot_Projectile_TeamCheck, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_center, &y, "FOV", CFG::Aimbot_Projectile_FOV, 0.f, 180.f, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_center, &y, "Smoothing", CFG::Aimbot_Projectile_Smoothing, 0.f, 20.f, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_center, &y, "Max Simulation Time", CFG::Aimbot_Projectile_MaxSimulationTime, 0.f, 5.f, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        int_slider(x_center, &y, "Max Targets", CFG::Aimbot_Projectile_MaxTargets, 1, 10, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center, &y, "Splash Bot", &CFG::Aimbot_Projectile_SplashBot, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_center, &y, "Splash Points", CFG::Aimbot_Projectile_SplashPoints, 10.f, 200.f, current_item++);
        // Melee (right column)
        y = start_y;
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_right - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_right, &y, "Melee Aimbot", &CFG::Aimbot_Melee_Active, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_right - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_right, &y, "Always Active", &CFG::Aimbot_Melee_AlwaysActive, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_right - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_right, &y, "Target Lag Records", &CFG::Aimbot_Melee_TargetLagRecords, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_right - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_right, &y, "Predict Swing", &CFG::Aimbot_Melee_PredictSwing, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_right - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_right, &y, "Walk To Target", &CFG::Aimbot_Melee_WalkToTarget, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_right - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_right, &y, "Whip Teammates", &CFG::Aimbot_Melee_WhipTeammates, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_right - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_right, &y, "Aim Type", &CFG::Aimbot_Melee_Mode, std::vector<std::string>{ "Aimlock", "Silent" }, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_right - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_right, &y, "Sort", &CFG::Aimbot_Melee_Sort, std::vector<std::string>{ "Distance", "FOV", "Health" }, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_right - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_right, &y, "FOV", CFG::Aimbot_Melee_FOV, 0.f, 180.f, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_right - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_right, &y, "Smoothing", CFG::Aimbot_Melee_Smoothing, 0.f, 20.f, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_right - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_right, &y, "Predict Swing Time", CFG::Aimbot_Melee_PredictSwingTime, 0.f, 1.f, current_item++);
        max_items = current_item - 1;
    }
    else if (CFG::CurrentSection == 1) {
        // Visual section divided into three columns: ESP left, Chams middle, Skeleton right
        int x_esp = 150;
        int x_chams = 350;
        int x_skel = 550;
        int start_y = y;
        // ESP column (left)
        y = start_y;
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP", &CFG::ESP_Enable, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP Team", &CFG::ESP_Team, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Box", &CFG::ESP_Box, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_esp, &y, "Box Type", &CFG::ESP_BoxType, std::vector<std::string>{"2D", "3D", "Corner"}, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Name", &CFG::ESP_Name, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Health", &CFG::ESP_Health, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_esp, &y, "Health Type", &CFG::ESP_HealthType, std::vector<std::string>{"Health bar", "Health number", "Number + bar"}, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Pickups", &CFG::ESP_Pickups, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Pickups Box", &CFG::ESP_PickupsBox, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Pickups Name", &CFG::ESP_PickupsName, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP Build", &CFG::ESP_Build, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP Build Only Enemy", &CFG::ESP_BuildOnlyEnemy, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP Capture Flag", &CFG::ESP_CaptureFlag, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Box Capture", &CFG::ESP_BoxCapture, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Name Capture", &CFG::ESP_NameCapture, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP Local Player", &CFG::ESP_LocalPlayer, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP Offscreen", &CFG::ESP_Offscreen, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP Hide Cloaked", &CFG::ESP_HideCloaked, false, 255, current_item++);
        // Add spacing before Aimbot FOV
        y += 30; // Increased spacing for better distance
        // Aimbot FOV option
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Aimbot FOV", &CFG::Aimbot_DrawFOV, false, 255, current_item++);
        // Chams column (middle)
        y = start_y;
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Chams Box", &CFG::ESP_ChamsBox, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Chams Team", &CFG::ESP_ChamsTeam, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Chams Pickups", &CFG::ESP_ChamsPickups, false, 255, current_item++); // Adicionado
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Chams Build", &CFG::ESP_ChamsBuild, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Chams Build Only Enemy", &CFG::ESP_ChamsBuildOnlyEnemy, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Chams Capture Flag", &CFG::ESP_ChamsCaptureFlag, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Chams Local Player", &CFG::ESP_ChamsLocalPlayer, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Chams Hide Cloaked", &CFG::ESP_ChamsHideCloaked, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Backtrack", &CFG::ESP_Chams_Backtrack, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        multi_combo(x_chams, &y, "Backtrack Type", &CFG::ESP_Chams_BacktrackType, std::vector<std::string>{"Enemies", "Team", "Local Player", "All"}, 255, current_item++);
        // Add spacing before Bullet Tracer options
        y += 30; // Increased spacing for better distance
        // Bullet Tracer options under Chams
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Tracer Effects", &CFG::BulletTracer, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        if (CFG::BulletTracer) {
            combo(x_chams, &y, "Type", &CFG::BulletTracer_Type, std::vector<std::string>{ "Default", "C.A.P.P.E.R", "Machina (White)", "Machina (Team)", "Big Nasty", "Short Circuit", "Merasmus Zap", "Random", "Random (No Zap)" }, false, 255, current_item++);
        }
        // Skeleton column (right)
        y = start_y;
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_skel, &y, "Skeleton ESP", &CFG::ESP_Skeleton, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_skel, &y, "Skeleton Team", &CFG::ESP_SkeletonTeam, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_skel, &y, "Skeleton Build", &CFG::ESP_SkeletonBuild, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_skel, &y, "Skeleton Build Only Enemy", &CFG::ESP_SkeletonBuildOnlyEnemy, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_skel, &y, "Skeleton Local Player", &CFG::ESP_SkeletonLocalPlayer, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_skel, &y, "Skeleton Hide Cloaked", &CFG::ESP_SkeletonHideCloaked, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_skel, &y, "Backtrack", &CFG::ESP_Skeleton_Backtrack, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        multi_combo(x_skel, &y, "Backtrack Type", &CFG::ESP_Skeleton_BacktrackType, std::vector<std::string>{"Enemies", "Team", "Local Player", "All"}, 255, current_item++);
        y += 30; // Distancia de y para nao ficar grudado
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_skel, &y, "Draw Movement Path Style", &CFG::Visuals_Draw_Movement_Path_Style, std::vector<std::string>{ "Off", "Line", "Dotted" }, false, 255, current_item++);
        y += 30;
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_skel, &y, "Logs", &CFG::Logs_Enable, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        multi_combo(x_skel, &y, "Logs Type", &CFG::Logs_Type, std::vector<std::string>{"Chat", "Console", "Screen", "All"}, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        multi_combo(x_skel, &y, "Players Logs Type", &CFG::PlayersLogs_Type, std::vector<std::string>{"Damage", "Respawn", "Enter", "Exit", "Playerlist", "Class"}, 255, current_item++);
        max_items = current_item - 1;
    }
    else if (CFG::CurrentSection == 2) { // Playerlist section
        int base_x = 150;
        int name_x = base_x;
        int ignored_x = base_x + 200;
        int cheater_x = base_x + 350;
        int retard_x = base_x + 500;
        int start_y = y;
        int header_y = start_y;
        y = start_y;
        // Headers
        H::Draw->String(font, name_x, y, Color_t(255, 255, 255, 255), POS_DEFAULT, "Name");
        H::Draw->String(font, ignored_x, y, Color_t(255, 255, 255, 255), POS_DEFAULT, "Ignored");
        H::Draw->String(font, cheater_x, y, Color_t(255, 255, 255, 255), POS_DEFAULT, "Cheater");
        H::Draw->String(font, retard_x, y, Color_t(255, 255, 255, 255), POS_DEFAULT, "Nigga");
        y += 15;
        // Horizontal line under headers
        H::Draw->Line(name_x, y, retard_x + 100, y, Color_t(255, 255, 255, 255));
        y += 15;
        std::vector<std::pair<int, std::string>> player_list;
        std::vector<int> ent_indices;
        std::vector<PlayerPriority> priorities;
        for (int i = 1; i <= 64; ++i) {
            if (i == I::EngineClient->GetLocalPlayer()) continue;
            player_info_t info{};
            if (I::EngineClient->GetPlayerInfo(i, &info) && !info.fakeplayer) {
                player_list.emplace_back(i, info.name);
                ent_indices.push_back(i);
                PlayerPriority pri{};
                F::Players->GetInfo(i, pri);
                priorities.push_back(pri);
            }
        }
        // Sort by name
        std::sort(player_list.begin(), player_list.end(), [](const auto& a, const auto& b) {
            return a.second < b.second;
            });
        int num_players = player_list.size();
        int num_columns = 3;
        int total_items = num_players * num_columns;
        int player_item_offset = 1; // Section is 1, players start from 2
        int max_player_item = total_items + player_item_offset;
        if (!menu::menu_locked) {
            int delta = 0;
            bool vertical = false;
            if (GetAsyncKeyState(VK_DOWN) & 1) { delta += num_columns; vertical = true; }
            if (GetAsyncKeyState(VK_UP) & 1) { delta -= num_columns; vertical = true; }
            if (GetAsyncKeyState(VK_RIGHT) & 1) delta += 1;
            if (GetAsyncKeyState(VK_LEFT) & 1) delta -= 1;
            int new_count = menu::item_count + delta;
            if (new_count > max_player_item) {
                if (vertical) new_count = menu::item_count; // Stay for vertical beyond
                else new_count = max_player_item;
            }
            if (new_count < 1) {
                if (vertical) new_count = menu::item_count;
                else new_count = 1;
            }
            menu::item_count = new_count;
        }
        for (int row = 0; row < num_players; ++row) {
            int player_y = y;
            // Draw name (non-interactive)
            H::Draw->String(font, name_x, player_y, Color_t(255, 255, 255, 255), POS_DEFAULT, player_list[row].second.c_str());
            for (int col = 0; col < num_columns; ++col) {
                int item_id = row * num_columns + col + player_item_offset + 1;
                int col_x;
                if (col == 0) col_x = ignored_x;
                else if (col == 1) col_x = cheater_x;
                else col_x = retard_x;
                if (!menu_locked && menu::item_count == item_id) {
                    H::Draw->String(font, col_x - 25, player_y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
                }
                int temp_y = player_y;
                bool* opt;
                if (col == 0) opt = &priorities[row].Ignored;
                else if (col == 1) opt = &priorities[row].Cheater;
                else opt = &priorities[row].RetardLegit;
                checkbox(col_x, &temp_y, "", opt, false, 255, item_id, ent_indices[row], &priorities[row], true);
            }
            y += 15; // Move to next row
        }
        max_items = max_player_item;
    }
    else if (CFG::CurrentSection == 3) {
        // Misc section
        int x_left = 150;
        int x_center = 350;
        int start_y = y;
        // Radio on left
        y = start_y;
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "Radio", &CFG::Radio, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "local music", &CFG::Radio_LocalMusic, false, 255, current_item++);
        if (CFG::Radio_LocalMusic) {
            if (!menu_locked && menu::item_count == current_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Pause", &CFG::Radio_Pause, false, 255, current_item++);
            if (!menu_locked && menu::item_count == current_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Next", &CFG::Radio_Next, false, 255, current_item++);
            if (!menu_locked && menu::item_count == current_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Previous", &CFG::Radio_Prev, false, 255, current_item++);
            if (!menu_locked && menu::item_count == current_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Volume Up", &CFG::Radio_VolUp, false, 255, current_item++);
            if (!menu_locked && menu::item_count == current_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Volume Down", &CFG::Radio_VolDown, false, 255, current_item++);
        }
        y += 30; // Espaço
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "AutoRocketJump", &CFG::Misc_AutoRocketJump_Enable, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        key_selector(x_left, &y, &CFG::Misc_AutoRocketJump_Key, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "AutoStrafer", &CFG::Misc_AutoStrafer_Enable, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_left, &y, "Strafe Intensity", CFG::Misc_AutoStrafer_Intensity, 0.f, 1.f, current_item++);
        // Auto jump, optimization and cfg in center
        y = start_y;
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center, &y, "Auto Jump", &CFG::Misc_AutoJump, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center, &y, "Fake Taunt", &CFG::Misc_Fake_Taunt, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center, &y, "Setup Bones Optimization", &CFG::Misc_SetupBones_Optimization, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center, &y, "Fake Latency", &CFG::Misc_FakeLatency_Enable, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_center, &y, "Latency Amount", CFG::Misc_FakeLatencyfloat_Enable, 0.f, 600.f, current_item++);
        // Refresh config files periodically (e.g., every render in misc)
        menu::config_files = ::RefreshConfigFiles();
        if (menu::config_files.empty()) {
            menu::config_files.push_back("default"); // Ensure at least one
        }
        if (menu::current_config_index >= static_cast<int>(menu::config_files.size())) {
            menu::current_config_index = 0;
        }
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_center, &y, "Config", &menu::current_config_index, menu::config_files, false, 255, current_item++);
        // No auto-load on selection change
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        text_input(x_center, &y, "name:", menu::config_name, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        button(x_center, &y, "Create config", CreateConfig, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        button(x_center, &y, "Save Config", SaveConfig, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        button(x_center, &y, "Load Config", LoadConfig, current_item++);
        max_items = current_item - 1;
    }
    // Clamp item_count
    if (menu::item_count > max_items) menu::item_count = 1;
    if (menu::item_count < 1) menu::item_count = max_items;
}