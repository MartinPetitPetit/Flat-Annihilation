
#include "Player.hpp"


Player::Player()
{
    std::cout << "nom du joueur : ? ";
    std::cin >> this->name;
    this->id = 0;
}

Player::Player(int i)
{
    name    = "IA" + std::to_string(i);
    this->id = i + 1;
}

Player::~Player()
{
    std::cout << "destruction du joueur " << name << "\n";
}

const std::string Player::getName() const { return name; }

int  Player::getWood()              const { return wood; }
void Player::addWood(int amount)          { wood += amount; }
bool Player::spendWood(int amount)
{
    if (wood < amount) return false;
    wood -= amount;
    return true;
}

bool Player::placeBuilding(BuildingType type, int mapX, int mapY, MAP map)
{
    const BuildingDef& def = getBuildingDef(type);

    // Vérifier les ressources
    if (!spendWood(def.costWood)) return false;

    // Vérifier que toutes les cellules sont libres
    for (int dx = 0; dx < def.sizeX; dx++) {
        for (int dy = 0; dy < def.sizeY; dy++) {
            int x = mapX + dx;
            int y = mapY + dy;
            if (x < 0 || x >= (int)map.size())             { addWood(def.costWood); return false; }
            if (y < 0 || y >= (int)map[0].size())           { addWood(def.costWood); return false; }
            if (map[x][y].type_terrain != Plain)             { addWood(def.costWood); return false; }
            if (map[x][y].buildingID != -1)                  { addWood(def.costWood); return false; }
            if (map[x][y].resource != nullptr)    { addWood(def.costWood); return false; }
        }
    }

    int bid = nextBuildingID++;
    buildings.push_back(std::make_unique<Building>(bid, type, id, mapX, mapY));

    // Marquer les cellules
    for (int dx = 0; dx < def.sizeX; dx++) {
        for (int dy = 0; dy < def.sizeY; dy++) {
            map[mapX + dx][mapY + dy].buildingID    = bid;
            map[mapX + dx][mapY + dy].buildingOwner = id;
            map[mapX + dx][mapY + dy].walkable      = false;
        }
    }

    return true;
}

Building* Player::getBuilding(int buildingID) const
{
    for (const auto& b : buildings)
        if (b->getTeam() == buildingID) return b.get();
    return nullptr;
}

const std::vector<std::unique_ptr<Building>>& Player::getBuildings() const
{
    return buildings;
}