#include "map.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>

MAP cree_map(int width, int height)
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

void paint_mountain_thickness(MAP& map, int x, int y, int dir, int thickness)
{
    if (thickness < 1) {
        thickness = 1;
    }

    /*
        Direções compatíveis com o DisplayMap:

        x aumenta para a direita na tela
        y aumenta para baixo na tela

        NORTH      = y--
        SOUTH      = y++
        EAST       = x++
        WEST       = x--
    */

    int dx[8] = {
         0, // NORTH
         1, // NORTH_EAST
         1, // EAST
         1, // SOUTH_EAST
         0, // SOUTH
        -1, // SOUTH_WEST
        -1, // WEST
        -1  // NORTH_WEST
    };

    int dy[8] = {
        -1, // NORTH
        -1, // NORTH_EAST
         0, // EAST
         1, // SOUTH_EAST
         1, // SOUTH
         1, // SOUTH_WEST
         0, // WEST
        -1  // NORTH_WEST
    };

    /*
        Vetor perpendicular à direção da montanha.
        A espessura cresce perpendicularmente ao caminho.
    */
    int px = -dy[dir];
    int py = dx[dir];

    int start = -(thickness / 2);

    for (int t = 0; t < thickness; t++) {
        int offset = start + t;

        int tx = x + offset * px;
        int ty = y + offset * py;

        set_terrain(map, tx, ty, Montain);
    }
}

void create_montains(MAP& map, std::vector<MONTAIN>& montains)
{
    int dx[8] = {
         0, // NORTH
         1, // NORTH_EAST
         1, // EAST
         1, // SOUTH_EAST
         0, // SOUTH
        -1, // SOUTH_WEST
        -1, // WEST
        -1  // NORTH_WEST
    };

    int dy[8] = {
        -1, // NORTH
        -1, // NORTH_EAST
         0, // EAST
         1, // SOUTH_EAST
         1, // SOUTH
         1, // SOUTH_WEST
         0, // WEST
        -1  // NORTH_WEST
    };

    for (int i = 0; i < static_cast<int>(montains.size()); i++) {
        int x = montains[i].x_init;
        int y = montains[i].y_init;

        int current_size = 0;

        set_terrain(map, x, y, Montain);

        while (in_map(map, x, y) &&
               current_size < max_size &&
               (std::rand() % 100) >= montains[i].stop_chance) {

            if ((std::rand() % 100) < montains[i].turne_chance) {
                int turn_direction = std::rand() % 2;

                if (turn_direction == 0) {
                    montains[i].DIR--;
                } else {
                    montains[i].DIR++;
                }

                if (montains[i].DIR > 7) {
                    montains[i].DIR = 0;
                }

                if (montains[i].DIR < 0) {
                    montains[i].DIR = 7;
                }
            }

            for (int step = 0; step < montains[i].size; step++) {
                x += dx[montains[i].DIR];
                y += dy[montains[i].DIR];

                if (!in_map(map, x, y)) {
                    break;
                }

                paint_mountain_thickness(
                    map,
                    x,
                    y,
                    montains[i].DIR,
                    montains[i].thickness
                );

                current_size++;
            }

            montains[i].stop_chance++;

            if (montains[i].stop_chance > 100) {
                montains[i].stop_chance = 100;
            }
        }
    }
}

void generate_map(MAP& map)
{
    static bool srand_done = false;

    if (!srand_done) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        srand_done = true;
    }

    if (map.empty()) {
        return;
    }

    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            map[x][y].type_terrain = Plain;
            map[x][y].type_struct = None_Struct;
            map[x][y].type_unit = None_Unit;
        }
    }

    std::vector<MONTAIN> montains(Max_montain_quantity);

    for (int i = 0; i < Max_montain_quantity; i++) {
        montains[i].x_init = std::rand() % width;
        montains[i].y_init = std::rand() % height;
        montains[i].size = 1 + std::rand() % Max_montain_size;
        montains[i].DIR = std::rand() % 8;
        montains[i].thickness = 1 + std::rand() % thickness_max;
        montains[i].turne_chance = std::rand() % turne_chance_max;
        montains[i].stop_chance = 1 + std::rand() % stop_chance_max;
    }

    create_montains(map, montains);
}
