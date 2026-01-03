// Modified CGameEventManager_FireEventIntern.cpp
#include "../src/SDK/SDK.h"

#include "../CFG.h"
#include "../Features/PlayersList/PlayersList.h"
#include "../src/Features/Menu/notification_system/notifs.h"

MAKE_SIGNATURE(CGameEventManager_FireEventIntern, "engine.dll", "44 88 44 24 ? 48 89 4C 24 ? 55 57", 0x0);

static constexpr int PL_LOCAL = 1 << 0;
static constexpr int PL_FRIED = 1 << 1;
static constexpr int PL_NAME = 1 << 2;
static constexpr int PL_DAMAGE = 1 << 3;
static constexpr int PL_RESPAWN = 1 << 4;
static constexpr int PL_ENTER = 1 << 5;
static constexpr int PL_EXIT = 1 << 6;
static constexpr int PL_PLAYERLIST = 1 << 7;
static constexpr int PL_CLASS = 1 << 8;
static constexpr int PL_ENEMYVOTE = 1 << 9;
static constexpr int PL_TEAMVOTE = 1 << 10;

Color_t PREFIX_COLOR = Color_t(0, 150, 255, 255); // Cyan for [x69]

void LogMessage(std::string msg, Color_t color = Color_t(255, 255, 255, 255), std::string chat_colored_msg = "") {
    if (!CFG::Logs_Enable) return;

    // Add prefix to msg for console and screen
    std::string prefixed_msg = std::format("[x69] {}", msg);

    bool to_chat = (CFG::Logs_Type & 1);
    bool to_console = (CFG::Logs_Type & 2);
    bool to_screen = (CFG::Logs_Type & 4);

    if (to_chat) {
        std::string chat_msg;
        if (!chat_colored_msg.empty()) {
            chat_msg = "\x08" + PREFIX_COLOR.toHexStr() + "[x69]" + chat_colored_msg;
        }
        else {
            chat_msg = "\x08" + PREFIX_COLOR.toHexStr() + "[x69]\x01" + msg;
        }
        I::ClientModeShared->m_pChatElement->ChatPrintf(0, chat_msg.c_str());
    }

    if (to_console) {
        I::CVar->ConsoleColorPrintf(color, "%s\n", prefixed_msg.c_str());
    }

    if (to_screen) {
        g_notification_system->add_notification(prefixed_msg, 5000);
    }
}

void OnVoteCast(IGameEvent* event) {
    if (!CFG::Logs_Enable) return;

    const auto team = event->GetInt("team");
    if (team == -1) return;

    const auto pLocal = H::Entities->GetLocal();
    if (!pLocal) return;

    const auto voter = event->GetInt("entityid");
    bool isLocal = voter == I::EngineClient->GetLocalPlayer();

    if (isLocal) {
        if (!(CFG::PlayersLogs_Type & PL_LOCAL)) return;
    }
    else {
        bool isTeamVote = pLocal->m_iTeamNum() == team;
        int reqBit = isTeamVote ? PL_TEAMVOTE : PL_ENEMYVOTE;
        if (!(CFG::PlayersLogs_Type & reqBit)) return;
    }

    player_info_t pi{};
    if (!I::EngineClient->GetPlayerInfo(voter, &pi)) return;

    bool yes = event->GetInt("vote_option") == 0;
    Color_t col = yes ? Color_t(46, 204, 113, 255) : Color_t(231, 76, 60, 255);
    std::string msg = std::format("{} voted {}", pi.name, yes ? "YES" : "NO");
    std::string chat_msg = "\x01" + std::string(pi.name) + " voted \x08" + col.toHexStr() + (yes ? "YES" : "NO");

    LogMessage(msg, col, chat_msg);
}

void OnPlayerConnect(IGameEvent* event) {
    if (!CFG::Logs_Enable || !(CFG::PlayersLogs_Type & PL_ENTER)) return;

    std::string name = event->GetString("name");
    std::string msg = std::format("{} has joined the game", name);
    LogMessage(msg);
}

void OnPlayerDisconnect(IGameEvent* event) {
    if (!CFG::Logs_Enable || !(CFG::PlayersLogs_Type & PL_EXIT)) return;

    std::string name = event->GetString("name");
    std::string reason = event->GetString("reason");
    std::string msg = std::format("{} has left the game ({})", name, reason);
    LogMessage(msg);
}

