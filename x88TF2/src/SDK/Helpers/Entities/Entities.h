// Entities.h
#pragma once
#include "../../TF2/c_tf_player.h"
#include <map>
#include <vector>
#include <string>

enum class EEntGroup
{
    PLAYERS_ALL,
    PLAYERS_ENEMIES,
    PLAYERS_TEAMMATES,
    PLAYERS_OBSERVER,
    BUILDINGS_ALL,
    BUILDINGS_ENEMIES,
    BUILDINGS_TEAMMATES,
    PROJECTILES_ALL,
    PROJECTILES_ENEMIES,
    PROJECTILES_TEAMMATES,
    PROJECTILES_LOCAL_STICKIES,
    HEALTHPACKS,
    AMMOPACKS,
    HALLOWEEN_GIFT,
    MVM_MONEY,
    OBJECTIVES  // Added for capture flags, control points, payload carts
};

class CEntityHelper
{
public:
    C_TFPlayer* GetLocal();
    C_TFWeaponBase* GetWeapon();

    // Movido para public: Acessível externamente (ex: ESP.cpp), como no Amalgam
    bool IsHealthPack(C_BaseEntity* pEntity);
    bool IsAmmoPack(C_BaseEntity* pEntity);

private:
    std::map<EEntGroup, std::vector<C_BaseEntity*>> m_mapGroups = {};
    std::map<int, bool> m_mapHealthPacks = {};
    std::map<int, bool> m_mapAmmoPacks = {};
    bool m_bModelIndexesUpdated = false;  // Otimização: Flag para update lazy (Amalgam-style)

public:
    void UpdateCache();
    void UpdateModelIndexes();  // Chama apenas se !m_bModelIndexesUpdated
    void ClearCache();
    void ClearModelIndexes()
    {
        m_mapHealthPacks.clear();
        m_mapAmmoPacks.clear();
        m_bModelIndexesUpdated = false;
    }

    const std::vector<C_BaseEntity*>& GetGroup(const EEntGroup group) { return m_mapGroups[group]; }

    // Adicionado: Filtrar objectives por team (opcional, como no Amalgam)
    void UpdateObjectivesTeam(int localTeam);
};

MAKE_SINGLETON_SCOPED(CEntityHelper, Entities, H);