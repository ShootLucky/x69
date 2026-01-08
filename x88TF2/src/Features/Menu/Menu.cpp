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
#include <functional>
#include <cstdio>
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
void multi_combo(
    int x,
    int* y,
    std::string text,
    int* option,
    std::vector<std::string> aliases,
    int alpha = 255,
    int item_counts = 1
) {
    if (menu::menu_locked)
        alpha = static_cast<int>(alpha * 0.f);

    int text_width = 0, text_height = 0;
    const CFont& font = H::Fonts->Get(EFonts::Menu);

    wchar_t wtext[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext, 1024);
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wtext, text_width, text_height);

    H::Draw->String(font, x, *y, Color_t(255, 255, 255, 255), POS_DEFAULT, text.c_str());

    static bool editing_multi[256] = {};
    static int current_index[256] = {};

    if (item_counts == menu::item_count && !menu::menu_locked) {
        if (GetAsyncKeyState(VK_RETURN) & 1) {
            if (!editing_multi[item_counts]) {
                editing_multi[item_counts] = true;
            }
            else {
                int bit = 1 << current_index[item_counts];
                *option ^= bit; // TOGGLE IMEDIATO
            }
        }

        if (editing_multi[item_counts]) {
            if (GetAsyncKeyState(VK_LEFT) & 1) {
                current_index[item_counts] =
                    (current_index[item_counts] - 1 + (int)aliases.size()) % (int)aliases.size();
            }

            if (GetAsyncKeyState(VK_RIGHT) & 1) {
                current_index[item_counts] =
                    (current_index[item_counts] + 1) % (int)aliases.size();
            }

            if (GetAsyncKeyState(VK_ESCAPE) & 1) {
                editing_multi[item_counts] = false; // ESC só fecha
            }
        }
    }

    std::string display;
    Color_t display_color = Color_t(31, 144, 217, 255);

    if (editing_multi[item_counts]) {
        int idx = current_index[item_counts];
        display = "< " + aliases.at(idx) + " >";
        display_color = (*option & (1 << idx))
            ? Color_t(0, 255, 0, 255)
            : Color_t(255, 0, 0, 255);
    }
    else {
        for (size_t i = 0; i < aliases.size(); ++i) {
            if (*option & (1 << i)) {
                if (!display.empty()) display += ", ";
                display += aliases[i];
            }
        }

        if (display.empty())
            display = "None";

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
            option -= 1.00f;
        else if (GetAsyncKeyState(VK_RIGHT) & 1)
            option += 1.00f;
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
void button(int x, int* y, std::string text, std::function<void()> action, int item_counts = 1) {
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
        if (GetAsyncKeyState(VK_RETURN) & 1) {
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
    H::Draw->String(H::Fonts->Get(EFonts::Menu), 5, 40, Color_t(255, 150, 150, 255), POS_DEFAULT, "x69 BETA");
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
    combo(x, &y, "Section", &CFG::CurrentSection, std::vector<std::string>{ "Legit", "Visual", "Playerlist", "Misc", "Colors" }, false, 255, current_item++);
    int max_items = current_item - 1;
    // Mouse input
    static int mouse_x = 0, mouse_y = 0;
    bool mouse_down = false;
    if (!menu_locked) {
        POINT p;
        if (GetCursorPos(&p)) {
            if (ScreenToClient(FindWindowA(nullptr, "Team Fortress 2"), &p)) {
                mouse_x = p.x;
                mouse_y = p.y;
                mouse_down = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            }
        }
    }
    static Color_t copied_color = Color_t(0, 0, 0, 0);
    if (CFG::CurrentSection == 0) {
        // LEGIT / AIMBOT: divided into columns - Aimbot on left, Project Aimbot on center, Aimbot Combat on right
        int x_left = 150;
        int x_center = 350;
        int x_right = 550;
        int start_y = y;
        y = start_y;
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
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
        float_slider(x_left, &y, "Multipoint Scale", CFG::Aimbot_Hitscan_Multipoint_Scale, 0.f, 1.f, current_item++);
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
        checkbox(x_center, &y, "Splash DEBUG", &CFG::Debug_SplashPoints, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_center, &y, "Splash Points", CFG::Aimbot_Projectile_SplashPoints, 10.f, 512.f, current_item++);
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
        // Visual section
        int x_left = 150;
        int x_center = 350;
        int x_right = 550;
        int start_y = y;
        static int visuals_sub_section = 0;
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_left, &y, "Visuals Sub Tab", &visuals_sub_section, std::vector<std::string>{ "ESP", "Skeleton", "Others", "Chams", "Outiline" }, false, 255, current_item++);
        y += 15;
        start_y = y;
        int current_sub_item = current_item;
        if (visuals_sub_section == 0) { // ESP
            y = start_y;
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "ESP Master", &CFG::ESP_Enable, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Team Check", &CFG::ESP_Team, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Box", &CFG::ESP_Box, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            combo(x_left, &y, "Box Style", &CFG::ESP_BoxType, std::vector<std::string>{"2D", "3D", "Corner"}, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Name", &CFG::ESP_Name, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Health", &CFG::ESP_Health, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            combo(x_left, &y, "Health Type", &CFG::ESP_HealthType, std::vector<std::string>{"Health bar", "Health number", "Number + bar"}, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "World Pickups", &CFG::ESP_Pickups, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Pickups Box", &CFG::ESP_PickupsBox, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Pickups Name", &CFG::ESP_PickupsName, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Flag ESP", &CFG::ESP_CaptureFlag, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Box Capture", &CFG::ESP_BoxCapture, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Name Capture", &CFG::ESP_NameCapture, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Show Local Player", &CFG::ESP_LocalPlayer, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Hide Cloaked Players", &CFG::ESP_HideCloaked, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Aimbot FOV", &CFG::Aimbot_DrawFOV, false, 255, current_sub_item++);
            y = start_y;
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center, &y, "ESP Build", &CFG::ESP_Build, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center, &y, "ESP Build Only Enemy", &CFG::ESP_BuildOnlyEnemy, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center, &y, "Offscreen Indicators", &CFG::ESP_Offscreen, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            float_slider(x_center, &y, "Offscreen Radius", CFG::ESP_Offscreen_Radius, 10.0f, 500.0f, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            float_slider(x_center, &y, "Offscreen Max Distance", CFG::ESP_Offscreen_MaxDist, 0.0f, 2000.0f, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            combo(x_center, &y, "Offscreen Style", &CFG::ESP_Offscreen_Style, std::vector<std::string>{"Triangle", "Circle", "Bar"}, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center, &y, "Offscreen Filled", &CFG::ESP_Offscreen_Filled, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center, &y, "Player Conditions", &CFG::ESP_Conds, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center, &y, "ESP Sniper Lines", &CFG::ESP_SniperLines, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center, &y, "Player Tracers", &CFG::ESP_Tracer, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center, &y, "UberCharge Status", &CFG::ESP_Uber, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center, &y, "UberCharge Bar", &CFG::ESP_UberBar, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center, &y, "Distance Enemy", &CFG::ESP_DistanceEnemy, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            combo(x_center, &y, "Distance Position", &CFG::ESP_DistancePosition, std::vector<std::string>{"Side", "Bottom"}, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center, &y, "Buffs", &CFG::ESP_Buffs, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center, &y, "Debuffs", &CFG::ESP_Debuffs, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center, &y, "Latency (Ping)", &CFG::ESP_Ping, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center, &y, "KDR Player", &CFG::ESP_KRDPlayer, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center, &y, "Lag Compensation", &CFG::ESP_LagCompensation, false, 255, current_sub_item++);
            max_items = current_sub_item - 1;
        }
        else if (visuals_sub_section == 1) { // Skeleton
            y = start_y;
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Skeleton ESP", &CFG::ESP_Skeleton, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Skeleton Team", &CFG::ESP_SkeletonTeam, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Skeleton Build", &CFG::ESP_SkeletonBuild, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Skeleton Build Only Enemy", &CFG::ESP_SkeletonBuildOnlyEnemy, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Skeleton Local Player", &CFG::ESP_SkeletonLocalPlayer, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Skeleton Hide Cloaked", &CFG::ESP_SkeletonHideCloaked, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left, &y, "Backtrack", &CFG::ESP_Skeleton_Backtrack, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            multi_combo(x_left, &y, "Backtrack Type", &CFG::ESP_Skeleton_BacktrackType, std::vector<std::string>{"Enemies", "Team", "Local Player", "All"}, 255, current_sub_item++);
            max_items = current_sub_item - 1;
        }
        else if (visuals_sub_section == 2) { // Others
            y = start_y;
            // Define column positions (adjust widths as needed)
            int x_left_col = x_left;
            int x_center_col = x_left + 200; // Example width
            int x_right_col = x_center_col + 200; // Example width
            // Separate y for each column
            int y_left = start_y;
            int y_center = start_y;
            int y_right = start_y;
            // --- Left Column: Player Vision (View Model, Thirdperson) ---
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left_col, &y_left, "View Model", &CFG::Visuals_ViewModel_Enable, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            float_slider(x_left_col, &y_left, "Offsets Forward", CFG::Visuals_ViewModel_Forward, -50.f, 50.f, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            float_slider(x_left_col, &y_left, "Offsets Right", CFG::Visuals_ViewModel_Right, -50.f, 50.f, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            float_slider(x_left_col, &y_left, "Offsets Up", CFG::Visuals_ViewModel_Up, -50.f, 50.f, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left_col, &y_left, "Third Person", &CFG::Misc_ThirdPerson_Enable, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            combo(x_left_col, &y_left, "Key Mode", &CFG::Misc_ThirdPerson_KeyMode, std::vector<std::string>{"Hold", "Toggle", "Always On"}, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            key_selector(x_left_col, &y_left, &CFG::Misc_ThirdPerson_Key, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            float_slider(x_left_col, &y_left, "Distance offsets", CFG::Misc_ThirdPerson_Distance, -50.f, 100.f, current_sub_item++); // Assuming range 0-100, adjust as needed
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            float_slider(x_left_col, &y_left, "Side Offsets", CFG::Misc_ThirdPerson_SideOffset, -50.f, 50.f, current_sub_item++); // Assuming range -50-50
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            float_slider(x_left_col, &y_left, "Fov", CFG::Misc_ThirdPerson_Fov, 0.f, 120.f, current_sub_item++);
            // Add space below left column for logs
            y_left += 20; // Example distance in y (adjust as needed)
            // Logs below left column
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_left_col, &y_left, "Logs", &CFG::Logs_Enable, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            multi_combo(x_left_col, &y_left, "Logs Type", &CFG::Logs_Type, std::vector<std::string>{"Chat", "Console", "Screen"}, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            multi_combo(x_left_col, &y_left, "Players Logs Type", &CFG::PlayersLogs_Type, std::vector<std::string>{"Local", "Fried", "Name", "Damage", "Respawn", "Enter", "Exit", "Playerlist", "Class", "Enemy Vote", "Team vote"}, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            // --- Center Column: Weapons (Tracer Effects, Draw Path, Remove Scope/Zoom/Punch/Fire, Fov) ---
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center_col, &y_center, "Tracer Effects", &CFG::BulletTracer, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            if (CFG::BulletTracer) {
                combo(x_center_col, &y_center, "Type", &CFG::BulletTracer_Type, std::vector<std::string>{ "Default", "C.A.P.P.E.R", "Machina (White)", "Machina (Team)", "Big Nasty", "Short Circuit", "Merasmus Zap", "Random", "Random (No Zap)" }, false, 255, current_sub_item++);
            }
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            combo(x_center_col, &y_center, "Draw Movement Path Style", &CFG::Visuals_Draw_Movement_Path_Style, std::vector<std::string>{ "Off", "Line", "Dotted", "Line + Box" }, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center_col, &y_center, "Remove punch", &CFG::Visuals_RemovePunch, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center_col, &y_center, "Fov", &CFG::Visuals_CustomFov_Enable, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            float_slider(x_center_col, &y_center, "Fov Amount", CFG::Visuals_CustomFov_Amount, 0.f, 120.f, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center_col, &y_center, "Remove Scoped zoom", &CFG::Visuals_RemoveScopedZoom, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center_col, &y_center, "Remove Scoped", &CFG::Visuals_RemoveScoped, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_center_col, &y_center, "Remove Fire", &CFG::Visuals_RemoveFire, false, 255, current_sub_item++);
            // --- Right Column: Map (Rain) ---
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            checkbox(x_right_col, &y_right, "Rain Map", &CFG::Visuals_Rain, false, 255, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            float_slider(x_right_col, &y_right, "Radius", CFG::Visuals_Rain_Radius, 0.f, 100.f, current_sub_item++); // Assuming range 0-100, adjust as needed
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            float_slider(x_right_col, &y_right, "Width", CFG::Visuals_Rain_Width, 0.f, 10.f, current_sub_item++); // Assuming separate width and length
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            float_slider(x_right_col, &y_right, "Length", CFG::Visuals_Rain_Length, 0.f, 100.f, current_sub_item++);
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            float_slider(x_right_col, &y_right, "Wind Direction", CFG::Visuals_Rain_WindDirection, 0.f, 360.f, current_sub_item++); // 0-360 degrees
            if (!menu_locked && menu::item_count == current_sub_item) {
                H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            float_slider(x_right_col, &y_right, "Wind Speed", CFG::Visuals_Rain_WindSpeed, 0.f, 50.f, current_sub_item++); // Assuming range 0-50
            max_items = current_sub_item - 1;
        }

else if (visuals_sub_section == 3) { // Chams
    y = start_y;
    // Define column positions (adjust widths as needed)
    int x_left_col = x_left;
    int x_center_col = x_left + 200; // Example width
    int x_right_col = x_center_col + 200; // Example width
    int x_hands_col = x_right_col + 200; // New column for Hands
    // Separate y for each column
    int y_left = start_y;
    int y_center = start_y;
    int y_right = start_y;
    int y_hands = start_y;
    // --- Left Column: Players Materials ---
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_left_col, &y_left, "Players Active", &CFG::Materials_Players_Active, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    combo(x_left_col, &y_left, "Players Material", &CFG::Materials_Players_Material, std::vector<std::string>{"Off", "Flat", "Shaded", "Glossy", "Glow", "Plastic", "Fresnel"}, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_left_col, &y_left, "Players Hidden Material", &CFG::Materials_Players_HiddenMaterial, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    float_slider(x_left_col, &y_left, "Players Alpha", CFG::Materials_Players_Alpha, 0.f, 1.f, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_left_col, &y_left, "Players No Depth", &CFG::Materials_Players_No_Depth, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    combo(x_left_col, &y_left, "Players Two Models", &CFG::Materials_Players_TwoModels, std::vector<std::string>{"Off", "Overlay", "KSOverlay", "EsoOverlay", "FlatOverlay"}, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_left_col, &y_left, "Ignore Local", &CFG::Materials_Players_Ignore_Local, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_left_col, &y_left, "Ignore Friends", &CFG::Materials_Players_Ignore_Friends, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_left_col, &y_left, "Ignore Teammates", &CFG::Materials_Players_Ignore_Teammates, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_left_col, &y_left, "Show Teammate Medics", &CFG::Materials_Players_Show_Teammate_Medics, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_left_col, &y_left, "Ignore Enemies", &CFG::Materials_Players_Ignore_Enemies, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_left_col, &y_left, "Ignore LagRecords", &CFG::Materials_Players_Ignore_LagRecords, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    combo(x_left_col, &y_left, "LagRecords Style", &CFG::Materials_Players_LagRecords_Style, std::vector<std::string>{"Flat", "Shaded"}, false, 255, current_sub_item++);
    // --- Center Column: Buildings Materials ---
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_center_col, &y_center, "Buildings Active", &CFG::Materials_Buildings_Active, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    combo(x_center_col, &y_center, "Buildings Material", &CFG::Materials_Buildings_Material, std::vector<std::string>{"Off", "Flat", "Shaded", "Glossy", "Glow", "Plastic", "Fresnel"}, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_center_col, &y_center, "Buildings Hidden Material", &CFG::Materials_Buildings_HiddenMaterial, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    float_slider(x_center_col, &y_center, "Buildings Alpha", CFG::Materials_Buildings_Alpha, 0.f, 1.f, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_center_col, &y_center, "Buildings No Depth", &CFG::Materials_Buildings_No_Depth, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    combo(x_center_col, &y_center, "Buildings Two Models", &CFG::Materials_Buildings_TwoModels, std::vector<std::string>{"Off", "Overlay", "KSOverlay", "EsoOverlay", "FlatOverlay"}, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_center_col, &y_center, "Ignore Local", &CFG::Materials_Buildings_Ignore_Local, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_center_col, &y_center, "Ignore Teammates", &CFG::Materials_Buildings_Ignore_Teammates, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_center_col, &y_center, "Show Teammate Dispensers", &CFG::Materials_Buildings_Show_Teammate_Dispensers, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_center_col, &y_center, "Ignore Enemies", &CFG::Materials_Buildings_Ignore_Enemies, false, 255, current_sub_item++);
    // --- Right Column: World Materials ---
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_right_col, &y_right, "World Active", &CFG::Materials_World_Active, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    combo(x_right_col, &y_right, "World Material", &CFG::Materials_World_Material, std::vector<std::string>{"Off", "Flat", "Shaded", "Glossy", "Glow", "Plastic", "Fresnel"}, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_right_col, &y_right, "World Hidden Material", &CFG::Materials_World_HiddenMaterial, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    float_slider(x_right_col, &y_right, "World Alpha", CFG::Materials_World_Alpha, 0.f, 1.f, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_right_col, &y_right, "World No Depth", &CFG::Materials_World_No_Depth, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    combo(x_right_col, &y_right, "World Two Models", &CFG::Materials_World_TwoModels, std::vector<std::string>{"Off", "Overlay", "KSOverlay", "EsoOverlay", "FlatOverlay"}, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_right_col, &y_right, "Ignore HealthPacks", &CFG::Materials_World_Ignore_HealthPacks, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_right_col, &y_right, "Ignore AmmoPacks", &CFG::Materials_World_Ignore_AmmoPacks, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_right_col, &y_right, "Ignore Halloween Gift", &CFG::Materials_World_Ignore_Halloween_Gift, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_right_col, &y_right, "Ignore MVM Money", &CFG::Materials_World_Ignore_MVM_Money, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_right_col, &y_right, "Ignore Local Projectiles", &CFG::Materials_World_Ignore_LocalProjectiles, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_right_col, &y_right, "Ignore Teammate Projectiles", &CFG::Materials_World_Ignore_TeammateProjectiles, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_right_col, &y_right, "Ignore Enemy Projectiles", &CFG::Materials_World_Ignore_EnemyProjectiles, false, 255, current_sub_item++);
    // Menu part
                // --- Hands Column: Hands Materials ---
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_hands_col - 25, y_hands, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_hands_col, &y_hands, "Hands Active", &CFG::Materials_Hands_Active, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_hands_col - 25, y_hands, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    combo(x_hands_col, &y_hands, "Hands Material", &CFG::Materials_Hands_Material, std::vector<std::string>{"Off", "Flat", "Shaded", "Glossy", "Glow", "Plastic", "Fresnel"}, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_hands_col - 25, y_hands, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_hands_col, &y_hands, "Hands Hidden Material", &CFG::Materials_Hands_HiddenMaterial, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_hands_col - 25, y_hands, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    float_slider(x_hands_col, &y_hands, "Hands Alpha", CFG::Materials_Hands_Alpha, 0.f, 1.f, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_hands_col - 25, y_hands, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_hands_col, &y_hands, "Hands No Depth", &CFG::Materials_Hands_No_Depth, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_hands_col - 25, y_hands, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    combo(x_hands_col, &y_hands, "Hands Two Models", &CFG::Materials_Hands_TwoModels, std::vector<std::string>{"Off", "Overlay", "KSOverlay", "EsoOverlay", "FlatOverlay"}, false, 255, current_sub_item++);
    // --- Weapons ---
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_hands_col - 25, y_hands, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_hands_col, &y_hands, "Weapons Active", &CFG::Materials_Weapons_Active, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_hands_col - 25, y_hands, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    combo(x_hands_col, &y_hands, "Weapons Material", &CFG::Materials_Weapons_Material, std::vector<std::string>{"Off", "Flat", "Shaded", "Glossy", "Glow", "Plastic", "Fresnel"}, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_hands_col - 25, y_hands, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_hands_col, &y_hands, "Weapons Hidden Material", &CFG::Materials_Weapons_HiddenMaterial, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_hands_col - 25, y_hands, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    float_slider(x_hands_col, &y_hands, "Weapons Alpha", CFG::Materials_Weapons_Alpha, 0.f, 1.f, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_hands_col - 25, y_hands, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    checkbox(x_hands_col, &y_hands, "Weapons No Depth", &CFG::Materials_Weapons_No_Depth, false, 255, current_sub_item++);
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_hands_col - 25, y_hands, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    combo(x_hands_col, &y_hands, "Weapons Two Models", &CFG::Materials_Weapons_TwoModels, std::vector<std::string>{"Off", "Overlay", "KSOverlay", "EsoOverlay", "FlatOverlay"}, false, 255, current_sub_item++);
    max_items = current_sub_item - 1;
    }
else if (visuals_sub_section == 4) { // Outlines
        y = start_y;
        // Define column positions (adjust widths as needed)
        int x_left_col = x_left;
        int x_center_col = x_left + 200; // Example width
        int x_right_col = x_center_col + 200; // Example width
        // Separate y for each column
        int y_left = start_y;
        int y_center = start_y;
        int y_right = start_y;
        // --- Left Column: Global & Players Outlines ---
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left_col, &y_left, "Outlines Active", &CFG::Outlines_Active, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        combo(x_left_col, &y_left, "Outlines Style", &CFG::Outlines_Style, std::vector<std::string>{"Bloom", "Stencil", "Glow"}, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_left_col, &y_left, "Bloom Amount", CFG::Outlines_Bloom_Amount, 0.f, 5.f, current_sub_item++);
        y_left += 20; // Space for Players
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_left_col, &y_left, "Players Alpha", CFG::Outlines_Players_Alpha, 0.f, 1.f, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left_col, &y_left, "Players Active", &CFG::Outlines_Players_Active, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left_col, &y_left, "Ignore Local", &CFG::Outlines_Players_Ignore_Local, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left_col, &y_left, "Ignore Friends", &CFG::Outlines_Players_Ignore_Friends, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left_col, &y_left, "Ignore Teammates", &CFG::Outlines_Players_Ignore_Teammates, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left_col, &y_left, "Show Teammate Medics", &CFG::Outlines_Players_Show_Teammate_Medics, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_left_col - 25, y_left, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left_col, &y_left, "Ignore Enemies", &CFG::Outlines_Players_Ignore_Enemies, false, 255, current_sub_item++);
        // --- Center Column: Buildings Outlines ---
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_center_col, &y_center, "Buildings Alpha", CFG::Outlines_Buildings_Alpha, 0.f, 1.f, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center_col, &y_center, "Buildings Active", &CFG::Outlines_Buildings_Active, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center_col, &y_center, "Ignore Local", &CFG::Outlines_Buildings_Ignore_Local, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center_col, &y_center, "Ignore Teammates", &CFG::Outlines_Buildings_Ignore_Teammates, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center_col, &y_center, "Show Teammate Dispensers", &CFG::Outlines_Buildings_Show_Teammate_Dispensers, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_center_col - 25, y_center, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_center_col, &y_center, "Ignore Enemies", &CFG::Outlines_Buildings_Ignore_Enemies, false, 255, current_sub_item++);
        // --- Right Column: World Outlines ---
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_right_col, &y_right, "World Active", &CFG::Outlines_World_Active, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        float_slider(x_right_col, &y_right, "World Alpha", CFG::Outlines_World_Alpha, 0.f, 1.f, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_right_col, &y_right, "Ignore HealthPacks", &CFG::Outlines_World_Ignore_HealthPacks, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_right_col, &y_right, "Ignore AmmoPacks", &CFG::Outlines_World_Ignore_AmmoPacks, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_right_col, &y_right, "Ignore Halloween Gift", &CFG::Outlines_World_Ignore_Halloween_Gift, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_right_col, &y_right, "Ignore MVM Money", &CFG::Outlines_World_Ignore_MVM_Money, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_right_col, &y_right, "Ignore Local Projectiles", &CFG::Outlines_World_Ignore_LocalProjectiles, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_right_col, &y_right, "Ignore Teammate Projectiles", &CFG::Outlines_World_Ignore_TeammateProjectiles, false, 255, current_sub_item++);
        if (!menu_locked && menu::item_count == current_sub_item) {
            H::Draw->String(font, x_right_col - 25, y_right, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_right_col, &y_right, "Ignore Enemy Projectiles", &CFG::Outlines_World_Ignore_EnemyProjectiles, false, 255, current_sub_item++);
        max_items = current_sub_item - 1;
        }
    }
else if (CFG::CurrentSection == 2) { // Playerlist section
    static int selected_player_index = -1; // Entity index of selected player for main list, -1 means none/invalid
    static bool in_submenu = false; // Flag to indicate if we're in the per-player submenu
    static int previous_item_count = 0;
    // Key repeat timers (static to persist across frames)
    static float last_down_time = 0.0f;
    static float last_up_time = 0.0f;
    static float last_right_time = 0.0f;
    static float last_left_time = 0.0f;
    const float initial_delay = 0.3f; // Seconds before repeat starts
    const float repeat_rate = 0.05f; // Seconds between repeats
    float current_time = I::EngineClient->Time(); // Assuming engine has Time() for current game time
    int base_x = 150;
    int name_x = base_x;
    int start_y = y;
    int header_y = start_y;
    y = start_y;
    // Collect player data separated by teams and status
    std::vector<std::pair<int, std::string>> spectator_players;
    std::vector<std::pair<int, std::string>> red_players;
    std::vector<std::pair<int, std::string>> blu_players;
    int max_clients = I::EngineClient->GetMaxClients();
    for (int i = 1; i <= max_clients; ++i) {
        if (i == I::EngineClient->GetLocalPlayer()) continue;
        player_info_t info{};
        if (I::EngineClient->GetPlayerInfo(i, &info) && info.name[0] != '\0') { // Check if connected and has name
            C_BaseEntity* ent = reinterpret_cast<C_BaseEntity*>(I::ClientEntityList->GetClientEntity(i));
            std::string name = info.name;
            int team = ent ? ent->m_iTeamNum() : 0; // If no entity, treat as unassigned (0)
            if (team == 0 || team == 1) {
                spectator_players.emplace_back(i, name);
            }
            else if (team == 2) {
                red_players.emplace_back(i, name);
            }
            else if (team == 3) {
                blu_players.emplace_back(i, name);
            }
        }
    }
    // Sort by name within groups
    std::sort(spectator_players.begin(), spectator_players.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
        });
    std::sort(red_players.begin(), red_players.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
        });
    std::sort(blu_players.begin(), blu_players.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
        });
    // Flat list of entity indices for navigation
    std::vector<int> all_indices;
    for (const auto& p : spectator_players) all_indices.push_back(p.first);
    for (const auto& p : red_players) all_indices.push_back(p.first);
    for (const auto& p : blu_players) all_indices.push_back(p.first);
    int num_players = all_indices.size();
    if (!in_submenu) {
        int max_player_item = num_players > 0 ? num_players : 1;
        // Find current position
        int current_pos = -1;
        for (int i = 0; i < num_players; ++i) {
            if (all_indices[i] == selected_player_index) {
                current_pos = i;
                break;
            }
        }
        if (current_pos == -1) {
            if (num_players > 0) {
                current_pos = 0;
                selected_player_index = all_indices[0];
            }
            else {
                selected_player_index = -1;
            }
        }
        // Sync menu::item_count
        if (current_pos != -1) {
            menu::item_count = current_pos + 1;
        }
        bool section_changed = false;
        if (!menu::menu_locked && num_players > 0) {
            int delta = 0;
            // Handle DOWN (pressed + held)
            bool down_pressed = GetAsyncKeyState(VK_DOWN) & 1;
            bool down_held = GetAsyncKeyState(VK_DOWN) & 0x8000;
            if (down_pressed) {
                delta += 1;
                last_down_time = current_time;
            }
            else if (down_held && current_time - last_down_time > initial_delay) {
                if (current_time - last_down_time > repeat_rate) {
                    delta += 1;
                    last_down_time = current_time - std::fmod((current_time - last_down_time - initial_delay), repeat_rate) + initial_delay;
                }
            }
            // Handle UP (pressed + held)
            bool up_pressed = GetAsyncKeyState(VK_UP) & 1;
            bool up_held = GetAsyncKeyState(VK_UP) & 0x8000;
            if (up_pressed) {
                delta -= 1;
                last_up_time = current_time;
            }
            else if (up_held && current_time - last_up_time > initial_delay) {
                if (current_time - last_up_time > repeat_rate) {
                    delta -= 1;
                    last_up_time = current_time - std::fmod((current_time - last_up_time - initial_delay), repeat_rate) + initial_delay;
                }
            }
            // Handle RIGHT (pressed + held)
            bool right_pressed = GetAsyncKeyState(VK_RIGHT) & 1;
            bool right_held = GetAsyncKeyState(VK_RIGHT) & 0x8000;
            if (right_pressed) {
                delta += 1;
                last_right_time = current_time;
            }
            else if (right_held && current_time - last_right_time > initial_delay) {
                if (current_time - last_right_time > repeat_rate) {
                    delta += 1;
                    last_right_time = current_time - std::fmod((current_time - last_right_time - initial_delay), repeat_rate) + initial_delay;
                }
            }
            // Handle LEFT (pressed + held)
            bool left_pressed = GetAsyncKeyState(VK_LEFT) & 1;
            bool left_held = GetAsyncKeyState(VK_LEFT) & 0x8000;
            if (left_pressed) {
                delta -= 1;
                last_left_time = current_time;
            }
            else if (left_held && current_time - last_left_time > initial_delay) {
                if (current_time - last_left_time > repeat_rate) {
                    delta -= 1;
                    last_left_time = current_time - std::fmod((current_time - last_left_time - initial_delay), repeat_rate) + initial_delay;
                }
            }
            // Special handling for changing section when on last item
            // Apply delta to current_pos with wrap-around
            if (delta != 0) {
                int new_pos = current_pos + delta;
                if (new_pos >= num_players) new_pos = 0;
                if (new_pos < 0) new_pos = num_players - 1;
                selected_player_index = all_indices[new_pos];
                menu::item_count = new_pos + 1; // Sync global item_count
            }
        }
        if (section_changed) {
            in_submenu = false;
            return;
        }
        // Main player list mode
        int x_left = 150;
        int x_mid = 350;
        int x_right = 550;
        int y_left = start_y;
        int y_mid = start_y;
        int y_right = start_y;
        int row = 0;
        const int max_name_width = 180; // Max width before truncation
        // Left: Spectator (always show header)
        int header_width_left = 0, header_height_left = 0;
        wchar_t wheader_left[256] = {};
        MultiByteToWideChar(CP_UTF8, 0, "Spectator", -1, wheader_left, 256);
        I::MatSystemSurface->GetTextSize(font.m_dwFont, wheader_left, header_width_left, header_height_left);
        H::Draw->String(font, x_left, y_left, Color_t(255, 255, 255, 255), POS_DEFAULT, "Spectator");
        y_left += 15;
        for (const auto& p : spectator_players) {
            int player_y = y_left;
            std::string display_name = p.second;
            int text_width = 0, text_height = 0;
            wchar_t wname[256] = {};
            MultiByteToWideChar(CP_UTF8, 0, display_name.c_str(), -1, wname, 256);
            I::MatSystemSurface->GetTextSize(font.m_dwFont, wname, text_width, text_height);
            // Truncate if too wide
            while (text_width > max_name_width && display_name.length() > 3) {
                display_name = display_name.substr(0, display_name.length() - 1);
                MultiByteToWideChar(CP_UTF8, 0, (display_name + "....").c_str(), -1, wname, 256);
                I::MatSystemSurface->GetTextSize(font.m_dwFont, wname, text_width, text_height);
            }
            if (text_width > max_name_width) {
                display_name = display_name.substr(0, display_name.length() - 4) + "....";
                MultiByteToWideChar(CP_UTF8, 0, display_name.c_str(), -1, wname, 256);
                I::MatSystemSurface->GetTextSize(font.m_dwFont, wname, text_width, text_height);
            }
            // Get priority to determine color
            PlayerPriority priority{};
            F::Players->GetInfo(p.first, priority);
            Color_t name_color = Color_t(200, 200, 200, 255); // Default spectator
            if (priority.Cheater || priority.CheaterLight || priority.RijinUser || priority.LmaoboxUser || priority.NethookUser) {
                name_color = Color_t(255, 0, 0, 255); // Red for cheaters
            }
            else if (priority.RetardLegit || priority.Suspect) {
                name_color = Color_t(255, 255, 0, 255); // Yellow for suspects
            }
            else if (priority.Ignored) {
                name_color = Color_t(128, 128, 128, 255); // Gray for ignored
            }
            if (!menu_locked && menu::item_count - 1 == row) {
                H::Draw->String(font, x_left - 25, player_y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            H::Draw->String(font, x_left, player_y, name_color, POS_DEFAULT, display_name.c_str());
            y_left += 15;
            row++;
        }
        // Middle: RED Team
        int header_width_mid = 0, header_height_mid = 0;
        wchar_t wheader_mid[256] = {};
        MultiByteToWideChar(CP_UTF8, 0, "RED Team", -1, wheader_mid, 256);
        I::MatSystemSurface->GetTextSize(font.m_dwFont, wheader_mid, header_width_mid, header_height_mid);
        H::Draw->String(font, x_mid, y_mid, Color_t(255, 255, 255, 255), POS_DEFAULT, "RED Team");
        y_mid += 15;
        for (const auto& p : red_players) {
            int player_y = y_mid;
            std::string display_name = p.second;
            int text_width = 0, text_height = 0;
            wchar_t wname[256] = {};
            MultiByteToWideChar(CP_UTF8, 0, display_name.c_str(), -1, wname, 256);
            I::MatSystemSurface->GetTextSize(font.m_dwFont, wname, text_width, text_height);
            // Truncate if too wide
            while (text_width > max_name_width && display_name.length() > 3) {
                display_name = display_name.substr(0, display_name.length() - 1);
                MultiByteToWideChar(CP_UTF8, 0, (display_name + "....").c_str(), -1, wname, 256);
                I::MatSystemSurface->GetTextSize(font.m_dwFont, wname, text_width, text_height);
            }
            if (text_width > max_name_width) {
                display_name = display_name.substr(0, display_name.length() - 4) + "....";
                MultiByteToWideChar(CP_UTF8, 0, display_name.c_str(), -1, wname, 256);
                I::MatSystemSurface->GetTextSize(font.m_dwFont, wname, text_width, text_height);
            }
            // Get priority to determine color
            PlayerPriority priority{};
            F::Players->GetInfo(p.first, priority);
            Color_t name_color = Color_t(255, 0, 0, 255); // Default red team
            if (priority.Cheater || priority.CheaterLight || priority.RijinUser || priority.LmaoboxUser || priority.NethookUser) {
                name_color = CFG::Color_Cheater; // Assume red, or from config
            }
            else if (priority.RetardLegit || priority.Suspect) {
                name_color = CFG::Color_RetardLegit; // Assume yellow
            }
            else if (priority.Ignored) {
                name_color = Color_t(128, 128, 128, 255); // Gray
            }
            if (!menu_locked && menu::item_count - 1 == row) {
                H::Draw->String(font, x_mid - 25, player_y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            H::Draw->String(font, x_mid, player_y, name_color, POS_DEFAULT, display_name.c_str());
            y_mid += 15;
            row++;
        }
        // Right: BLU Team
        int header_width_right = 0, header_height_right = 0;
        wchar_t wheader_right[256] = {};
        MultiByteToWideChar(CP_UTF8, 0, "BLU Team", -1, wheader_right, 256);
        I::MatSystemSurface->GetTextSize(font.m_dwFont, wheader_right, header_width_right, header_height_right);
        H::Draw->String(font, x_right, y_right, Color_t(255, 255, 255, 255), POS_DEFAULT, "BLU Team");
        y_right += 15;
        for (const auto& p : blu_players) {
            int player_y = y_right;
            std::string display_name = p.second;
            int text_width = 0, text_height = 0;
            wchar_t wname[256] = {};
            MultiByteToWideChar(CP_UTF8, 0, display_name.c_str(), -1, wname, 256);
            I::MatSystemSurface->GetTextSize(font.m_dwFont, wname, text_width, text_height);
            // Truncate if too wide
            while (text_width > max_name_width && display_name.length() > 3) {
                display_name = display_name.substr(0, display_name.length() - 1);
                MultiByteToWideChar(CP_UTF8, 0, (display_name + "....").c_str(), -1, wname, 256);
                I::MatSystemSurface->GetTextSize(font.m_dwFont, wname, text_width, text_height);
            }
            if (text_width > max_name_width) {
                display_name = display_name.substr(0, display_name.length() - 4) + "....";
                MultiByteToWideChar(CP_UTF8, 0, display_name.c_str(), -1, wname, 256);
                I::MatSystemSurface->GetTextSize(font.m_dwFont, wname, text_width, text_height);
            }
            // Get priority to determine color
            PlayerPriority priority{};
            F::Players->GetInfo(p.first, priority);
            Color_t name_color = Color_t(0, 0, 255, 255); // Default blue team
            if (priority.Cheater || priority.CheaterLight || priority.RijinUser || priority.LmaoboxUser || priority.NethookUser) {
                name_color = CFG::Color_Cheater;
            }
            else if (priority.RetardLegit || priority.Suspect) {
                name_color = CFG::Color_RetardLegit;
            }
            else if (priority.Ignored) {
                name_color = Color_t(128, 128, 128, 255); // Gray
            }
            if (!menu_locked && menu::item_count - 1 == row) {
                H::Draw->String(font, x_right - 25, player_y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            H::Draw->String(font, x_right, player_y, name_color, POS_DEFAULT, display_name.c_str());
            y_right += 15;
            row++;
        }
        if (num_players == 0) {
            H::Draw->String(font, name_x, start_y, Color_t(255, 255, 255, 255), POS_DEFAULT, "No players");
        }
        // Check for Enter to enter submenu
        if (GetAsyncKeyState(VK_RETURN) & 1 && num_players > 0) {
            in_submenu = true;
            previous_item_count = menu::item_count;
            menu::item_count = 1;
            return; // <<< ISSO É CRÍTICO
        }
        max_items = max_player_item;
    }
    else {
        // Submenu mode for selected player
        player_info_t player_info{};
        if (!I::EngineClient->GetPlayerInfo(selected_player_index, &player_info)) {
            in_submenu = false;
            menu::item_count = previous_item_count;
            return;
        }
        PlayerPriority priority{};
        F::Players->GetInfo(selected_player_index, priority); // Fetch current priorities
        // Draw player name as header
        H::Draw->String(font, name_x, y, Color_t(255, 255, 255, 255), POS_DEFAULT, ("Flags for: " + std::string(player_info.name)).c_str());
        y += 15;
        // Horizontal line
        H::Draw->Line(name_x, y, name_x + 200, y, Color_t(255, 255, 255, 255));
        y += 15;
        // List of flags (vertical checkboxes)
        struct FlagInfo {
            std::string name;
            bool* value;
        };
        FlagInfo flags[] = {
            {"Ignored", &priority.Ignored},
            {"Cheater", &priority.Cheater},
            {"Retard Legit", &priority.RetardLegit},
            {"Cheater Light", &priority.CheaterLight},
            {"Rijin User", &priority.RijinUser},
            {"Lmaobox User", &priority.LmaoboxUser},
            {"Suspect", &priority.Suspect},
            {"Nethook User", &priority.NethookUser}
        };
        const int max_submenu_items = sizeof(flags) / sizeof(flags[0]);
        for (int i = 0; i < max_submenu_items; ++i) {
            int flag_y = y;
            int item_id = i + 1;
            if (!menu_locked && menu::item_count == item_id) {
                H::Draw->String(font, name_x - 25, flag_y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            H::Draw->String(font, name_x, flag_y, Color_t(255, 255, 255, 255), POS_DEFAULT, flags[i].name.c_str());
            std::string state = *(flags[i].value) ? "ON" : "OFF";
            H::Draw->String(font, name_x + 150, flag_y, *(flags[i].value) ? Color_t(31, 144, 217, 255) : Color_t(255, 255, 255, 255), POS_DEFAULT, state.c_str());
            y += 15;
        }
        // Key handling for navigation (only up/down)
        if (!menu::menu_locked) {
            int delta = 0;
            bool down_pressed = GetAsyncKeyState(VK_DOWN) & 1;
            bool down_held = GetAsyncKeyState(VK_DOWN) & 0x8000;
            if (down_pressed) {
                delta += 1;
                last_down_time = current_time;
            }
            else if (down_held && current_time - last_down_time > initial_delay) {
                if (current_time - last_down_time > repeat_rate) {
                    delta += 1;
                    last_down_time = current_time - std::fmod((current_time - last_down_time - initial_delay), repeat_rate) + initial_delay;
                }
            }
            bool up_pressed = GetAsyncKeyState(VK_UP) & 1;
            bool up_held = GetAsyncKeyState(VK_UP) & 0x8000;
            if (up_pressed) {
                delta -= 1;
                last_up_time = current_time;
            }
            else if (up_held && current_time - last_up_time > initial_delay) {
                if (current_time - last_up_time > repeat_rate) {
                    delta -= 1;
                    last_up_time = current_time - std::fmod((current_time - last_up_time - initial_delay), repeat_rate) + initial_delay;
                }
            }
            int new_count = menu::item_count + delta;
            if (new_count > max_submenu_items) new_count = 1;
            if (new_count < 1) new_count = max_submenu_items;
            menu::item_count = new_count;
            // Toggle on Enter
            if (GetAsyncKeyState(VK_RETURN) & 1) {
                int selected_flag = menu::item_count - 1;
                if (*(flags[selected_flag].value)) {
                    *(flags[selected_flag].value) = false;
                }
                else {
                    for (int j = 0; j < max_submenu_items; ++j) {
                        *(flags[j].value) = false;
                    }
                    *(flags[selected_flag].value) = true;
                }
                F::Players->Mark(selected_player_index, priority);
            }
            // Exit on Esc or Backspace
            if ((GetAsyncKeyState(VK_ESCAPE) & 1) || (GetAsyncKeyState(VK_BACK) & 1)) {
                in_submenu = false;
                menu::item_count = previous_item_count;
            }
        }
        max_items = max_submenu_items;
    }
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
        checkbox(x_left, &y, "SeedPredict BETA", &CFG::Exploits_SeedPred_Active, false, 255, current_item++);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        checkbox(x_left, &y, "SeedIndicator", &CFG::Exploits_SeedPred_DrawIndicator, false, 255, current_item++);
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
        checkbox(x_left, &y, "Anti AFK", &CFG::Misc_AntiAFK_Enable, false, 255, current_item++);
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
else if (CFG::CurrentSection == 4) { // Colors section
    int x_left = 150;
    int x_center = x_left + 200;
    int x_right = x_center + 200;
    int start_y = y;
    y = start_y;
    static int color_sub_section = 0;
    int current_sub_item = current_item;
    if (!menu_locked && menu::item_count == current_sub_item) {
        H::Draw->String(font, x_left - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
    }
    combo(x_left, &y, "Sub Section", &color_sub_section, std::vector<std::string>{"ESP", "Chams", "Outlines"}, false, 255, current_sub_item++);
    struct ColorEntry {
        std::string name;
        Color_t* color;
    };
    std::vector<ColorEntry> color_entries;
    if (color_sub_section == 0) { // ESP
        color_entries.push_back(ColorEntry{ "Team Red", &CFG::Color_TeamRed });
        color_entries.push_back(ColorEntry{ "Team Blue", &CFG::Color_TeamBlue });
        color_entries.push_back(ColorEntry{ "Local", &CFG::Color_Local });
        color_entries.push_back(ColorEntry{ "Ammo", &CFG::Color_Ammo });
        color_entries.push_back(ColorEntry{ "Medkit", &CFG::Color_Medkit });
        color_entries.push_back(ColorEntry{ "Flag", &CFG::Color_Flag });
        color_entries.push_back(ColorEntry{ "Uber", &CFG::Color_Uber });
        color_entries.push_back(ColorEntry{ "ESP Outline", &CFG::Color_ESP_Outline });
        color_entries.push_back(ColorEntry{ "Health Bar BG", &CFG::Color_HealthBarBG });
        color_entries.push_back(ColorEntry{ "Health Low", &CFG::Color_HealthLow });
        color_entries.push_back(ColorEntry{ "Health High", &CFG::Color_HealthHigh });
        color_entries.push_back(ColorEntry{ "Overheal", &CFG::Color_Overheal });
        color_entries.push_back(ColorEntry{ "Conds Text", &CFG::Color_CondsText });
        color_entries.push_back(ColorEntry{ "Sniper Line", &CFG::Color_SniperLine });
        color_entries.push_back(ColorEntry{ "Tracer Line", &CFG::Color_TracerLine });
        color_entries.push_back(ColorEntry{ "Uber Text", &CFG::Color_UberText });
        color_entries.push_back(ColorEntry{ "Uber Bar", &CFG::Color_UberBar });
        color_entries.push_back(ColorEntry{ "Uber Outline", &CFG::Color_UberOutline });
        color_entries.push_back(ColorEntry{ "Backtrack Skeleton", &CFG::Color_BacktrackSkeleton });
        color_entries.push_back(ColorEntry{ "Aimbot FOV", &CFG::Color_AimbotFOV });
        color_entries.push_back(ColorEntry{ "Proj FOV", &CFG::Color_ProjFOV });
        color_entries.push_back(ColorEntry{ "Melee FOV", &CFG::Color_MeleeFOV });
        color_entries.push_back(ColorEntry{ "Offscreen Arrow", &CFG::Color_OffscreenArrow });
        color_entries.push_back(ColorEntry{ "Name", &CFG::Color_Name });
        color_entries.push_back(ColorEntry{ "Health Text", &CFG::Color_HealthText });
        color_entries.push_back(ColorEntry{ "Skeleton", &CFG::Color_Skeleton });
        color_entries.push_back(ColorEntry{ "Building Team", &CFG::Color_BuildingTeam });
        color_entries.push_back(ColorEntry{ "Building Enemy", &CFG::Color_BuildingEnemy });
        color_entries.push_back(ColorEntry{ "Building Name", &CFG::Color_BuildingName });
        color_entries.push_back(ColorEntry{ "Ammo Pack", &CFG::Color_AmmoPack });
        color_entries.push_back(ColorEntry{ "Cheater", &CFG::Color_Cheater });
        color_entries.push_back(ColorEntry{ "Enemy", &CFG::Color_Enemy });
        color_entries.push_back(ColorEntry{ "Friend", &CFG::Color_Friend });
        color_entries.push_back(ColorEntry{ "Halloween Gift", &CFG::Color_Halloween_Gift });
        color_entries.push_back(ColorEntry{ "Health Pack", &CFG::Color_HealthPack });
        color_entries.push_back(ColorEntry{ "Invisible", &CFG::Color_Invisible });
        color_entries.push_back(ColorEntry{ "Invulnerable", &CFG::Color_Invulnerable });
        color_entries.push_back(ColorEntry{ "MVM Money", &CFG::Color_MVM_Money });
        color_entries.push_back(ColorEntry{ "Over Heal", &CFG::Color_OverHeal });
        color_entries.push_back(ColorEntry{ "Retard Legit", &CFG::Color_RetardLegit });
        color_entries.push_back(ColorEntry{ "Target", &CFG::Color_Target });
        color_entries.push_back(ColorEntry{ "Teammate", &CFG::Color_Teammate });
    }
    else if (color_sub_section == 1) { // Chams
        color_entries.push_back(ColorEntry{ "Hands", &CFG::Color_Hands });
        color_entries.push_back(ColorEntry{ "Hands Overlay", &CFG::Color_Hands_Overlay });
        color_entries.push_back(ColorEntry{ "Hands Sheen", &CFG::Color_Hands_Sheen });
        color_entries.push_back(ColorEntry{ "Props", &CFG::Color_Props });
        color_entries.push_back(ColorEntry{ "Weapon", &CFG::Color_Weapon });
        color_entries.push_back(ColorEntry{ "Weapon Sheen", &CFG::Color_Weapon_Sheen });
        color_entries.push_back(ColorEntry{ "Weapons", &CFG::Color_Weapons });
        color_entries.push_back(ColorEntry{ "Weapons Overlay", &CFG::Color_Weapons_Overlay });
        color_entries.push_back(ColorEntry{ "Players Friends", &CFG::Color_Players_Friends });
        color_entries.push_back(ColorEntry{ "Players LagRecords", &CFG::Color_Players_LagRecords });
        color_entries.push_back(ColorEntry{ "Players Local", &CFG::Color_Players_Local });
        color_entries.push_back(ColorEntry{ "Players Overlay Local", &CFG::Color_Players_Overlay_Local });
        color_entries.push_back(ColorEntry{ "Players Overlay Friends", &CFG::Color_Players_Overlay_Friends });
        color_entries.push_back(ColorEntry{ "Players Teammates", &CFG::Color_Players_Teammates });
        color_entries.push_back(ColorEntry{ "Players Overlay Teammates", &CFG::Color_Players_Overlay_Teammates });
        color_entries.push_back(ColorEntry{ "Players Enemies", &CFG::Color_Players_Enemies });
        color_entries.push_back(ColorEntry{ "Players Overlay Enemies", &CFG::Color_Players_Overlay_Enemies });
        color_entries.push_back(ColorEntry{ "Buildings Local", &CFG::Color_Buildings_Local });
        color_entries.push_back(ColorEntry{ "Buildings Overlay Local", &CFG::Color_Buildings_Overlay_Local });
        color_entries.push_back(ColorEntry{ "Buildings Teammates", &CFG::Color_Buildings_Teammates });
        color_entries.push_back(ColorEntry{ "Buildings Overlay Teammates", &CFG::Color_Buildings_Overlay_Teammates });
        color_entries.push_back(ColorEntry{ "Buildings Enemies", &CFG::Color_Buildings_Enemies });
        color_entries.push_back(ColorEntry{ "Buildings Overlay Enemies", &CFG::Color_Buildings_Overlay_Enemies });
        color_entries.push_back(ColorEntry{ "Projectiles Local", &CFG::Color_Projectiles_Local });
        color_entries.push_back(ColorEntry{ "Projectiles Overlay Local", &CFG::Color_Projectiles_Overlay_Local });
        color_entries.push_back(ColorEntry{ "Projectiles Teammates", &CFG::Color_Projectiles_Teammates });
        color_entries.push_back(ColorEntry{ "Projectiles Overlay Teammates", &CFG::Color_Projectiles_Overlay_Teammates });
        color_entries.push_back(ColorEntry{ "Projectiles Enemies", &CFG::Color_Projectiles_Enemies });
        color_entries.push_back(ColorEntry{ "Projectiles Overlay Enemies", &CFG::Color_Projectiles_Overlay_Enemies });
        color_entries.push_back(ColorEntry{ "Ammo Pack Overlay", &CFG::Color_AmmoPack_Overlay });
        color_entries.push_back(ColorEntry{ "Health Pack Overlay", &CFG::Color_HealthPack_Overlay });
        color_entries.push_back(ColorEntry{ "Halloween Gift Overlay", &CFG::Color_Halloween_Gift_Overlay });
        color_entries.push_back(ColorEntry{ "MVM Money Overlay", &CFG::Color_MVM_Money_Overlay });
    }
    else if (color_sub_section == 2) { // Outlines
        color_entries.push_back(ColorEntry{ "Teammates", &CFG::Outlines_Color_Teammates });
        color_entries.push_back(ColorEntry{ "Local Player", &CFG::Outlines_Color_LocalPlayer });
        color_entries.push_back(ColorEntry{ "Friends", &CFG::Outlines_Color_Friends });
        color_entries.push_back(ColorEntry{ "Teammate Medics", &CFG::Outlines_Color_TeammateMedics });
        color_entries.push_back(ColorEntry{ "Enemies", &CFG::Outlines_Color_Enemies });
        color_entries.push_back(ColorEntry{ "Local Buildings", &CFG::Outlines_Color_LocalBuildings });
        color_entries.push_back(ColorEntry{ "Teammate Dispensers", &CFG::Outlines_Color_TeammateDispensers });
        color_entries.push_back(ColorEntry{ "Teammate Buildings", &CFG::Outlines_Color_TeammateBuildings });
        color_entries.push_back(ColorEntry{ "Enemy Buildings", &CFG::Outlines_Color_EnemyBuildings });
        color_entries.push_back(ColorEntry{ "Health Pack", &CFG::Outlines_Color_HealthPack });
        color_entries.push_back(ColorEntry{ "Ammo Pack", &CFG::Outlines_Color_AmmoPack });
        color_entries.push_back(ColorEntry{ "Halloween Gift", &CFG::Outlines_Color_Halloween_Gift });
        color_entries.push_back(ColorEntry{ "MVM Money", &CFG::Outlines_Color_MVM_Money });
        color_entries.push_back(ColorEntry{ "Local Projectiles", &CFG::Outlines_Color_LocalProjectiles });
        color_entries.push_back(ColorEntry{ "Teammate Projectiles", &CFG::Outlines_Color_TeammateProjectiles });
        color_entries.push_back(ColorEntry{ "Enemy Projectiles", &CFG::Outlines_Color_EnemyProjectiles });
    }
    int num_colors = color_entries.size();
    int num_rows = (num_colors + 2) / 3; // ceil(num / 3)
    int base_item = current_item;
    static int editing_color = -1;
    static bool picker_open = false;
    static int return_to_item = 1;
    static int prev_editing_color = -1;
    static bool last_mouse_down = false;
    int max_name_width = 0;
    for (const auto& entry : color_entries) {
        int width = 0, height = 0;
        wchar_t wname[1024] = {};
        MultiByteToWideChar(CP_UTF8, 0, entry.name.c_str(), -1, wname, 1024);
        I::MatSystemSurface->GetTextSize(font.m_dwFont, wname, width, height);
        if (width > max_name_width) max_name_width = width;
    }
    for (int row = 0; row < num_rows; ++row) {
        for (int col = 0; col < 3; ++col) {
            int idx = row * 3 + col;
            if (idx >= num_colors) continue;
            int col_x = x_left + col * 200;
            int this_item = current_item;
            if (!menu_locked && menu::item_count == this_item) {
                H::Draw->String(font, col_x - 25, y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
            }
            if (!menu_locked && menu::item_count == this_item && (GetAsyncKeyState(VK_RETURN) & 1)) {
                editing_color = idx;
                picker_open = true;
                return_to_item = this_item;
            }
            if (!menu_locked && mouse_down && !last_mouse_down && mouse_x >= col_x && mouse_x <= col_x + max_name_width + 25 && mouse_y >= y - 10 && mouse_y <= y + 5) {
                editing_color = idx;
                picker_open = true;
                return_to_item = this_item;
            }
            // Draw name first
            H::Draw->String(font, col_x, y, Color_t(255, 255, 255, 255), POS_DEFAULT, color_entries[idx].name.c_str());
            // Compute name width for this specific name
            int name_width = 0, name_height = 0;
            wchar_t wname[1024] = {};
            MultiByteToWideChar(CP_UTF8, 0, color_entries[idx].name.c_str(), -1, wname, 1024);
            I::MatSystemSurface->GetTextSize(font.m_dwFont, wname, name_width, name_height);
            // Draw square after the name
            int square_size = name_height - 4; // proporcional ao texto
            if (square_size < 8) square_size = 8; // limite mínimo
            int square_x = col_x + name_width + 8;
            int square_y = y + (name_height / 2) - (square_size / 2);
            // outline
            H::Draw->OutlinedRect(
                square_x - 1,
                square_y - 1,
                square_size + 2,
                square_size + 2,
                Color_t(0, 0, 0, 255)
            );
            // fill
            H::Draw->Rect(
                square_x,
                square_y,
                square_size,
                square_size,
                *color_entries[idx].color
            );
            current_item++;
        }
        y += 15;
    }
    int max_color_item = base_item + num_colors - 1;
    if (picker_open && editing_color >= 0 && editing_color < num_colors) {
        if (prev_editing_color != editing_color) {
            prev_editing_color = editing_color;
        }
        Color_t* cur_color = color_entries[editing_color].color;
        int picker_x = x_left;
        int picker_y = y + 20;
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, picker_x - 25, picker_y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        int r = static_cast<int>(cur_color->r);
        int_slider(picker_x, &picker_y, "Red", r, 0, 255, current_item++);
        cur_color->r = static_cast<unsigned char>(r);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, picker_x - 25, picker_y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        int g = static_cast<int>(cur_color->g);
        int_slider(picker_x, &picker_y, "Green", g, 0, 255, current_item++);
        cur_color->g = static_cast<unsigned char>(g);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, picker_x - 25, picker_y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        int b = static_cast<int>(cur_color->b);
        int_slider(picker_x, &picker_y, "Blue", b, 0, 255, current_item++);
        cur_color->b = static_cast<unsigned char>(b);
        if (!menu_locked && menu::item_count == current_item) {
            H::Draw->String(font, picker_x - 25, picker_y, Color_t(0, 255, 0, 255), POS_DEFAULT, ">");
        }
        int a = static_cast<int>(cur_color->a);
        int_slider(picker_x, &picker_y, "Alpha", a, 0, 255, current_item++);
        cur_color->a = static_cast<unsigned char>(a);
        // Close with ESC
        if (GetAsyncKeyState(VK_ESCAPE) & 1) {
            picker_open = false;
            menu::item_count = return_to_item;
        }
        max_items = current_item - 1;
    }
    else {
        picker_open = false;
        max_items = max_color_item;
    }
    last_mouse_down = mouse_down;
    }
    // Clamp item_count
    if (menu::item_count > max_items) menu::item_count = 1;
    if (menu::item_count < 1) menu::item_count = max_items;
}