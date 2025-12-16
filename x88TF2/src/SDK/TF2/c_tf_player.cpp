#include "c_tf_player.h"

#include "../src/Features/PlayersList/PlayersList.h"

bool C_TFPlayer::IsPlayerOnSteamFriendsList()
{
    auto result{ reinterpret_cast<bool(__fastcall*)(void*, void*)>(Signatures::CTFPlayer_IsPlayerOnSteamFriendsList.Get())(this, this) };

    if (!result)
    {
        PlayerPriority info{};

        return F::Players->GetInfo(this->entindex(), info) && info.Ignored;
    }

    return result;
}