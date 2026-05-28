#include "Building.hpp"
#include <stdexcept>

static const BuildingDef BUILDING_DEFS[] = {
    { BuildingType::TownCenter, "Town Center", 100, 3, 3, 500 },
};

const BuildingDef& getBuildingDef(BuildingType type)
{
    for (const BuildingDef& def : BUILDING_DEFS) {
        if (def.type == type) return def;
    }
    throw std::runtime_error("BuildingDef not found");
}

Building::Building(int id, BuildingType type, int ownerID, int mapX, int mapY)
    : type(type)
{
    this->id     = id;
    this->team   = ownerID;
    this->health = getBuildingDef(type).maxHP;
    this->getPosRef().setX(mapX);
    this->getPosRef().setY(mapY);
}

BuildingType Building::getType()  const { return type;              }
int          Building::getMapX()  const { return getPos().getX();   }
int          Building::getMapY()  const { return getPos().getY();   }