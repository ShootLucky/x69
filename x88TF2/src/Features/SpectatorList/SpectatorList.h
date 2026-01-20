#pragma once

#include "../src/SDK/SDK.h"

class CSpectatorList
{
public:
	struct Spectator_t
	{
		std::wstring Name = {};
		int m_nMode = 0;
		float m_fRespawnTime = 0.0f;    // tempo até respawn (opcional)
		float m_flAnimationTime = 0.0f; // timestamp para animações de entrada
		bool m_bRemoving = false;       // marcado para remoção (animação de saída)
		float m_flRemoveTime = 0.0f;    // timestamp de início da animação de saída

		Spectator_t() = default;
		Spectator_t(const std::wstring& name, int mode, float respawn = 0.0f, float animTime = 0.0f, bool removing = false, float removeTime = 0.0f)
			: Name(name), m_nMode(mode), m_fRespawnTime(respawn), m_flAnimationTime(animTime), m_bRemoving(removing), m_flRemoveTime(removeTime) {
		}
	};

private:
	std::vector<Spectator_t> m_vecSpectators = {};

	// --- Drag & layout state ---
	int m_nX = 0;                         // posição X do painel (persistente)
	int m_nY = 48;                        // posição Y do painel (persistente)
	bool m_bDragging = false;             // se está arrastando
	int m_nDragOffsetX = 0;               // offset entre mouse e canto superior esquerdo ao iniciar drag
	int m_nDragOffsetY = 0;

	bool GetSpectators();
	void Drag();

public:
	void Run();
};

MAKE_SINGLETON_SCOPED(CSpectatorList, SpectatorList, F);