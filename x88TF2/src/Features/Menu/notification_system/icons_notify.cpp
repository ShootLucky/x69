// icons_notify.cpp
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
    H::Draw->Rect(cx - size / 3, cy - letter_height / 2, letter_width, letter_height, color);
    H::Draw->Rect(cx - size / 3, cy - letter_height / 2, size / 3, letter_width, color);
    H::Draw->Rect(cx - size / 3 + size / 3 - letter_width, cy - letter_height / 2, letter_width, letter_height, color);

    // T
    H::Draw->Rect(cx + size / 12, cy - letter_height / 2, size / 3, letter_width, color);
    H::Draw->Rect(cx + size / 4, cy - letter_height / 2, letter_width, letter_height, color);
}

// Lmaobox - Uma caixa (MAIOR)
void DrawLmaoboxIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;
    int box_size = size * 3 / 4;

    // Sombra da caixa
    Color_t shadow = Color_t(0, 0, 0, alpha / 2);
    H::Draw->Rect(cx - box_size / 2 + 2, cy - box_size / 2 + 2, box_size, box_size, shadow);

    // Caixa principal com gradiente
    Color_t box_light = color;
    Color_t box_dark = Color_t(color.r * 0.7f, color.g * 0.7f, color.b * 0.7f, alpha);
    H::Draw->GradientRect(cx - box_size / 2, cy - box_size / 2, box_size, box_size, box_light, box_dark, false);

    // Borda da caixa
    H::Draw->OutlinedRect(cx - box_size / 2, cy - box_size / 2, box_size, box_size, Color_t(255, 255, 255, alpha));

    // Tampa da caixa (linha no topo)
    Color_t lid = Color_t(255, 255, 255, alpha / 2);
    H::Draw->Rect(cx - box_size / 2, cy - box_size / 2, box_size, 4, lid);

    // Detalhes (fita adesiva)
    H::Draw->Rect(cx - 2, cy - box_size / 2, 4, box_size, Color_t(200, 180, 100, alpha));
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
    H::Draw->Rect(cx - size / 4, cy - letter_height / 2, letter_width, letter_height, color);

    // Top curve (semicircle)
    H::Draw->Rect(cx - size / 4, cy - letter_height / 2, size / 2, letter_width, color);
    H::Draw->Rect(cx - size / 4 + size / 2 - letter_width, cy - letter_height / 2, letter_width, letter_height / 3, color);
    H::Draw->Rect(cx - size / 4, cy - letter_height / 2 + letter_height / 3, size / 2, letter_width, color);

    // Diagonal leg
    for (int i = 0; i < letter_height / 2; i++) {
        H::Draw->Rect(cx - size / 4 + i, cy + i / 2, letter_width, letter_width, color);
    }
}

// Cheater - Placa de aviso (triângulo com !) (MAIOR)
void DrawCheaterIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    // Triângulo de aviso - MAIOR
    std::array<Vec2, 3> warning = {
        Vec2(cx, y + 1),
        Vec2(x + 1, y + size - 1),
        Vec2(x + size - 1, y + size - 1)
    };
    H::Draw->FilledTriangle(warning, color);

    // Borda preta
    Color_t border = Color_t(0, 0, 0, alpha);
    H::Draw->Line(cx, y + 1, x + 1, y + size - 1, border);
    H::Draw->Line(cx, y + 1, x + size - 1, y + size - 1, border);
    H::Draw->Line(x + 1, y + size - 1, x + size - 1, y + size - 1, border);

    // Ponto de exclamação - MAIOR
    Color_t excl = Color_t(0, 0, 0, alpha);
    H::Draw->Rect(cx - 2, cy - 8, 4, 12, excl);
    H::Draw->Rect(cx - 2, cy + 6, 4, 3, excl);
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
    H::Draw->FilledCircle(cx, cy - 6, 6, 20, color);
    H::Draw->Rect(cx + 4, cy - 6, qmark_width, 9, color);

    // Vertical line
    H::Draw->Rect(cx - qmark_width / 2, cy + 2, qmark_width, 6, color);

    // Dot
    H::Draw->Rect(cx - qmark_width / 2, cy + 9, qmark_width, qmark_width, color);
}

