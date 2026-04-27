#ifndef MAP_HPP
#define MAP_HPP

#include <vector>

constexpr int X_Max = 50;
constexpr int Y_Max = 50;

// mountain related
constexpr int Max_montain_quantity = 4;
constexpr int Max_montain_size = 1;
constexpr int thickness_max = 3;
constexpr int turne_chance_max = 25;
constexpr int stop_chance_max = 5;
constexpr int max_size = 100;

enum TERRAIN {
    Plain,
    Montain,
    Lake,
    Bush,
    ravine
};

enum STRUCTURE {
    None_Struct,
    Usine,
    Production,
    Resource
};

enum RESOURCE {
    None_Resource,
    tree,
    gold,
    Sapling
};

enum UNIT {
    None_Unit,
    archer,
    MONK
};

enum DIR {
    NORTH = 0,
    NORTH_EAST = 1,
    EAST = 2,
    SOUTH_EAST = 3,
    SOUTH = 4,
    SOUTH_WEST = 5,
    WEST = 6,
    NORTH_WEST = 7
};

struct CARRE {
    TERRAIN type_terrain;
    STRUCTURE type_struct;
    UNIT type_unit;
};

using MAP = std::vector<std::vector<CARRE>>;

struct MONTAIN {
    int x_init;
    int y_init;
    int size;
    int DIR;
    int thickness;
    int turne_chance;
    float stop_chance;
    int lateral_noise_chance;
};

MAP cree_map(int width, int height);
void generate_map(MAP& map);
void affiche_map(const MAP& map);

bool in_map(const MAP& map, int x, int y);
void set_terrain(MAP& map, int x, int y, TERRAIN terrain);

void create_montain(MAP& map, std::vector<MONTAIN>& montains);
void paint_mountain_brush(MAP& map, int cx, int cy, int thickness);

#endif
