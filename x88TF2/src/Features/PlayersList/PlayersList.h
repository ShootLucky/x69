// PlayersList.h - Complete version with automatic F2P detection
#pragma once
#include "../src/SDK/SDK.h"
#include <unordered_map>
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
	struct Player
	{
		hash::hash_t SteamID = {};
		PlayerPriority Info = {};
	};
	std::unordered_map<hash::hash_t, PlayerPriority> m_Players;
	std::filesystem::path m_LogPath;
public:
	// Initialization and persistence
	void Parse();
	void Save();
	// Player priority management (manual flags)
	void Mark(int entindex, const PlayerPriority& info);
	void SetInfo(int entindex, const PlayerPriority& info) { Mark(entindex, info); }
	bool GetInfo(int entindex, PlayerPriority& out);
	bool GetInfoGUID(const std::string& guid, PlayerPriority& out);
	// Utility
	void Clear() { m_Players.clear(); }
	size_t GetPlayerCount() const { return m_Players.size(); }
	// Path management
	void SetLogPath(const std::filesystem::path& path) { m_LogPath = path; }
	std::filesystem::path GetLogPath() const { return m_LogPath; }
};
MAKE_SINGLETON_SCOPED(CPlayersList, Players, F);