/*
 * Backend/Building/Building.hpp
 *
 * Rôle du fichier :
 * Declares building types, building definitions, unit production kinds, and the Building class interface.
 *
 * Notes de lecture :
 * Ce fichier appartient au module Building. Il décrit les bâtiments, leurs tailles, leurs coûts, leurs points de vie et leur production.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#pragma once

#include <string>
#include <vector>

#include "../Entity/Entity.hpp"

enum class BuildingType {
    TownCenter,
    Barracks,
};

enum class UnitKind {
    Soldier,
    Collector,
};

struct BuildingDef {
    BuildingType type;
    std::string  name;
    int          costWood;
    int          sizeX;
    int          sizeY;
    int          maxHP;
};

const BuildingDef& getBuildingDef(BuildingType type);

class Building : public Entity {
public:
    Building(int id, BuildingType type, int ownerID, int mapX, int mapY);
    ~Building() override = default;

    BuildingType getType()    const;
    int          getMapX()    const;
    int          getMapY()    const;

    /*
     * Production.
     * La file garde maintenant le type de l'unité à créer.
     */
    void         tick(float dt);
    bool         queueUnit(UnitKind kind);
    int          getQueueSize() const;
    int          getMaxQueue() const;
    float        getProductionProgress() const;
    bool         hasPendingSpawn() const;
    UnitKind     getPendingSpawnKind() const;
    void         consumeSpawn();

private:
    BuildingType type;

    std::vector<UnitKind> productionQueue;
    int   maxQueue          { 10    };
    float productionTimer   { 0.0f  };
    float productionTime    { 1.0f  };
    bool  pendingSpawn      { false };
    UnitKind pendingSpawnKind { UnitKind::Soldier };
};
