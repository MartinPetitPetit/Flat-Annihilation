#include "Player.hpp"

namespace
{
    bool canPlaceBuildingCells(BuildingType type, int mapX, int mapY, const MAP& map)
    {
        const BuildingDef& def = getBuildingDef(type);

        for (int dx = 0; dx < def.sizeX; dx++) {
            for (int dy = 0; dy < def.sizeY; dy++) {
                int x = mapX + dx;
                int y = mapY + dy;

                if (x < 0 || x >= static_cast<int>(map.size())) {
                    return false;
                }

                if (map.empty() || y < 0 || y >= static_cast<int>(map[0].size())) {
                    return false;
                }

                if (map[x][y].type_terrain != Plain) {
                    return false;
                }

                if (map[x][y].buildingID != -1) {
                    return false;
                }

                if (map[x][y].resource != nullptr) {
                    return false;
                }

                if (map[x][y].unit != nullptr) {
                    return false;
                }
            }
        }

        return true;
    }

    void markBuildingCells(Building* building, MAP& map)
    {
        if (building == nullptr) {
            return;
        }

        const BuildingDef& def = getBuildingDef(building->getType());

        for (int dx = 0; dx < def.sizeX; dx++) {
            for (int dy = 0; dy < def.sizeY; dy++) {
                int x = building->getMapX() + dx;
                int y = building->getMapY() + dy;

                map[x][y].buildingID    = building->getId();
                map[x][y].buildingOwner = building->getTeam();
                map[x][y].walkable      = false;
            }
        }
    }
}

Player::Player()
{
    std::cout << "nom du joueur : ? ";
    std::cin >> this->name;
    this->id = 0;
}

Player::Player(int i)
{
    name = "IA" + std::to_string(i);
    id = i + 1;
}

Player::~Player()
{
    std::cout << "destruction du joueur " << name << "\n";
}

int Player::getId() const
{
    return id;
}

const std::string Player::getName() const
{
    return name;
}

int Player::getWood() const
{
    return wood;
}

void Player::addWood(int amount)
{
    wood += amount;
}

bool Player::spendWood(int amount)
{
    if (wood < amount) {
        return false;
    }

    wood -= amount;
    return true;
}

int Player::getFood() const
{
    return food;
}

void Player::addFood(int amount)
{
    food += amount;
}

bool Player::spendFood(int amount)
{
    if (food < amount) {
        return false;
    }

    food -= amount;
    return true;
}

bool Player::placeBuilding(BuildingType type, int mapX, int mapY, MAP& map)
{
    const BuildingDef& def = getBuildingDef(type);

    if (!canPlaceBuildingCells(type, mapX, mapY, map)) {
        return false;
    }

    if (!spendWood(def.costWood)) {
        return false;
    }

    int bid = nextBuildingID++;

    buildings.push_back(
        std::make_unique<Building>(bid, type, id, mapX, mapY)
    );

    markBuildingCells(buildings.back().get(), map);

    return true;
}

bool Player::createBuildingFree(BuildingType type, int mapX, int mapY, MAP& map)
{
    if (!canPlaceBuildingCells(type, mapX, mapY, map)) {
        return false;
    }

    int bid = nextBuildingID++;

    buildings.push_back(
        std::make_unique<Building>(bid, type, id, mapX, mapY)
    );

    markBuildingCells(buildings.back().get(), map);

    return true;
}

Building* Player::getBuilding(int buildingID) const
{
    for (const auto& b : buildings) {
        if (b && b->getId() == buildingID) {
            return b.get();
        }
    }

    return nullptr;
}

const std::vector<std::unique_ptr<Building>>& Player::getBuildings() const
{
    return buildings;
}


bool Player::removeDeadBuildings(MAP& map)
{
    bool removed = false;

    for (auto it = buildings.begin(); it != buildings.end(); ) {
        Building* building = it->get();

        if (building == nullptr || building->isAlive()) {
            ++it;
            continue;
        }

        const BuildingDef& def = getBuildingDef(building->getType());

        for (int dx = 0; dx < def.sizeX; dx++) {
            for (int dy = 0; dy < def.sizeY; dy++) {
                int x = building->getMapX() + dx;
                int y = building->getMapY() + dy;

                if (!in_map(map, x, y)) {
                    continue;
                }

                if (map[x][y].buildingID == building->getId() &&
                    map[x][y].buildingOwner == building->getTeam()) {
                    map[x][y].buildingID = -1;
                    map[x][y].buildingOwner = -1;
                    map[x][y].walkable = true;
                }
            }
        }

        it = buildings.erase(it);
        removed = true;
    }

    return removed;
}
