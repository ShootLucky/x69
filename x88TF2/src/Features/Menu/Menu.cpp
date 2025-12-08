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
static int item_count = 1;
static int item_countx3 = 12;
const int EVENT_DEBUG_ID_INIT = 42; // Define the missing constant, common placeholder value
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
void checkbox(int x, int* y, std::string text, bool* option, bool special = false, int alpha = 255, int item_counts = 1) {
    if (menu::menu_locked)
        alpha = static_cast<int>(alpha * 0.f);
    int text_width = 0, text_height = 0;
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);
    H::Draw->String(font, x, *y, Color_t(255, 255, 255, 255), POS_DEFAULT, text.c_str());
    if (item_counts == item_count && !menu::menu_locked) {
        if (GetAsyncKeyState(VK_RETURN) & 1)
            *option = !*option;
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
    if (item_counts == item_count && !menu::menu_locked) {
        if ((GetAsyncKeyState(VK_RIGHT) & 1))
            *option = (*option + 1) % static_cast<int>(aliases.size());
        if ((GetAsyncKeyState(VK_LEFT) & 1))
            *option = (*option - 1 + static_cast<int>(aliases.size())) % static_cast<int>(aliases.size());
    }
    int value_x = x + text_width + 10; // Dynamic position: after the text width + padding
    std::string display = "< " + aliases.at(*option) + " >";
    H::Draw->String(font, value_x, *y, (*option != 0 ? (special ? Color_t(255, 10, 10, 255) : Color_t(31, 144, 217, 255)) : Color_t(255, 255, 255, 255)), POS_DEFAULT, display.c_str());
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
    if (item_counts == item_count && !menu::menu_locked) {
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
    if (item_counts == item_count && !menu::menu_locked) {
        if (GetAsyncKeyState(VK_LEFT) & 1)
            option -= 0.1f;
        else if (GetAsyncKeyState(VK_RIGHT) & 1)
            option += 0.1f;
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
    static bool waitingForKey = false;
    static int waitingItemIndex = -1;
    if (item_counts == item_count && !menu::menu_locked) {
        if ((GetAsyncKeyState(VK_RETURN) & 1) || (GetAsyncKeyState(VK_RIGHT) & 1)) {
            waitingForKey = true;
            waitingItemIndex = item_counts;
        }
    }
    std::string display = (waitingForKey && waitingItemIndex == item_counts) ? "Press any key..." : GetKeyName(*key);
    std::string value_str = "[" + display + "]";
    int value_x = x + text_width + 10;
    H::Draw->String(font, value_x, *y, Color_t(180, 240, 255, 255), POS_DEFAULT, value_str.c_str());
    if (waitingForKey && waitingItemIndex == item_counts && !menu::menu_locked) {
        for (int vk = 1; vk <= 254; ++vk) {
            if (GetAsyncKeyState(vk) & 0x8000) {
                *key = vk;
                waitingForKey = false;
                waitingItemIndex = -1;
                break;
            }
        }
    }
    *y += 15;
}
// Simple text input function (basic, allows alphanumeric input)
static std::string config_name = "default"; // Default config name
static bool editing_config_name = false;
static int editing_item_index = -1;
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
    if (item_counts == item_count && !menu::menu_locked) {
        if (GetAsyncKeyState(VK_RETURN) & 1) {
            editing_config_name = !editing_config_name;
            editing_item_index = editing_config_name ? item_counts : -1;
        }
    }
    std::string display = (editing_config_name && editing_item_index == item_counts) ? input_str + "_" : input_str;
    int value_x = x + text_width + 10;
    H::Draw->String(font, value_x, *y, Color_t(180, 240, 255, 255), POS_DEFAULT, display.c_str());
    if (editing_config_name && editing_item_index == item_counts && !menu::menu_locked) {
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
    if (item_counts == item_count && !menu::menu_locked) {
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
static std::vector<std::string> config_files;
static int current_config_index = 0;
void LoadSelectedConfig() {
    if (config_files.empty() || current_config_index < 0 || current_config_index >= static_cast<int>(config_files.size())) return;
    std::string selected_name = config_files[current_config_index];
    std::filesystem::path config_path = U::Storage->GetConfigFolder() / (selected_name + ".json");
    Config::Load(config_path);
}
void CreateConfig() {
    if (config_name.empty()) return;
    std::filesystem::path config_path = U::Storage->GetConfigFolder() / (config_name + ".json");
    Config::Save(config_path);
    // Refresh and select the new config
    config_files = RefreshConfigFiles();
    auto it = std::find(config_files.begin(), config_files.end(), config_name);
    if (it != config_files.end()) {
        current_config_index = static_cast<int>(std::distance(config_files.begin(), it));
    }
    config_name.clear(); // Clear after create
}
void SaveConfig() {
    if (config_files.empty() || current_config_index < 0 || current_config_index >= static_cast<int>(config_files.size())) return;
    std::string selected_name = config_files[current_config_index];
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
            animated_hello = full_hello.substr(0, char_count) + "_";
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
    static bool showAimbotKeyOptions = false;
    static bool showHitboxOptions = false;
    const CFont& font = H::Fonts->Get(EFonts::Menu);
    if (!menu::menu_locked)
    {
        if (GetAsyncKeyState(VK_DOWN) & 1) item_count += 1;
        if (GetAsyncKeyState(VK_UP) & 1) item_count -= 1;
    }
    y = menu_start_y;
    int current_item = 1;
    if (!menu_locked && item_count == current_item) {
        H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    combo(x, &y, "Section", &CFG::CurrentSection, std::vector<std::string>{ "Legit", "Visual", "Misc" }, false, 255, current_item++);
    int max_items = current_item - 1;
    if (CFG::CurrentSection == 0) {
        // LEGIT / AIMBOT: show only the requested options
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x, &y, "Aimbot (master)", &CFG::Aimbot_Enable, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x, &y, "Aimlock", &CFG::Aimbot_Aimlock, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x, &y, "Aimbot FOV", CFG::Aimbot_FOV, 0.f, 180.f, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x, &y, "Smooth (smaller = faster)", CFG::Aimbot_Hitscan_Smoothing, 1.f, 50.f, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x, &y, "Hitbox Options (press LEFT/RIGHT)", &showHitboxOptions, false, 255, current_item++);
        if (showHitboxOptions) {
            if (!menu_locked && item_count == current_item) {
                H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x, &y, "Head", &CFG::Aimbot_Hitbox_Head, false, 255, current_item++);
            if (!menu_locked && item_count == current_item) {
                H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x, &y, "Neck", &CFG::Aimbot_Hitbox_Neck, false, 255, current_item++);
            if (!menu_locked && item_count == current_item) {
                H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x, &y, "Chest", &CFG::Aimbot_Hitbox_Chest, false, 255, current_item++);
            if (!menu_locked && item_count == current_item) {
                H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x, &y, "Pelvis", &CFG::Aimbot_Hitbox_Pelvis, false, 255, current_item++);
        }
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x, &y, "Keybind Options (press LEFT/RIGHT)", &showAimbotKeyOptions, false, 255, current_item++);
        if (showAimbotKeyOptions) {
            if (!menu_locked && item_count == current_item) {
                H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            combo(x, &y, "Key Mode", &CFG::Aimbot_KeyMode, std::vector<std::string>{ "Hold", "Toggle", "Always" }, false, 255, current_item++);
            if (!menu_locked && item_count == current_item) {
                H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            key_selector(x, &y, &CFG::Aimbot_Key, current_item++);
            text(x, &y, "Tip: set 0 to use mouse click (IN_ATTACK)", text_type::extra);
        }
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
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP", &CFG::ESP_Enable, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP Team", &CFG::ESP_Team, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Box", &CFG::ESP_Box, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Name", &CFG::ESP_Name, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Health", &CFG::ESP_Health, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Pickups", &CFG::ESP_Pickups, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Pickups Box", &CFG::ESP_PickupsBox, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Pickups Name", &CFG::ESP_PickupsName, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP Build", &CFG::ESP_Build, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP Build Only Enemy", &CFG::ESP_BuildOnlyEnemy, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP Capture Flag", &CFG::ESP_CaptureFlag, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Box Capture", &CFG::ESP_BoxCapture, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Name Capture", &CFG::ESP_NameCapture, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP Local Player", &CFG::ESP_LocalPlayer, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP Offscreen", &CFG::ESP_Offscreen, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "ESP Hide Cloaked", &CFG::ESP_HideCloaked, false, 255, current_item++);
        // Add spacing before Radio options
        y += 30; // Increased spacing for better distance
        // Radio options under ESP
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "Radio", &CFG::Radio, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "local music", &CFG::Radio_LocalMusic, false, 255, current_item++);
        static bool prev_internacional = false;
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_esp, &y, "internacional", &CFG::Radio_Internacional, false, 255, current_item++);
        if (!prev_internacional && CFG::Radio_Internacional) {
            CFG::Radio_LocalMusic = false;
        }
        prev_internacional = CFG::Radio_Internacional;
        static int country_index = 0;
        static std::vector<std::string> countries = { "Russia", "Polonia" };
        if (CFG::Radio_Internacional) {
            if (!menu_locked && item_count == current_item) {
                H::Draw->String(font, x_esp - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            combo(x_esp, &y, "Country", &country_index, countries, false, 255, current_item++);
            CFG::Radio_Country = country_index;
        }
        // Chams column (middle)
        y = start_y;
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Chams Box", &CFG::ESP_ChamsBox, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Chams Team", &CFG::ESP_ChamsTeam, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Chams Build", &CFG::ESP_ChamsBuild, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Chams Build Only Enemy", &CFG::ESP_ChamsBuildOnlyEnemy, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Chams Capture Flag", &CFG::ESP_ChamsCaptureFlag, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Chams Local Player", &CFG::ESP_ChamsLocalPlayer, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Chams Hide Cloaked", &CFG::ESP_ChamsHideCloaked, false, 255, current_item++);
        // Add spacing before Bullet Tracer options
        y += 30; // Increased spacing for better distance
        // Bullet Tracer options under Chams
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_chams, &y, "Bullet tracer", &CFG::BulletTracer, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_chams - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_chams, &y, "Type", &CFG::BulletTracer_Type, std::vector<std::string>{ "line", "line+box", "box" }, false, 255, current_item++);
        // Skeleton column (right)
        y = start_y;
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_skel, &y, "Skeleton ESP", &CFG::ESP_Skeleton, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_skel, &y, "Skeleton Team", &CFG::ESP_SkeletonTeam, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_skel, &y, "Skeleton Build", &CFG::ESP_SkeletonBuild, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_skel, &y, "Skeleton Build Only Enemy", &CFG::ESP_SkeletonBuildOnlyEnemy, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_skel, &y, "Skeleton Capture Flag", &CFG::ESP_SkeletonCaptureFlag, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_skel, &y, "Skeleton Local Player", &CFG::ESP_SkeletonLocalPlayer, false, 255, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x_skel - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_skel, &y, "Skeleton Hide Cloaked", &CFG::ESP_SkeletonHideCloaked, false, 255, current_item++);
        max_items = current_item - 1;
    }
    else if (CFG::CurrentSection == 2) {
        // Misc section
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x, &y, "Auto Jump", &CFG::Misc_AutoJump, false, 255, current_item++);
        // Refresh config files periodically (e.g., every render in misc)
        config_files = RefreshConfigFiles();
        if (config_files.empty()) {
            config_files.push_back("default"); // Ensure at least one
        }
        if (current_config_index >= static_cast<int>(config_files.size())) {
            current_config_index = 0;
        }
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x, &y, "Config", &current_config_index, config_files, false, 255, current_item++);
        // No auto-load on selection change
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        text_input(x, &y, "name:", config_name, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        button(x, &y, "Create config", CreateConfig, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        button(x, &y, "Save Config", SaveConfig, current_item++);
        if (!menu_locked && item_count == current_item) {
            H::Draw->String(font, x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        button(x, &y, "Load Config", LoadConfig, current_item++);
        max_items = current_item - 1;
    }
    // Clamp item_count
    if (item_count > max_items) item_count = 1;
    if (item_count < 1) item_count = max_items;
}