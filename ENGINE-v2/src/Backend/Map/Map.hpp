#pragma once

#include <utility>
#include <vector>

#include "../Cell/Cell.hpp"






class Map
{
    public:
        Map(int width, int height);
        virtual ~Map();

    private:
        std::vector<std::vector<Cell>> grid;
        int width;
        int height;

    protected:

};





/*
 = =================================*==========================
 MAP DIMENSIONS
 ============================================================
 */

// Number of rows in the map.
constexpr int X_Max = 0;

// Number of columns in the map.
constexpr int Y_Max = 0;


/*
 = =================================*==========================
 MOUNTAIN GENERATION CONSTANTS
 ============================================================
 */

// Number of mountain chains that the generator will try to create.
constexpr int Max_montain_quantity = 60;

// Number of cells advanced at each mountain generation step.
// Currently 1 means the chain grows cell by cell.
constexpr int Max_montain_size = 1;

// Maximum random value used to define mountain thickness.
// Final thickness is usually generated as rand() % thickness_max + 2.
constexpr int thickness_max = 3;

// Maximum random value used to define the chance of a mountain chain turning.
constexpr int turne_chance_max = 25;

// Maximum random value used to define the initial chance of a mountain chain stopping.
constexpr int stop_chance_max = 5;

// Maximum number of steps allowed for a single mountain chain.
constexpr int max_size = 100;


/*
 = =================================*==========================
 RIVER AND LAKE GENERATION CONSTANTS
 ============================================================
 */

// Number of attempts to find a mountain cell where a river can start.
constexpr int source_search_attempts = 500;

// Number of rivers that the generator will try to create.
constexpr int Max_river_quantity = 20;

// Maximum length of a river before it stops and creates a lake.
constexpr int Max_river_size = 120;

// Minimum lake area, in number of cells, when a river creates a lake.
constexpr int lake_min_area = 15;

// Maximum lake area, in number of cells, when a river creates a lake.
constexpr int lake_max_area = 350;

// Minimum river thickness.
constexpr int river_min_thickness = 1;

// Maximum river thickness.
constexpr int river_max_thickness = 3;

// Chance, in percent, that a river changes direction at each step.
constexpr int river_turn_chance = 25;


/*
 = =================================*==========================
 RAVINE GENERATION CONSTANTS
 ============================================================
 */

// Number of ravines that the generator will try to create.
constexpr int Max_ravine_quantity = 8;

// Maximum number of steps for one ravine.
constexpr int Max_ravine_size = 45;

// Chance, in percent, that a ravine slightly changes direction.
constexpr int ravine_turn_chance = 12;

// Chance, in percent, that a ravine makes a stronger direction change.
constexpr int ravine_hard_turn_chance = 8;

// Initial chance, in percent, that a ravine stops.
constexpr int ravine_stop_chance = 2;

// Chance, in percent, that a ravine creates a small lateral branch.
constexpr int ravine_branch_chance = 12;

// Number of attempts to find a valid ravine starting point.
constexpr int ravine_source_attempts = 1000;


/*
 = =================================*==========================
 FOREST AND TREE GENERATION CONSTANTS
 ============================================================
 */

// Number of dense forest patches that the generator will try to create.
constexpr int Max_forest_quantity = 12;

// Minimum radius of a dense forest patch.
constexpr int forest_min_radius = 4;

// Maximum radius of a dense forest patch.
constexpr int forest_max_radius = 12;

// Number of attempts to find a valid center for a forest patch.
constexpr int forest_center_search_attempts = 1000;

// Chance, in percent, of placing trees near the center of a forest.
constexpr int forest_core_chance = 85;

// Chance, in percent, of placing trees near the edge of a forest.
constexpr int forest_edge_chance = 20;

// Base chance, in percent, of placing scattered trees on plain terrain.
constexpr int scattered_tree_chance = 2;

// Bonus chance added when a tree is near water.
constexpr int near_water_tree_bonus = 6;

// Bonus chance added when a tree is near another tree.
constexpr int near_forest_tree_bonus = 3;

// Penalty applied when a tree is near a ravine.
constexpr int near_ravine_tree_penalty = 4;


/*
 = =================================*==========================
 ENUMERATIONS
 ============================================================
 */

// Terrain layer: represents the ground itself.
enum TERRAIN {
    Plain,      // Basic walkable terrain.
    Montain,   // Mountain terrain.
    Lake,      // Lake water.
    River,     // River water.
    Bush,      // Legacy vegetation terrain. Prefer RESOURCE::tree for trees.
    ravine     // Cracked or torn terrain.
};

// Structure layer: represents buildings placed on a cell.
enum STRUCTURE {
    None_Struct, // No structure.
    Usine,       // Factory.
    Production,  // Production building.
    Resource     // Resource-related structure.
};

