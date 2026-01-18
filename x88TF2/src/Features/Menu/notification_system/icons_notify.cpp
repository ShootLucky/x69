#include "icons_notify.h"
#include <cmath>  // For std::cos, std::sin, etc.
#include <algorithm>  // For std::min

// Nethook - "NT" em roxo (MAIOR)
void DrawNethookIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    // "NT" text - MAIOR
    int letter_width = 4;
    int letter_height = size * 3 / 4;

    // N
    H::Draw->RectFilled(cx - size / 3, cy - letter_height / 2, letter_width, letter_height, color);
    H::Draw->RectFilled(cx - size / 3, cy - letter_height / 2, size / 3, letter_width, color);
    H::Draw->RectFilled(cx - size / 3 + size / 3 - letter_width, cy - letter_height / 2, letter_width, letter_height, color);

    // T
    H::Draw->RectFilled(cx + size / 12, cy - letter_height / 2, size / 3, letter_width, color);
    H::Draw->RectFilled(cx + size / 4, cy - letter_height / 2, letter_width, letter_height, color);
}

// Lmaobox - Uma caixa (MAIOR)
void DrawLmaoboxIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;
    int box_size = size * 3 / 4;

    // Sombra da caixa
    Color_t shadow = Color_t(0, 0, 0, alpha / 2);
    H::Draw->RectFilled(cx - box_size / 2 + 2, cy - box_size / 2 + 2, box_size, box_size, shadow);

    // Caixa principal com gradiente
    Color_t box_light = color;
    Color_t box_dark = Color_t(color.r * 0.7f, color.g * 0.7f, color.b * 0.7f, alpha);
    H::Draw->RectGradient(cx - box_size / 2, cy - box_size / 2, box_size, box_size, box_light, box_dark, false);

    // Borda da caixa
    H::Draw->RectOutlined(cx - box_size / 2, cy - box_size / 2, box_size, box_size, Color_t(255, 255, 255, alpha), Color_t(255, 255, 255, alpha));

    // Tampa da caixa (linha no topo)
    Color_t lid = Color_t(255, 255, 255, alpha / 2);
    H::Draw->RectFilled(cx - box_size / 2, cy - box_size / 2, box_size, 4, lid);

    // Detalhes (fita adesiva)
    H::Draw->RectFilled(cx - 2, cy - box_size / 2, 4, box_size, Color_t(200, 180, 100, alpha));
}

// Rijin - "R" azul/magenta (MAIOR)
void DrawRijinIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    // "R" letter - MAIOR
    int letter_width = 4;
    int letter_height = size * 3 / 4;

    // Vertical line
    H::Draw->RectFilled(cx - size / 4, cy - letter_height / 2, letter_width, letter_height, color);

    // Top curve (semicircle)
    H::Draw->RectFilled(cx - size / 4, cy - letter_height / 2, size / 2, letter_width, color);
    H::Draw->RectFilled(cx - size / 4 + size / 2 - letter_width, cy - letter_height / 2, letter_width, letter_height / 3, color);
    H::Draw->RectFilled(cx - size / 4, cy - letter_height / 2 + letter_height / 3, size / 2, letter_width, color);

    // Diagonal leg
    for (int i = 0; i < letter_height / 2; i++) {
        H::Draw->RectFilled(cx - size / 4 + i, cy + i / 2, letter_width, letter_width, color);
    }
}

// Cheater - Placa de aviso (triângulo com !) (MAIOR)
void DrawCheaterIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    // Triângulo de aviso - MAIOR
    const Vec2 p1(cx, y + 1);
    const Vec2 p2(x + 1, y + size - 1);
    const Vec2 p3(x + size - 1, y + size - 1);
    H::Draw->TriangleFilled(p1, p2, p3, color);

    // Borda preta
    Color_t border = Color_t(0, 0, 0, alpha);
    H::Draw->Line(cx, y + 1, x + 1, y + size - 1, border);
    H::Draw->Line(cx, y + 1, x + size - 1, y + size - 1, border);
    H::Draw->Line(x + 1, y + size - 1, x + size - 1, y + size - 1, border);

    // Ponto de exclamação - MAIOR
    Color_t excl = Color_t(0, 0, 0, alpha);
    H::Draw->RectFilled(cx - 2, cy - 8, 4, 12, excl);
    H::Draw->RectFilled(cx - 2, cy + 6, 4, 3, excl);
}

