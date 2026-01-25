#include "Draw.h"
#include "../../TF2/cdll_int.h"
#include "../../TF2/ivrenderview.h"
#include <cstdarg>
#include <algorithm>

#pragma warning (disable : 6385)
#pragma warning (disable : 4996)

// ============================================================================
// ANIMATED VALUE (SMOOTH TRANSITIONS)
// ============================================================================

void AnimatedValue::Update(float newTarget, float currentTime, float deltaTime)
{
	target = newTarget;

	if (lastUpdateTime == 0.0f) {
		current = target;
		lastUpdateTime = currentTime;
		return;
	}

	float difference = target - current;

	if (fabsf(difference) < 0.01f) {
		current = target;
		lastUpdateTime = currentTime;
		return;
	}

	float maxChange = speed * deltaTime;

	if (fabsf(difference) <= maxChange) {
		current = target;
	}
	else {
		current += (difference > 0.0f) ? maxChange : -maxChange;
	}

	lastUpdateTime = currentTime;
}

// ============================================================================
// BOUNDING BOX 2D
// ============================================================================

bool BBox2D::IsOnScreen(int screenW, int screenH) const
{
	return !(right < 0 || bottom < 0 || left >= screenW || top >= screenH);
}

bool BBox2D::IsPartiallyOnScreen(int screenW, int screenH) const
{
	// Mais permissivo - permite parte da box fora da tela
	const float margin = 50.0f; // Margem para evitar corte brusco
	return !(right < -margin || bottom < -margin ||
		left >= screenW + margin || top >= screenH + margin);
}

void BBox2D::ClipToScreen(int screenW, int screenH)
{
	left = std::max(0.0f, std::min(left, static_cast<float>(screenW)));
	right = std::max(0.0f, std::min(right, static_cast<float>(screenW)));
	top = std::max(0.0f, std::min(top, static_cast<float>(screenH)));
	bottom = std::max(0.0f, std::min(bottom, static_cast<float>(screenH)));
}

// ============================================================================
// INICIALIZAÇÃO E ATUALIZAÇÃO
// ============================================================================

void CDraw::UpdateScreenSize()
{
	m_nScreenW = I::BaseClientDLL->GetScreenWidth();
	m_nScreenH = I::BaseClientDLL->GetScreenHeight();
}

void CDraw::UpdateW2SMatrix()
{
	CViewSetup viewSetup = {};
	if (!I::BaseClientDLL->GetPlayerView(viewSetup))
		return;

	VMatrix worldToView, viewToProjection, worldToPixels;
	I::RenderView->GetMatricesForView(viewSetup, &worldToView, &viewToProjection, &m_WorldToProjection, &worldToPixels);
}

void CDraw::ClearCache()
{
	m_TextCache.valid = false;
}

void CDraw::ClearAllAnimations()
{
	m_AnimatedValues.clear();
}

bool CDraw::InitPolygonTexture()
{
	if (m_nPolygonTextureID == 0 || !I::MatSystemSurface->IsTextureIDValid(m_nPolygonTextureID))
	{
		m_nPolygonTextureID = I::MatSystemSurface->CreateNewTextureID(true);
		return m_nPolygonTextureID != 0;
	}
	return true;
}

// ============================================================================
// SISTEMA DE ANIMAÇÃO SMOOTH
// ============================================================================

float CDraw::GetAnimatedValue(int id, float targetValue, float speed)
{
	auto it = m_AnimatedValues.find(id);
	if (it == m_AnimatedValues.end()) {
		AnimatedValue newAnim;
		newAnim.speed = speed;
		newAnim.SetImmediate(targetValue);
		m_AnimatedValues[id] = newAnim;
		return targetValue;
	}

	// Obter tempo atual a partir da interface de engine (substitui I::GlobalVars)
	float currentTime = 0.0f;
	if (I::EngineClient) {
		currentTime = I::EngineClient->Time();
	}
	else {
		// Fallback robusto: aproximar com último tempo conhecido
		currentTime = it->second.lastUpdateTime + 0.016f; // ~60 FPS
	}

	// Calcular deltaTime a partir do último update armazenado
	float deltaTime = currentTime - it->second.lastUpdateTime;
	if (deltaTime <= 0.0f) {
		// Garantir um delta mínimo para evitar comportamento estranho
		deltaTime = 0.016f;
	}

	it->second.speed = speed;
	it->second.Update(targetValue, currentTime, deltaTime);

	return it->second.Get();
}

