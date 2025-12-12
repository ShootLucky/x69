#include "Fonts.h"
#include <windows.h>
#include <cstring>
void CFontManager::Reload()
{
	m_mapFonts[EFonts::Menu] = { "Tahoma", 14, FONTFLAG_OUTLINE, 700 };
	m_mapFonts[EFonts::ESP] = { "Verdana", 12, FONTFLAG_OUTLINE, 0 };
	m_mapFonts[EFonts::ESP_CONDS] = { "Small Fonts", 9, FONTFLAG_OUTLINE, 0 };
	m_mapFonts[EFonts::ESP_SMALL] = { "Small Fonts", 11, FONTFLAG_OUTLINE, 0 };
	m_mapFonts[EFonts::VerdanaBold] = { "Verdana", 12, FONTFLAG_ANTIALIAS, 700 };
	for (auto& v : m_mapFonts)
	{
		I::MatSystemSurface->SetFontGlyphSet
		(
			v.second.m_dwFont = I::MatSystemSurface->CreateFont(),
			v.second.m_szName,    // Font name
			v.second.m_nTall,     // Font size
			v.second.m_nWeight,   // Font weight
			0,                    // Blur
			0,                    // Scanlines
			v.second.m_nFlags     // Font flags
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