// Retard Legit - #$@ amarelo (MAIOR)
void DrawRetardLegitIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    // Símbolos de palavrão estilizados - MAIOR
    int symbol_width = 3;

    // #
    H::Draw->Rect(cx - 9, cy - 8, symbol_width, 16, color);
    H::Draw->Rect(cx - 3, cy - 8, symbol_width, 16, color);
    H::Draw->Rect(cx - 10, cy - 4, 10, symbol_width, color);
    H::Draw->Rect(cx - 10, cy + 3, 10, symbol_width, color);

    // $
    H::Draw->Rect(cx + 2, cy - 8, 6, symbol_width, color);
    H::Draw->Rect(cx + 2, cy - 1, 6, symbol_width, color);
    H::Draw->Rect(cx + 2, cy + 6, 6, symbol_width, color);
    H::Draw->Rect(cx + 2, cy - 6, symbol_width, 7, color);
    H::Draw->Rect(cx + 5, cy + 1, symbol_width, 7, color);
    H::Draw->Rect(cx + 4, cy - 9, 2, 18, color);
}

// Ignored - OK cinza (MAIOR)
void DrawIgnoredIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    // Checkmark (✓) - MAIOR
    int check_width = 4;

    // Short line (bottom left to middle)
    for (int i = 0; i < 6; i++) {
        H::Draw->Rect(cx - 5 + i, cy + 3 - i, check_width, check_width, color);
    }

    // Long line (middle to top right)
    for (int i = 0; i < 9; i++) {
        H::Draw->Rect(cx + 1 + i, cy - 3 - i, check_width, check_width, color);
    }
}

// Ícone de sino (notificação padrão) (MAIOR)
void DrawNotificationIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    // Bell body - MAIOR
    H::Draw->FilledCircle(cx, cy - 2, size / 2, 24, color);

    // Bell sides with gradient
    H::Draw->GradientRect(cx - 4 - size / 6, cy + 3, size / 3, size / 3, color, Color_t(color.r * 0.8f, color.g * 0.8f, color.b * 0.8f, alpha), true);
    H::Draw->GradientRect(cx + 4 - size / 6, cy + 3, size / 3, size / 3, color, Color_t(color.r * 0.8f, color.g * 0.8f, color.b * 0.8f, alpha), true);

    // Top hook with shine
    Color_t shine = Color_t(255, 255, 255, alpha / 2);
    H::Draw->Rect(cx - 2, y + 1, 4, 6, color);
    H::Draw->FilledCircle(cx, y + 2, 3, 16, shine);

    // Bottom edge with flare
    H::Draw->Rect(cx - size / 2, cy + size / 3, size, 3, color);
    H::Draw->GradientRect(cx - size / 2, cy + size / 3 + 3, size, 2, shine, Color_t(0, 0, 0, 0), false);

    // Clapper with shadow
    Color_t darker = Color_t(color.r * 0.6f, color.g * 0.6f, color.b * 0.6f, alpha);
    H::Draw->FilledCircle(cx + 1, y + size - 2, 4, 16, Color_t(0, 0, 0, alpha / 4));
    H::Draw->FilledCircle(cx, y + size - 3, 4, 16, darker);
}

