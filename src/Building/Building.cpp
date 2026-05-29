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
    this->maxHealth = getBuildingDef(type).maxHP;
    this->health = maxHealth;
    this->getPosRef().setX(mapX);
    this->getPosRef().setY(mapY);
}

BuildingType Building::getType() const
{
    return type;
}

int Building::getMapX() const
{
    return getPos().getX();
}

int Building::getMapY() const
{
    return getPos().getY();
}

int Building::getQueueSize() const
{
    return static_cast<int>(productionQueue.size());
}

int Building::getMaxQueue() const
{
    return maxQueue;
}

float Building::getProductionProgress() const
{
    if (productionQueue.empty()) {
        return 0.0f;
    }

    return productionTimer / productionTime;
}

bool Building::hasPendingSpawn() const
{
    return pendingSpawn;
}

UnitKind Building::getPendingSpawnKind() const
{
    return pendingSpawnKind;
}

void Building::consumeSpawn()
{
    pendingSpawn = false;
}

bool Building::queueUnit(UnitKind kind)
{
    if (static_cast<int>(productionQueue.size()) >= maxQueue) {
        return false;
    }

    productionQueue.push_back(kind);
    return true;
}

void Building::tick(float dt)
{
    if (pendingSpawn) {
        return;
    }

    if (productionQueue.empty()) {
        productionTimer = 0.0f;
        return;
    }

    productionTimer += dt;

    if (productionTimer >= productionTime) {
        productionTimer = 0.0f;
        pendingSpawnKind = productionQueue.front();
        productionQueue.erase(productionQueue.begin());
        pendingSpawn = true;
    }
}
