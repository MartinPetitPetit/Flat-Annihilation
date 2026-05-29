#pragma once

#include "../Map/Map.hpp"
#include "../Coordinate/Coordinate.hpp"

#include <cstddef>
#include <vector>

class Unit;

namespace AIPath
{
    enum class GoalMode
    {
        ExactCell,
        AdjacentToGoal
    };

    struct PathResult
    {
        bool success { false };
        std::vector<Coordinate> nodes;
    };

    PathResult findPath(
        const MAP& map,
        Coordinate start,
        Coordinate goal,
        const Unit* self,
        GoalMode goalMode = GoalMode::ExactCell,
        int maxVisitedNodes = 20000
    );

    std::vector<Coordinate> findPathOrEmpty(
        const MAP& map,
        Coordinate start,
        Coordinate goal,
        const Unit* self,
        GoalMode goalMode = GoalMode::ExactCell,
        int maxVisitedNodes = 20000
    );

    bool moveOneStepAlongPath(
        MAP& map,
        Unit* unit,
        std::vector<Coordinate>& path,
        std::size_t& pathIndex,
        int& ticksWaited,
        int tickRate,
        int movesPerSecond = 5
    );
}