// Ícone de ping (MAIOR)
void DrawPingIcon(int x, int y, int size, Color_t color, int alpha, int ping_value) {
    color.a = static_cast<unsigned char>(alpha);
    int bar_count = 4;
    int bar_width = size / 4;
    int spacing = 2;
    int active_bars = 4;

    if (ping_value >= 150) active_bars = 1;
    else if (ping_value >= 100) active_bars = 2;
    else if (ping_value >= 50) active_bars = 3;

    for (int i = 0; i < bar_count; i++) {
        int bar_height = (i + 1) * (size / bar_count);
        Color_t bar_color = (i < active_bars) ? color : Color_t(40, 40, 40, alpha);
        Color_t bar_glow = Color_t(bar_color.r, bar_color.g, bar_color.b, alpha / 4);

        int bar_x = x + i * (bar_width + spacing);
        int bar_y = y + size - bar_height;

        H::Draw->Rect(bar_x - 1, bar_y - 1, bar_width + 2, bar_height + 2, bar_glow);
        H::Draw->Rect(bar_x, bar_y + 2, bar_width, bar_height - 2, bar_color);
        H::Draw->FilledCircle(bar_x + bar_width / 2, bar_y + 2, bar_width / 2, 16, bar_color);
    }
}

// Ícone de dano (coração estilo Minecraft - MAIOR)
void DrawDamageIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);

    // Pixel size MAIOR para o estilo Minecraft
    int pixel = size / 7;  // Coração será maior (de 9 pixels agora usa 7 divisões para pixels maiores)

    // Ajuste de posição para centralizar melhor
    int offset_x = -pixel;
    int offset_y = -pixel / 2;

    // Cores para dar profundidade (estilo Minecraft)
    Color_t dark = Color_t(color.r * 0.6f, color.g * 0.6f, color.b * 0.6f, alpha);
    Color_t light = Color_t(
        std::min(255, static_cast<int>(color.r * 1.3f)),
        std::min(255, static_cast<int>(color.g * 1.3f)),
        std::min(255, static_cast<int>(color.b * 1.3f)),
        alpha
    );

    // Matriz do coração do Minecraft (9x9)
    // 1 = pixel escuro, 2 = pixel normal, 3 = pixel claro, 0 = vazio
    int heart[9][9] = {
        {0, 2, 2, 0, 0, 0, 2, 2, 0},
        {2, 3, 2, 2, 0, 2, 2, 3, 2},
        {2, 3, 2, 2, 2, 2, 2, 2, 2},
        {2, 2, 2, 2, 2, 2, 2, 2, 2},
        {0, 2, 2, 2, 2, 2, 2, 2, 0},
        {0, 0, 2, 2, 2, 2, 2, 0, 0},
        {0, 0, 0, 2, 2, 2, 0, 0, 0},
        {0, 0, 0, 0, 2, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    // Desenhar o coração pixel por pixel
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            int px = x + offset_x + col * pixel;
            int py = y + offset_y + row * pixel;

            Color_t pixel_color;
            switch (heart[row][col]) {
            case 1: pixel_color = dark; break;
            case 2: pixel_color = color; break;
            case 3: pixel_color = light; break;
            default: continue; // Pula pixels vazios
            }

            // Desenha o pixel MAIOR
            H::Draw->Rect(px, py, pixel, pixel, pixel_color);

            // Borda escura para dar definição (estilo Minecraft)
            Color_t border = Color_t(0, 0, 0, alpha / 2);
            H::Draw->OutlinedRect(px, py, pixel, pixel, border);
        }
    }

    // Rachadura estilo Minecraft (linha diagonal pixelada)
    Color_t crack = Color_t(20, 0, 0, alpha);
    int crack_pixels[][2] = {
        {3, 1}, {4, 2}, {4, 3}, {5, 4}, {4, 5}, {4, 6}
    };

    for (const auto& crack_pos : crack_pixels) {
        int px = x + offset_x + crack_pos[0] * pixel;
        int py = y + offset_y + crack_pos[1] * pixel;
        H::Draw->Rect(px, py, pixel, pixel, crack);
    }
}

