#include "Map.hpp"
#include "../Unit/Unit.hpp"
#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>

/*
 * ============================================================
 * A* PATHFINDING
 * ============================================================
 */

static bool cell_walkable(const MAP& map,
                           int x, int y,
                           const std::vector<std::unique_ptr<Unit>>& units,
                           int selfId,
                           Coordinate goal)
{
    if (!in_map(map, x, y)) return false;

    const Cell& c = map[x][y];

    // Terrain infranchissable
    if (c.type_terrain == Montain ||
        c.type_terrain == Lake    ||
        c.type_terrain == River   ||
        c.type_terrain == ravine)
        return false;

    // Bâtiment
    if (c.buildingID != -1) return false;

    // Unité présente (sauf soi-même, sauf destination)
    if (c.unit != nullptr) {
        if (x == goal.getX() && y == goal.getY()) return true;
        if (c.unit->getId() == selfId) return true;
        return false;
    }

    return true;
}
struct AStarNode {
    int x, y;
    float f, g;
    bool operator>(const AStarNode& o) const { return f > o.f; }
};

static float heuristic(int ax, int ay, int bx, int by)
{
    // Chebyshev : adapté aux 8 directions
    return static_cast<float>(std::max(std::abs(ax - bx), std::abs(ay - by)));
}

std::vector<Coordinate> findPath(
    const MAP& map,
    Coordinate start,
    Coordinate goal,
    const std::vector<std::unique_ptr<Unit>>& units,
    int selfId)
{
    const int dx[8] = {-1,-1, 0, 1, 1, 1, 0,-1};
    const int dy[8] = { 0, 1, 1, 1, 0,-1,-1,-1};

    int width  = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    // Vérification basique
    if (!in_map(map, start.getX(), start.getY())) return {};
    if (!in_map(map, goal.getX(),  goal.getY()))  return {};

    // Tables A*
    std::vector<std::vector<float>> gScore(width,
        std::vector<float>(height, 1e9f));
    std::vector<std::vector<bool>> closed(width,
        std::vector<bool>(height, false));

    // Parent : encodé comme index linéaire
    std::vector<std::vector<int>> parent(width,
        std::vector<int>(height, -1));

    auto encode = [&](int x, int y) { return x * height + y; };

    std::priority_queue<AStarNode,
                        std::vector<AStarNode>,
                        std::greater<AStarNode>> open;

    int sx = start.getX(), sy = start.getY();
    int gx = goal.getX(),  gy = goal.getY();

    gScore[sx][sy] = 0.f;
    open.push({ sx, sy, heuristic(sx, sy, gx, gy), 0.f });

    bool found = false;

    while (!open.empty()) {
        AStarNode cur = open.top(); open.pop();

        if (closed[cur.x][cur.y]) continue;
        closed[cur.x][cur.y] = true;

        if (cur.x == gx && cur.y == gy) { found = true; break; }

        for (int d = 0; d < 8; d++) {
            int nx = cur.x + dx[d];
            int ny = cur.y + dy[d];

            if (!in_map(map, nx, ny))   continue;
            if (closed[nx][ny])         continue;
            if (!cell_walkable(map, nx, ny, units, selfId, goal)) continue;

            // Coût diagonal légèrement supérieur
            float step = (d % 2 == 0) ? 1.f : 1.414f;
            float ng   = gScore[cur.x][cur.y] + step;

            if (ng < gScore[nx][ny]) {
                gScore[nx][ny] = ng;
                parent[nx][ny] = encode(cur.x, cur.y);
                float f = ng + heuristic(nx, ny, gx, gy);
                open.push({ nx, ny, f, ng });
            }
        }
    }

    if (!found) return {};

    // Reconstruction du chemin
    std::vector<Coordinate> path;
    int cx = gx, cy = gy;

    while (!(cx == sx && cy == sy)) {
        path.push_back(Coordinate(cx, cy));
        int p = parent[cx][cy];
        if (p == -1) return {}; // sécurité
        cx = p / height;
        cy = p % height;
    }

    std::reverse(path.begin(), path.end());
    return path;
}

/*
 * ============================================================
 * FORMATION : destinations décalées autour d'un centre
 * ============================================================
 */

std::vector<Coordinate> formationDestinations(
    const MAP& map,
    Coordinate center,
    int count)
{
    std::vector<Coordinate> result;

    // Spirale carrée autour du centre
    result.push_back(center);

    int radius = 1;
    while (static_cast<int>(result.size()) < count && radius < 20) {
        for (int ox = -radius; ox <= radius && static_cast<int>(result.size()) < count; ox++) {
            for (int oy = -radius; oy <= radius && static_cast<int>(result.size()) < count; oy++) {
                if (std::abs(ox) != radius && std::abs(oy) != radius) continue; // bord seulement

                int x = center.getX() + ox;
                int y = center.getY() + oy;

                if (!in_map(map, x, y)) continue;

                const Cell& c = map[x][y];
                if (c.type_terrain == Montain ||
                    c.type_terrain == Lake    ||
                    c.type_terrain == ravine  ||
                    c.buildingID   != -1)
                    continue;

                result.push_back(Coordinate(x, y));
            }
        }
        radius++;
    }

    return result;
}