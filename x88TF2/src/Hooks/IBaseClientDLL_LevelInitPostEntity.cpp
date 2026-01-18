// Modified IBaseClientDLL_LevelInitPostEntity.cpp
#include "../src/SDK/SDK.h"

#include "../Features/PlayersList/PlayersList.h"
#include "../CFG.h"
#include "../src/Features/Menu/notification_system/notifs.h"

extern Color_t BRACKET_COLOR;

MAKE_HOOK(IBaseClientDLL_LevelInitPostEntity, Memory::GetVFunc(I::BaseClientDLL, 6), void, __fastcall,
    void* ecx) {
    CALL_ORIGINAL(ecx);

    H::Entities->UpdateModelIndexes();

    if (CFG::Logs_Enable && (CFG::PlayersLogs_Type & (1 << 7))) {  // PL_PLAYERLIST
        for (int n = 1; n <= I::EngineClient->GetMaxClients(); ++n) {
            player_info_t pi_game{};
            if (!I::EngineClient->GetPlayerInfo(n, &pi_game) || pi_game.fakeplayer) continue;

            PlayerPriority pri{};
            if (F::Players->GetInfoGUID(pi_game.guid, pri)) {
                if (pri.Ignored) {
                    std::string msg = std::format("{} is marked as [Ignored]", pi_game.name);
                    Color_t col = IGNORED_COLOR;
                    std::string chat_msg = "\x01 " + std::string(pi_game.name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Ignored\x08" + BRACKET_COLOR.toHexStr() + "]";
                    // Copy of LogMessage
                    if (CFG::Logs_Enable) {
                        bool to_chat = (CFG::Logs_Type & 1);
                        bool to_console = (CFG::Logs_Type & 2);
                        bool to_screen = (CFG::Logs_Type & 4);

                        if (to_chat) {
                            I::ClientModeShared->m_pChatElement->ChatPrintf(0, chat_msg.c_str());
                        }

                        if (to_console) {
                            I::CVar->ConsoleColorPrintf(col, "%s\n", msg.c_str());
                        }

                        if (to_screen) {
                            gNotify.add_notification(msg, 5000);
                        }
                    }
                }

                if (pri.Cheater) {
                    std::string msg = std::format("{} is marked as [Cheater]", pi_game.name);
                    Color_t col = CHEATER_COLOR;
                    std::string chat_msg = "\x01 " + std::string(pi_game.name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Cheater\x08" + BRACKET_COLOR.toHexStr() + "]";
                    // Same LogMessage copy
                    if (CFG::Logs_Enable) {
                        bool to_chat = (CFG::Logs_Type & 1);
                        bool to_console = (CFG::Logs_Type & 2);
                        bool to_screen = (CFG::Logs_Type & 4);

                        if (to_chat) {
                            I::ClientModeShared->m_pChatElement->ChatPrintf(0, chat_msg.c_str());
                        }

                        if (to_console) {
                            I::CVar->ConsoleColorPrintf(col, "%s\n", msg.c_str());
                        }

                        if (to_screen) {
                            gNotify.add_notification(msg, 5000);
                        }
                    }
                }

                if (pri.RetardLegit) {
                    std::string msg = std::format("{} is marked as [Retard Legit]", pi_game.name);
                    Color_t col = RETARD_LEGIT_COLOR;
                    std::string chat_msg = "\x01 " + std::string(pi_game.name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Retard Legit\x08" + BRACKET_COLOR.toHexStr() + "]";
                    // Same
                    if (CFG::Logs_Enable) {
                        bool to_chat = (CFG::Logs_Type & 1);
                        bool to_console = (CFG::Logs_Type & 2);
                        bool to_screen = (CFG::Logs_Type & 4);

                        if (to_chat) {
                            I::ClientModeShared->m_pChatElement->ChatPrintf(0, chat_msg.c_str());
                        }

                        if (to_console) {
                            I::CVar->ConsoleColorPrintf(col, "%s\n", msg.c_str());
                        }

                        if (to_screen) {
                            gNotify.add_notification(msg, 5000);
                        }
                    }
                }

                if (pri.CheaterLight) {
                    std::string msg = std::format("{} is marked as [Cheater Light]", pi_game.name);
                    Color_t col = CHEATER_LIGHT_COLOR;
                    std::string chat_msg = "\x01 " + std::string(pi_game.name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Cheater Light\x08" + BRACKET_COLOR.toHexStr() + "]";
                    if (CFG::Logs_Enable) {
                        bool to_chat = (CFG::Logs_Type & 1);
                        bool to_console = (CFG::Logs_Type & 2);
                        bool to_screen = (CFG::Logs_Type & 4);

                        if (to_chat) {
                            I::ClientModeShared->m_pChatElement->ChatPrintf(0, chat_msg.c_str());
                        }

                        if (to_console) {
                            I::CVar->ConsoleColorPrintf(col, "%s\n", msg.c_str());
                        }

                        if (to_screen) {
                            gNotify.add_notification(msg, 5000);
                        }
                    }
                }

                if (pri.RijinUser) {
                    std::string msg = std::format("{} is marked as [Rijin User]", pi_game.name);
                    Color_t col = RIJIN_COLOR;
                    std::string chat_msg = "\x01 " + std::string(pi_game.name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Rijin User\x08" + BRACKET_COLOR.toHexStr() + "]";
                    if (CFG::Logs_Enable) {
                        bool to_chat = (CFG::Logs_Type & 1);
                        bool to_console = (CFG::Logs_Type & 2);
                        bool to_screen = (CFG::Logs_Type & 4);

                        if (to_chat) {
                            I::ClientModeShared->m_pChatElement->ChatPrintf(0, chat_msg.c_str());
                        }

                        if (to_console) {
                            I::CVar->ConsoleColorPrintf(col, "%s\n", msg.c_str());
                        }

                        if (to_screen) {
                            gNotify.add_notification(msg, 5000);
                        }
                    }
                }

                if (pri.LmaoboxUser) {
                    std::string msg = std::format("{} is marked as [Lmaobox User]", pi_game.name);
                    Color_t col = LMAOBOX_COLOR;
                    std::string chat_msg = "\x01 " + std::string(pi_game.name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Lmaobox User\x08" + BRACKET_COLOR.toHexStr() + "]";
                    if (CFG::Logs_Enable) {
                        bool to_chat = (CFG::Logs_Type & 1);
                        bool to_console = (CFG::Logs_Type & 2);
                        bool to_screen = (CFG::Logs_Type & 4);

                        if (to_chat) {
                            I::ClientModeShared->m_pChatElement->ChatPrintf(0, chat_msg.c_str());
                        }

                        if (to_console) {
                            I::CVar->ConsoleColorPrintf(col, "%s\n", msg.c_str());
                        }

                        if (to_screen) {
                            gNotify.add_notification(msg, 5000);
                        }
                    }
                }

                if (pri.Suspect) {
                    std::string msg = std::format("{} is marked as [Suspect]", pi_game.name);
                    Color_t col = SUSPECT_COLOR;
                    std::string chat_msg = "\x01 " + std::string(pi_game.name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Suspect\x08" + BRACKET_COLOR.toHexStr() + "]";
                    if (CFG::Logs_Enable) {
                        bool to_chat = (CFG::Logs_Type & 1);
                        bool to_console = (CFG::Logs_Type & 2);
                        bool to_screen = (CFG::Logs_Type & 4);

                        if (to_chat) {
                            I::ClientModeShared->m_pChatElement->ChatPrintf(0, chat_msg.c_str());
                        }

                        if (to_console) {
                            I::CVar->ConsoleColorPrintf(col, "%s\n", msg.c_str());
                        }

                        if (to_screen) {
                            gNotify.add_notification(msg, 5000);
                        }
                    }
                }

                if (pri.NethookUser) {
                    std::string msg = std::format("{} is marked as [Nethook User]", pi_game.name);
                    Color_t col = NETHOOK_COLOR;
                    std::string chat_msg = "\x01 " + std::string(pi_game.name) + " is marked as \x08" + BRACKET_COLOR.toHexStr() + "[\x08" + col.toHexStr() + "Nethook User\x08" + BRACKET_COLOR.toHexStr() + "]";
                    if (CFG::Logs_Enable) {
                        bool to_chat = (CFG::Logs_Type & 1);
                        bool to_console = (CFG::Logs_Type & 2);
                        bool to_screen = (CFG::Logs_Type & 4);

                        if (to_chat) {
                            I::ClientModeShared->m_pChatElement->ChatPrintf(0, chat_msg.c_str());
                        }

                        if (to_console) {
                            I::CVar->ConsoleColorPrintf(col, "%s\n", msg.c_str());
                        }

                        if (to_screen) {
                            gNotify.add_notification(msg, 5000);
                        }
                    }
                }
            }
        }
    }
}