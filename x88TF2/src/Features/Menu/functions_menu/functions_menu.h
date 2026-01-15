// functions_menu.h
#pragma once

// Ensure Menu.h (which defines 'text_type') is included first to avoid
// missing-identifier errors when this header is included standalone.
#include "../Menu.h"  // For menu namespace and text_type

#include <string>
#include <vector>
#include <functional>

#include "../../../SDK/SDK.h"  // For Color_t, etc.
#include "../../../SDK/Helpers/Draw/Draw.h"  // For H::Draw

// Forward declaration if needed
class PlayerPriority;

std::string GetKeyName(int vk);
Color_t HSVToRGB(float h, float s, float v);
void RGBToHSV(const Color_t& rgb, float& h, float& s, float& v);

// Helper function to check if mouse is in rect
bool IsMouseInRectHelper(int x, int y, int w, int h);

// NOTE: use the `text_type` enum declared in Menu.h
// Added final `bool calc_height = false` parameter to match call sites in Menu.cpp.
// Default values left so existing calls without the parameter keep working.
void text(int x, int* y, std::string text, text_type type, int alpha = 255, bool calc_height = false);
void checkbox(int x, int* y, std::string text, bool* option, bool special = false, int alpha = 255, int item_counts = 1, int entindex = 0, PlayerPriority* pri = nullptr, bool use_return = false, bool calc_height = false);
void combo(int x, int* y, std::string text, int* option, std::vector<std::string> aliases, bool special = false, int alpha = 255, int item_counts = 1, bool calc_height = false);
void multi_combo(int x, int* y, std::string text, int* option, std::vector<std::string> aliases, int alpha = 255, int item_counts = 1, bool calc_height = false);
void int_slider(int x, int* y, std::string text, int& option, int min_value = 0, int max_value = 0, int item_counts = 1, bool calc_height = false);
void float_slider(int x, int* y, std::string text, float& option, float min_value = 0.f, float max_value = 0.f, int item_counts = 1, bool calc_height = false);
void key_selector(int x, int* y, int* key, int item_counts = 1, bool calc_height = false);
void text_input(int x, int* y, std::string text_label, std::string& input_str, int item_counts = 1, bool calc_height = false);
void button(int x, int* y, std::string text, std::function<void()> action, int item_counts = 1, bool calc_height = false);

std::vector<std::string> RefreshConfigFiles();
void LoadSelectedConfig();
void CreateConfig();
void SaveConfig();
void LoadConfig();

// Moved helper functions declarations
namespace menu {
    bool IsMouseInRect(int x, int y, int w, int h);
    void HandleDrag();
    void HandleTabClick();
    void HandleScroll();

    // ============= SCROLL SYSTEM DECLARATIONS =============
    // Variáveis de scroll por tab
    extern int scroll_offset_aimbot;
    extern int scroll_offset_antiaim;
    extern int scroll_offset_visuals;
    extern int scroll_offset_skins;
    extern int scroll_offset_misc;

    // Área visível de conteúdo
    extern int content_visible_height;
    extern int total_content_height;

    // ScrollBar
    extern bool scrollbar_dragging;
    extern int scrollbar_drag_start_y;
    extern int scrollbar_drag_start_offset;

    // Funções do scroll system
    int& GetCurrentScrollOffset();
    void ResetScrollOnTabChange();
}