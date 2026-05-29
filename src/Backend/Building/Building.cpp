#include "Building.hpp"
#include <stdexcept>

static const BuildingDef BUILDING_DEFS[] = {
    { BuildingType::TownCenter, "Town Center", 100, 3, 3, 500 },
    { BuildingType::Barracks,   "Barracks",     80, 3, 3, 300 },
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

BuildingType Building::getType()  const { return type;            }
int          Building::getMapX()  const { return getPos().getX(); }
int          Building::getMapY()  const { return getPos().getY(); }
int          Building::getQueueSize()          const { return productionQueue; }
int          Building::getMaxQueue()           const { return maxQueue;        }
float        Building::getProductionProgress() const
{
    if (productionQueue == 0) return 0.0f;
    return productionTimer / productionTime;
}
bool Building::hasPendingSpawn() const { return pendingSpawn; }
void Building::consumeSpawn()          { pendingSpawn = false; }

bool Building::queueUnit()
{
    if (productionQueue >= maxQueue) return false;
    productionQueue++;
    return true;
}

void Building::tick(float dt)
{
    if (pendingSpawn) return; // attendre que le spawn soit consommé
    if (productionQueue <= 0) return;

    productionTimer += dt;
    if (productionTimer >= productionTime)
    {
        productionTimer = 0.0f;
        productionQueue--;
        pendingSpawn = true;
    }
}