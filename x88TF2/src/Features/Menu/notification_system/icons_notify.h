#pragma once

#include "../src/SDK/Helpers/Draw/Draw.h"  // Assuming this provides H::Draw, Color_t, Vec2
#include <array>  // For std::array<Vec2, 3>

// Declarations of icon drawing functions

void DrawNethookIcon(int x, int y, int size, Color_t color, int alpha);
void DrawLmaoboxIcon(int x, int y, int size, Color_t color, int alpha);
void DrawRijinIcon(int x, int y, int size, Color_t color, int alpha);
void DrawCheaterIcon(int x, int y, int size, Color_t color, int alpha);
void DrawCheaterLightIcon(int x, int y, int size, Color_t color, int alpha);
void DrawSuspectIcon(int x, int y, int size, Color_t color, int alpha);
void DrawRetardLegitIcon(int x, int y, int size, Color_t color, int alpha);
void DrawIgnoredIcon(int x, int y, int size, Color_t color, int alpha);
void DrawNotificationIcon(int x, int y, int size, Color_t color, int alpha);
void DrawPingIcon(int x, int y, int size, Color_t color, int alpha, int ping_value);
void DrawDamageIcon(int x, int y, int size, Color_t color, int alpha);
void DrawDeathIcon(int x, int y, int size, Color_t color, int alpha);
void DrawRespawnIcon(int x, int y, int size, Color_t color, int alpha);
void DrawClassIcon(int x, int y, int size, Color_t color, int alpha);