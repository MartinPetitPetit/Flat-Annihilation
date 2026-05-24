#pragma once
#include <string>
#include "../Entity/Entity.hpp"

enum class BuildingType {
    TownCenter,
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

private:
    BuildingType type;
};