void OnPlayerChangeName(IGameEvent* event) {
    if (!CFG::Logs_Enable || !(CFG::PlayersLogs_Type & PL_NAME)) return;

    std::string oldname = event->GetString("oldname");
    std::string newname = event->GetString("newname");
    std::string msg = std::format("{} changed name to {}", oldname, newname);
    LogMessage(msg);
}

void OnPlayerChangeClass(IGameEvent* event) {
    if (!CFG::Logs_Enable) return;

    int userid = event->GetInt("userid");
    int index = I::EngineClient->GetPlayerForUserID(userid);
    bool isLocal = index == I::EngineClient->GetLocalPlayer();

    if (isLocal) {
        if (!(CFG::PlayersLogs_Type & PL_LOCAL)) return;
    }
    else {
        if (!(CFG::PlayersLogs_Type & PL_CLASS)) return;
    }

    player_info_t pi{};
    if (!I::EngineClient->GetPlayerInfo(index, &pi)) return;

    int classid = event->GetInt("class");
    std::string classname;
    switch (classid) {
    case 1: classname = "Scout"; break;
    case 2: classname = "Sniper"; break;
    case 3: classname = "Soldier"; break;
    case 4: classname = "Demoman"; break;
    case 5: classname = "Medic"; break;
    case 6: classname = "Heavy"; break;
    case 7: classname = "Pyro"; break;
    case 8: classname = "Spy"; break;
    case 9: classname = "Engineer"; break;
    default: classname = "Unknown"; break;
    }

    std::string msg = std::format("{} changed class to {}", pi.name, classname);
    LogMessage(msg);
}

void OnPlayerSpawn(IGameEvent* event) {
    if (!CFG::Logs_Enable) return;

    int userid = event->GetInt("userid");
    int index = I::EngineClient->GetPlayerForUserID(userid);
    bool isLocal = index == I::EngineClient->GetLocalPlayer();

    if (isLocal) {
        if (!(CFG::PlayersLogs_Type & PL_LOCAL)) return;
    }
    else {
        if (!(CFG::PlayersLogs_Type & PL_RESPAWN)) return;
    }

    player_info_t pi{};
    if (!I::EngineClient->GetPlayerInfo(index, &pi)) return;

    std::string msg = std::format("{} respawned", pi.name);
    LogMessage(msg);
}

void OnPlayerHurt(IGameEvent* event) {
    if (!CFG::Logs_Enable || !(CFG::PlayersLogs_Type & PL_DAMAGE)) return;

    int attacker = I::EngineClient->GetPlayerForUserID(event->GetInt("attacker"));
    if (attacker != I::EngineClient->GetLocalPlayer()) return;

    int victim = I::EngineClient->GetPlayerForUserID(event->GetInt("userid"));
    player_info_t pi{};
    if (!I::EngineClient->GetPlayerInfo(victim, &pi)) return;

    int dmg = event->GetInt("damageamount");
    if (dmg <= 0) return; // Prevent logging zero or negative damage

    int remaining = event->GetInt("health");
    std::string msg = std::format("Damaged {} for {} ({} remaining)", pi.name, dmg, remaining);
    Color_t col(255, 255, 0, 255);
    LogMessage(msg, col);
}

void OnPlayerDeath(IGameEvent* event) {
    if (!CFG::Logs_Enable || !(CFG::PlayersLogs_Type & PL_FRIED)) return;

    int attacker = I::EngineClient->GetPlayerForUserID(event->GetInt("attacker"));
    if (attacker != I::EngineClient->GetLocalPlayer()) return;

    int victim = I::EngineClient->GetPlayerForUserID(event->GetInt("userid"));
    player_info_t pi{};
    if (!I::EngineClient->GetPlayerInfo(victim, &pi)) return;

    std::string weapon = event->GetString("weapon");
    std::string msg = std::format("You fried {} with {}", pi.name, weapon);
    Color_t col(255, 0, 0, 255);
    LogMessage(msg, col);
}

