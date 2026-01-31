#pragma once

#include "../src/App/App.h"
#include "../../Utils/fnv1a.h"
#include "../src/SDK/TF2/tf_shareddefs.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <array>

namespace attributes
{
	constexpr uint16_t paintkit_proto_def_index = 834;
	constexpr uint16_t custom_paintkit_seed_lo = 866;
	constexpr uint16_t custom_paintkit_seed_hi = 867;
	constexpr uint16_t has_team_color_paintkit = 745;
	constexpr uint16_t set_item_texture_wear = 725;
	constexpr uint16_t weapon_allow_inspect = 731;
	constexpr uint16_t set_attached_particle_static = 370;
	constexpr uint16_t set_attached_particle = 134;
	constexpr uint16_t is_festivized = 2053;
	constexpr uint16_t is_australium_item = 2027;
	constexpr uint16_t loot_rarity = 2022;
	constexpr uint16_t item_style_override = 542;
	constexpr uint16_t set_turn_to_gold = 150;
	constexpr uint16_t killstreak_tier = 2025;
	constexpr uint16_t killstreak_effect = 2013;
	constexpr uint16_t killstreak_idleeffect = 2014;
	constexpr uint16_t halloween_pumpkin_explosions = 1007;
	constexpr uint16_t halloween_green_flames = 1008;
	constexpr uint16_t halloween_voice_modulation = 1006;
	constexpr uint16_t add_jingle_to_footsteps = 364;
	constexpr uint16_t set_custom_buildmenu = 295;

#define HashCase(str) case FNV1A::HashConst(#str): return str

	inline uint16_t StringToAttribute(std::string str)
	{
		FNV1A_t hash = FNV1A::Hash(str.c_str());

		switch (hash)
		{
			HashCase(paintkit_proto_def_index);
			HashCase(custom_paintkit_seed_lo);
			HashCase(custom_paintkit_seed_hi);
			HashCase(has_team_color_paintkit);
			HashCase(set_item_texture_wear);
			HashCase(weapon_allow_inspect);
			HashCase(set_attached_particle_static);
			HashCase(set_attached_particle);
			HashCase(is_festivized);
			HashCase(is_australium_item);
			HashCase(loot_rarity);
			HashCase(item_style_override);
			HashCase(set_turn_to_gold);
			HashCase(killstreak_tier);
			HashCase(killstreak_effect);
			HashCase(killstreak_idleeffect);
			HashCase(halloween_pumpkin_explosions);
			HashCase(halloween_green_flames);
			HashCase(halloween_voice_modulation);
			HashCase(add_jingle_to_footsteps);
			HashCase(set_custom_buildmenu);
		default:
			return 0;
		}
	}

	// Helper to get attribute name from index
	inline const char* AttributeToString(uint16_t attr)
	{
		switch (attr)
		{
		case paintkit_proto_def_index: return "paintkit_proto_def_index";
		case custom_paintkit_seed_lo: return "custom_paintkit_seed_lo";
		case custom_paintkit_seed_hi: return "custom_paintkit_seed_hi";
		case has_team_color_paintkit: return "has_team_color_paintkit";
		case set_item_texture_wear: return "set_item_texture_wear";
		case weapon_allow_inspect: return "weapon_allow_inspect";
		case set_attached_particle_static: return "set_attached_particle_static";
		case set_attached_particle: return "set_attached_particle";
		case is_festivized: return "is_festivized";
		case is_australium_item: return "is_australium_item";
		case loot_rarity: return "loot_rarity";
		case item_style_override: return "item_style_override";
		case set_turn_to_gold: return "set_turn_to_gold";
		case killstreak_tier: return "killstreak_tier";
		case killstreak_effect: return "killstreak_effect";
		case killstreak_idleeffect: return "killstreak_idleeffect";
		case halloween_pumpkin_explosions: return "halloween_pumpkin_explosions";
		case halloween_green_flames: return "halloween_green_flames";
		case halloween_voice_modulation: return "halloween_voice_modulation";
		case add_jingle_to_footsteps: return "add_jingle_to_footsteps";
		case set_custom_buildmenu: return "set_custom_buildmenu";
		default: return "unknown";
		}
	}
}

