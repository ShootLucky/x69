// notifs.cpp - x88 Theme Edition
#include "notifs.h"
#include "icons_notify.h"
#include "../menu.h"
#include <cmath>

// ===== X88 DESIGN CONSTANTS =====
constexpr int MAX_ALPHA = 255;
constexpr float MOVE_SPEED = 0.20f;
constexpr float SLIDE_IN_OFFSET = 60.0f;

// Positioning (base 1920x1080)
constexpr int BASE_START_Y = 100;
constexpr int BASE_START_X_OFFSET = 25;
constexpr int BASE_SPACING = 8;
constexpr int BASE_PADDING_X = 18;
constexpr int BASE_PADDING_Y = 12;
constexpr int BASE_NOTIF_WIDTH = 360;
constexpr int BASE_CORNER_RADIUS = 4;
constexpr int BASE_ICON_SIZE = 20;
constexpr int BASE_WIDTH = 1920;
constexpr int BASE_HEIGHT = 1080;
constexpr size_t MAX_NOTIFICATIONS = 20;
constexpr int MAX_TEXT_WIDTH = 270;

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

inline Color_t apply_alpha(Color_t c, int alpha) {
    return Color_t(c.r, c.g, c.b, static_cast<unsigned char>(alpha));
}

// Enum para tipos de ícones
enum class IconType {
    NOTIFICATION,
    PING,
    DAMAGE,
    DEATH,
    RESPAWN,
    CLASS_CHANGE,
    // Player flags
    NETHOOK,
    LMAOBOX,
    RIJIN,
    CHEATER,
    CHEATER_LIGHT,
    SUSPECT,
    RETARD_LEGIT,
    IGNORED
};

// Detectar tipo de ícone pela mensagem
IconType DetectIconType(const std::string& message) {
    // Player flags (prioridade máxima)
    if (message.find("[Nethook User]") != std::string::npos) return IconType::NETHOOK;
    if (message.find("[Lmaobox User]") != std::string::npos) return IconType::LMAOBOX;
    if (message.find("[Rijin User]") != std::string::npos) return IconType::RIJIN;
    if (message.find("[Cheater Light]") != std::string::npos) return IconType::CHEATER_LIGHT;
    if (message.find("[Cheater]") != std::string::npos) return IconType::CHEATER;
    if (message.find("[Suspect]") != std::string::npos) return IconType::SUSPECT;
    if (message.find("[Retard Legit]") != std::string::npos) return IconType::RETARD_LEGIT;
    if (message.find("[Ignored]") != std::string::npos) return IconType::IGNORED;

    // Game events
    if (message.find("High ping:") != std::string::npos) return IconType::PING;
    if (message.find("You fried") != std::string::npos ||
        message.find("killed") != std::string::npos ||
        message.find("frag") != std::string::npos) return IconType::DEATH;
    if (message.find("Damaged") != std::string::npos) return IconType::DAMAGE;
    if (message.find("respawned") != std::string::npos ||
        message.find("spawned") != std::string::npos) return IconType::RESPAWN;
    if (message.find("changed class") != std::string::npos) return IconType::CLASS_CHANGE;

    return IconType::NOTIFICATION;
}

struct TextSize {
    int width;
    int height;
};

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

// Texto com outline mais sutil e moderno
void DrawModernText(const CFont& font, int x, int y, Color_t color, const char* text, int alpha) {
    // Sombra suave
    Color_t shadow = Color_t(0, 0, 0, alpha * 0.6f);
    H::Draw->String(font, x + 1, y + 1, shadow, POS_DEFAULT, text);

    // Texto principal
    color.a = static_cast<unsigned char>(alpha);
    H::Draw->String(font, x, y, color, POS_DEFAULT, text);
}

