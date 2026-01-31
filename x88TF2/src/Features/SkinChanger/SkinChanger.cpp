#include "SkinChanger.h"

#include <array>
#include <format>
#include <fstream>
#include <iostream>
#include "../include/nlohmann/json.hpp"
#include "../src/SDK/TF2/tf_shareddefs.h"

#ifndef _WIN64
SIGNATURE(GetItemSchema, "client.dll", "E8 ? ? ? ? 83 C0 ? C3 CC");
SIGNATURE(CEconItemSchema_GetAttributeDefinition, "client.dll", "55 8B EC 83 EC ? 53 56 8B D9 8D 4D ? 57 E8 ? ? ? ? 8B 45");
SIGNATURE(CAttributeList_SetRuntimeAttributeValue, "client.dll", "55 8B EC 83 EC ? 33 C0 53 8B D9 56 57 8B 7D");
#else
MAKE_SIGNATURE(GetItemSchema, "client.dll", "48 83 EC ? E8 ? ? ? ? 48 83 C0 ? 48 83 C4 ? C3 CC CC CC", 0);
MAKE_SIGNATURE(CEconItemSchema_GetAttributeDefinition, "client.dll", "89 54 24 ? 53 48 83 EC ? 48 8B D9 48 8D 54 24 ? 48 81 C1 ? ? ? ? E8 ? ? ? ? 8B D0 3B 83 ? ? ? ? 73 ? 8B 83 ? ? ? ? 83 F8 ? 74 ? 3B D0 7F ? 48 81 C3 ? ? ? ? 44 8B C2 83 FA ? 74 ? 48 8B 03 8B CA", 0);
MAKE_SIGNATURE(CAttributeList_SetRuntimeAttributeValue, "client.dll", "48 89 5C 24 10 55 56 57 48 8B EC 48 83 EC 50 44", 0);
#endif

class CEconItemAttribute
{
public:
	void* pad = 0;
#ifndef _WIN64
	uint16_t m_iAttributeDefinitionIndex;
#else
	unsigned int m_iAttributeDefinitionIndex;
#endif
	union
	{
		int m_iRawValue32;
		float m_flValue;
	};
	int m_nRefundableCurrency = 0;

	inline CEconItemAttribute(uint16_t iAttributeDefinitionIndex, float flValue)
	{
		m_iAttributeDefinitionIndex = iAttributeDefinitionIndex;
		m_flValue = flValue;
	}
};

class CAttributeList
{
public:
	void* pad;
	CUtlVector<CEconItemAttribute, CUtlMemory<CEconItemAttribute>> m_Attributes;
	void* m_pManager;

	inline void AddAttribute(int iIndex, float flValue)
	{
		if (m_Attributes.Count() > 14)
			return;

		CEconItemAttribute attr(iIndex, flValue);
		m_Attributes.AddToTail(attr);
	}

#ifndef _WIN64
	using GetItemSchemaFN = void* (__cdecl*)();
	using GetAttributeDefinitionFN = void* (__thiscall*)(void*, int);
	using SetRuntimeAttributeValueFN = void(__thiscall*)(CAttributeList*, void*, float);
#else
	using GetItemSchemaFN = void* (__fastcall*)();
	using GetAttributeDefinitionFN = void* (__fastcall*)(void*, int);
	using SetRuntimeAttributeValueFN = void(__fastcall*)(CAttributeList*, void*, float);
#endif

	void SetAttribute(int index, float value)
	{
		auto schema = reinterpret_cast<GetItemSchemaFN>(Signatures::GetItemSchema.Get())();

		auto attributeDefinition = reinterpret_cast<GetAttributeDefinitionFN>(Signatures::CEconItemSchema_GetAttributeDefinition.Get())(schema, index);
		if (!attributeDefinition)
			return;

		reinterpret_cast<SetRuntimeAttributeValueFN>(Signatures::CAttributeList_SetRuntimeAttributeValue.Get())(this, attributeDefinition, value);
	}
};

#define Redirect(from, to) case from: { nWeaponIndex = to; break; }

