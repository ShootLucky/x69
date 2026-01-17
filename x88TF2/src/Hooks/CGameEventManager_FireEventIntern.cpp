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
Color_t PREFIX_COLOR = Color_t(0, 150, 255, 255); // Cyan for x69
Color_t BRACKET_COLOR = Color_t(255, 255, 255, 255); // White for []

std::pair<std::string, std::string> GetTaggedName(const player_info_t& pi, Color_t& tag_col) {
    std::string name_str = pi.name;
    std::string chat_name = std::string(pi.name);
    PlayerPriority pri{};
    tag_col = Color_t(255, 255, 255, 255); // Default white
    std::string tag_str;
    std::string chat_tag;

    if (F::Players->GetInfoGUID(pi.guid, pri)) {
        if (pri.Cheater) {
            tag_str += " [Cheater]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + CFG::Color_Cheater.toHexStr() + "Cheater\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = CFG::Color_Cheater;
        }
        else if (pri.CheaterLight) {
            tag_str += " [Cheater Light]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + Color_t(255, 150, 150, 255).toHexStr() + "Cheater Light\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = Color_t(255, 150, 150, 255);
        }
        else if (pri.RijinUser) {
            tag_str += " [Rijin User]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + Color_t(255, 0, 255, 255).toHexStr() + "Rijin User\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = Color_t(255, 0, 255, 255);
        }
        else if (pri.LmaoboxUser) {
            tag_str += " [Lmaobox User]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + Color_t(0, 255, 255, 255).toHexStr() + "Lmaobox User\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = Color_t(0, 255, 255, 255);
        }
        else if (pri.NethookUser) {
            tag_str += " [Nethook User]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + Color_t(128, 0, 128, 255).toHexStr() + "Nethook User\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = Color_t(128, 0, 128, 255);
        }
        else if (pri.RetardLegit) {
            tag_str += " [Retard Legit]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + CFG::Color_RetardLegit.toHexStr() + "Retard Legit\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = CFG::Color_RetardLegit;
        }
        else if (pri.Suspect) {
            tag_str += " [Suspect]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + Color_t(255, 255, 0, 255).toHexStr() + "Suspect\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = Color_t(255, 255, 0, 255);
        }
        else if (pri.Ignored) {
            tag_str += " [Ignored]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + CFG::Color_Friend.toHexStr() + "Ignored\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = CFG::Color_Friend;
        }
    }

    name_str += tag_str;
    chat_name += chat_tag;
    return { name_str, "\x01" + chat_name };
}

std::pair<std::string, std::string> GetTaggedNameByGUID(const std::string& guid, const std::string& name, Color_t& tag_col) {
    std::string name_str = name;
    std::string chat_name = std::string(name);
    PlayerPriority pri{};
    tag_col = Color_t(255, 255, 255, 255); // Default white
    std::string tag_str;
    std::string chat_tag;

    if (F::Players->GetInfoGUID(guid, pri)) {
        if (pri.Cheater) {
            tag_str += " [Cheater]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + CFG::Color_Cheater.toHexStr() + "Cheater\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = CFG::Color_Cheater;
        }
        else if (pri.CheaterLight) {
            tag_str += " [Cheater Light]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + Color_t(255, 150, 150, 255).toHexStr() + "Cheater Light\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = Color_t(255, 150, 150, 255);
        }
        else if (pri.RijinUser) {
            tag_str += " [Rijin User]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + Color_t(255, 0, 255, 255).toHexStr() + "Rijin User\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = Color_t(255, 0, 255, 255);
        }
        else if (pri.LmaoboxUser) {
            tag_str += " [Lmaobox User]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + Color_t(0, 255, 255, 255).toHexStr() + "Lmaobox User\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = Color_t(0, 255, 255, 255);
        }
        else if (pri.NethookUser) {
            tag_str += " [Nethook User]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + Color_t(128, 0, 128, 255).toHexStr() + "Nethook User\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = Color_t(128, 0, 128, 255);
        }
        else if (pri.RetardLegit) {
            tag_str += " [Retard Legit]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + CFG::Color_RetardLegit.toHexStr() + "Retard Legit\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = CFG::Color_RetardLegit;
        }
        else if (pri.Suspect) {
            tag_str += " [Suspect]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + Color_t(255, 255, 0, 255).toHexStr() + "Suspect\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = Color_t(255, 255, 0, 255);
        }
        else if (pri.Ignored) {
            tag_str += " [Ignored]";
            chat_tag += " \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + CFG::Color_Friend.toHexStr() + "Ignored\x08" + BRACKET_COLOR.toHexStr() + "]";
            tag_col = CFG::Color_Friend;
        }
    }

    name_str += tag_str;
    chat_name += chat_tag;
    return { name_str, "\x01" + chat_name };
}