// Cheater Light - Placa de aviso rosa/clara
void DrawCheaterLightIcon(int x, int y, int size, Color_t color, int alpha) {
    DrawCheaterIcon(x, y, size, color, alpha); // Mesmo ícone, cor diferente
}

// Suspect - ??? amarelo (MAIOR)
void DrawSuspectIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    // "?" symbol - MAIOR
    int qmark_width = 4;

    // Top curve
    H::Draw->CircleFilled(cx, cy - 6, 6, 20, color);
    H::Draw->RectFilled(cx + 4, cy - 6, qmark_width, 9, color);

    // Vertical line
    H::Draw->RectFilled(cx - qmark_width / 2, cy + 2, qmark_width, 6, color);

    // Dot
    H::Draw->RectFilled(cx - qmark_width / 2, cy + 9, qmark_width, qmark_width, color);
}

// Retard Legit - #$@ amarelo (MAIOR)
void DrawRetardLegitIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    // "#" symbol - MAIOR
    int hash_size = size / 2;
    H::Draw->RectFilled(cx - hash_size / 2, cy - hash_size / 2 - 2, hash_size, 4, color);
    H::Draw->RectFilled(cx - hash_size / 2, cy + hash_size / 2 - 2, hash_size, 4, color);
    H::Draw->RectFilled(cx - hash_size / 2 - 2, cy - hash_size / 2, 4, hash_size, color);
    H::Draw->RectFilled(cx + hash_size / 2 - 2, cy - hash_size / 2, 4, hash_size, color);

    // "$" symbol - MAIOR
    H::Draw->CircleFilled(cx + hash_size, cy, 6, 16, color);
    H::Draw->RectFilled(cx + hash_size - 2, cy - 8, 4, 16, color);

    // "@" symbol - MAIOR
    H::Draw->CircleFilled(cx - hash_size, cy + hash_size / 2, 8, 20, color);
    H::Draw->CircleFilled(cx - hash_size + 4, cy + hash_size / 2, 4, 12, Color_t(0, 0, 0, alpha));
}

// Ignored - X cinza (MAIOR)
void DrawIgnoredIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    // X symbol - MAIOR
    int x_length = size * 3 / 4;
    int x_width = 4;

    // Left arm
    for (int i = -x_width / 2; i <= x_width / 2; i++) {
        H::Draw->Line(cx - x_length / 2 + i, cy - x_length / 2 - i, cx + x_length / 2 + i, cy + x_length / 2 - i, color);
    }

    // Right arm
    for (int i = -x_width / 2; i <= x_width / 2; i++) {
        H::Draw->Line(cx - x_length / 2 + i, cy + x_length / 2 + i, cx + x_length / 2 + i, cy - x_length / 2 + i, color);
    }
}

// Ícone genérico (MAIOR)
void DrawNotificationIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    // Bell shape - MAIOR
    std::array<Vec2, 3> bell_top = {
        Vec2(cx - size / 2, cy),
        Vec2(cx + size / 2, cy),
        Vec2(cx, cy - size / 2)
    };
    H::Draw->TriangleFilled(bell_top[0], bell_top[1], bell_top[2], color);

    H::Draw->RectFilled(cx - size / 4, cy, size / 2, size / 4, color);

    H::Draw->CircleFilled(cx, cy + size / 2, 3, 12, color);
}

