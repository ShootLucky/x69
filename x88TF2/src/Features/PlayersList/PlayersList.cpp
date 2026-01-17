// PlayersList.cpp - Fixed implementation with auto file creation
#include "PlayersList.h"
#include "../src/SDK/SDK.h"
#include <fstream>
#include <sstream>
#include <Windows.h>

// Simple hash function for SteamID strings
static uint64_t HashString(const char* str)
{
    uint64_t hash = 0xcbf29ce484222325ULL;
    const uint64_t prime = 0x100000001b3ULL;
    while (*str)
    {
        hash ^= static_cast<uint64_t>(*str++);
        hash *= prime;
    }
    return hash;
}

// Parse players from file
void CPlayersList::Parse()
{
    // Initialize file path if not set
    if (m_LogPath.empty())
    {
        // Get the current module (DLL) path
        char dllPath[MAX_PATH];
        GetModuleFileNameA(NULL, dllPath, MAX_PATH);

        // Convert to filesystem path and get parent directory (should be tf2 root)
        std::filesystem::path gamePath = std::filesystem::path(dllPath).parent_path();

        // Create path: [TF2_ROOT]\x69\players.txt
        m_LogPath = gamePath / "x69" / "players.txt";

        // Create directory if it doesn't exist
        auto dir = m_LogPath.parent_path();
        if (!std::filesystem::exists(dir))
        {
            std::filesystem::create_directories(dir);
        }

        // Create file if it doesn't exist
        if (!std::filesystem::exists(m_LogPath))
        {
            std::ofstream file(m_LogPath, std::ios::app);
            if (!file.is_open())
            {
                OutputDebugStringA("[PlayersList] Failed to create file\n");
                return;
            }
            file << "# PlayersList - Format: SteamID Flags\n";
            file << "# Flags: I=Ignored, C=Cheater, R=RetardLegit, L=CheaterLight, J=RijinUser, M=LmaoboxUser, S=Suspect, N=NethookUser\n\n";
            file.close();
            OutputDebugStringA("[PlayersList] File created at: ");
            OutputDebugStringA(m_LogPath.string().c_str());
            OutputDebugStringA("\n");
        }
    }

    // If already parsed, don't parse again
    if (!m_Players.empty())
    {
        return;
    }

    // Open and parse file
    std::ifstream file(m_LogPath);
    if (!file.is_open() || file.peek() == std::ifstream::traits_type::eof())
    {
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string steamid;
        std::string priority_str;

        if (!(iss >> steamid >> priority_str))
            continue;

        uint64_t hash = HashString(steamid.c_str());

        PlayerPriority priority{};
        // Parse priority flags
        if (priority_str.find("I") != std::string::npos) priority.Ignored = true;
        if (priority_str.find("C") != std::string::npos) priority.Cheater = true;
        if (priority_str.find("R") != std::string::npos) priority.RetardLegit = true;
        if (priority_str.find("L") != std::string::npos) priority.CheaterLight = true;
        if (priority_str.find("J") != std::string::npos) priority.RijinUser = true;
        if (priority_str.find("M") != std::string::npos) priority.LmaoboxUser = true;
        if (priority_str.find("S") != std::string::npos) priority.Suspect = true;
        if (priority_str.find("N") != std::string::npos) priority.NethookUser = true;

        m_Players[hash] = priority;
        m_SteamIDCache[hash] = steamid;
    }

    file.close();
}

// Mark a player with priority flags
void CPlayersList::Mark(int entindex, const PlayerPriority& info)
{
    // Don't mark local player
    if (entindex == I::EngineClient->GetLocalPlayer())
    {
        return;
    }

    player_info_t pi{};
    if (!I::EngineClient->GetPlayerInfo(entindex, &pi) || pi.fakeplayer)
    {
        return;
    }

    uint64_t hash = HashString(pi.guid);

    // Check if all flags are false (clearing)
    bool isEmpty = !info.Ignored && !info.Cheater && !info.RetardLegit &&
        !info.CheaterLight && !info.RijinUser && !info.LmaoboxUser &&
        !info.Suspect && !info.NethookUser;

    if (isEmpty)
    {
        // Remove from map if clearing
        m_Players.erase(hash);
        m_SteamIDCache.erase(hash);
    }
    else
    {
        // Add or update
        m_Players[hash] = info;
        m_SteamIDCache[hash] = pi.guid;
    }

    // Save to file
    Save();
}

// Save players to file
void CPlayersList::Save()
{
    // Make sure path is initialized
    if (m_LogPath.empty())
    {
        Parse(); // This will initialize the path
        if (m_LogPath.empty())
        {
            OutputDebugStringA("[PlayersList] Failed to initialize path for saving\n");
            return;
        }
    }

    // Create directory if it doesn't exist
    auto dir = m_LogPath.parent_path();
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directories(dir);
    }

    // Open file for writing (truncate mode)
    std::ofstream file(m_LogPath, std::ios::trunc);
    if (!file.is_open())
    {
        OutputDebugStringA("[PlayersList] Failed to open file for writing\n");
        return;
    }

    // Write header
    file << "# PlayersList - Format: SteamID Flags\n";
    file << "# Flags: I=Ignored, C=Cheater, R=RetardLegit, L=CheaterLight, J=RijinUser, M=LmaoboxUser, S=Suspect, N=NethookUser\n\n";

    // Write all players
    for (const auto& [hash, priority] : m_Players)
    {
        // Get SteamID from cache
        auto cache_it = m_SteamIDCache.find(hash);
        if (cache_it == m_SteamIDCache.end())
        {
            // If not in cache, try to find in current players
            std::string steamid;
            for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++)
            {
                player_info_t pi{};
                if (I::EngineClient->GetPlayerInfo(i, &pi))
                {
                    if (HashString(pi.guid) == hash)
                    {
                        steamid = pi.guid;
                        m_SteamIDCache[hash] = steamid;
                        break;
                    }
                }
            }
            if (steamid.empty())
                continue;
        }

        std::string steamid = m_SteamIDCache[hash];

        // Build flags string
        std::string flags;
        if (priority.Ignored) flags += "I";
        if (priority.Cheater) flags += "C";
        if (priority.RetardLegit) flags += "R";
        if (priority.CheaterLight) flags += "L";
        if (priority.RijinUser) flags += "J";
        if (priority.LmaoboxUser) flags += "M";
        if (priority.Suspect) flags += "S";
        if (priority.NethookUser) flags += "N";

        if (!flags.empty())
            file << steamid << " " << flags << "\n";
    }

    file.close();

    char msg[256];
    sprintf_s(msg, "[PlayersList] Saved %zu players to file\n", m_Players.size());
    OutputDebugStringA(msg);
}

// Get player priority info by entindex
bool CPlayersList::GetInfo(int entindex, PlayerPriority& out)
{
    if (entindex == I::EngineClient->GetLocalPlayer())
    {
        return false;
    }

    player_info_t pi{};
    if (!I::EngineClient->GetPlayerInfo(entindex, &pi) || pi.fakeplayer)
    {
        return false;
    }

    return GetInfoGUID(pi.guid, out);
}

// Get player priority info by GUID
bool CPlayersList::GetInfoGUID(const std::string& guid, PlayerPriority& out)
{
    uint64_t hash = HashString(guid.c_str());
    auto it = m_Players.find(hash);
    if (it != m_Players.end())
    {
        out = it->second;
        return true;
    }

    return false;
}