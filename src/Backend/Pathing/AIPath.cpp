/*
 * Backend/Pathing/AIPath.cpp
 *
 * Rôle du fichier :
 * Implements A* pathfinding with exact or adjacent goal modes and step-by-step movement along computed paths.
 *
 * Notes de lecture :
 * Ce fichier appartient au module Pathing. Il regroupe les règles de déplacement, le calcul de chemin et les plans de déplacement de groupe.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#include "AIPath.hpp"

#include "MovementRules.hpp"
#include "../Unit/Unit.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>

namespace
{
    struct QueueNode
    {
        int index;
        int fCost;
        int hCost;
    };

    struct QueueCompare
    {
        bool operator()(const QueueNode& a, const QueueNode& b) const
        {
            if (a.fCost == b.fCost) {
                return a.hCost > b.hCost;
            }

            return a.fCost > b.fCost;
        }
    };

    struct NodeData
    {
        int gCost { std::numeric_limits<int>::max() };
        int fCost { std::numeric_limits<int>::max() };
        int parent { -1 };
        bool closed { false };
    };

    constexpr std::array<std::array<int, 2>, 8> DIRECTIONS {{
        {{ 1,  0}},
        {{-1,  0}},
        {{ 0,  1}},
        {{ 0, -1}},
        {{ 1,  1}},
        {{ 1, -1}},
        {{-1,  1}},
        {{-1, -1}}
    }};

    int distanceAbs(int value)
    {
        return value < 0 ? -value : value;
    }

    int movementCost(int dx, int dy)
    {
        return (dx != 0 && dy != 0) ? 14 : 10;
    }

    int heuristic(Coordinate a, Coordinate b)
    {
        int dx = distanceAbs(a.getX() - b.getX());
        int dy = distanceAbs(a.getY() - b.getY());

        int diagonal = std::min(dx, dy);
        int straight = std::max(dx, dy) - diagonal;

        return diagonal * 14 + straight * 10;
    }

    bool sameCell(Coordinate a, Coordinate b)
    {
        return a.getX() == b.getX() && a.getY() == b.getY();
    }

    bool isAdjacentToGoal(Coordinate current, Coordinate goal)
    {
        int dx = distanceAbs(current.getX() - goal.getX());
        int dy = distanceAbs(current.getY() - goal.getY());

        return dx <= 1 && dy <= 1 && !(dx == 0 && dy == 0);
    }

    bool reachedGoal(
        Coordinate current,
        Coordinate goal,
        AIPath::GoalMode goalMode
    ) {
        if (goalMode == AIPath::GoalMode::ExactCell) {
            return sameCell(current, goal);
        }

        return isAdjacentToGoal(current, goal);
    }

    int toIndex(int x, int y, int height)
    {
        return x * height + y;
    }

    Coordinate toCoordinate(int index, int height)
    {
        int x = index / height;
        int y = index % height;

        return Coordinate(x, y);
    }

    std::vector<Coordinate> reconstructPath(
        const std::vector<NodeData>& nodes,
        int endIndex,
        int height
    ) {
        std::vector<Coordinate> path;

        int current = endIndex;

        while (current != -1) {
            path.push_back(toCoordinate(current, height));
            current = nodes[current].parent;
        }

        std::reverse(path.begin(), path.end());
        return path;
    }
}

namespace AIPath
{
    PathResult findPath(
        const MAP& map,
        Coordinate start,
        Coordinate goal,
        const Unit* self,
        GoalMode goalMode,
        int maxVisitedNodes
    ) {
        PathResult result;

        if (map.empty() || map[0].empty()) {
            return result;
        }

        if (!in_map(map, start.getX(), start.getY()) ||
            !in_map(map, goal.getX(), goal.getY())) {
            return result;
        }

        if (reachedGoal(start, goal, goalMode)) {
            result.success = true;
            result.nodes.push_back(start);
            return result;
        }

        if (!MovementRules::isFreeCell(
            map,
            start.getX(),
            start.getY(),
            self
        )) {
            return result;
        }

        if (goalMode == GoalMode::ExactCell &&
            !MovementRules::isFreeCell(
                map,
                goal.getX(),
                goal.getY(),
                self
            )) {
            return result;
        }

        int width = static_cast<int>(map.size());
        int height = static_cast<int>(map[0].size());
        int total = width * height;

        std::vector<NodeData> nodes(total);
        std::priority_queue<QueueNode, std::vector<QueueNode>, QueueCompare> open;

        int startIndex = toIndex(start.getX(), start.getY(), height);

        nodes[startIndex].gCost = 0;
        nodes[startIndex].fCost = heuristic(start, goal);
        nodes[startIndex].parent = -1;

        open.push(QueueNode{
            startIndex,
            nodes[startIndex].fCost,
            heuristic(start, goal)
        });

        int visited = 0;

        while (!open.empty()) {
            QueueNode currentQueueNode = open.top();
            open.pop();

            int currentIndex = currentQueueNode.index;

            if (nodes[currentIndex].closed) {
                continue;
            }

            nodes[currentIndex].closed = true;
            visited++;

            if (maxVisitedNodes > 0 && visited > maxVisitedNodes) {
                return result;
            }

            Coordinate current = toCoordinate(currentIndex, height);

            if (reachedGoal(current, goal, goalMode)) {
                result.success = true;
                result.nodes = reconstructPath(nodes, currentIndex, height);
                return result;
            }

            for (const auto& direction : DIRECTIONS) {
                int dx = direction[0];
                int dy = direction[1];

                if (!MovementRules::canMoveToNeighbour(
                    map,
                    self,
                    current,
                    dx,
                    dy
                )) {
                    continue;
                }

                int nx = current.getX() + dx;
                int ny = current.getY() + dy;
                int neighbourIndex = toIndex(nx, ny, height);

                if (nodes[neighbourIndex].closed) {
                    continue;
                }

                int tentativeG = nodes[currentIndex].gCost + movementCost(dx, dy);

                if (tentativeG >= nodes[neighbourIndex].gCost) {
                    continue;
                }

                Coordinate neighbour(nx, ny);
                int h = heuristic(neighbour, goal);

                nodes[neighbourIndex].gCost = tentativeG;
                nodes[neighbourIndex].fCost = tentativeG + h;
                nodes[neighbourIndex].parent = currentIndex;

                open.push(QueueNode{
                    neighbourIndex,
                    nodes[neighbourIndex].fCost,
                    h
                });
            }
        }

        return result;
    }

    std::vector<Coordinate> findPathOrEmpty(
        const MAP& map,
        Coordinate start,
        Coordinate goal,
        const Unit* self,
        GoalMode goalMode,
        int maxVisitedNodes
    ) {
        PathResult result = findPath(
            map,
            start,
            goal,
            self,
            goalMode,
            maxVisitedNodes
        );

        if (!result.success) {
            return {};
        }

        return result.nodes;
    }

    bool moveOneStepAlongPath(
        MAP& map,
        Unit* unit,
        std::vector<Coordinate>& path,
        std::size_t& pathIndex,
        int& ticksWaited,
        int tickRate,
        int movesPerSecond
    ) {
        if (!unit || path.empty()) {
            return false;
        }

        Coordinate current = unit->getPos();

        while (pathIndex < path.size() && sameCell(path[pathIndex], current)) {
            pathIndex++;
        }

        if (pathIndex >= path.size()) {
            path.clear();
            pathIndex = 0;
            ticksWaited = 0;
            return false;
        }

        int safeMovesPerSecond = std::max(1, movesPerSecond);
        int moveDelay = std::max(1, tickRate / safeMovesPerSecond);

        ticksWaited++;

        if (ticksWaited < moveDelay) {
            return true;
        }

        ticksWaited = 0;

        Coordinate next = path[pathIndex];

        int dx = next.getX() - current.getX();
        int dy = next.getY() - current.getY();

        if (dx < -1 || dx > 1 || dy < -1 || dy > 1) {
            path.clear();
            pathIndex = 0;
            return false;
        }

        if (!MovementRules::canMoveToNeighbour(map, unit, current, dx, dy)) {
            path.clear();
            pathIndex = 0;
            return false;
        }

        int oldX = current.getX();
        int oldY = current.getY();
        int newX = next.getX();
        int newY = next.getY();

        if (in_map(map, oldX, oldY) && map[oldX][oldY].unit == unit) {
            map[oldX][oldY].unit = nullptr;
        }

        map[newX][newY].unit = unit;
        unit->moveTo(newX, newY);

        pathIndex++;

        if (pathIndex >= path.size()) {
            path.clear();
            pathIndex = 0;
            ticksWaited = 0;
        }

        return true;
    }
}
