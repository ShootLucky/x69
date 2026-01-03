// Updated notifs.cpp
#include "notifs.h"
#include "../menu.h"
#include <cmath>

void NotificationSystem::add_notification(const std::string& message, int duration_ms) {
    notifications.emplace_back(message, duration_ms);
}

struct TextSize {
    int width;
    int height;
};

// Accept font by reference and use H::Fonts / I::MatSystemSurface text API
TextSize get_string_size(const CFont& font, const char* text) {
    if (!text || !text[0])
        return { 0, 0 };
    wchar_t wbuf[1024];
    if (MultiByteToWideChar(CP_UTF8, 0, text, -1, wbuf, 1024) == 0) {
        return { 0, 0 };
    }
    int width = 0;
    int height = 0;
    I::MatSystemSurface->GetTextSize(font.m_dwFont, wbuf, width, height);
    return { width, height };
}

// Appearance tuning
constexpr int MAX_ALPHA = 255;
constexpr float MOVE_SPEED = 0.2f; // Smooth movement factor (0.0 to 1.0)

// Positioning tuning (base values for 1920x1080 resolution)
constexpr int BASE_START_Y = 20; // Top offset
constexpr int BASE_START_X = 20; // Left offset
constexpr int BASE_SPACING = 10;
constexpr int BASE_PADDING_X = 10;
constexpr int BASE_PADDING_Y = 5;
constexpr int BASE_WIDTH = 1920;
constexpr int BASE_HEIGHT = 1080;

constexpr size_t MAX_NOTIFICATIONS = 20;

inline Color_t apply_alpha(Color_t c, int alpha) {
    return Color_t(c.r, c.g, c.b, static_cast<unsigned char>(alpha));
}

// Function to draw outlined text
void DrawOutlinedString(const CFont& font, int x, int y, Color_t color, unsigned long alignment, const char* text, int alpha) {
    Color_t outline_color = Color_t(0, 0, 0, static_cast<unsigned char>(alpha)); // Black outline

    // Draw outlines (4 directions for simple outline)
    H::Draw->String(font, x - 1, y, outline_color, alignment, text);
    H::Draw->String(font, x + 1, y, outline_color, alignment, text);
    H::Draw->String(font, x, y - 1, outline_color, alignment, text);
    H::Draw->String(font, x, y + 1, outline_color, alignment, text);

    // Draw main text
    H::Draw->String(font, x, y, color, alignment, text);
}

