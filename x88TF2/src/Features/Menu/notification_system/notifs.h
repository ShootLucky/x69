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

struct Notification {
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

    Notification(const std::string& msg, int duration)
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

class NotificationSystem {
public:
    void add_notification(const std::string& message, int duration_ms = 2000);
    void run();

private:
    std::deque<Notification> notifications;
};

inline NotificationSystem* g_notification_system = new NotificationSystem();