void SkinChanger::RedirectIndex(int& nWeaponIndex)
{
	switch (nWeaponIndex)
	{
		Redirect(Soldier_m_RocketLauncher, Soldier_m_RocketLauncherR);
		Redirect(Scout_m_Scattergun, Scout_m_ScattergunR);
		Redirect(Pyro_m_FlameThrower, Pyro_m_FlameThrowerR);
		Redirect(Demoman_m_GrenadeLauncher, Demoman_m_GrenadeLauncherR);
		Redirect(Demoman_s_StickybombLauncher, Demoman_s_StickybombLauncherR);
		Redirect(Heavy_m_Minigun, Heavy_m_MinigunR);
		Redirect(Engi_t_Wrench, Engi_t_WrenchR);
		Redirect(Medic_s_MediGun, Medic_s_MediGunR);
		Redirect(Sniper_m_SniperRifle, Sniper_m_SniperRifleR);
		Redirect(Sniper_s_SMG, Sniper_s_SMGR);
		Redirect(Spy_t_Knife, Spy_t_KnifeR);
		Redirect(Spy_m_Revolver, Spy_m_RevolverR);
		Redirect(Engi_s_EngineersPistol, Engi_s_PistolR);
		Redirect(Soldier_s_SoldiersShotgun, Soldier_s_ShotgunR);
		Redirect(Pyro_s_PyrosShotgun, Pyro_s_ShotgunR);
		Redirect(Heavy_s_HeavysShotgun, Heavy_s_ShotgunR);
		Redirect(Engi_m_EngineersShotgun, Engi_m_ShotgunR);
		Redirect(Scout_t_Bat, Scout_t_BatR);
		Redirect(Soldier_t_Shovel, Soldier_t_ShovelR);
		Redirect(Pyro_t_FireAxe, Pyro_t_FireAxeR);
		Redirect(Demoman_t_Bottle, Demoman_t_BottleR);
		Redirect(Medic_t_Bonesaw, Medic_t_BonesawR);
		Redirect(Sniper_t_Kukri, Sniper_t_KukriR);
	default: break;
	}
}

void SkinChanger::ApplySkin(Weapon* pWeapon)
{
	if (!pWeapon)
		return;

	int& nWeaponIndex = pWeapon->m_iItemDefinitionIndex();
	RedirectIndex(nWeaponIndex);

#ifndef _WIN64
	auto attributeList = reinterpret_cast<CAttributeList*>(reinterpret_cast<std::uintptr_t>(pWeapon) + 0x9C4);
#else
	auto attributeList = reinterpret_cast<CAttributeList*>(reinterpret_cast<std::uintptr_t>(pWeapon) + 3512);
#endif

	if (!attributeList)
		return;

#ifdef _DEBUG
	if (attributeList->m_Attributes.Count() > 0 && m_Skins.find(nWeaponIndex) == m_Skins.end())
	{
		// This weapon seems to already have a skin applied, but we don't have it in our map
		// Let's print out what attributes it has
		std::cout << "Weapon that needs to have different pre-filled attributes: " << nWeaponIndex << std::endl;

		for (const auto& attribute : attributeList->m_Attributes)
		{
			std::cout << "Attribute: " << attribute.m_iAttributeDefinitionIndex << " Value: " << attribute.m_flValue << std::endl;
		}
	}
#endif

	auto PreFilledAttributeCount = [&](int index) -> int
		{
			// Most weapons have no attributes, some have more than one.
			// Seems all snipers have this "no_jump" attribute
			switch (index)
			{
			case Sniper_m_TheBazaarBargain:
			case Sniper_m_SniperRifle:
			case Sniper_m_SniperRifleR:
				return 1;

			default: return 0;
			}
		};

	// If we have attributes, we've already applied all the attributes we want
	if (attributeList->m_Attributes.Count() > PreFilledAttributeCount(m_nCurrentWeaponIndex))
		return;

	// Not a weapon we plan to add attributes to
	if (m_Skins.find(nWeaponIndex) == m_Skins.end())
		return;

	// Apply the attributes if we have requested attributes for it
	const auto& vecAttributes = m_Skins[nWeaponIndex].m_Attributes;
	if (vecAttributes.empty())
		return;

	for (const auto& attribute : vecAttributes)
		attributeList->SetAttribute(attribute.attributeIndex, attribute.attributeValue);
}