void CDraw::ResetAnimatedValue(int id)
{
	m_AnimatedValues.erase(id);
}

// ============================================================================
// FUNÇÕES AUXILIARES PRIVADAS
// ============================================================================

inline void CDraw::ApplyTextAlignment(int& x, int& y, int textWidth, int textHeight, short align) const
{
	if (align & ALIGN_LEFT)
		x -= textWidth;
	else if (align & ALIGN_CENTER_H)
		x -= textWidth / 2;

	if (align & ALIGN_TOP)
		y -= textHeight;
	else if (align & ALIGN_CENTER_V)
		y -= textHeight / 2;
}

inline void CDraw::ApplyRectAlignment(int& x, int& y, int width, int height, short align) const
{
	if (align & ALIGN_LEFT)
		x -= width;
	else if (align & ALIGN_CENTER_H)
		x -= width / 2;

	if (align & ALIGN_TOP)
		y -= height;
	else if (align & ALIGN_CENTER_V)
		y -= height / 2;
}

float CDraw::GetDistanceScale(float distance, float referenceDistance) const
{
	if (distance <= 0.0f) return 1.0f;

	// Escala inversa com distância, mas com limites
	float scale = referenceDistance / distance;
	scale = std::clamp(scale, 0.5f, 2.0f); // Limita entre 50% e 200%

	return scale;
}

// ============================================================================
// WORLD TO SCREEN (MELHORADO)
// ============================================================================

bool CDraw::W2S(const Vec3& vOrigin, Vec3& vScreen) const
{
	const matrix3x4_t& w2s = m_WorldToProjection.As3x4();
	const float w = w2s[3][0] * vOrigin.x + w2s[3][1] * vOrigin.y + w2s[3][2] * vOrigin.z + w2s[3][3];

	// Melhor threshold para evitar divisão por zero
	if (w < 0.001f)
		return false;

	const float invW = 1.0f / w;
	const float halfWidth = m_nScreenW * 0.5f;
	const float halfHeight = m_nScreenH * 0.5f;

	vScreen.x = halfWidth + (w2s[0][0] * vOrigin.x + w2s[0][1] * vOrigin.y + w2s[0][2] * vOrigin.z + w2s[0][3]) * invW * halfWidth;
	vScreen.y = halfHeight - (w2s[1][0] * vOrigin.x + w2s[1][1] * vOrigin.y + w2s[1][2] * vOrigin.z + w2s[1][3]) * invW * halfHeight;
	vScreen.z = 0.0f;

	return true;
}

bool CDraw::ClipTransform(const Vec3& point, Vec3& pClip) const
{
	const matrix3x4_t& w2s = m_WorldToProjection.As3x4();

	pClip.x = w2s[0][0] * point.x + w2s[0][1] * point.y + w2s[0][2] * point.z + w2s[0][3];
	pClip.y = w2s[1][0] * point.x + w2s[1][1] * point.y + w2s[1][2] * point.z + w2s[1][3];
	pClip.z = 0.0f;

	const float w = w2s[3][0] * point.x + w2s[3][1] * point.y + w2s[3][2] * point.z + w2s[3][3];
	const bool behind = w < 0.001f;

	if (behind)
	{
		pClip.x *= 100000.0f;
		pClip.y *= 100000.0f;
	}
	else
	{
		const float invW = 1.0f / w;
		pClip.x *= invW;
		pClip.y *= invW;
	}

	return behind;
}

bool CDraw::ScreenPosition(const Vec3& vPoint, Vec3& vScreen) const
{
	const bool behind = ClipTransform(vPoint, vScreen);

	const float halfWidth = m_nScreenW * 0.5f;
	const float halfHeight = m_nScreenH * 0.5f;

	vScreen.x = halfWidth * vScreen.x + halfWidth;
	vScreen.y = -halfHeight * vScreen.y + halfHeight;

	return behind;
}

