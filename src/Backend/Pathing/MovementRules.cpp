/*
 * Backend/Pathing/MovementRules.cpp
 *
 * Rôle du fichier :
 * Centralizes terrain, building, unit, and diagonal movement validation rules.
 *
 * Notes de lecture :
 * Ce fichier appartient au module Pathing. Il regroupe les règles de déplacement, le calcul de chemin et les plans de déplacement de groupe.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#include "MovementRules.hpp"

#include "../Unit/Unit.hpp"

namespace MovementRules
{
    bool isBlockedTerrain(TERRAIN terrain)
    {
        return terrain == Montain ||
               terrain == Lake ||
               terrain == River ||
               terrain == ravine;
    }

    bool isFreeCell(
        const MAP& map,
        int x,
        int y,
        const Unit* self
    ) {
        if (!in_map(map, x, y)) {
            return false;
        }

        const Cell& cell = map[x][y];

        if (!cell.walkable) {
            return false;
        }

        if (isBlockedTerrain(cell.type_terrain)) {
            return false;
        }

        if (cell.buildingID != -1) {
            return false;
        }

        if (cell.unit != nullptr && cell.unit != self) {
            return false;
        }

        return true;
    }

    bool canMoveToNeighbour(
        const MAP& map,
        const Unit* self,
        Coordinate current,
        int dx,
        int dy
    ) {
        int nx = current.getX() + dx;
        int ny = current.getY() + dy;

        if (!isFreeCell(map, nx, ny, self)) {
            return false;
        }

        /*
         * Pour les déplacements diagonaux, on évite de traverser un angle fermé.
         * Le passage reste possible si au moins un des deux côtés orthogonaux est libre.
         */
        if (dx != 0 && dy != 0) {
            bool sideA = isFreeCell(
                map,
                current.getX() + dx,
                current.getY(),
                self
            );

            bool sideB = isFreeCell(
                map,
                current.getX(),
                current.getY() + dy,
                self
            );

            if (!sideA && !sideB) {
                return false;
            }
        }

        return true;
    }
}
