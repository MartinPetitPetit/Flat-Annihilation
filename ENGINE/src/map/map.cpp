#include "map.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <queue>  // Pour la version itérative

MAP create_map(int width, int height)
{
    CARRE empty_cell{ Plain, None1, None2 };
    return MAP(width, std::vector<CARRE>(height, empty_cell));
}

void generate_map(MAP& map)
{
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        seeded = true;
    }

    int width  = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++) {
            map[x][y].type_terrain = static_cast<TERRAIN>(std::rand() % 5);
            map[x][y].type_struct  = None1;
            map[x][y].type_unit    = None2;
        }
}

void affiche_map(const MAP& map)
{
    int width  = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    for (int x = 0; x < width; x++) {
        std::cout << "|";
        for (int y = 0; y < height; y++)
            std::cout << " " << map[x][y].type_terrain << " |";
        std::cout << "\n";
    }
}

// Version ITÉRATIVE (BFS) — remplace la récursion qui stack overflow sur grande map
int terrain_size(const MAP& map, int x, int y)
{
    int width  = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    if (x < 0 || x >= width || y < 0 || y >= height) return 0;

    TERRAIN target = map[x][y].type_terrain;

    std::vector<std::vector<bool>> visited(width, std::vector<bool>(height, false));
    std::queue<std::pair<int,int>> queue;

    queue.push({x, y});
    visited[x][y] = true;
    int count = 0;

    // 8 directions (comme ta version récursive)
    const int dx[] = {1,-1,0,0, 1, 1,-1,-1};
    const int dy[] = {0,0,1,-1, 1,-1, 1,-1};

    while (!queue.empty()) {
        auto [cx, cy] = queue.front();
        queue.pop();
        count++;

        for (int d = 0; d < 8; d++) {
            int nx = cx + dx[d];
            int ny = cy + dy[d];

            if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
            if (visited[nx][ny]) continue;
            if (map[nx][ny].type_terrain != target) continue;

            visited[nx][ny] = true;
            queue.push({nx, ny});
        }
    }

    return count;
}