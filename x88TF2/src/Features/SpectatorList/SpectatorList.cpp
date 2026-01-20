#include "SpectatorList.h"
#include "CFG.h"
#include "../Menu/Menu.h"
#include "../VisualUtils/VisualUtils.h"
#include "../Menu/notification_system/icons_notify.h"
#include <algorithm>

namespace
{
	constexpr int LIST_WIDTH = 220;
	constexpr int HEADER_HEIGHT = 20;
	constexpr int ITEM_HEIGHT = 18;
	constexpr int ITEM_SPACING = 2;
	constexpr int PADDING_X = 8;
	constexpr int PADDING_Y = 6;
	constexpr float ANIMATION_DURATION = 0.5f; // duração da animação em segundos
	constexpr float FADE_OUT_DURATION = 0.35f; // duração do fade out na saída

	// Paleta ultra minimalista (sem transparência no fundo; usaremos alpha para texto)
	const Color_t BG_COLOR = { 20, 20, 22, 255 };
	const Color_t ACCENT_COLOR = { 80, 200, 255, 255 };
	const Color_t BORDER_COLOR = { 50, 50, 55, 255 };
	const Color_t TEXT_COLOR = { 200, 200, 200, 255 };
	const Color_t TEXT_DIM = { 120, 120, 120, 255 };
	const Color_t ANIMATION_COLOR = { 100, 200, 255, 255 }; // cor da animação (azul)

	// Ordem de exibição de modos: IN_EYE, CHASE, DEATHCAM, FREEZECAM
	inline int ModePriority(int mode)
	{
		if (mode == OBS_MODE_IN_EYE) return 0;
		if (mode == OBS_MODE_CHASE) return 1;
		if (mode == OBS_MODE_DEATHCAM) return 2;
		if (mode == OBS_MODE_FREEZECAM) return 3;
		return 4;
	}
}

// Merge inteligente: preserva estados de animação/remoção entre frames,
// marca entradas novas com animation time e marca saídas com removing + removeTime.
bool CSpectatorList::GetSpectators()
{
	const auto pLocal = H::Entities->GetLocal();
	if (!pLocal)
		return false;

	std::vector<Spectator_t> present;

	// coletar espectadores que estão nos grupos apropriados
	for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PLAYERS_OBSERVER))
	{
		const auto pPlayer = pEntity->As<C_TFPlayer>();
		if (!pPlayer)
			continue;

		const auto pTarget = pPlayer->m_hObserverTarget().Get();
		if (pTarget != pLocal)
			continue;

		const int nMode = pPlayer->m_iObserverMode();
		// inclui DEATHCAM e FREEZECAM
		if (nMode != OBS_MODE_IN_EYE && nMode != OBS_MODE_CHASE && nMode != OBS_MODE_DEATHCAM && nMode != OBS_MODE_FREEZECAM)
			continue;

		player_info_t playerInfo = {};
		if (!I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &playerInfo))
			continue;

		// respawn time: apenas quando é do seu time e o jogador está morto
		float respawnTime = 0.0f;
		if (pPlayer->m_iTeamNum() == pLocal->m_iTeamNum() && !pPlayer->IsAlive())
		{
			// usa PlayerResource para obter next respawn absolute time
			if (auto* pr = GetTFPlayerResource())
			{
				int idx = pPlayer->entindex();
				float nextRespawn = pr->GetNextRespawnTime(idx);
				float remain = nextRespawn - static_cast<float>(Plat_FloatTime());
				if (remain > 0.0f) respawnTime = remain;
			}
		}

		present.emplace_back(Utils::ConvertUtf8ToWide(playerInfo.name), nMode, respawnTime, 0.0f, false, 0.0f);
	}

	// Se você está espectando alguém vivo, mostra quem mais está espectando essa pessoa
	const auto pObserverTarget = pLocal->m_hObserverTarget().Get();
	if (pObserverTarget && pObserverTarget != pLocal)
	{
		for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PLAYERS_OBSERVER))
		{
			const auto pPlayer = pEntity->As<C_TFPlayer>();
			if (!pPlayer || pPlayer == pLocal)
				continue;

			if (pPlayer->m_hObserverTarget().Get() != pObserverTarget)
				continue;

			const int nMode = pPlayer->m_iObserverMode();
			if (nMode != OBS_MODE_IN_EYE && nMode != OBS_MODE_CHASE && nMode != OBS_MODE_DEATHCAM && nMode != OBS_MODE_FREEZECAM)
				continue;

			player_info_t playerInfo = {};
			if (!I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &playerInfo))
				continue;

			float respawnTime = 0.0f;
			if (pPlayer->m_iTeamNum() == pLocal->m_iTeamNum() && !pPlayer->IsAlive())
			{
				if (auto* pr = GetTFPlayerResource())
				{
					int idx = pPlayer->entindex();
					float nextRespawn = pr->GetNextRespawnTime(idx);
					float remain = nextRespawn - static_cast<float>(Plat_FloatTime());
					if (remain > 0.0f) respawnTime = remain;
				}
			}

			present.emplace_back(Utils::ConvertUtf8ToWide(playerInfo.name), nMode, respawnTime, 0.0f, false, 0.0f);
		}
	}

	// Merge: manter antigos, iniciar animação para novos, marcar removals para os que sumiram
	std::vector<Spectator_t> merged;
	const float now = static_cast<float>(Plat_FloatTime());

	// 1) preserve or start for present entries
	for (auto& p : present)
	{
		bool preserved = false;
		for (auto& old : m_vecSpectators)
		{
			if (old.Name == p.Name)
			{
				// preserve animation time if it was already marked and not removing
				p.m_flAnimationTime = old.m_bRemoving ? p.m_flAnimationTime : old.m_flAnimationTime;
				// preserve remove state if old was removing (shouldn't happen for present, but safe)
				p.m_bRemoving = old.m_bRemoving;
				p.m_flRemoveTime = old.m_flRemoveTime;
				p.m_fRespawnTime = p.m_fRespawnTime > 0.0f ? p.m_fRespawnTime : old.m_fRespawnTime;
				preserved = true;
				break;
			}
		}
		if (!preserved)
		{
			// new entry: set entry animation start
			p.m_flAnimationTime = now;
			p.m_bRemoving = false;
			p.m_flRemoveTime = 0.0f;
		}
		merged.push_back(p);
	}

	// 2) detect removals (present in old but not in merged), keep them flagged to animate out
	for (const auto& old : m_vecSpectators)
	{
		// skip already present (we handled above)
		bool stillPresent = false;
		for (const auto& m : merged)
		{
			if (m.Name == old.Name)
			{
				stillPresent = true;
				break;
			}
		}
		if (!stillPresent)
		{
			// if already flagged removing, preserve timing; otherwise start remove animation
			Spectator_t rem = old;
			if (!rem.m_bRemoving)
			{
				rem.m_bRemoving = true;
				rem.m_flRemoveTime = now;
			}
			merged.push_back(rem);
		}
	}

	// 3) sort: non-removing first by mode priority + name, then removing items (so fade-outs don't jump)
	std::sort(merged.begin(), merged.end(), [](const Spectator_t& a, const Spectator_t& b) {
		if (a.m_bRemoving != b.m_bRemoving) return !a.m_bRemoving; // non-removing first
		if (a.m_bRemoving && b.m_bRemoving) // both removing: preserve original order by remove time
			return a.m_flRemoveTime < b.m_flRemoveTime;
		int pa = ModePriority(a.m_nMode);
		int pb = ModePriority(b.m_nMode);
		if (pa != pb) return pa < pb;
		return a.Name < b.Name;
		});

	m_vecSpectators = std::move(merged);
	return !m_vecSpectators.empty();
}