// Ícone de morte/explosão (ULTRA MELHORADO - VERSÃO FINAL)
void DrawDeathIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    // ===== CAMADA 1: ONDAS DE CHOQUE (base) =====
    Color_t shockwave1 = Color_t(255, 100, 0, alpha / 6);
    Color_t shockwave2 = Color_t(255, 150, 50, alpha / 4);
    H::Draw->FilledCircle(cx, cy, size / 2 + 3, 32, shockwave1);
    H::Draw->FilledCircle(cx, cy, size / 2 + 1, 28, shockwave2);

    // ===== CAMADA 2: RAIOS PRINCIPAIS (12 raios) =====
    int inner_radius = size / 3;
    int outer_radius = size / 2;

    for (int i = 0; i < 12; i++) {
        float angle = (i * 30.0f) * (3.14159f / 180.0f);

        // Varia tamanho dos raios para efeito irregular
        float ray_length = 1.0f;
        if (i % 3 == 0) ray_length = 1.4f;      // Raios grandes
        else if (i % 3 == 1) ray_length = 1.1f; // Raios médios
        else ray_length = 0.85f;                // Raios pequenos

        // Pontos do raio
        int x1 = cx + static_cast<int>(std::cos(angle - 0.25f) * inner_radius * 0.9f);
        int y1 = cy + static_cast<int>(std::sin(angle - 0.25f) * inner_radius * 0.9f);
        int x2 = cx + static_cast<int>(std::cos(angle) * outer_radius * ray_length);
        int y2 = cy + static_cast<int>(std::sin(angle) * outer_radius * ray_length);
        int x3 = cx + static_cast<int>(std::cos(angle + 0.25f) * inner_radius * 0.9f);
        int y3 = cy + static_cast<int>(std::sin(angle + 0.25f) * inner_radius * 0.9f);

        // Cor varia: vermelho → laranja → amarelo
        Color_t ray_color;
        if (i % 3 == 0) ray_color = Color_t(255, 80, 0, alpha);      // Vermelho
        else if (i % 3 == 1) ray_color = Color_t(255, 150, 30, alpha); // Laranja
        else ray_color = Color_t(255, 200, 80, alpha);                // Amarelo-laranja

        std::array<Vec2, 3> ray = { Vec2(x1, y1), Vec2(x2, y2), Vec2(x3, y3) };
        H::Draw->FilledTriangle(ray, ray_color);

        // Brilho interno do raio (gradiente)
        Color_t ray_glow = Color_t(255, 255, 150, alpha / 3);
        int glow_x1 = cx + static_cast<int>(std::cos(angle - 0.15f) * inner_radius * 0.5f);
        int glow_y1 = cy + static_cast<int>(std::sin(angle - 0.15f) * inner_radius * 0.5f);
        int glow_x2 = cx + static_cast<int>(std::cos(angle) * outer_radius * ray_length * 0.7f);
        int glow_y2 = cy + static_cast<int>(std::sin(angle) * outer_radius * ray_length * 0.7f);
        int glow_x3 = cx + static_cast<int>(std::cos(angle + 0.15f) * inner_radius * 0.5f);
        int glow_y3 = cy + static_cast<int>(std::sin(angle + 0.15f) * inner_radius * 0.5f);

        std::array<Vec2, 3> glow_ray = { Vec2(glow_x1, glow_y1), Vec2(glow_x2, glow_y2), Vec2(glow_x3, glow_y3) };
        H::Draw->FilledTriangle(glow_ray, ray_glow);
    }

    // ===== CAMADA 3: ANÉIS DE FOGO (4 camadas) =====
    Color_t ring_outer = Color_t(255, 60, 0, alpha * 0.4f);
    H::Draw->FilledCircle(cx, cy, size / 2 - 1, 32, ring_outer);

    Color_t ring_red = Color_t(255, 100, 20, alpha * 0.7f);
    H::Draw->FilledCircle(cx, cy, size / 2 - 3, 28, ring_red);

    Color_t ring_orange = Color_t(255, 160, 40, alpha * 0.85f);
    H::Draw->FilledCircle(cx, cy, size / 3, 28, ring_orange);

    Color_t ring_yellow = Color_t(255, 220, 100, alpha);
    H::Draw->FilledCircle(cx, cy, size / 4, 24, ring_yellow);

    // ===== CAMADA 4: PARTÍCULAS DE FOGO (30+ partículas) =====
    Color_t particle_white = Color_t(255, 255, 255, alpha);
    Color_t particle_bright = Color_t(255, 255, 150, alpha);
    Color_t particle_yellow = Color_t(255, 220, 100, alpha);
    Color_t particle_orange = Color_t(255, 160, 40, alpha);
    for (int i = 0; i < 12; i++) {
        float angle = (i * 30.0f + 15.0f) * (3.14159f / 180.0f);
        int px = cx + static_cast<int>(std::cos(angle) * (size / 3 + 2));
        int py = cy + static_cast<int>(std::sin(angle) * (size / 3 + 2));

        Color_t p_color;
        if (i % 3 == 0) p_color = particle_bright;
        else if (i % 3 == 1) p_color = particle_yellow;
        else p_color = particle_orange;

        H::Draw->FilledCircle(px, py, 2, 8, p_color);
    }

    // Faíscas (mini partículas voando)
    for (int i = 0; i < 16; i++) {
        float angle = (i * 22.5f) * (3.14159f / 180.0f);
        int dist = size / 2 + 2;
        int px = cx + static_cast<int>(std::cos(angle) * dist);
        int py = cy + static_cast<int>(std::sin(angle) * dist);

        H::Draw->FilledCircle(px, py, 1, 6, particle_white);
    }

    // ===== CAMADA 5: NÚCLEO BRILHANTE (centro) =====
    // Brilho externo branco
    Color_t white_outer = Color_t(255, 255, 255, alpha / 4);
    H::Draw->FilledCircle(cx, cy, size / 4, 20, white_outer);

    // Núcleo amarelo brilhante
    Color_t core_yellow = Color_t(255, 255, 200, alpha);
    H::Draw->FilledCircle(cx, cy, size / 5, 20, core_yellow);

    // Centro branco puro (ponto mais quente)
    Color_t core_white = Color_t(255, 255, 255, alpha);
    H::Draw->FilledCircle(cx, cy, size / 7, 16, core_white);

    // Brilho final super intenso
    Color_t final_shine = Color_t(255, 255, 255, alpha);
    H::Draw->FilledCircle(cx, cy, size / 10, 12, final_shine);

    // ===== CAMADA 6: DETALHES FINAIS =====
    // Pequenos brilhos ao redor do núcleo
    for (int i = 0; i < 4; i++) {
        float angle = (i * 90.0f + 45.0f) * (3.14159f / 180.0f);
        int px = cx + static_cast<int>(std::cos(angle) * (size / 8));
        int py = cy + static_cast<int>(std::sin(angle) * (size / 8));

        H::Draw->FilledCircle(px, py, 2, 8, Color_t(255, 255, 255, alpha / 2));
    }
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

        std::array<Vec2, 3> spike = {
            Vec2(base_x1, base_y1),
            Vec2(base_x2, base_y2),
            Vec2(tip_x, tip_y)
        };
        H::Draw->FilledTriangle(spike, color);

        Color_t spike_glow = Color_t(color.r, color.g, color.b, alpha / 4);
        H::Draw->FilledTriangle({ Vec2(base_x1 + 1, base_y1 + 1), Vec2(base_x2 + 1, base_y2 + 1), Vec2(tip_x + 1, tip_y + 1) }, spike_glow);
    }

    // Core brilhante - MAIOR
    Color_t bright = Color_t(
        std::min(255, color.r + 80),
        std::min(255, color.g + 80),
        std::min(255, color.b + 80),
        alpha
    );
    H::Draw->FilledCircle(cx, cy, core_size, 20, bright);

    Color_t white = Color_t(255, 255, 255, alpha);
    H::Draw->FilledCircle(cx, cy, 3, 12, white);
}