// ============================================================================
// COMPUTE BOUNDING BOX 2D (NOVA FUNÇÃO)
// ============================================================================

BBox2D CDraw::ComputeBBox2D(const Vec3* worldPoints, int numPoints) const
{
	BBox2D result;
	result.valid = false;

	if (!worldPoints || numPoints < 1) {
		return result;
	}

	std::vector<Vec3> screenPoints;
	screenPoints.reserve(numPoints);

	int validPoints = 0;
	int behindPoints = 0;

	// Projetar todos os pontos
	for (int i = 0; i < numPoints; ++i) {
		Vec3 screen;
		if (W2S(worldPoints[i], screen)) {
			// Validar se o ponto é finito
			if (std::isfinite(screen.x) && std::isfinite(screen.y)) {
				screenPoints.push_back(screen);
				validPoints++;
			}
		}
		else {
			behindPoints++;
		}
	}

	// Se todos os pontos estão atrás da câmera, a box não é válida
	if (behindPoints == numPoints) {
		return result;
	}

	// Se temos muito poucos pontos válidos, não podemos fazer uma box confiável
	if (validPoints < 2) {
		return result;
	}

	// Calcular bounding box dos pontos projetados
	float minX = std::numeric_limits<float>::max();
	float minY = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float maxY = std::numeric_limits<float>::lowest();

	for (const auto& pt : screenPoints) {
		minX = std::min(minX, pt.x);
		minY = std::min(minY, pt.y);
		maxX = std::max(maxX, pt.x);
		maxY = std::max(maxY, pt.y);
	}

	// Validar dimensões
	const float width = maxX - minX;
	const float height = maxY - minY;

	// Box muito pequena ou muito grande = inválida
	if (width < 2.0f || height < 2.0f || width > m_nScreenW * 3.0f || height > m_nScreenH * 3.0f) {
		return result;
	}

	result.left = minX;
	result.top = minY;
	result.right = maxX;
	result.bottom = maxY;
	result.valid = true;

	return result;
}

// ============================================================================
// TEXTO
// ============================================================================

void CDraw::GetTextSize(HFont font, const char* text, int& width, int& height)
{
	if (!text || !*text)
	{
		width = height = 0;
		return;
	}

	wchar_t wideText[1024];
	wsprintfW(wideText, L"%hs", text);
	I::MatSystemSurface->GetTextSize(font, wideText, width, height);
}

void CDraw::GetTextSize(HFont font, const wchar_t* text, int& width, int& height)
{
	if (!text || !*text)
	{
		width = height = 0;
		return;
	}

	I::MatSystemSurface->GetTextSize(font, text, width, height);
}

void CDraw::Text(int x, int y, HFont font, Color_t color, short align, const char* text)
{
	if (!text || !*text)
		return;

	wchar_t wideText[1024];
	wsprintfW(wideText, L"%hs", text);
	Text(x, y, font, color, align, wideText);
}

void CDraw::Text(int x, int y, HFont font, Color_t color, short align, const wchar_t* text)
{
	if (!text || !*text)
		return;

	if (align != ALIGN_DEFAULT)
	{
		int w, h;
		GetTextSize(font, text, w, h);
		ApplyTextAlignment(x, y, w, h, align);
	}

	I::MatSystemSurface->DrawSetTextFont(font);
	I::MatSystemSurface->DrawSetTextColor(color.r, color.g, color.b, color.a);
	I::MatSystemSurface->DrawSetTextPos(x, y);
	I::MatSystemSurface->DrawPrintText(text, static_cast<int>(wcslen(text)));
}

void CDraw::TextF(int x, int y, HFont font, Color_t color, short align, const char* fmt, ...)
{
	if (!fmt)
		return;

	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	Text(x, y, font, color, align, buffer);
}

void CDraw::TextF(int x, int y, HFont font, Color_t color, short align, const wchar_t* fmt, ...)
{
	if (!fmt)
		return;

	wchar_t buffer[1024];
	va_list args;
	va_start(args, fmt);
	vswprintf(buffer, sizeof(buffer) / sizeof(wchar_t), fmt, args);
	va_end(args);

	Text(x, y, font, color, align, buffer);
}

// ============================================================================
// PRIMITIVAS BÁSICAS
// ============================================================================