void CSpectatorList::Drag()
{
	const int mouseX = H::Input->GetMouseX();
	const int mouseY = H::Input->GetMouseY();
	const short LMB = 0x01;

	const int headerX = m_nX + PADDING_X;
	const int headerY = m_nY + PADDING_Y;
	const int headerW = LIST_WIDTH - (PADDING_X * 2);
	const int headerH = HEADER_HEIGHT;

	if (!m_bDragging && H::Input->IsPressed(LMB))
	{
		if (mouseX >= headerX && mouseX <= headerX + headerW && mouseY >= headerY && mouseY <= headerY + headerH)
		{
			m_bDragging = true;
			m_nDragOffsetX = mouseX - m_nX;
			m_nDragOffsetY = mouseY - m_nY;
		}
	}

	if (m_bDragging)
	{
		if (H::Input->IsDown(LMB))
		{
			int screenW = 0, screenH = 0;
			I::EngineClient->GetScreenSize(screenW, screenH);

			m_nX = mouseX - m_nDragOffsetX;
			m_nY = mouseY - m_nDragOffsetY;

			int maxX = screenW - LIST_WIDTH;
			int maxY = screenH - (HEADER_HEIGHT + (ITEM_HEIGHT + ITEM_SPACING) * static_cast<int>(std::max<size_t>(1, m_vecSpectators.size())));
			if (maxX < 0) maxX = 0;
			if (maxY < 0) maxY = 0;

			m_nX = std::max(0, std::min(m_nX, maxX));
			m_nY = std::max(0, std::min(m_nY, maxY));
		}
		else
		{
			m_bDragging = false;
		}
	}
}