void LogMessage(const std::string& msg, Color_t col, const std::string& chat_colored_msg) {
    if (!CFG::Logs_Enable) return;
    bool to_chat = (CFG::Logs_Type & 1);
    bool to_console = (CFG::Logs_Type & 2);
    bool to_screen = (CFG::Logs_Type & 4);
    if (to_chat) {
        I::ClientModeShared->m_pChatElement->ChatPrintf(0, chat_colored_msg.c_str());
    }
    if (to_console) {
        I::CVar->ConsoleColorPrintf(col, "%s\n", msg.c_str());
    }
    if (to_screen) {
        g_notification_system->add_notification(msg, 5000);
    }
}

void OnVoteCast(IGameEvent* event) {
    if (!CFG::Logs_Enable) return;
    int team = event->GetInt("team");
    int voteOption = event->GetInt("vote_option");
    int entityid = event->GetInt("entityid");
    player_info_t pi{};
    if (!I::EngineClient->GetPlayerInfo(entityid, &pi)) return;
    int localTeam = 0;
    auto pLocal = H::Entities->GetLocal();
    if (pLocal) localTeam = pLocal->m_iTeamNum();
    bool isEnemy = (team != localTeam);
    if (isEnemy && !(CFG::PlayersLogs_Type & PL_ENEMYVOTE)) return;
    if (!isEnemy && !(CFG::PlayersLogs_Type & PL_TEAMVOTE)) return;
    Color_t tag_col;
    auto [tagged_name, tagged_chat_name] = GetTaggedName(pi, tag_col);
    std::string voteStr = (voteOption == 1) ? "Yes" : "No";
    std::string msg = std::format("{} voted: {}", tagged_name, voteStr);
    Color_t col(255, 165, 0, 255);
    std::string chat_colored_msg = tagged_chat_name + "\x01 voted: " + voteStr;
    LogMessage(msg, col, chat_colored_msg);
}

void OnPlayerConnect(IGameEvent* event) {
    if (!CFG::Logs_Enable || !(CFG::PlayersLogs_Type & PL_ENTER)) return;
    const char* name = event->GetString("name");
    int idx = event->GetInt("index");
    int userid = event->GetInt("userid");
    Color_t tag_col;
    auto [tagged_name, tagged_chat_name] = GetTaggedNameByGUID(event->GetString("networkid"), name, tag_col);
    std::string msg = std::format("{} connected", tagged_name);
    Color_t col(0, 255, 0, 255);
    std::string chat_colored_msg = tagged_chat_name + "\x01 connected";
    LogMessage(msg, col, chat_colored_msg);
}

void OnPlayerDisconnect(IGameEvent* event) {
    if (!CFG::Logs_Enable || !(CFG::PlayersLogs_Type & PL_EXIT)) return;
    const char* name = event->GetString("name");
    std::string reason = event->GetString("reason");
    Color_t tag_col;
    auto [tagged_name, tagged_chat_name] = GetTaggedNameByGUID(event->GetString("networkid"), name, tag_col);
    std::string msg = std::format("{} disconnected ({})", tagged_name, reason);
    Color_t col(255, 0, 0, 255);
    std::string chat_colored_msg = tagged_chat_name + "\x01 disconnected (" + reason + ")";
    LogMessage(msg, col, chat_colored_msg);
}

