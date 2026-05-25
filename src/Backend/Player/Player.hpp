#pragma once
#include <string>
#include <vector>
#include <memory>
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

    // Bâtiments
    bool      placeBuilding(BuildingType type, int mapX, int mapY, MAP& map);
    Building* getBuilding(int buildingID) const;
    const std::vector<std::unique_ptr<Building>>& getBuildings() const;

protected:
    int         id;
    std::string name;
    std::vector<Entity*> entities;

    int wood { 300 };  // ressource de départ

    std::vector<std::unique_ptr<Building>> buildings;
    int nextBuildingID { 0 };
};