void NotificationSystem::run() {
    // Use current screen size each frame via H::Draw
    H::Draw->UpdateScreenSize();
    int SCREEN_WIDTH = H::Draw->GetScreenW();
    int SCREEN_HEIGHT = H::Draw->GetScreenH();

    // Calculate scale factor based on resolution (preserve aspect ratio)
    float scale_x = static_cast<float>(SCREEN_WIDTH) / BASE_WIDTH;
    float scale_y = static_cast<float>(SCREEN_HEIGHT) / BASE_HEIGHT;
    float scale = std::min(scale_x, scale_y);

    // Scale positioning and sizing values
    int start_y = static_cast<int>(BASE_START_Y * scale);
    int start_x = static_cast<int>(BASE_START_X * scale);
    int spacing = static_cast<int>(BASE_SPACING * scale);
    int padding_x = static_cast<int>(BASE_PADDING_X * scale);
    int padding_y = static_cast<int>(BASE_PADDING_Y * scale);

    const CFont& font = H::Fonts->Get(EFonts::OTHER); // Assuming font is fixed-size; if scalable, adjust here

    auto now = std::chrono::steady_clock::now();

    // Calculate delta_time for frame-rate independent movement
    static auto last_time = now;
    float delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_time).count();
    last_time = now;

    // Check for high ping every 5 seconds
    static auto last_ping_check = now - std::chrono::seconds(6); // Initial offset to check immediately
    if (now - last_ping_check > std::chrono::seconds(5)) {
        last_ping_check = now;
        auto* netchan = I::EngineClient->GetNetChannelInfo();
        if (netchan) {
            float latency = netchan->GetLatency(FLOW_OUTGOING) + netchan->GetLatency(FLOW_INCOMING);
            int ping = static_cast<int>(latency * 1000.f);
            if (ping >= 100) {
                bool has_ping_notif = false;
                for (const auto& n : notifications) {
                    if (n.message.find("High ping:") == 0) {
                        has_ping_notif = true;
                        break;
                    }
                }
                if (!has_ping_notif) {
                    std::string msg = std::format("High ping: {} ms", ping);
                    add_notification(msg, 3000);
                }
            }
        }
    }

    // Force older notifications to fade faster if over limit
    if (notifications.size() > MAX_NOTIFICATIONS) {
        size_t excess = notifications.size() - MAX_NOTIFICATIONS;
        for (size_t i = 0; i < excess && i < notifications.size(); ++i) {
            auto& notif = notifications[i];
            notif.fade_out_speed = FADE_SPEED * 2.0f; // Faster fade out
            long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - notif.timestamp).count();
            if (elapsed < notif.duration_ms) {
                notif.timestamp = now - std::chrono::milliseconds(notif.duration_ms + 1); // Force start fading
            }
        }
    }

    // --- Processing & Cleanup Logic ---
    std::deque<Notification> updated_notifications;
    int current_target_y = start_y;
    while (!notifications.empty()) {
        auto notif = notifications.front(); // Pop later, use a copy or reference for manipulation
        notifications.pop_front();
        long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - notif.timestamp).count();
        bool should_expire = elapsed >= notif.duration_ms;

        // 4. Set the Target Y for the *current* notification
        notif.target_y = current_target_y;

        // 1. Initialize Y position on first run (after setting target_y)
        if (notif.current_y == 0) {
            // New notification. Initialize its current_y slightly above its target for a smooth drop.
            notif.current_y = notif.target_y - static_cast<int>(20 * scale);
        }

        // 2. Opacity Update (Fade In / Fade Out)
        if (should_expire) {
            // Fade Out
            notif.opacity = std::max(0.0f, notif.opacity - notif.fade_out_speed * delta_time);
        }
        else if (notif.opacity < 1.0f) {
            // Fade In
            notif.opacity = std::min(1.0f, notif.opacity + FADE_SPEED * delta_time);
        }

        // 3. Only keep notifications that are still visible or fading out
        if (notif.opacity > 0.0f) {
            // 5. Smooth Movement: Move current_y towards target_y
            float movement = (static_cast<float>(notif.target_y) - static_cast<float>(notif.current_y)) * MOVE_SPEED;
            notif.current_y += static_cast<int>(movement);

            // Calculate size for the next target Y offset
            TextSize size = get_string_size(font, notif.message.c_str());
            int box_height = size.height + spacing; // Adjusted without padding_y since no box

            // Update the target Y for the next notification
            current_target_y += box_height;
            updated_notifications.push_back(std::move(notif));
        }
    }
    notifications.clear();
    notifications = std::move(updated_notifications);

    for (const auto& notif : notifications) {
        std::string message = notif.message;
        int current_y = notif.current_y;
        int text_x = start_x;
        int alpha = static_cast<int>(notif.opacity * MAX_ALPHA);

        Color_t text_color = Colors::WHITE;
        text_color.a = static_cast<unsigned char>(alpha);

        // Check if it's a ping notification and set color accordingly
        if (message.find("High ping:") == 0) {
            size_t num_start = 11; // After "High ping: "
            size_t num_end = message.find(" ms");
            if (num_end != std::string::npos) {
                int ping_val = std::stoi(message.substr(num_start, num_end - num_start));
                if (ping_val >= 150) {
                    text_color = Color_t(255, 0, 0, static_cast<unsigned char>(alpha)); // Red
                }
                else if (ping_val >= 100) {
                    text_color = Color_t(255, 255, 0, static_cast<unsigned char>(alpha)); // Yellow
                }
            }
        }

        Color_t blue_color = Color_t(0, 0, 255, static_cast<unsigned char>(alpha)); // Blue color

        // No background or borders drawn

        // Draw the message with selective coloring for "x69" inside []
        std::string remaining = message;
        while (!remaining.empty()) {
            size_t start_pos = remaining.find("[");
            if (start_pos == std::string::npos) {
                // No more [, draw the remaining text with outline
                DrawOutlinedString(font, text_x, current_y, text_color, POS_DEFAULT, remaining.c_str(), alpha);
                break;
            }

            // Draw the text before the [
            std::string before = remaining.substr(0, start_pos);
            DrawOutlinedString(font, text_x, current_y, text_color, POS_DEFAULT, before.c_str(), alpha);
            text_x += get_string_size(font, before.c_str()).width;

            // Check if it's exactly "[x69]"
            if (remaining.size() >= start_pos + 5 && remaining.substr(start_pos, 5) == "[x69]") {
                // Draw [
                DrawOutlinedString(font, text_x, current_y, text_color, POS_DEFAULT, "[", alpha);
                text_x += get_string_size(font, "[").width;

                // Draw x69 in blue
                DrawOutlinedString(font, text_x, current_y, blue_color, POS_DEFAULT, "x69", alpha);
                text_x += get_string_size(font, "x69").width;

                // Draw ]
                DrawOutlinedString(font, text_x, current_y, text_color, POS_DEFAULT, "]", alpha);
                text_x += get_string_size(font, "]").width;

                // Move past the [x69]
                remaining = remaining.substr(start_pos + 5);
            }
            else {
                // Not [x69], draw the [ and continue
                DrawOutlinedString(font, text_x, current_y, text_color, POS_DEFAULT, "[", alpha);
                text_x += get_string_size(font, "[").width;
                remaining = remaining.substr(start_pos + 1);
            }
        }
    }
}