void OnPlayerChangeName(IGameEvent* event) {
    if (!CFG::Logs_Enable || !(CFG::PlayersLogs_Type & PL_NAME)) return;
    int userid = event->GetInt("userid");
    const char* oldname = event->GetString("oldname");
    const char* newname = event->GetString("newname");
    int idx = I::EngineClient->GetPlayerForUserID(userid);
    player_info_t pi{};
    if (!I::EngineClient->GetPlayerInfo(idx, &pi)) return;
    std::string msg = std::format("{} changed name to {}", oldname, newname);
    Color_t col(255, 255, 0, 255);
    std::string chat_colored_msg = "\x01" + std::string(oldname) + " changed name to " + std::string(newname);
    LogMessage(msg, col, chat_colored_msg);
}

void OnPlayerChangeClass(IGameEvent* event) {
    if (!CFG::Logs_Enable || !(CFG::PlayersLogs_Type & PL_CLASS)) return;
    int userid = event->GetInt("userid");
    int playerClass = event->GetInt("class");
    int idx = I::EngineClient->GetPlayerForUserID(userid);
    player_info_t pi{};
    if (!I::EngineClient->GetPlayerInfo(idx, &pi)) return;
    static const char* classNames[] = { "Unknown", "Scout", "Sniper", "Soldier", "Demoman", "Medic", "Heavy", "Pyro", "Spy", "Engineer" };
    const char* className = (playerClass >= 1 && playerClass <= 9) ? classNames[playerClass] : "Unknown";
    Color_t tag_col;
    auto [tagged_name, tagged_chat_name] = GetTaggedName(pi, tag_col);
    std::string msg = std::format("{} changed class to {}", tagged_name, className);
    Color_t col(100, 200, 255, 255);
    std::string chat_colored_msg = tagged_chat_name + "\x01 changed class to " + className;
    LogMessage(msg, col, chat_colored_msg);
}

void OnPlayerSpawn(IGameEvent* event) {
    if (!CFG::Logs_Enable || !(CFG::PlayersLogs_Type & PL_RESPAWN)) return;
    int userid = event->GetInt("userid");
    int idx = I::EngineClient->GetPlayerForUserID(userid);
    player_info_t pi{};
    if (!I::EngineClient->GetPlayerInfo(idx, &pi)) return;
    Color_t tag_col;
    auto [tagged_name, tagged_chat_name] = GetTaggedName(pi, tag_col);
    std::string msg = std::format("{} respawned", tagged_name);
    std::string chat_colored_msg = tagged_chat_name + "\x01 respawned";
    LogMessage(msg, Color_t(255, 255, 255, 255), chat_colored_msg);
}

void OnPlayerHurt(IGameEvent* event) {
    if (!CFG::Logs_Enable || !(CFG::PlayersLogs_Type & PL_DAMAGE)) return;
    int attacker = I::EngineClient->GetPlayerForUserID(event->GetInt("attacker"));
    if (attacker != I::EngineClient->GetLocalPlayer()) return;
    int victim = I::EngineClient->GetPlayerForUserID(event->GetInt("userid"));
    player_info_t pi{};
    if (!I::EngineClient->GetPlayerInfo(victim, &pi)) return;
    int dmg = event->GetInt("damageamount");
    if (dmg <= 0) return;
    int remaining = event->GetInt("health");
    Color_t tag_col;
    auto [tagged_name, tagged_chat_name] = GetTaggedName(pi, tag_col);
    std::string msg = std::format("Damaged {} for {} ({} remaining)", tagged_name, dmg, remaining);
    Color_t col(0, 150, 255, 255);
    std::string chat_colored_msg = "\x01 Damaged " + tagged_chat_name + "\x01 for \x08" + col.toHexStr() + std::to_string(dmg) + "\x01 (" + std::to_string(remaining) + " remaining)";
    LogMessage(msg, col, chat_colored_msg);
}

