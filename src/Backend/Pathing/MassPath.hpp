#pragma once

#include "../Coordinate/Coordinate.hpp"
#include "../Map/Map.hpp"

#include <vector>

class Unit;

namespace MassPath
{
    void requestGroupMove(
        const MAP& map,
        const std::vector<Unit*>& selectedUnits,
        Coordinate clickedGoal
    );

    void processPathSearch(const MAP& map, int nodeBudget = 0);

    void processRepathRequests(const MAP& map);

    bool consumeFailedMove(std::vector<Unit*>& outUnits);

    bool hasActiveSearch();

    bool hasPlan(const Unit* unit);

    bool syncPlanWithUnit(
        const MAP& map,
        Unit* unit,
        Coordinate currentPosition,
        Coordinate& outTarget
    );

    Coordinate getCurrentTarget(const Unit* unit);

    void reportUnitBlocked(Unit* unit);

    void reportUnitMoved(Unit* unit);

    void requestRepathForUnit(Unit* unit);

    void clearPlan(const Unit* unit);

    bool isBlockedForMassPath(const MAP& map, int x, int y);

    bool hasLineOfSight(const MAP& map, Coordinate a, Coordinate b);
}