inline void CDraw::SetColor(Color_t color)
{
	I::MatSystemSurface->DrawSetColor(color.r, color.g, color.b, color.a);
}

void CDraw::Line(int x1, int y1, int x2, int y2, Color_t color)
{
	SetColor(color);
	I::MatSystemSurface->DrawLine(x1, y1, x2, y2);
}

void CDraw::Rect(int x, int y, int w, int h, Color_t color)
{
	SetColor(color);
	I::MatSystemSurface->DrawOutlinedRect(x, y, x + w, y + h);
}

void CDraw::RectFilled(int x, int y, int w, int h, Color_t color)
{
	SetColor(color);
	I::MatSystemSurface->DrawFilledRect(x, y, x + w, y + h);
}

void CDraw::RectOutlined(int x, int y, int w, int h, Color_t color, Color_t outlineColor)
{
	RectFilled(x, y, w, h, color);
	Rect(x, y, w, h, outlineColor);
}

void CDraw::RectGradient(int x, int y, int w, int h, Color_t colorStart, Color_t colorEnd, bool horizontal)
{
	SetColor(colorStart);
	I::MatSystemSurface->DrawFilledRectFade(x, y, x + w, y + h, 255, 0, horizontal);

	SetColor(colorEnd);
	I::MatSystemSurface->DrawFilledRectFade(x, y, x + w, y + h, 0, 255, horizontal);
}

void CDraw::RectRounded(int x, int y, int w, int h, int radius, Color_t color)
{
	// Limita o raio ao tamanho do retângulo
	radius = (radius > w / 2) ? w / 2 : radius;
	radius = (radius > h / 2) ? h / 2 : radius;

	constexpr int segments = 16;
	Vertex_t vertices[segments * 4];

	// Gera vértices para os 4 cantos arredondados
	for (int corner = 0; corner < 4; ++corner)
	{
		const int centerX = x + ((corner < 2) ? (w - radius) : radius);
		const int centerY = y + ((corner % 3) ? (h - radius) : radius);
		const float angleStart = corner * 90.0f;

		for (int i = 0; i < segments; ++i)
		{
			const float angle = DEG2RAD(angleStart + i * (90.0f / segments));
			vertices[corner * segments + i] = Vertex_t(
				Vector2D(
					centerX + radius * sinf(angle),
					centerY - radius * cosf(angle)
				)
			);
		}
	}

	Polygon(segments * 4, vertices, color);
}

void CDraw::RectRoundedOutlined(int x, int y, int w, int h, int radius, Color_t color)
{
	radius = (radius > w / 2) ? w / 2 : radius;
	radius = (radius > h / 2) ? h / 2 : radius;

	constexpr int segments = 16;
	std::vector<Vec2> points;
	points.reserve(segments * 4);

	for (int corner = 0; corner < 4; ++corner)
	{
		const int centerX = x + ((corner < 2) ? (w - radius) : radius);
		const int centerY = y + ((corner % 3) ? (h - radius) : radius);
		const float angleStart = corner * 90.0f;

		for (int i = 0; i < segments; ++i)
		{
			const float angle = DEG2RAD(angleStart + i * (90.0f / segments));
			points.emplace_back(
				centerX + radius * sinf(angle),
				centerY - radius * cosf(angle)
			);
		}
	}

	PolygonOutlined(points.size(), points.data(), color);
}

// ============================================================================
// CÍRCULOS
// ============================================================================

void CDraw::Circle(int x, int y, int radius, int segments, Color_t color)
{
	SetColor(color);
	I::MatSystemSurface->DrawOutlinedCircle(x, y, radius, segments);
}

void CDraw::CircleFilled(int x, int y, int radius, int segments, Color_t color)
{
	if (!InitPolygonTexture())
		return;

	std::vector<Vertex_t> vertices;
	vertices.reserve(segments);

	const float step = (2.0f * static_cast<float>(PI)) / segments;

	for (int i = 0; i < segments; ++i)
	{
		const float angle = i * step;
		vertices.emplace_back(Vector2D(
			x + radius * cosf(angle),
			y + radius * sinf(angle)
		));
	}

	Polygon(segments, vertices.data(), color);
}

