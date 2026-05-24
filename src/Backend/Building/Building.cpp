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
    : id(id), type(type), ownerID(ownerID), mapX(mapX), mapY(mapY)
{
    hp = getBuildingDef(type).maxHP;
}

BuildingType Building::getType()    const { return type;    }
int          Building::getID()      const { return id;      }
int          Building::getOwnerID() const { return ownerID; }
int          Building::getMapX()    const { return mapX;    }
int          Building::getMapY()    const { return mapY;    }
int          Building::getHP()      const { return hp;      }
bool         Building::isAlive()    const { return hp > 0;  }

void Building::takeDamage(int amount)
{
    hp -= amount;
    if (hp < 0) hp = 0;
}