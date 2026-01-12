// functions_menu.h
#pragma once

#include <string>
#include <vector>
#include <functional>
#include "../../../SDK/SDK.h"  // For Color_t, etc.
#include "../src/SDK/Helpers/Draw/Draw.h"  // For H::Draw

enum text_type {
    info = 0,
    regular,
    enabled,
    enabled_green,
    warning,
    extra
};

class PlayerPriority;  // Forward declaration if needed

std::string GetKeyName(int vk);
Color_t HSVToRGB(float h, float s, float v);
void RGBToHSV(const Color_t& rgb, float& h, float& s, float& v);

void text(int x, int* y, std::string text, text_type type, int alpha = 255);
void checkbox(int x, int* y, std::string text, bool* option, bool special = false, int alpha = 255, int item_counts = 1, int entindex = 0, PlayerPriority* pri = nullptr, bool use_return = false);
void combo(int x, int* y, std::string text, int* option, std::vector<std::string> aliases, bool special = false, int alpha = 255, int item_counts = 1);
void multi_combo(int x, int* y, std::string text, int* option, std::vector<std::string> aliases, int alpha = 255, int item_counts = 1);
void int_slider(int x, int* y, std::string text, int& option, int min_value = 0, int max_value = 0, int item_counts = 1);
void float_slider(int x, int* y, std::string text, float& option, float min_value = 0.f, float max_value = 0.f, int item_counts = 1);
void key_selector(int x, int* y, int* key, int item_counts = 1);
void text_input(int x, int* y, std::string text_label, std::string& input_str, int item_counts = 1);
void button(int x, int* y, std::string text, std::function<void()> action, int item_counts = 1);

std::vector<std::string> RefreshConfigFiles();
void LoadSelectedConfig();
void CreateConfig();
void SaveConfig();
void LoadConfig();