// Ícone de ping (MAIOR)
void DrawPingIcon(int x, int y, int size, Color_t color, int alpha, int value) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    // Bars - MAIORES
    int bar_width = size / 5;
    int max_height = size * 3 / 4;

    for (int i = 0; i < 4; i++) {
        int bar_height = max_height * (i + 1) / 4;
        int bar_x = x + i * (bar_width + 2);
        Color_t bar_color = (value > i * 50) ? color : Color_t(100, 100, 100, alpha / 2);
        H::Draw->RectFilled(bar_x, y + size - bar_height, bar_width, bar_height, bar_color);
    }

    // Number - MAIOR
    std::string val_str = std::to_string(value);
    int text_x = cx - 5;
    int text_y = cy + size / 4 + 2;
    H::Draw->Text(text_x, text_y, H::Fonts->Get(EFonts::ESP_SMALL).m_dwFont, Color_t(255, 255, 255, alpha), ALIGN_CENTER_H, val_str.c_str());
}

// Ícone de dano (MAIOR)
void DrawDamageIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    // Crosshair-like - MAIOR
    int cross_length = size / 2;
    int cross_width = 3;

    H::Draw->RectFilled(cx - cross_length / 2, cy - cross_width / 2, cross_length, cross_width, color);
    H::Draw->RectFilled(cx - cross_width / 2, cy - cross_length / 2, cross_width, cross_length, color);

    Color_t glow = Color_t(color.r, color.g, color.b, alpha / 3);
    H::Draw->RectFilled(cx - cross_length / 2 - 1, cy - cross_width / 2 - 1, cross_length + 2, cross_width + 2, glow);
    H::Draw->RectFilled(cx - cross_width / 2 - 1, cy - cross_length / 2 - 1, cross_width + 2, cross_length + 2, glow);
}

// Ícone de morte (MAIOR)
void DrawDeathIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    // Skull - MAIOR
    H::Draw->CircleFilled(cx, cy - 2, size / 3 + 2, 24, color);

    // Eyes
    H::Draw->CircleFilled(cx - size / 6, cy - size / 6, 2, 8, Color_t(0, 0, 0, alpha));
    H::Draw->CircleFilled(cx + size / 6, cy - size / 6, 2, 8, Color_t(0, 0, 0, alpha));

    // Mouth
    H::Draw->RectFilled(cx - size / 6, cy + size / 6, size / 3, 2, Color_t(0, 0, 0, alpha));

    // Teeth
    for (int i = 0; i < 3; i++) {
        int tx = cx - size / 6 + i * (size / 6);
        H::Draw->RectFilled(tx, cy + size / 6 - 2, 1, 2, Color_t(255, 255, 255, alpha));
        H::Draw->RectFilled(tx, cy + size / 6 + 2, 1, 2, Color_t(255, 255, 255, alpha));
    }

    // Bones
    H::Draw->Line(cx - size / 4, cy + size / 3, cx + size / 4, cy + size / 3 + 4, color);
    H::Draw->Line(cx, cy + size / 3 + 8, cx, cy + size / 2, color);
}

// Ícone de respawn (MAIOR)
void DrawRespawnIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    int core_size = size / 3;
    int spike_length = size / 2;

    // 8 spikes - MAIORES
    for (int i = 0; i < 8; i++) {
        float angle = (i * 45.0f) * (3.14159f / 180.0f);
        int start_x = cx + static_cast<int>(std::cos(angle) * 3);
        int start_y = cy + static_cast<int>(std::sin(angle) * 3);

        float perpendicular = angle + (3.14159f / 2.0f);
        int base_width = (i % 2 == 0) ? 4 : 3;

        int base_x1 = start_x + static_cast<int>(std::cos(perpendicular) * base_width);
        int base_y1 = start_y + static_cast<int>(std::sin(perpendicular) * base_width);
        int base_x2 = start_x - static_cast<int>(std::cos(perpendicular) * base_width);
        int base_y2 = start_y - static_cast<int>(std::sin(perpendicular) * base_width);

        int tip_x = cx + static_cast<int>(std::cos(angle) * spike_length);
        int tip_y = cy + static_cast<int>(std::sin(angle) * spike_length);

        H::Draw->TriangleFilled(Vec2(base_x1, base_y1), Vec2(base_x2, base_y2), Vec2(tip_x, tip_y), color);

        Color_t spike_glow = Color_t(color.r, color.g, color.b, alpha / 4);
        H::Draw->TriangleFilled(Vec2(base_x1 + 1, base_y1 + 1), Vec2(base_x2 + 1, base_y2 + 1), Vec2(tip_x + 1, tip_y + 1), spike_glow);
    }

    // Core brilhante - MAIOR
    Color_t bright = Color_t(
        std::min(255, color.r + 80),
        std::min(255, color.g + 80),
        std::min(255, color.b + 80),
        alpha
    );
    H::Draw->CircleFilled(cx, cy, core_size, 20, bright);

    Color_t white = Color_t(255, 255, 255, alpha);
    H::Draw->CircleFilled(cx, cy, 3, 12, white);
}