namespace weapon_unusual_effects
{
	constexpr int weapon_unusual_hot = 701;
	constexpr int weapon_unusual_isotope = 702;
	constexpr int weapon_unusual_cool = 703;
	constexpr int weapon_unusual_energyorb = 704;
}

// Wear levels for warpaints
namespace wear_levels
{
	constexpr float FACTORY_NEW = 0.0f;
	constexpr float MINIMAL_WEAR = 0.2f;
	constexpr float FIELD_TESTED = 0.5f;
	constexpr float WELL_WORN = 0.7f;
	constexpr float BATTLE_SCARRED = 1.0f;
}

__forceinline float IntToStupidFloat(int desiredValue)
{
	return *reinterpret_cast<float*>(&desiredValue);
}

__forceinline int StupidFloatToInt(float desiredValue)
{
	return *reinterpret_cast<int*>(&desiredValue);
}

struct Attribute
{
	uint16_t attributeIndex;
	float attributeValue;
};

struct SkinInfo
{
	std::vector<Attribute> m_Attributes;

	// Helper function to check if attribute exists
	bool HasAttribute(uint16_t index) const
	{
		for (const auto& attr : m_Attributes)
		{
			if (attr.attributeIndex == index)
				return true;
		}
		return false;
	}

	// Helper function to get attribute value
	float GetAttributeValue(uint16_t index) const
	{
		for (const auto& attr : m_Attributes)
		{
			if (attr.attributeIndex == index)
				return attr.attributeValue;
		}
		return 0.0f;
	}
};

class Weapon;

#define NETVAR(_name, type, table, name) inline type &_name() \
{ \
    static std::size_t offset = NetVars::GetNetVar(table, name); \
    return *reinterpret_cast<type *>(reinterpret_cast<std::uintptr_t>(this) + offset); \
}

#define NETVAR_OFFSET(_name, type, name, offset) inline type &_name() \
{ \
    return *reinterpret_cast<type *>(reinterpret_cast<std::uintptr_t>(this) + offset); \
}

#define MAX_WEAPONS 48
using MyWeapons = std::array<CHandle<Weapon>, MAX_WEAPONS>;

class Player
{
public:
	NETVAR(m_hActiveWeapon, CHandle<Weapon>, "CBaseCombatCharacter", "m_hActiveWeapon");
	NETVAR(m_hMyWeapons, MyWeapons, "CBaseCombatCharacter", "m_hMyWeapons");
};

class Weapon
{
public:
#ifndef _WIN64
	NETVAR_OFFSET(m_iItemDefinitionIndex, int, "m_iItemDefinitionIndex", 2364);
#else
	NETVAR_OFFSET(m_iItemDefinitionIndex, int, "m_iItemDefinitionIndex", 3344);
#endif
};

class SkinChanger
{
	std::unordered_map<int, SkinInfo> m_Skins = {};
	bool m_bForceFullUpdate = false;
	int m_nCurrentWeaponIndex = -1;
	bool m_bInitialSkinLoad = false;
	bool m_bUsePatternScanning = false;

public:
	void RedirectIndex(int& weaponIndex);
	void ApplySkin(Weapon* pWeapon);
	void ApplySkins();
	int GetWeaponIndex() const { return m_nCurrentWeaponIndex; }

	void SetAttribute(int index, std::string attributeStr, float value);
	void RemoveAttribute(int index, std::string attributeStr);

	// New: Apply warpaint with proper settings
	void ApplyWarPaint(int weaponIndex, int warpaintID, float wear = wear_levels::FACTORY_NEW,
		int seedLo = 0, int seedHi = 0, bool teamColor = true);

	// New: Remove all warpaint attributes from a weapon
	void RemoveWarPaint(int weaponIndex);

	// New: Check if weapon has warpaint
	bool HasWarPaint(int weaponIndex);

	inline const SkinInfo& GetSkinInfo(int index)
	{
		if (m_Skins.find(index) == m_Skins.end())
		{
			static SkinInfo empty = {};
			return empty;
		}
		return m_Skins[index];
	}

	// Enable/disable pattern scanning mode
	inline void SetPatternScanningMode(bool enable)
	{
		m_bUsePatternScanning = enable;
	}

	inline bool IsUsingPatternScanning() const
	{
		return m_bUsePatternScanning;
	}

	void Save();
	void Load();
};

inline SkinChanger g_SkinChanger;