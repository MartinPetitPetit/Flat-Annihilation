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

    for (int x = 0; x < width; x++) {
        std::cout << "|";

        for (int y = 0; y < height; y++) {
            if (map[x][y].type_terrain == Plain) {
                std::cout << "🟩";
            } else {
                std::cout << "🟫";
            }
        }

        std::cout << '\n';
    }
}

void paint_mountain_brush(MAP& map, int cx, int cy, int thickness)
{
    int radius = thickness / 2;

    for (int dx = -radius; dx <= radius; dx++) {
        for (int dy = -radius; dy <= radius; dy++) {

            int dist2 = dx * dx + dy * dy;
            int r2 = radius * radius;

            if (dist2 > r2) {
                continue;
            }

            if (dx == 0 && dy == 0) {
                set_terrain(map, cx, cy, Montain);
            }
            else if (dist2 <= 1 && std::rand() % 100 < 85) {
                set_terrain(map, cx + dx, cy + dy, Montain);
            }
            else if (dist2 < r2 && std::rand() % 100 < 65) {
                set_terrain(map, cx + dx, cy + dy, Montain);
            }
            else if (dist2 == r2 && std::rand() % 100 < 35) {
                set_terrain(map, cx + dx, cy + dy, Montain);
            }
        }
    }
}

void create_montain(MAP& map, std::vector<MONTAIN>& montains)
{
    /*
        No seu sistema original:

        x-- = norte
        x++ = sul
        y++ = leste
        y-- = oeste
    */

    const int dx[8] = {
        -1, // NORTH
        -1, // NORTH_EAST
         0, // EAST
         1, // SOUTH_EAST
         1, // SOUTH
         1, // SOUTH_WEST
         0, // WEST
        -1  // NORTH_WEST
    };

    const int dy[8] = {
         0, // NORTH
         1, // NORTH_EAST
         1, // EAST
         1, // SOUTH_EAST
         0, // SOUTH
        -1, // SOUTH_WEST
        -1, // WEST
        -1  // NORTH_WEST
    };

    for (int i = 0; i < static_cast<int>(montains.size()); i++) {

        int x = montains[i].x_init;
        int y = montains[i].y_init;
        int steps = 0;

        set_terrain(map, x, y, Montain);

        while ((std::rand() % 100) >= montains[i].stop_chance &&
               steps < max_size &&
               in_map(map, x, y)) {

            /*
                Mudança de direção principal.
            */
            if ((std::rand() % 100) < montains[i].turne_chance) {
                int turn = (std::rand() % 2) ? 1 : -1;

                montains[i].DIR += turn;

                if (montains[i].DIR > 7) {
                    montains[i].DIR = 0;
                }

                if (montains[i].DIR < 0) {
                    montains[i].DIR = 7;
                }
            }

            for (int step = 0;
                 step < montains[i].size && steps < max_size;
                 step++) {

                int dir = montains[i].DIR;

                /*
                    Ruído lateral.

                    px, py é a direção perpendicular ao movimento principal.
                    Isso substitui todos aqueles casos manuais por direção.
                */
                if ((std::rand() % 100) < montains[i].lateral_noise_chance) {
                    int side = (std::rand() % 2) ? 1 : -1;

                    int px = -dy[dir];
                    int py =  dx[dir];

                    x += side * px;
                    y += side * py;
                }

                /*
                    Movimento principal.
                */
                x += dx[dir];
                y += dy[dir];

                if (!in_map(map, x, y)) {
                    break;
                }

                paint_mountain_brush(map, x, y, montains[i].thickness);

                steps++;
            }

            /*
                A chance de parar aumenta aos poucos.
            */
            montains[i].stop_chance += 0.25f;
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

    /*
        Parte 1: tudo começa como planície.
    */
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            map[x][y].type_terrain = Plain;
            map[x][y].type_struct = None_Struct;
            map[x][y].type_unit = None_Unit;
        }
    }

    /*
        Parte 2: criação dos parâmetros das montanhas.
    */
    std::vector<MONTAIN> montains(Max_montain_quantity);

    for (int i = 0; i < Max_montain_quantity; i++) {
        montains[i].x_init = std::rand() % width;
        montains[i].y_init = std::rand() % height;

        montains[i].size = 1 + std::rand() % Max_montain_size;

        montains[i].DIR = std::rand() % 8;

        /*
            Espessura ímpar: 3, 5 ou 7.
            Melhor para brush, porque tem centro claro.
        */
        montains[i].thickness = 3 + 2 * (std::rand() % thickness_max);

        montains[i].turne_chance = std::rand() % turne_chance_max;

        montains[i].stop_chance = 1 + std::rand() % stop_chance_max;

        montains[i].lateral_noise_chance = 25 + std::rand() % 30;
    }

    create_montain(map, montains);
}