// Ícone de troca de classe (MAIOR)
void DrawClassIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    int helmet_radius = size / 2;
    H::Draw->CircleFilled(cx, cy - 3, helmet_radius + 2, 28, color);

    Color_t helmet_dark = Color_t(color.r * 0.8f, color.g * 0.8f, color.b * 0.8f, alpha);
    Color_t helmet_darker = Color_t(color.r * 0.6f, color.g * 0.6f, color.b * 0.6f, alpha);

    H::Draw->TriangleFilled(Vec2(cx - helmet_radius - 2, cy - 2), Vec2(cx - helmet_radius / 2, cy - helmet_radius - 2), Vec2(cx, cy + 2), helmet_dark);
    H::Draw->TriangleFilled(Vec2(cx + helmet_radius + 2, cy - 2), Vec2(cx + helmet_radius / 2, cy - helmet_radius - 2), Vec2(cx, cy + 2), helmet_dark);

    H::Draw->RectGradient(cx - helmet_radius - 3, cy + 3, (helmet_radius + 3) * 2, 5, color, helmet_darker, false);

    H::Draw->RectFilled(x + 1, cy + 1, 5, 10, helmet_dark);
    H::Draw->RectOutlined(x + 1, cy + 1, 5, 10, Color_t(color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, alpha), Color_t(color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, alpha));

    H::Draw->RectFilled(x + size - 6, cy + 1, 5, 10, helmet_dark);
    H::Draw->RectOutlined(x + size - 6, cy + 1, 5, 10, Color_t(color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, alpha), Color_t(color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, alpha));

    Color_t visor_dark = Color_t(10, 10, 20, alpha);
    Color_t visor_shine = Color_t(100, 150, 200, alpha / 2);
    int visor_width = helmet_radius + 4;
    int visor_height = 6;
    int visor_y = cy - 1;

    H::Draw->RectFilled(cx - visor_width / 2, visor_y, visor_width, visor_height, visor_dark);
    H::Draw->CircleFilled(cx - visor_width / 2, visor_y + visor_height / 2, visor_height / 2, 16, visor_dark);
    H::Draw->CircleFilled(cx + visor_width / 2, visor_y + visor_height / 2, visor_height / 2, 16, visor_dark);

    H::Draw->RectGradient(cx - visor_width / 2 + 2, visor_y + 1, visor_width / 3, 2, visor_shine, Color_t(0, 0, 0, 0), true);

    Color_t crest = Color_t(
        std::min(255, color.r + 60),
        std::min(255, color.g + 60),
        std::min(255, color.b + 60),
        alpha
    );
    H::Draw->RectFilled(cx - 2, cy - helmet_radius - 1, 4, helmet_radius + 2, crest);

    Color_t crest_detail = Color_t(255, 255, 255, alpha / 4);
    H::Draw->Line(cx - 3, cy - helmet_radius, cx - 3, cy, crest_detail);
    H::Draw->Line(cx + 3, cy - helmet_radius, cx + 3, cy, crest_detail);

    Color_t shine = Color_t(255, 255, 255, alpha / 3);
    H::Draw->CircleFilled(cx - 4, cy - helmet_radius, 3, 10, shine);

    Color_t screw = Color_t(80, 80, 90, alpha);
    H::Draw->CircleFilled(x + 3, cy + 5, 2, 8, screw);
    H::Draw->CircleFilled(x + size - 3, cy + 5, 2, 8, screw);
}