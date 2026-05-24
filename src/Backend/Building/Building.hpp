#pragma once
#include <string>

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

class Building {
public:
    Building(int id, BuildingType type, int ownerID, int mapX, int mapY);
    ~Building() = default;

    BuildingType getType()    const;
    int          getID()      const;
    int          getOwnerID() const;
    int          getMapX()    const;
    int          getMapY()    const;
    int          getHP()      const;
    bool         isAlive()    const;

    void takeDamage(int amount);

private:
    int          id;
    BuildingType type;
    int          ownerID;
    int          mapX;
    int          mapY;
    int          hp;
};