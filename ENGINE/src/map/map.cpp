#include "map.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>

MAP create_map(int width, int height)
{
    MAP map(width, std::vector<CARRE>(height));

    return map;
}

bool in_map(const MAP& map, int x, int y)
{
    if (map.empty()) {
        return false;
    }

    return x >= 0 &&
           x < static_cast<int>(map.size()) &&
           y >= 0 &&
           y < static_cast<int>(map[0].size());
}

void set_terrain(MAP& map, int x, int y, TERRAIN terrain)
{
    if (in_map(map, x, y)) {
        map[x][y].type_terrain = terrain;
    }
}

void affiche_map(const MAP& map)
{
    if (map.empty()) {
        return;
    }

    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    for (int y = 0; y < height; y++) {
        std::cout << "|";
        for (int x = 0; x < width; x++) {
            std::cout << " " << map[x][y].type_terrain << " |";
        }
        std::cout << '\n';
    }
}

int rac_TERRAIN_size_rec(
    const MAP& map,
    int x,
    int y,
    TERRAIN type_,
    std::vector<std::vector<int>>& visited
)
{
    if (!in_map(map, x, y)) {
        return 0;
    }

    if (visited[x][y] == 1) {
        return 0;
    }

    if (map[x][y].type_terrain != type_) {
        return 0;
    }

    visited[x][y] = 1;

    int total = 1;

    total += rac_TERRAIN_size_rec(map, x + 1, y,     type_, visited);
    total += rac_TERRAIN_size_rec(map, x - 1, y,     type_, visited);
    total += rac_TERRAIN_size_rec(map, x,     y + 1, type_, visited);
    total += rac_TERRAIN_size_rec(map, x,     y - 1, type_, visited);

    total += rac_TERRAIN_size_rec(map, x + 1, y + 1, type_, visited);
    total += rac_TERRAIN_size_rec(map, x + 1, y - 1, type_, visited);
    total += rac_TERRAIN_size_rec(map, x - 1, y + 1, type_, visited);
    total += rac_TERRAIN_size_rec(map, x - 1, y - 1, type_, visited);

    return total;
}


void generate_map(MAP& map)
{
    static bool srand_done = false;
    if (!srand_done) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        srand_done = true;
    }

    if (map.empty()) return;

    int width  = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    // 1. Initialisation : tout en plaine, puis bruit aléatoire de montagnes
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++) {
            map[x][y].type_struct = None_Struct;
            map[x][y].type_unit   = None_Unit;
            // ~20% de chance d'être une montagne au départ
            map[x][y].type_terrain = (std::rand() % 100 < 70) ? Montain : Plain;
        }

    // 2. Automate cellulaire — plusieurs passes de lissage
    int iterations = 18; // plus on itère, plus les zones sont grandes et lisses
    int birth_limit = 6; // nb de voisins montagne pour qu'une plaine devienne montagne
    int death_limit = 5; // nb de voisins montagne en dessous duquel une montagne meurt

    const int dx[] = {1,-1,0,0, 1, 1,-1,-1};
    const int dy[] = {0,0,1,-1, 1,-1, 1,-1};

    for (int iter = 0; iter < iterations; iter++) {
        // Copie de la map pour lire l'état N et écrire l'état N+1
        MAP next = map;

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                // Compter les voisins montagne (8 directions)
                int mountain_neighbors = 0;
                for (int d = 0; d < 8; d++) {
                    int nx = x + dx[d];
                    int ny = y + dy[d];
                    if (!in_map(map, nx, ny)) {
                        // Bord de map compté comme montagne — crée un cadre naturel
                        mountain_neighbors++;
                    } else if (map[nx][ny].type_terrain == Montain) {
                        mountain_neighbors++;
                    }
                }

                if (map[x][y].type_terrain == Montain) {
                    // Une montagne survit si elle a assez de voisins
                    next[x][y].type_terrain = (mountain_neighbors >= death_limit)
                        ? Montain : Plain;
                } else {
                    // Une plaine devient montagne si elle a assez de voisins montagne
                    next[x][y].type_terrain = (mountain_neighbors >= birth_limit)
                        ? Montain : Plain;
                }
            }
        }
        map = next;
    }
}