void SkinChanger::ApplySkins()
{
	if (!m_bInitialSkinLoad)
	{
		Load();
		m_bInitialSkinLoad = true;
	}

	auto pLocal = (Player*)I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
	if (!pLocal)
		return;

	auto pWeapon = pLocal->m_hActiveWeapon().Get();
	if (m_bForceFullUpdate)
	{
		I::ClientState->ForceFullUpdate();
		m_bForceFullUpdate = false;
	}

	if (!pWeapon)
		return;

	int& nWeaponIndex = pWeapon->m_iItemDefinitionIndex();
	RedirectIndex(nWeaponIndex);

	m_nCurrentWeaponIndex = nWeaponIndex;

	const auto& m_hMyWeapons = pLocal->m_hMyWeapons();
	for (int i = 0; m_hMyWeapons[i].IsValid(); i++)
	{
		auto pWeapon = m_hMyWeapons[i].Get();
		if (!pWeapon)
			continue;

		ApplySkin(pWeapon);
	}
}

void SkinChanger::SetAttribute(int index, std::string attributeStr, float value)
{
	if (index == -1)
		return;

	uint16_t attributeIndex = attributes::StringToAttribute(attributeStr);

	if (attributeIndex == attributes::paintkit_proto_def_index)
		value = IntToStupidFloat(static_cast<int>(value));

	if (m_Skins.find(index) == m_Skins.end())
		m_Skins[index] = SkinInfo();

	// Check if attribute already exists, if so, update it
	bool bFound = false;

	for (auto& attribute : m_Skins[index].m_Attributes)
	{
		if (attribute.attributeIndex == attributeIndex)
		{
			attribute.attributeValue = value;

			bFound = true;
			break;
		}
	}

	if (!bFound)// Attribute doesn't exist, add it
		m_Skins[index].m_Attributes.push_back({ attributeIndex, value });

	m_bForceFullUpdate = true;
}

void SkinChanger::RemoveAttribute(int index, std::string attributeStr)
{
	if (m_Skins.find(index) == m_Skins.end())
		return;

	auto& attributes = m_Skins[index].m_Attributes;

	uint16_t attributeIndex = attributes::StringToAttribute(attributeStr);

	// Find attribute
	for (auto it = attributes.begin(); it != attributes.end(); ++it)
	{
		if (it->attributeIndex == attributeIndex)
		{
			attributes.erase(it);
			m_bForceFullUpdate = true;

			return;
		}
	}
}

void SkinChanger::ApplyWarPaint(int weaponIndex, int warpaintID, float wear, int seedLo, int seedHi, bool teamColor)
{
	if (weaponIndex == -1)
		return;

	// Remove any existing warpaint attributes first
	RemoveWarPaint(weaponIndex);

	// Apply the warpaint ID (as stupid float)
	SetAttribute(weaponIndex, "paintkit_proto_def_index", static_cast<float>(warpaintID));

	// Apply wear level (0.0 = Factory New, 1.0 = Battle Scarred)
	SetAttribute(weaponIndex, "set_item_texture_wear", wear);

	// Apply seed values for pattern variation
	if (seedLo != 0)
		SetAttribute(weaponIndex, "custom_paintkit_seed_lo", static_cast<float>(seedLo));

	if (seedHi != 0)
		SetAttribute(weaponIndex, "custom_paintkit_seed_hi", static_cast<float>(seedHi));

	// Apply team color if needed
	if (teamColor)
		SetAttribute(weaponIndex, "has_team_color_paintkit", 1.0f);

	// Allow inspect
	SetAttribute(weaponIndex, "weapon_allow_inspect", 1.0f);

	m_bForceFullUpdate = true;

#ifdef _DEBUG
	std::cout << "[SkinChanger] Applied warpaint ID " << warpaintID << " to weapon " << weaponIndex << std::endl;
	std::cout << "  Wear: " << wear << ", SeedLo: " << seedLo << ", SeedHi: " << seedHi << ", TeamColor: " << teamColor << std::endl;
#endif
}

