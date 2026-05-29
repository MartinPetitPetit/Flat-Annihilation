/*
 * Backend/Pathing/MassPath.cpp
 *
 * Rôle du fichier :
 * Stores simple group movement plans and exposes synchronization helpers for units following mass movement orders.
 *
 * Notes de lecture :
 * Ce fichier appartient au module Pathing. Il regroupe les règles de déplacement, le calcul de chemin et les plans de déplacement de groupe.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#include "MassPath.hpp"

#include <unordered_map>

namespace MassPath
{
    struct UnitPlan
    {
        Coordinate target {0, 0};
    };

    static std::unordered_map<const Unit*, UnitPlan> plans;

    void requestGroupMove(
        const MAP& map,
        const std::vector<Unit*>& selectedUnits,
        Coordinate clickedGoal
    )
    {
        (void)map;

        for (Unit* unit : selectedUnits) {
            if (!unit) {
                continue;
            }

            plans[unit] = UnitPlan{clickedGoal};
        }
    }

    void processPathSearch(const MAP& map, int nodeBudget)
    {
        (void)map;
        (void)nodeBudget;
    }

    void processRepathRequests(const MAP& map)
    {
        (void)map;
    }

    bool consumeFailedMove(std::vector<Unit*>& outUnits)
    {
        outUnits.clear();
        return false;
    }

    bool hasActiveSearch()
    {
        return false;
    }

    bool hasPlan(const Unit* unit)
    {
        return plans.find(unit) != plans.end();
    }

    bool syncPlanWithUnit(
        const MAP& map,
        Unit* unit,
        Coordinate currentPosition,
        Coordinate& outTarget
    )
    {
        (void)map;

        auto it = plans.find(unit);

        if (it == plans.end()) {
            return false;
        }

        outTarget = it->second.target;

        if (currentPosition.getX() == outTarget.getX() &&
            currentPosition.getY() == outTarget.getY()) {
            plans.erase(it);
        return false;
            }

            return true;
    }

    Coordinate getCurrentTarget(const Unit* unit)
    {
        auto it = plans.find(unit);

        if (it == plans.end()) {
            return Coordinate(0, 0);
        }

        return it->second.target;
    }

    void reportUnitBlocked(Unit* unit)
    {
        (void)unit;
    }

    void reportUnitMoved(Unit* unit)
    {
        (void)unit;
    }

    void requestRepathForUnit(Unit* unit)
    {
        (void)unit;
    }

    void clearPlan(const Unit* unit)
    {
        plans.erase(unit);
    }

    bool isBlockedForMassPath(const MAP& map, int x, int y)
    {
        if (!in_map(map, x, y)) {
            return true;
        }

        const Cell& cell = map[x][y];

        if (!cell.walkable) {
            return true;
        }

        if (cell.type_terrain == Montain ||
            cell.type_terrain == Lake ||
            cell.type_terrain == River ||
            cell.type_terrain == ravine) {
            return true;
            }

            if (cell.buildingID != -1) {
                return true;
            }

            return false;
    }

    bool hasLineOfSight(const MAP& map, Coordinate a, Coordinate b)
    {
        (void)map;
        (void)a;
        (void)b;

        return true;
    }
}
