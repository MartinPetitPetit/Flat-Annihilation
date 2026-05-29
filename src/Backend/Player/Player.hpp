/*
 * Backend/Player/Player.hpp
 *
 * Rôle du fichier :
 * Declares the Player class, resources, building ownership, and building management functions.
 *
 * Notes de lecture :
 * Ce fichier appartient au module Player. Il gère les ressources, les bâtiments et les actions propres à un joueur.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../Building/Building.hpp"
#include "../Entity/Entity.hpp"
#include "../Map/Map.hpp"
#include "../Resource/Resource.hpp"
#include "../Building/Building.hpp"

using MAP = std::vector<std::vector<class Cell>>;

class Player
{
public:
    Player();
    Player(int i);
    virtual ~Player();

    int getId() const;
    const std::string getName() const;

    int  getWood() const;
    void addWood(int amount);
    bool spendWood(int amount);

    int  getFood() const;
    void addFood(int amount);
    bool spendFood(int amount);

    bool placeBuilding(BuildingType type, int mapX, int mapY, MAP& map);

    /*
     * Création gratuite utilisée pour l'état initial du jeu.
     * Exemple : base de départ de l'IA.
     */
    bool createBuildingFree(BuildingType type, int mapX, int mapY, MAP& map);

    Building* getBuilding(int buildingID) const;
    const std::vector<std::unique_ptr<Building>>& getBuildings() const;

    /*
     * Supprime les bâtiments détruits et libère leurs cellules sur la carte.
     */
    bool removeDeadBuildings(MAP& map);

protected:
    int         id { 0 };
    std::string name;
    std::vector<Entity*> entities;

    int wood { 300 };
    int food { 1000 };

    std::vector<std::unique_ptr<Building>> buildings;
    int nextBuildingID { 0 };
};