MAKE_HOOK(CGameEventManager_FireEventIntern, Signatures::CGameEventManager_FireEventIntern.Get(), bool, __fastcall,
    void* ecx, IGameEvent* event, bool bServerOnly, bool bClientOnly) {
    if (event) {
        static constexpr auto vote_cast{ HASH_CT("vote_cast") };
        static constexpr auto player_connect_client{ HASH_CT("player_connect_client") };
        static constexpr auto player_disconnect{ HASH_CT("player_disconnect") };
        static constexpr auto player_changename{ HASH_CT("player_changename") };
        static constexpr auto player_changeclass{ HASH_CT("player_changeclass") };
        static constexpr auto player_spawn{ HASH_CT("player_spawn") };
        static constexpr auto player_hurt{ HASH_CT("player_hurt") };
        static constexpr auto player_death{ HASH_CT("player_death") };

        auto hash = HASH_RT(event->GetName());

        if (hash == vote_cast && bClientOnly) {
            OnVoteCast(event);
        }

        if (hash == player_connect_client && bClientOnly) {
            if (CFG::Logs_Enable && (CFG::PlayersLogs_Type & PL_PLAYERLIST)) {
                PlayerPriority pi{};

                if (F::Players->GetInfoGUID(event->GetString("networkid"), pi)) {
                    const char* const name{ event->GetString("name") };

                    if (pi.Ignored) {
                        std::string msg = std::format("{} is marked as [Ignored]", name);
                        Color_t col = CFG::Color_Friend;
                        std::string chat_msg = "\x01" + std::string(name) + " is marked as \x08" + col.toHexStr() + "[Ignored]";
                        LogMessage(msg, col, chat_msg);
                    }

                    if (pi.Cheater) {
                        std::string msg = std::format("{} is marked as [Cheater]", name);
                        Color_t col = CFG::Color_Cheater;
                        std::string chat_msg = "\x01" + std::string(name) + " is marked as \x08" + col.toHexStr() + "[Cheater]";
                        LogMessage(msg, col, chat_msg);
                    }

                    if (pi.RetardLegit) {
                        std::string msg = std::format("{} is marked as [Retard Legit]", name);
                        Color_t col = CFG::Color_RetardLegit;
                        std::string chat_msg = "\x01" + std::string(name) + " is marked as \x08" + col.toHexStr() + "[Retard Legit]";
                        LogMessage(msg, col, chat_msg);
                    }

                    if (pi.CheaterLight) {
                        std::string msg = std::format("{} is marked as [Cheater Light]", name);
                        Color_t col = Color_t(255, 150, 150, 255); // Light red
                        std::string chat_msg = "\x01" + std::string(name) + " is marked as \x08" + col.toHexStr() + "[Cheater Light]";
                        LogMessage(msg, col, chat_msg);
                    }

                    if (pi.RijinUser) {
                        std::string msg = std::format("{} is marked as [Rijin User]", name);
                        Color_t col = Color_t(255, 0, 255, 255); // Magenta
                        std::string chat_msg = "\x01" + std::string(name) + " is marked as \x08" + col.toHexStr() + "[Rijin User]";
                        LogMessage(msg, col, chat_msg);
                    }

                    if (pi.LmaoboxUser) {
                        std::string msg = std::format("{} is marked as [Lmaobox User]", name);
                        Color_t col = Color_t(0, 255, 255, 255); // Cyan
                        std::string chat_msg = "\x01" + std::string(name) + " is marked as \x08" + col.toHexStr() + "[Lmaobox User]";
                        LogMessage(msg, col, chat_msg);
                    }

                    if (pi.Suspect) {
                        std::string msg = std::format("{} is marked as [Suspect]", name);
                        Color_t col = Color_t(255, 255, 0, 255); // Yellow
                        std::string chat_msg = "\x01" + std::string(name) + " is marked as \x08" + col.toHexStr() + "[Suspect]";
                        LogMessage(msg, col, chat_msg);
                    }

                    if (pi.NethookUser) {
                        std::string msg = std::format("{} is marked as [Nethook User]", name);
                        Color_t col = Color_t(128, 0, 128, 255); // Purple
                        std::string chat_msg = "\x01" + std::string(name) + " is marked as \x08" + col.toHexStr() + "[Nethook User]";
                        LogMessage(msg, col, chat_msg);
                    }
                }
            }

            OnPlayerConnect(event);
        }

        if (hash == player_disconnect && bClientOnly) {
            OnPlayerDisconnect(event);
        }

        if (hash == player_changename && bClientOnly) {
            OnPlayerChangeName(event);
        }

        if (hash == player_changeclass && bClientOnly) {
            OnPlayerChangeClass(event);
        }

        if (hash == player_spawn && bClientOnly) {
            OnPlayerSpawn(event);
        }

        if (hash == player_hurt && bClientOnly) {
            OnPlayerHurt(event);
        }

        if (hash == player_death && bClientOnly) {
            OnPlayerDeath(event);
        }
    }

    return CALL_ORIGINAL(ecx, event, bServerOnly, bClientOnly);
}