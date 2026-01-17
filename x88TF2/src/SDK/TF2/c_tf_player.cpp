#include "c_tf_player.h"

#include "../src/Features/PlayersList/PlayersList.h"

bool C_TFPlayer::IsPlayerOnSteamFriendsList()
{
	// Chama a função original do TF2
	auto result{ reinterpret_cast<bool(__fastcall*)(void*, void*)>(
		Signatures::CTFPlayer_IsPlayerOnSteamFriendsList.Get())(this, this) };

	// Se o resultado da função original for false, verifica se está na lista de Ignored
	if (!result)
	{
		PlayerPriority info{};

		// Retorna true se o jogador está marcado como Ignored (para compatibilidade)
		return F::Players->GetInfo(this->entindex(), info) && info.Ignored;
	}

	return result;
}