std::vector<std::string> WrapText(const std::string& text, const CFont& font, int max_width) {
    std::vector<std::string> lines;
    std::string current_line;
    std::string word;

    for (char c : text) {
        if (c == ' ') {
            std::string test_line = current_line.empty() ? word : current_line + " " + word;
            if (get_string_size(font, test_line.c_str()).width <= max_width) {
                current_line = test_line;
            }
            else {
                if (!current_line.empty()) lines.push_back(current_line);
                current_line = word;
            }
            word.clear();
        }
        else {
            word += c;
        }
    }

    if (!word.empty()) {
        std::string test_line = current_line.empty() ? word : current_line + " " + word;
        if (get_string_size(font, test_line.c_str()).width <= max_width) {
            current_line = test_line;
        }
        else {
            if (!current_line.empty()) lines.push_back(current_line);
            current_line = word;
        }
    }

    if (!current_line.empty()) lines.push_back(current_line);

    return lines.empty() ? std::vector<std::string>{text} : lines;
}

// X88 Style Notification Box - Clean & Modern
void DrawNotificationBox(int x, int y, int w, int h, int alpha, int corner_radius, Color_t accent_color) {
    // Background principal - mais escuro
    Color_t bg = apply_alpha(BG_PRIMARY, alpha);
    H::Draw->FillRectRounded(x, y, w, h, corner_radius, bg);

    // Barra lateral esquerda (accent color) - usando FillRectRounded com corner_radius 0
    Color_t accent_bar = apply_alpha(accent_color, alpha);
    H::Draw->FillRectRounded(x, y, 3, h, 0, accent_bar);
}

void NotificationSystem::add_notification(const std::string& message, int duration_ms) {
    notifications.emplace_back(message, duration_ms);
}