// Ícone de troca de classe (MAIOR)
void DrawClassIcon(int x, int y, int size, Color_t color, int alpha) {
    color.a = static_cast<unsigned char>(alpha);
    int cx = x + size / 2;
    int cy = y + size / 2;

    int helmet_radius = size / 2;
    H::Draw->FilledCircle(cx, cy - 3, helmet_radius + 2, 28, color);

    Color_t helmet_dark = Color_t(color.r * 0.8f, color.g * 0.8f, color.b * 0.8f, alpha);
    Color_t helmet_darker = Color_t(color.r * 0.6f, color.g * 0.6f, color.b * 0.6f, alpha);

    std::array<Vec2, 3> left_side = {
        Vec2(cx - helmet_radius - 2, cy - 2),
        Vec2(cx - helmet_radius / 2, cy - helmet_radius - 2),
        Vec2(cx, cy + 2)
    };
    H::Draw->FilledTriangle(left_side, helmet_dark);

    std::array<Vec2, 3> right_side = {
        Vec2(cx + helmet_radius + 2, cy - 2),
        Vec2(cx + helmet_radius / 2, cy - helmet_radius - 2),
        Vec2(cx, cy + 2)
    };
    H::Draw->FilledTriangle(right_side, helmet_dark);

    H::Draw->GradientRect(cx - helmet_radius - 3, cy + 3, (helmet_radius + 3) * 2, 5,
        color, helmet_darker, false);

    H::Draw->Rect(x + 1, cy + 1, 5, 10, helmet_dark);
    H::Draw->OutlinedRect(x + 1, cy + 1, 5, 10, Color_t(color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, alpha));

    H::Draw->Rect(x + size - 6, cy + 1, 5, 10, helmet_dark);
    H::Draw->OutlinedRect(x + size - 6, cy + 1, 5, 10, Color_t(color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, alpha));

    Color_t visor_dark = Color_t(10, 10, 20, alpha);
    Color_t visor_shine = Color_t(100, 150, 200, alpha / 2);
    int visor_width = helmet_radius + 4;
    int visor_height = 6;
    int visor_y = cy - 1;

    H::Draw->Rect(cx - visor_width / 2, visor_y, visor_width, visor_height, visor_dark);
    H::Draw->FilledCircle(cx - visor_width / 2, visor_y + visor_height / 2, visor_height / 2, 16, visor_dark);
    H::Draw->FilledCircle(cx + visor_width / 2, visor_y + visor_height / 2, visor_height / 2, 16, visor_dark);

    H::Draw->GradientRect(cx - visor_width / 2 + 2, visor_y + 1, visor_width / 3, 2,
        visor_shine, Color_t(0, 0, 0, 0), true);

    Color_t crest = Color_t(
        std::min(255, color.r + 60),
        std::min(255, color.g + 60),
        std::min(255, color.b + 60),
        alpha
    );
    H::Draw->Rect(cx - 2, cy - helmet_radius - 1, 4, helmet_radius + 2, crest);

    Color_t crest_detail = Color_t(255, 255, 255, alpha / 4);
    H::Draw->Line(cx - 3, cy - helmet_radius, cx - 3, cy, crest_detail);
    H::Draw->Line(cx + 3, cy - helmet_radius, cx + 3, cy, crest_detail);

    Color_t shine = Color_t(255, 255, 255, alpha / 3);
    H::Draw->FilledCircle(cx - 4, cy - helmet_radius, 3, 10, shine);

    Color_t screw = Color_t(80, 80, 90, alpha);
    H::Draw->FilledCircle(x + 3, cy + 5, 2, 8, screw);
    H::Draw->FilledCircle(x + size - 3, cy + 5, 2, 8, screw);
}