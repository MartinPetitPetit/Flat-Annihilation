#pragma once
#include <string>
#include "../Entity/Entity.hpp"

enum class BuildingType {
    TownCenter,
    Barracks,
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

    // Production
    void         tick(float dt);
    bool         queueUnit();
    int          getQueueSize()    const;
    int          getMaxQueue()     const;
    float        getProductionProgress() const; // 0.0 -> 1.0
    bool         hasPendingSpawn() const;
    void         consumeSpawn();

private:
    BuildingType type;

    // File de production
    int   productionQueue   { 0     };
    int   maxQueue          { 10    };
    float productionTimer   { 0.0f  };
    float productionTime    { 1.0f  }; // secondes
    bool  pendingSpawn      { false };
};