void NotificationSystem::run() {
    H::Draw->UpdateScreenSize();
    int SCREEN_WIDTH = H::Draw->GetScreenW();
    int SCREEN_HEIGHT = H::Draw->GetScreenH();

    float scale_x = static_cast<float>(SCREEN_WIDTH) / BASE_WIDTH;
    float scale_y = static_cast<float>(SCREEN_HEIGHT) / BASE_HEIGHT;
    float scale = std::min(scale_x, scale_y);

    int start_y = static_cast<int>(BASE_START_Y * scale);
    int start_x = SCREEN_WIDTH - static_cast<int>((BASE_NOTIF_WIDTH + BASE_START_X_OFFSET) * scale);
    int spacing = static_cast<int>(BASE_SPACING * scale);
    int padding_x = static_cast<int>(BASE_PADDING_X * scale);
    int padding_y = static_cast<int>(BASE_PADDING_Y * scale);
    int notif_width = static_cast<int>(BASE_NOTIF_WIDTH * scale);
    int corner_radius = static_cast<int>(BASE_CORNER_RADIUS * scale);
    int icon_size = static_cast<int>(BASE_ICON_SIZE * scale);
    int max_text_width = static_cast<int>(MAX_TEXT_WIDTH * scale);

    const CFont& font = H::Fonts->Get(EFonts::OTHER);

    auto now = std::chrono::steady_clock::now();
    static auto last_time = now;
    float delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_time).count();
    last_time = now;

    static auto last_ping_check = now - std::chrono::seconds(6);
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
                    add_notification(std::format("High ping: {} ms", ping), 3000);
                }
            }
        }
    }

    if (notifications.size() > MAX_NOTIFICATIONS) {
        size_t excess = notifications.size() - MAX_NOTIFICATIONS;
        for (size_t i = 0; i < excess && i < notifications.size(); ++i) {
            auto& notif = notifications[i];
            notif.fade_out_speed = FADE_SPEED * 2.0f;
            long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - notif.timestamp).count();
            if (elapsed < notif.duration_ms) {
                notif.timestamp = now - std::chrono::milliseconds(notif.duration_ms + 1);
            }
        }
    }

    std::deque<Notification> updated_notifications;
    int current_target_y = start_y;

    while (!notifications.empty()) {
        auto notif = notifications.front();
        notifications.pop_front();

        long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - notif.timestamp).count();
        bool should_expire = elapsed >= notif.duration_ms;

        notif.target_y = current_target_y;

        if (notif.current_y == 0) {
            notif.current_y = notif.target_y;
            notif.slide_offset = SLIDE_IN_OFFSET * scale;
        }

        if (should_expire) {
            notif.opacity = std::max(0.0f, notif.opacity - notif.fade_out_speed * delta_time);
        }
        else if (notif.opacity < 1.0f) {
            notif.opacity = std::min(1.0f, notif.opacity + FADE_SPEED * delta_time);
        }

        if (notif.slide_offset > 0.0f) {
            notif.slide_offset = std::max(0.0f, notif.slide_offset - (SLIDE_IN_OFFSET * 3.5f * delta_time));
        }

        if (notif.opacity > 0.0f) {
            float movement = (static_cast<float>(notif.target_y) - static_cast<float>(notif.current_y)) * MOVE_SPEED;
            notif.current_y += static_cast<int>(movement);

            auto lines = WrapText(notif.message, font, max_text_width);
            int line_height = get_string_size(font, "A").height;
            int box_height = (line_height * lines.size()) + padding_y * 2 + spacing;

            current_target_y += box_height;

            updated_notifications.push_back(std::move(notif));
        }
    }

    notifications.clear();
    notifications = std::move(updated_notifications);

    for (const auto& notif : notifications) {
        int alpha = static_cast<int>(notif.opacity * MAX_ALPHA);
        int draw_x = start_x + static_cast<int>(notif.slide_offset);
        int draw_y = notif.current_y;

        auto lines = WrapText(notif.message, font, max_text_width);
        int line_height = get_string_size(font, "A").height;
        int box_height = (line_height * lines.size()) + padding_y * 2;

        IconType icon_type = DetectIconType(notif.message);
        Color_t icon_color = ACCENT_PRIMARY;
        int ping_value = 0;

        switch (icon_type) {
        case IconType::PING: {
            size_t num_start = notif.message.find(":") + 2;
            size_t num_end = notif.message.find(" ms");
            if (num_end != std::string::npos && num_start < num_end) {
                ping_value = std::stoi(notif.message.substr(num_start, num_end - num_start));
                if (ping_value >= 150) icon_color = PING_RED;
                else if (ping_value >= 100) icon_color = PING_YELLOW;
                else icon_color = PING_GREEN;
            }
            else {
                icon_color = PING_YELLOW;
            }
            break;
        }
        case IconType::DAMAGE: icon_color = DAMAGE_COLOR; break;
        case IconType::DEATH: icon_color = DEATH_COLOR; break;
        case IconType::RESPAWN: icon_color = RESPAWN_COLOR; break;
        case IconType::CLASS_CHANGE: icon_color = CLASS_COLOR; break;
        case IconType::NETHOOK: icon_color = NETHOOK_COLOR; break;
        case IconType::LMAOBOX: icon_color = LMAOBOX_COLOR; break;
        case IconType::RIJIN: icon_color = RIJIN_COLOR; break;
        case IconType::CHEATER: icon_color = CHEATER_COLOR; break;
        case IconType::CHEATER_LIGHT: icon_color = CHEATER_LIGHT_COLOR; break;
        case IconType::SUSPECT: icon_color = SUSPECT_COLOR; break;
        case IconType::RETARD_LEGIT: icon_color = RETARD_LEGIT_COLOR; break;
        case IconType::IGNORED: icon_color = IGNORED_COLOR; break;
        default: icon_color = ACCENT_PRIMARY; break;
        }

        // Draw box with accent color
        DrawNotificationBox(draw_x, draw_y, notif_width, box_height, alpha, corner_radius, icon_color);

        // Draw icon
        int icon_x = draw_x + padding_x;
        int icon_y = draw_y + (box_height - icon_size) / 2;

        switch (icon_type) {
        case IconType::PING: DrawPingIcon(icon_x, icon_y, icon_size, icon_color, alpha, ping_value); break;
        case IconType::DAMAGE: DrawDamageIcon(icon_x, icon_y, icon_size, icon_color, alpha); break;
        case IconType::DEATH: DrawDeathIcon(icon_x, icon_y, icon_size, icon_color, alpha); break;
        case IconType::RESPAWN: DrawRespawnIcon(icon_x, icon_y, icon_size, icon_color, alpha); break;
        case IconType::CLASS_CHANGE: DrawClassIcon(icon_x, icon_y, icon_size, icon_color, alpha); break;
        case IconType::NETHOOK: DrawNethookIcon(icon_x, icon_y, icon_size, icon_color, alpha); break;
        case IconType::LMAOBOX: DrawLmaoboxIcon(icon_x, icon_y, icon_size, icon_color, alpha); break;
        case IconType::RIJIN: DrawRijinIcon(icon_x, icon_y, icon_size, icon_color, alpha); break;
        case IconType::CHEATER: DrawCheaterIcon(icon_x, icon_y, icon_size, icon_color, alpha); break;
        case IconType::CHEATER_LIGHT: DrawCheaterLightIcon(icon_x, icon_y, icon_size, icon_color, alpha); break;
        case IconType::SUSPECT: DrawSuspectIcon(icon_x, icon_y, icon_size, icon_color, alpha); break;
        case IconType::RETARD_LEGIT: DrawRetardLegitIcon(icon_x, icon_y, icon_size, icon_color, alpha); break;
        case IconType::IGNORED: DrawIgnoredIcon(icon_x, icon_y, icon_size, icon_color, alpha); break;
        default: DrawNotificationIcon(icon_x, icon_y, icon_size, ACCENT_PRIMARY, alpha); break;
        }

        // Draw text
        int text_x = draw_x + padding_x + icon_size + 12;
        int text_y = draw_y + padding_y;

        Color_t text_color = TEXT_PRIMARY;

        for (const auto& line : lines) {
            DrawModernText(font, text_x, text_y, text_color, line.c_str(), alpha);
            text_y += line_height;
        }
    }

    // Ping indicator - Modern style
    auto* netchan = I::EngineClient->GetNetChannelInfo();
    if (netchan) {
        float latency = netchan->GetLatency(FLOW_OUTGOING) + netchan->GetLatency(FLOW_INCOMING);
        int current_ping = static_cast<int>(latency * 1000.f);

        if (current_ping >= 100) {
            Color_t ping_color = (current_ping >= 150) ? PING_RED : PING_YELLOW;

            int indicator_x = static_cast<int>(25 * scale);
            int indicator_y = SCREEN_HEIGHT / 2 - static_cast<int>(40 * scale);
            int indicator_w = static_cast<int>(90 * scale);
            int indicator_h = static_cast<int>(70 * scale);

            // Pulse animation mais suave
            float pulse = (std::sin(std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count() * 0.002f) + 1.0f) * 0.5f;
            int pulse_alpha = static_cast<int>(180 + pulse * 75);

            DrawNotificationBox(indicator_x, indicator_y, indicator_w, indicator_h,
                pulse_alpha, corner_radius, ping_color);

            // Texto "PING" menor e mais discreto
            int text_offset_y = indicator_y + 12;
            DrawModernText(font, indicator_x + indicator_w / 2 - 18, text_offset_y,
                TEXT_SECONDARY, "PING", pulse_alpha);

            // Valor do ping maior e destacado
            std::string ping_str = std::to_string(current_ping) + "ms";
            TextSize ping_size = get_string_size(font, ping_str.c_str());
            DrawModernText(font, indicator_x + (indicator_w - ping_size.width) / 2,
                text_offset_y + 28, ping_color, ping_str.c_str(), pulse_alpha);
        }
    }
}