// ============================================================================
// POLÍGONOS E TRIÂNGULOS
// ============================================================================

void CDraw::Polygon(int numPoints, const Vertex_t* vertices, Color_t color)
{
	if (!InitPolygonTexture() || numPoints < 3)
		return;

	SetColor(color);
	I::MatSystemSurface->DrawSetTexture(m_nPolygonTextureID);
	I::MatSystemSurface->DrawTexturedPolygon(numPoints, const_cast<Vertex_t*>(vertices));
}

void CDraw::PolygonOutlined(int numPoints, const Vec2* points, Color_t color)
{
	if (numPoints < 2)
		return;

	for (int i = 0; i < numPoints; ++i)
	{
		const Vec2& p1 = points[i];
		const Vec2& p2 = points[(i + 1) % numPoints];
		Line(static_cast<int>(p1.x), static_cast<int>(p1.y),
			static_cast<int>(p2.x), static_cast<int>(p2.y), color);
	}
}

void CDraw::Triangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color_t color)
{
	Line(static_cast<int>(p1.x), static_cast<int>(p1.y), static_cast<int>(p2.x), static_cast<int>(p2.y), color);
	Line(static_cast<int>(p2.x), static_cast<int>(p2.y), static_cast<int>(p3.x), static_cast<int>(p3.y), color);
	Line(static_cast<int>(p3.x), static_cast<int>(p3.y), static_cast<int>(p1.x), static_cast<int>(p1.y), color);
}

void CDraw::TriangleFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color_t color)
{
	const Vertex_t vertices[3] = {
		Vertex_t(p1),
		Vertex_t(p2),
		Vertex_t(p3)
	};

	Polygon(3, vertices, color);
}

// ============================================================================
// ARCO
// ============================================================================

void CDraw::Arc(int x, int y, int radius, float thickness, float startAngle, float endAngle, Color_t color)
{
	constexpr float precision = 7.2f;
	const float innerRadius = radius - thickness;

	for (float angle = startAngle; angle < startAngle + endAngle; angle += precision)
	{
		const float angleRad1 = DEG2RAD(angle);
		const float angleRad2 = DEG2RAD(angle + precision);

		const float cos1 = cosf(angleRad1);
		const float sin1 = sinf(angleRad1);
		const float cos2 = cosf(angleRad2);
		const float sin2 = sinf(angleRad2);

		const Vec2 inner1(x + cos1 * innerRadius, y + sin1 * innerRadius);
		const Vec2 inner2(x + cos2 * innerRadius, y + sin2 * innerRadius);
		const Vec2 outer1(x + cos1 * radius, y + sin1 * radius);
		const Vec2 outer2(x + cos2 * radius, y + sin2 * radius);

		const Vertex_t quad[4] = {
			Vertex_t(outer1),
			Vertex_t(outer2),
			Vertex_t(inner2),
			Vertex_t(inner1)
		};

		Polygon(4, quad, color);
	}
}

// ============================================================================
// TEXTURAS
// ============================================================================

void CDraw::Texture(int x, int y, int w, int h, int textureID, short align, Color_t color)
{
	ApplyRectAlignment(x, y, w, h, align);

	SetColor(color);
	I::MatSystemSurface->DrawSetTexture(textureID);
	I::MatSystemSurface->DrawTexturedRect(x, y, x + w, y + h);
}

// ============================================================================
// CLIPPING
// ============================================================================

void CDraw::PushClipRect(int x, int y, int w, int h)
{
	I::MatSystemSurface->DisableClipping(false);
	I::MatSystemSurface->SetClippingRect(x, y, x + w, y + h);
}

void CDraw::PopClipRect()
{
	I::MatSystemSurface->DisableClipping(true);
}

// ============================================================================
// UTILIDADES
// ============================================================================

void CDraw::DrawPolyLine(const std::vector<Vec2>& points, Color_t color)
{
	if (points.size() < 2)
		return;

	for (size_t i = 0; i + 1 < points.size(); ++i)
	{
		Line(
			static_cast<int>(points[i].x),
			static_cast<int>(points[i].y),
			static_cast<int>(points[i + 1].x),
			static_cast<int>(points[i + 1].y),
			color
		);
	}
}