void CSpectatorList::Run()
{
	if (!CFG::Visual_Spectatorlist)
		return;

	if (CFG::Misc_Clean_Screenshot && I::EngineClient->IsTakingScreenshot())
		return;

	if (!F::Menu->IsOpen() && (I::EngineVGui->IsGameUIVisible() || SDKUtils::BInEndOfMatch()))
		return;

	int screenWidth = 0, screenHeight = 0;
	I::EngineClient->GetScreenSize(screenWidth, screenHeight);
	if (m_nX == 0)
		m_nX = (screenWidth / 2) - (LIST_WIDTH / 2);

	// processa arrasto (usa input atual)
	Drag();

	// atualiza lista (merge interno gerencia animações)
	GetSpectators();

	const int itemCount = static_cast<int>(m_vecSpectators.size());
	const int totalItemsHeight = itemCount > 0 ? (ITEM_HEIGHT + ITEM_SPACING) * itemCount : ITEM_HEIGHT;
	const int totalHeight = PADDING_Y * 2 + HEADER_HEIGHT + totalItemsHeight;

	// fundo base (flat, sem transparência)
	H::Draw->Rect(m_nX, m_nY, LIST_WIDTH, totalHeight, BG_COLOR);

	// linha acento no topo (flat)
	H::Draw->Rect(m_nX, m_nY, LIST_WIDTH, 3, ACCENT_COLOR);

	// título (minimal)
	H::Draw->String(
		H::Fonts->Get(EFonts::Menu),
		m_nX + PADDING_X,
		m_nY + PADDING_Y + HEADER_HEIGHT / 2,
		TEXT_DIM,
		POS_CENTERY,
		"spectators"
	);

	// separador fino abaixo do header
	H::Draw->Line(m_nX, m_nY + PADDING_Y + HEADER_HEIGHT, m_nX + LIST_WIDTH, m_nY + PADDING_Y + HEADER_HEIGHT, BORDER_COLOR);

	// desenho de cada espectador (com animações de entrada/saída)
	std::vector<size_t> toRemoveIndices;
	const float now = static_cast<float>(Plat_FloatTime());

	for (size_t i = 0; i < m_vecSpectators.size(); ++i)
	{
		const auto& spectator = m_vecSpectators[i];
		const int itemY = m_nY + PADDING_Y + HEADER_HEIGHT + static_cast<int>(i) * (ITEM_HEIGHT + ITEM_SPACING);

		// determinar alpha pela animação
		unsigned char alpha = 255;
		if (spectator.m_bRemoving)
		{
			float elapsed = now - spectator.m_flRemoveTime;
			if (elapsed >= FADE_OUT_DURATION)
			{
				// marcar para remoção após o loop
				toRemoveIndices.push_back(i);
				continue; // não desenhar
			}
			float t = std::clamp(1.0f - (elapsed / FADE_OUT_DURATION), 0.0f, 1.0f);
			alpha = static_cast<unsigned char>(255.0f * t);
		}
		else if (spectator.m_flAnimationTime > 0.0f)
		{
			float elapsed = now - spectator.m_flAnimationTime;
			if (elapsed < ANIMATION_DURATION)
			{
				float t = std::clamp(elapsed / ANIMATION_DURATION, 0.0f, 1.0f);
				alpha = static_cast<unsigned char>(255.0f * t);
			}
			else
			{
				alpha = 255;
			}
		}

		// cor do texto (pode ser temporariamente tint para entrada)
		Color_t itemTextColor = TEXT_COLOR;
		itemTextColor.a = alpha;

		// caso tenha respawnTime > 0 mostrar "(x.xs)"
		wchar_t displayText[256];
		if (spectator.m_fRespawnTime > 0.0f)
		{
			swprintf_s(displayText, L"[%hs] %s (%.1fs)",
				(spectator.m_nMode == OBS_MODE_IN_EYE) ? "1ST" :
				(spectator.m_nMode == OBS_MODE_CHASE) ? "3RD" :
				(spectator.m_nMode == OBS_MODE_DEATHCAM) ? "DEATH" :
				(spectator.m_nMode == OBS_MODE_FREEZECAM) ? "FREEZE" : "UNK",
				spectator.Name.c_str(),
				spectator.m_fRespawnTime
			);
		}
		else
		{
			swprintf_s(displayText, L"[%hs] %s",
				(spectator.m_nMode == OBS_MODE_IN_EYE) ? "1ST" :
				(spectator.m_nMode == OBS_MODE_CHASE) ? "3RD" :
				(spectator.m_nMode == OBS_MODE_DEATHCAM) ? "DEATH" :
				(spectator.m_nMode == OBS_MODE_FREEZECAM) ? "FREEZE" : "UNK",
				spectator.Name.c_str()
			);
		}

		H::Draw->String(
			H::Fonts->Get(EFonts::Menu),
			m_nX + PADDING_X,
			itemY + ITEM_HEIGHT / 2,
			itemTextColor,
			POS_CENTERY,
			displayText
		);
	}

	// remove itens que finalizaram animação de saída (iterar de trás para frente)
	for (auto it = toRemoveIndices.rbegin(); it != toRemoveIndices.rend(); ++it)
	{
		m_vecSpectators.erase(m_vecSpectators.begin() + *it);
	}

	// borda sutil
	H::Draw->OutlinedRect(m_nX, m_nY, LIST_WIDTH, totalHeight, BORDER_COLOR);
}