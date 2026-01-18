// notifs.h
#pragma once
#include "../src/SDK/SDK.h"
#include "../src/SDK/Helpers/Draw/Draw.h"
#include <string>
#include <deque>
#include <chrono>
#include <format>
#include <vector>
#include <algorithm>

// Appearance tuning (moved here for accessibility)
constexpr float FADE_SPEED = 4.5f; // Opacity units per second

// X88 Theme Colors - Dark & Professional
constexpr Color_t BG_PRIMARY = Color_t(12, 12, 15, 245);        // Darker background
constexpr Color_t BG_SECONDARY = Color_t(18, 18, 22, 230);      // Slightly lighter
constexpr Color_t ACCENT_PRIMARY = Color_t(120, 120, 255, 255); // Purple-ish blue
constexpr Color_t ACCENT_GLOW = Color_t(120, 120, 255, 40);     // Glow effect
constexpr Color_t BORDER_MAIN = Color_t(45, 45, 55, 200);       // Subtle border
constexpr Color_t BORDER_ACCENT = Color_t(120, 120, 255, 180);  // Accent border
constexpr Color_t TEXT_PRIMARY = Color_t(240, 240, 245, 255);   // Almost white
constexpr Color_t TEXT_SECONDARY = Color_t(180, 180, 190, 255); // Dimmed text

// Icon accent colors
constexpr Color_t PING_GREEN = Color_t(80, 220, 130, 255);
constexpr Color_t PING_YELLOW = Color_t(255, 200, 80, 255);
constexpr Color_t PING_RED = Color_t(255, 90, 90, 255);
constexpr Color_t DAMAGE_COLOR = Color_t(255, 100, 100, 255);
constexpr Color_t DEATH_COLOR = Color_t(255, 70, 70, 255);
constexpr Color_t RESPAWN_COLOR = Color_t(100, 255, 150, 255);
constexpr Color_t CLASS_COLOR = Color_t(255, 180, 100, 255);

// Player flag colors - more muted for x88 theme
constexpr Color_t NETHOOK_COLOR = Color_t(150, 100, 200, 255);
constexpr Color_t LMAOBOX_COLOR = Color_t(80, 220, 220, 255);
constexpr Color_t RIJIN_COLOR = Color_t(255, 100, 200, 255);
constexpr Color_t CHEATER_COLOR = Color_t(255, 80, 80, 255);
constexpr Color_t CHEATER_LIGHT_COLOR = Color_t(255, 140, 140, 255);
constexpr Color_t SUSPECT_COLOR = Color_t(255, 200, 80, 255);
constexpr Color_t RETARD_LEGIT_COLOR = Color_t(255, 180, 60, 255);
constexpr Color_t IGNORED_COLOR = Color_t(140, 140, 150, 255);

struct Notification_t {
    std::string message;
    std::chrono::steady_clock::time_point timestamp;
    int duration_ms; // Duration in milliseconds

    // --- Animation members ---
    float opacity = 0.0f;        // 0.0 (transparent) to 1.0 (opaque)
    int current_y = 0;           // The actual Y position used for rendering (smoothly updated)
    int target_y = 0;            // The intended final Y position in the layout
    float slide_offset = 0.0f;   // Horizontal slide animation offset
    float fade_out_speed = FADE_SPEED;
    // ---------------------------------

    Notification_t(const std::string& msg, int duration)
        : message(msg),
        timestamp(std::chrono::steady_clock::now()),
        duration_ms(duration),
        opacity(0.0f),
        current_y(0),
        target_y(0),
        slide_offset(0.0f),
        fade_out_speed(FADE_SPEED)
    {
    }
};

class CNotify {
public:
    void add_notification(const std::string& message, int duration_ms = 2000);
    void Draw();

private:
    std::deque<Notification_t> notifications;
};

extern CNotify gNotify;