// Resource layer: represents objects or resources placed over the terrain.
enum RESOURCE {
    None_Resource, // No resource.
    tree,          // Tree resource.
    stone,         // Stone resource.
    gold,          // Gold resource.
    iron,          // Iron resource.
    Sapling        // Young tree.
};

// Unit layer: represents units placed on the map.
enum UNIT {
    None_Unit, // No unit.
    archer,    // Archer unit.
    MONK       // Monk unit.
};


/*
 = =================================*==========================
 BASIC DATA STRUCTURES
 ============================================================
 */

// One map cell.
// Each cell has multiple layers: terrain, structure, resource and unit.
struct CARRE {
    TERRAIN type_terrain;
    STRUCTURE type_struct;
    RESOURCE type_resource;
    UNIT type_unit;
};

// The map is a 2D vector of cells.
using MAP = std::vector<std::vector<CARRE>>;

// Mountain generation data.
// Each mountain chain has a start point, direction, thickness and behavior chances.
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

// River generation data.
// Currently useful if you want to store river parameters before drawing them.
struct RIVER {
    int x_init;
    int y_init;
    int DIR;
    int max_length;
    int turn_chance;
};


/*
 = =================================*==========================
 MAP CREATION AND GENERAL UTILITIES
 ============================================================
 */

// Creates an empty map with the given width and height.
MAP create_map(int width, int height);

// Generates the full map by calling each generation step in order.
void generate_map(MAP& map);

// Prints the map in the terminal using emoji symbols.
void affiche_map(const MAP& map);

// Checks if a coordinate is inside the map.
bool in_map(const MAP& map, int x, int y);

// Changes the terrain of a cell if the coordinate is valid.
void set_terrain(MAP& map, int x, int y, TERRAIN terrain);


/*
 = =================================*==========================
 MOUNTAIN GENERATION
 ============================================================
 */

// Creates all mountain chains using the given mountain parameters.
void create_montain(MAP& map, std::vector<MONTAIN>& montains);

// Paints a mountain area around a center cell using a circular/random brush.
void paint_mountain_brush(MAP& map, int cx, int cy, int thickness);


/*
 = =================================*==========================
 RIVER AND LAKE GENERATION
 ============================================================
 */

// Finds a mountain cell that can be used as the source of a river.
bool find_mountain_source(const MAP& map, int& x, int& y);

// Creates all rivers on the map.
void create_rivers(MAP& map);

// Draws one river starting from a coordinate and following a direction.
void draw_river(MAP& map, int x, int y, int dir);

// Paints a river with a given thickness around a center cell.
void paint_river_brush(MAP& map, int cx, int cy, int thickness);

// Calculates the lake area based on river length and river thickness.
int calculate_lake_area(int river_length, int river_thickness);

// Checks if a cell can be replaced by lake terrain.
bool can_paint_lake_cell(const MAP& map, int x, int y);

// Creates a lake at the end of a river using river length and thickness.
void create_lake_from_river(MAP& map, int x, int y, int river_length, int river_thickness);

// Paints a lake by expanding from a center cell until reaching a target area.
void paint_lake_area(MAP& map, int cx, int cy, int target_area);

/*
 = =================================*==========================
 RAVINE GENERATION
 ============================================================
 */

// Finds a valid starting point for a ravine near mountains.
bool find_ravine_source(const MAP& map, int& x, int& y, int& dir);

// Creates all ravines on the map.
void create_ravines(MAP& map);

// Draws one ravine as a torn/cracked line across the terrain.
void draw_ravine(MAP& map, int x, int y, int dir);

// Paints the main ravine tear around a center cell.
void paint_ravine_tear_brush(MAP& map, int cx, int cy, int dir, int width);

// Draws a small lateral branch from a ravine.
void draw_ravine_branch(MAP& map, int x, int y, int dir, int length);


/*
 = =================================*==========================
 FOREST AND TREE GENERATION
 ============================================================
 */

// Checks if a tree can be placed on a cell.
bool can_place_tree(const MAP& map, int x, int y);

// Checks if a specific terrain exists near a coordinate.
bool has_terrain_near(const MAP& map, int cx, int cy, TERRAIN terrain, int radius);

// Checks if a specific resource exists near a coordinate.
bool has_resource_near(const MAP& map, int cx, int cy, RESOURCE resource, int radius);

// Checks if a coordinate is near a river or lake.
bool is_near_water(const MAP& map, int x, int y, int radius);

// Finds a valid center point for a dense forest patch.
bool find_forest_center(const MAP& map, int& x, int& y);

// Paints one dense forest patch around a center point.
void paint_forest_patch(MAP& map, int cx, int cy, int radius);

// Creates dense forest patches.
void create_forests(MAP& map);

// Creates scattered trees outside dense forests.
void create_scattered_trees(MAP& map);
