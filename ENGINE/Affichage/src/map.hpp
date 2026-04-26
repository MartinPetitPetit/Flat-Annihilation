#pragma once
#include <vector>

typedef enum { Plain, Montain, Lake, Bush, ravine } TERRAIN;
typedef enum { None1, Usine, Production, Resource } STRUCTURE;
typedef enum { None2, soldier, archer, MONK } UNIT;

typedef struct {
    TERRAIN type_terrain;
    STRUCTURE type_struct;
    UNIT type_unit;
} CARRE;

using MAP = std::vector<std::vector<CARRE>>;

MAP create_map(int width, int height);
void generate_map(MAP& map);
void affiche_map(const MAP& map);
int terrain_size(const MAP& map, int x, int y);