void OnPlayerDeath(IGameEvent* event) {
    if (!CFG::Logs_Enable || !(CFG::PlayersLogs_Type & PL_FRIED)) return;
    int attacker = I::EngineClient->GetPlayerForUserID(event->GetInt("attacker"));
    if (attacker != I::EngineClient->GetLocalPlayer()) return;
    int victim = I::EngineClient->GetPlayerForUserID(event->GetInt("userid"));
    player_info_t pi{};
    if (!I::EngineClient->GetPlayerInfo(victim, &pi)) return;
    std::string weapon = event->GetString("weapon");
    Color_t tag_col;
    auto [tagged_name, tagged_chat_name] = GetTaggedName(pi, tag_col);
    std::string msg = std::format("You fried {} with {}", tagged_name, weapon);
    Color_t col(255, 0, 0, 255);
    std::string chat_colored_msg = "\x01 You fried " + tagged_chat_name + "\x01 with " + weapon;
    LogMessage(msg, col, chat_colored_msg);
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
            const char* const name{ event->GetString("name") };

            if (CFG::Logs_Enable && (CFG::PlayersLogs_Type & PL_PLAYERLIST)) {
                PlayerPriority pri{};
                if (F::Players->GetInfoGUID(event->GetString("networkid"), pri)) {
                    if (pri.Ignored) {
                        std::string msg = std::format("{} is marked as [Ignored]", name);
                        Color_t col = CFG::Color_Friend;
                        std::string chat_msg = "\x01 " + std::string(name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Ignored\x08" + BRACKET_COLOR.toHexStr() + "]";
                        LogMessage(msg, col, chat_msg);
                    }
                    if (pri.Cheater) {
                        std::string msg = std::format("{} is marked as [Cheater]", name);
                        Color_t col = CFG::Color_Cheater;
                        std::string chat_msg = "\x01 " + std::string(name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Cheater\x08" + BRACKET_COLOR.toHexStr() + "]";
                        LogMessage(msg, col, chat_msg);
                    }
                    if (pri.RetardLegit) {
                        std::string msg = std::format("{} is marked as [Retard Legit]", name);
                        Color_t col = CFG::Color_RetardLegit;
                        std::string chat_msg = "\x01 " + std::string(name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Retard Legit\x08" + BRACKET_COLOR.toHexStr() + "]";
                        LogMessage(msg, col, chat_msg);
                    }
                    if (pri.CheaterLight) {
                        std::string msg = std::format("{} is marked as [Cheater Light]", name);
                        Color_t col = Color_t(255, 150, 150, 255);
                        std::string chat_msg = "\x01 " + std::string(name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Cheater Light\x08" + BRACKET_COLOR.toHexStr() + "]";
                        LogMessage(msg, col, chat_msg);
                    }
                    if (pri.RijinUser) {
                        std::string msg = std::format("{} is marked as [Rijin User]", name);
                        Color_t col = Color_t(255, 0, 255, 255);
                        std::string chat_msg = "\x01 " + std::string(name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Rijin User\x08" + BRACKET_COLOR.toHexStr() + "]";
                        LogMessage(msg, col, chat_msg);
                    }
                    if (pri.LmaoboxUser) {
                        std::string msg = std::format("{} is marked as [Lmaobox User]", name);
                        Color_t col = Color_t(0, 255, 255, 255);
                        std::string chat_msg = "\x01 " + std::string(name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Lmaobox User\x08" + BRACKET_COLOR.toHexStr() + "]";
                        LogMessage(msg, col, chat_msg);
                    }
                    if (pri.Suspect) {
                        std::string msg = std::format("{} is marked as [Suspect]", name);
                        Color_t col = Color_t(255, 255, 0, 255);
                        std::string chat_msg = "\x01 " + std::string(name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Suspect\x08" + BRACKET_COLOR.toHexStr() + "]";
                        LogMessage(msg, col, chat_msg);
                    }
                    if (pri.NethookUser) {
                        std::string msg = std::format("{} is marked as [Nethook User]", name);
                        Color_t col = Color_t(128, 0, 128, 255);
                        std::string chat_msg = "\x01 " + std::string(name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Nethook User\x08" + BRACKET_COLOR.toHexStr() + "]";
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
