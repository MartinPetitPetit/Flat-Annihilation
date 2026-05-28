#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "../Map/Map.hpp"
#include "../Entity/Entity.hpp"
#include "../Resource/Resource.hpp"
#include "../Building/Building.hpp"

// Forward declaration pour éviter l'include circulaire Map->Cell->Building
using MAP = std::vector<std::vector<class Cell>>;

class Player
{
public:
    Player();
    Player(int i);
    virtual ~Player();

    const std::string getName() const;

    // Ressources
    int  getWood()  const;
    void addWood(int amount);
    bool spendWood(int amount);
    int  getFood()  const;
    void addFood(int amount);
    bool spendFood(int amount);

    // Bâtiments
    bool      placeBuilding(BuildingType type, int mapX, int mapY, MAP& map);
    Building* getBuilding(int buildingID) const;
    const std::vector<std::unique_ptr<Building>>& getBuildings() const;

protected:
    int         id;
    std::string name;
    std::vector<Entity*> entities;

    int wood { 300 };
    int food { 1000 };

    std::vector<std::unique_ptr<Building>> buildings;
    int nextBuildingID { 0 };
};