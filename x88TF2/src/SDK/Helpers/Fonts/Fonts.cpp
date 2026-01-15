#include "Fonts.h"
#include "../Draw/Draw.h"
#include <windows.h>
#include <cstring>
void CFontManager::Reload()
{
	// fontes normais
	m_mapFonts[EFonts::Menu] = { "Segoe UI", 14, FONTFLAG_ANTIALIAS, 400 };
	m_mapFonts[EFonts::Tabs] = { "Arial Narrow", 15, FONTFLAG_ANTIALIAS, 700 };
	m_mapFonts[EFonts::ESP] = { "Verdana", 12, FONTFLAG_OUTLINE, 0 };
	m_mapFonts[EFonts::ESP_CONDS] = { "Small Fonts", 9, FONTFLAG_OUTLINE, 0 };
	m_mapFonts[EFonts::ESP_SMALL] = { "Small Fonts", 11, FONTFLAG_OUTLINE, 0 };
	m_mapFonts[EFonts::VerdanaBold] = { "Verdana", 12, FONTFLAG_ANTIALIAS, 700 };
	m_mapFonts[EFonts::OTHER] = { "Tahoma", 12, FONTFLAG_NONE, 400 }; // fonte padrão para OTHER

	// splash normal (caso queira usar)
	int splashSize = H::Draw->GetScreenH() / 8;
	m_mapFonts[EFonts::SPLASH] = {
	"Verdana",
	splashSize,
	FONTFLAG_OUTLINE, // sem AA aqui já melhora
	700
	};

	for (auto& v : m_mapFonts)
	{
		I::MatSystemSurface->SetFontGlyphSet
		(
			v.second.m_dwFont = I::MatSystemSurface->CreateFont(),
			v.second.m_szName,
			v.second.m_nTall,
			v.second.m_nWeight,
			0, // blur
			0, // scanlines
			v.second.m_nFlags
		);
	}
}
const CFont& CFontManager::Get(EFonts eFont)
{
	return m_mapFonts[eFont];
}
int CFontManager::GetFontHeight(EFonts eFont) const
{
	return m_mapFonts.at(eFont).m_nTall;
}
int CFont::GetStringWidth(const char* szString) const
{
	int wide = 0, tall = 0;
	size_t len = strlen(szString) + 1;
	wchar_t* wszBuffer = new wchar_t[len];
	MultiByteToWideChar(CP_UTF8, 0, szString, -1, wszBuffer, len);
	I::MatSystemSurface->GetTextSize(m_dwFont, wszBuffer, wide, tall);
	delete[] wszBuffer;
	return wide;
}