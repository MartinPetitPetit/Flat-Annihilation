/*
 * Backend/Pathing/MovementRules.hpp
 *
 * Rôle du fichier :
 * Declares reusable movement validation functions for units and pathfinding.
 *
 * Notes de lecture :
 * Ce fichier appartient au module Pathing. Il regroupe les règles de déplacement, le calcul de chemin et les plans de déplacement de groupe.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#pragma once

#include "../Map/Map.hpp"
#include "../Coordinate/Coordinate.hpp"

class Unit;

namespace MovementRules
{
    bool isBlockedTerrain(TERRAIN terrain);

    bool isFreeCell(
        const MAP& map,
        int x,
        int y,
        const Unit* self
    );

    bool canMoveToNeighbour(
        const MAP& map,
        const Unit* self,
        Coordinate current,
        int dx,
        int dy
    );
}