void SkinChanger::RemoveWarPaint(int weaponIndex)
{
	if (weaponIndex == -1 || m_Skins.find(weaponIndex) == m_Skins.end())
		return;

	// Remove all warpaint-related attributes
	RemoveAttribute(weaponIndex, "paintkit_proto_def_index");
	RemoveAttribute(weaponIndex, "set_item_texture_wear");
	RemoveAttribute(weaponIndex, "custom_paintkit_seed_lo");
	RemoveAttribute(weaponIndex, "custom_paintkit_seed_hi");
	RemoveAttribute(weaponIndex, "has_team_color_paintkit");
	RemoveAttribute(weaponIndex, "weapon_allow_inspect");

	m_bForceFullUpdate = true;

#ifdef _DEBUG
	std::cout << "[SkinChanger] Removed warpaint from weapon " << weaponIndex << std::endl;
#endif
}

bool SkinChanger::HasWarPaint(int weaponIndex)
{
	if (weaponIndex == -1 || m_Skins.find(weaponIndex) == m_Skins.end())
		return false;

	return m_Skins[weaponIndex].HasAttribute(attributes::paintkit_proto_def_index);
}

void SkinChanger::Save()
{
	std::ofstream file("skins.json");
	if (!file.good())
		return;

	nlohmann::json j;

	for (const auto& skin : m_Skins)
	{
		int index = skin.first;
		if (index == -1)
			continue;

		const auto& vecAttributes = skin.second.m_Attributes;
		std::string strIndex = std::to_string(index);

		for (const auto& attribute : vecAttributes)
		{
			// Save attribute name for readability
			std::string attrName = attributes::AttributeToString(attribute.attributeIndex);

			// Special handling for paintkit_proto_def_index (save as int, not float)
			if (attribute.attributeIndex == attributes::paintkit_proto_def_index)
			{
				int paintID = StupidFloatToInt(attribute.attributeValue);
				j[strIndex][attrName] = paintID;
			}
			else
			{
				j[strIndex][attrName] = attribute.attributeValue;
			}
		}
	}

	file << j.dump(4);

	file.close();

#ifdef _DEBUG
	std::cout << "[SkinChanger] Saved " << m_Skins.size() << " weapon configurations" << std::endl;
#endif
}

void SkinChanger::Load()
{
	std::ifstream file("skins.json");
	if (!file.good())
		return;

	m_Skins.clear();

	nlohmann::json j;

	try
	{
		j = nlohmann::json::parse(file);
	}
	catch (const std::exception& e)
	{
#ifdef _DEBUG
		std::cout << "[SkinChanger] Failed to parse skins.json: " << e.what() << std::endl;
#endif
		file.close();
		return;
	}

	for (auto it = j.begin(); it != j.end(); ++it)
	{
		int index = std::stoi(it.key());
		if (index == -1)
			continue;

		const auto& vecAttributes = it.value();

		for (auto it2 = vecAttributes.begin(); it2 != vecAttributes.end(); ++it2)
		{
			std::string attrName = it2.key();
			uint16_t attributeIndex = attributes::StringToAttribute(attrName);

			if (attributeIndex == 0)
			{
				// Try parsing as numeric index for backwards compatibility
				try
				{
					attributeIndex = static_cast<uint16_t>(std::stoi(attrName));
				}
				catch (...)
				{
					continue;
				}
			}

			float attributeValue;

			// Special handling for paintkit_proto_def_index
			if (attributeIndex == attributes::paintkit_proto_def_index)
			{
				int paintID = it2.value();
				attributeValue = IntToStupidFloat(paintID);
			}
			else
			{
				attributeValue = it2.value();
			}

			m_Skins[index].m_Attributes.push_back({ attributeIndex, attributeValue });
		}
	}

	file.close();

#ifdef _DEBUG
	std::cout << "[SkinChanger] Loaded " << m_Skins.size() << " weapon configurations" << std::endl;
#endif
}