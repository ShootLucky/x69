#pragma once

#include "../Fonts/Fonts.h"
#include <vector>
#include <array>

// Flags de posicionamento usando bitwise
enum EDrawPosition : short
{
	ALIGN_DEFAULT = 0,
	ALIGN_LEFT = (1 << 0),
	ALIGN_RIGHT = (1 << 1),
	ALIGN_TOP = (1 << 2),
	ALIGN_BOTTOM = (1 << 3),
	ALIGN_CENTER_H = (1 << 4),
	ALIGN_CENTER_V = (1 << 5),
	ALIGN_CENTER = ALIGN_CENTER_H | ALIGN_CENTER_V
};

// Estrutura para cache de texto
struct TextCache
{
	int width = 0;
	int height = 0;
	HFont font = 0;
	bool valid = false;
};

class CDraw
{
private:
	// Dimensões da tela (cached)
	int m_nScreenW = 0;
	int m_nScreenH = 0;

	// Matriz de projeção World to Screen
	VMatrix m_WorldToProjection = {};

	// Cache para evitar recalcular tamanhos de texto
	mutable TextCache m_TextCache = {};

	// ID de textura para polígonos (cached)
	int m_nPolygonTextureID = 0;

	// Funções auxiliares privadas
	inline void ApplyTextAlignment(int& x, int& y, int textWidth, int textHeight, short align) const;
	inline void ApplyRectAlignment(int& x, int& y, int width, int height, short align) const;
	bool InitPolygonTexture();

public:
	// Inicialização e atualização
	void UpdateScreenSize();
	void UpdateW2SMatrix();
	void ClearCache();

	// Getters inline para performance
	inline int GetScreenW() const { return m_nScreenW; }
	inline int GetScreenH() const { return m_nScreenH; }
	inline Vec2 GetScreenSize() const { return Vec2(static_cast<float>(m_nScreenW), static_cast<float>(m_nScreenH)); }

	// World to Screen transformations
	bool W2S(const Vec3& vOrigin, Vec3& vScreen) const;
	bool ClipTransform(const Vec3& point, Vec3& pClip) const;
	bool ScreenPosition(const Vec3& vPoint, Vec3& vScreen) const;

	// Texto - versões otimizadas
	void Text(int x, int y, HFont font, Color_t color, short align, const char* text);
	void Text(int x, int y, HFont font, Color_t color, short align, const wchar_t* text);
	void TextF(int x, int y, HFont font, Color_t color, short align, const char* fmt, ...);
	void TextF(int x, int y, HFont font, Color_t color, short align, const wchar_t* fmt, ...);

	// Obter tamanho do texto (com cache)
	void GetTextSize(HFont font, const char* text, int& width, int& height);
	void GetTextSize(HFont font, const wchar_t* text, int& width, int& height);

	// Primitivas básicas
	void Line(int x1, int y1, int x2, int y2, Color_t color);
	void Rect(int x, int y, int w, int h, Color_t color);
	void RectOutlined(int x, int y, int w, int h, Color_t color, Color_t outlineColor);
	void RectFilled(int x, int y, int w, int h, Color_t color);
	void RectGradient(int x, int y, int w, int h, Color_t colorStart, Color_t colorEnd, bool horizontal = false);
	void RectRounded(int x, int y, int w, int h, int radius, Color_t color);
	void RectRoundedOutlined(int x, int y, int w, int h, int radius, Color_t color);

	// Círculos
	void Circle(int x, int y, int radius, int segments, Color_t color);
	void CircleFilled(int x, int y, int radius, int segments, Color_t color);

	// Polígonos
	void Triangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color_t color);
	void TriangleFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color_t color);
	void Polygon(int numPoints, const Vertex_t* vertices, Color_t color);
	void PolygonOutlined(int numPoints, const Vec2* points, Color_t color);

	// Arco
	void Arc(int x, int y, int radius, float thickness, float startAngle, float endAngle, Color_t color);

	// Texturas
	void Texture(int x, int y, int w, int h, int textureID, short align = ALIGN_DEFAULT, Color_t color = Color_t(255, 255, 255, 255));

	// Clipping
	void PushClipRect(int x, int y, int w, int h);
	void PopClipRect();

	// Utilidades
	inline void SetColor(Color_t color);
	void DrawPolyLine(const std::vector<Vec2>& points, Color_t color);
};

MAKE_SINGLETON_SCOPED(CDraw, Draw, H);