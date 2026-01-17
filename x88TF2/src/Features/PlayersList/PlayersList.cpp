// PlayersList.cpp - Fixed implementation with correct hash function
#include "PlayersList.h"
#include "../src/SDK/SDK.h"
#include <fstream>
#include <sstream>
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
	m_Players.clear();
	if (m_LogPath.empty())
		return;
	std::ifstream file(m_LogPath);
	if (!file.is_open())
		return;
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
	}
	file.close();
}
// Save players to file
void CPlayersList::Save()
{
	if (m_LogPath.empty())
		return;
	std::ofstream file(m_LogPath);
	if (!file.is_open())
		return;
	file << "# PlayersList - Format: SteamID Flags\n";
	file << "# Flags: I=Ignored, C=Cheater, R=RetardLegit, L=CheaterLight, J=RijinUser, M=LmaoboxUser, S=Suspect, N=NethookUser\n\n";
	for (const auto& [hash, priority] : m_Players)
	{
		// Get SteamID from hash (we need to find it in game)
		std::string steamid;
		for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++)
		{
			player_info_t pi{};
			if (I::EngineClient->GetPlayerInfo(i, &pi))
			{
				if (HashString(pi.guid) == hash)
				{
					steamid = pi.guid;
					break;
				}
			}
		}
		if (steamid.empty())
			continue;
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
}
// Mark a player with priority flags
void CPlayersList::Mark(int entindex, const PlayerPriority& info)
{
	player_info_t pi{};
	if (!I::EngineClient->GetPlayerInfo(entindex, &pi))
		return;
	uint64_t hash = HashString(pi.guid);
	m_Players[hash] = info;
	Save();
}
// Get player priority info by entindex
bool CPlayersList::GetInfo(int entindex, PlayerPriority& out)
{
	player_info_t pi{};
	if (!I::EngineClient->GetPlayerInfo(entindex, &pi))
		return false;
	uint64_t hash = HashString(pi.guid);
	auto it = m_Players.find(hash);
	if (it != m_Players.end())
	{
		out = it->second;
		return true;
	}
	return false;
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