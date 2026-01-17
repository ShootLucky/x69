// PlayersList.h - Auto-initializing version
#pragma once
#include "../src/SDK/SDK.h"
#include <unordered_map>
#include <string>
#include <filesystem>

struct PlayerPriority
{
    bool Ignored{};
    bool Cheater{};
    bool RetardLegit{};
    bool CheaterLight{};
    bool RijinUser{};
    bool LmaoboxUser{};
    bool Suspect{};
    bool NethookUser{};
};

class CPlayersList
{
private:
    std::unordered_map<uint64_t, PlayerPriority> m_Players;
    mutable std::unordered_map<uint64_t, std::string> m_SteamIDCache; // mutable para poder modificar em Save()
    std::filesystem::path m_LogPath;

public:
    // Initialization and persistence
    void Parse(); // Auto-initializes path and creates file
    void Save();

    // Player priority management (manual flags)
    void Mark(int entindex, const PlayerPriority& info);
    void SetInfo(int entindex, const PlayerPriority& info) { Mark(entindex, info); }
    bool GetInfo(int entindex, PlayerPriority& out);
    bool GetInfoGUID(const std::string& guid, PlayerPriority& out);

    // Utility
    void Clear()
    {
        m_Players.clear();
        m_SteamIDCache.clear();
        Save(); // Save empty list
    }
    size_t GetPlayerCount() const { return m_Players.size(); }

    // Path management (optional - auto-initialized if not set)
    void SetLogPath(const std::filesystem::path& path) { m_LogPath = path; }
    std::filesystem::path GetLogPath() const { return m_LogPath; }
};

MAKE_SINGLETON_SCOPED(CPlayersList, Players, F);