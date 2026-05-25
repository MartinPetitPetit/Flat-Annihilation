#pragma once

#include <utility>
#include <vector>
#include "../Cell/Cell.hpp"
#include "../Unit/Unit.hpp"
#include "../Resource/Resource.hpp"

/*
 * ============================================================
 * MAP TYPE
 * ============================================================
 */

// Main procedural map type.
using MAP = std::vector<std::vector<Cell>>;


/*
 * ============================================================
 * classe Map
 * ============================================================
 */

class Map
{
	public:
		Map(int map_width, int map_height);

		virtual ~Map() = default;

		MAP& data()
		{
			return grid;
		};

		const MAP& data() const
		{
			return grid;
		};

		int get_width() const
		{
			return width;
		};

		int get_height() const
		{
			return height;
		};

		const MAP& getGrid() const;
		MAP setGrid();

	private:
		MAP grid;
		int width;
		int height;
};

/*
 * ============================================================
 * BASE GENERATION VALUES
 * ============================================================
 *
 * These values are used as a reference by make_generation_config().
 * The real generation values are dynamically calculated from map size.
 */

// Base reference map size.
constexpr int BASE_MAP_WIDTH = 300;
constexpr int BASE_MAP_HEIGHT = 300;

// Mountains.
// Fewer chains, but longer and wider.
constexpr int BASE_Max_montain_quantity = 9;
constexpr int BASE_Max_montain_size = 1;
constexpr int BASE_thickness_max = 5;
constexpr int BASE_turne_chance_max = 18;
constexpr int BASE_stop_chance_max = 3;
constexpr int BASE_min_montain_steps = 45;
constexpr int BASE_max_montain_steps = 160;
constexpr float BASE_montain_stop_growth = 0.10f;

// Rivers and lakes.
constexpr int BASE_source_search_attempts = 500;
constexpr int BASE_Max_river_quantity = 20;
constexpr int BASE_Max_river_size = 120;
constexpr int BASE_lake_min_area = 15;
constexpr int BASE_lake_max_area = 350;
constexpr int BASE_river_min_thickness = 1;
constexpr int BASE_river_max_thickness = 3;
constexpr int BASE_river_turn_chance = 25;

// Ravines.
constexpr int BASE_Max_ravine_quantity = 8;
constexpr int BASE_Max_ravine_size = 45;
constexpr int BASE_ravine_turn_chance = 12;
constexpr int BASE_ravine_hard_turn_chance = 8;
constexpr int BASE_ravine_stop_chance = 2;
constexpr int BASE_ravine_branch_chance = 12;
constexpr int BASE_ravine_source_attempts = 1000;

// Forests.
constexpr int BASE_Max_forest_quantity = 12;
constexpr int BASE_forest_min_radius = 4;
constexpr int BASE_forest_max_radius = 12;
constexpr int BASE_forest_center_search_attempts = 1000;
constexpr int BASE_forest_core_chance = 85;
constexpr int BASE_forest_edge_chance = 20;
constexpr int BASE_scattered_tree_chance = 2;
constexpr int BASE_near_water_tree_bonus = 6;
constexpr int BASE_near_forest_tree_bonus = 3;
constexpr int BASE_near_ravine_tree_penalty = 4;

// Wood type.
constexpr int BASE_wood_type_b_chance = 35;
constexpr int BASE_wood_type_c_chance = 15;

// Bushes and berries.
constexpr int BASE_Max_bush_patch_quantity = 10;
constexpr int BASE_bush_min_radius = 3;
constexpr int BASE_bush_max_radius = 7;
constexpr int BASE_bush_center_search_attempts = 1000;
constexpr int BASE_bush_core_chance = 80;
constexpr int BASE_bush_edge_chance = 20;
constexpr int BASE_scattered_bush_chance = 1;
constexpr int BASE_dense_bush_berry_chance = 45;
constexpr int BASE_scattered_bush_berry_chance = 25;
constexpr int BASE_near_water_bush_bonus = 4;
constexpr int BASE_near_ravine_bush_penalty = 3;

/*
 * ============================================================
 * DYNAMIC GENERATION CONFIG
 * ============================================================
 */

// Runtime generation values calculated from the map size.
struct GenerationConfig {
    int map_width;
    int map_height;

    double area_ratio;
    double linear_ratio;

    // Mountains.
    int max_montain_quantity;
    int max_montain_size;
    int thickness_max;
    int turne_chance_max;
    int stop_chance_max;
    int min_montain_steps;
    int max_montain_steps;
    float montain_stop_growth;

    // Rivers and lakes.
    int source_search_attempts;
    int max_river_quantity;
    int max_river_size;
    int lake_min_area;
    int lake_max_area;
    int river_min_thickness;
    int river_max_thickness;
    int river_turn_chance;

    // Ravines.
    int max_ravine_quantity;
    int max_ravine_size;
    int ravine_turn_chance;
    int ravine_hard_turn_chance;
    int ravine_stop_chance;
    int ravine_branch_chance;
    int ravine_source_attempts;

    // Forests.
    int max_forest_quantity;
    int forest_min_radius;
    int forest_max_radius;
    int forest_center_search_attempts;
    int forest_core_chance;
    int forest_edge_chance;
    int scattered_tree_chance;
    int near_water_tree_bonus;
    int near_forest_tree_bonus;
    int near_ravine_tree_penalty;

    // Wood type.
    int wood_type_b_chance;
    int wood_type_c_chance;

    // Bushes.
    int max_bush_patch_quantity;
    int bush_min_radius;
    int bush_max_radius;
    int bush_center_search_attempts;
    int bush_core_chance;
    int bush_edge_chance;
    int scattered_bush_chance;
    int dense_bush_berry_chance;
    int scattered_bush_berry_chance;
    int near_water_bush_bonus;
    int near_ravine_bush_penalty;
};

/*
 * ============================================================
 * GENERATION STRUCTURES
 * ============================================================
 */

// Mountain generation data.
struct MONTAIN {
    int x_init;
    int y_init;
    int size;
    int DIR;
    int thickness;      // Peak thickness used near the center of the chain.
    int tip_thickness;  // Thickness used near the tips.
    int target_steps;   // Intended mountain chain length.

    int turne_chance;
    float stop_chance;
    int lateral_noise_chance;
};

// River generation data.
struct RIVER {
    int x_init;
    int y_init;
    int DIR;
    int max_length;
    int turn_chance;
};

/*
 * ============================================================
 * MAP CREATION AND GENERAL UTILITIES
 * ============================================================
 */

GenerationConfig make_generation_config(int width, int height);

void generate_map(MAP& map);
void affiche_map(const MAP& map);

bool in_map(const MAP& map, int x, int y);
void set_terrain(MAP& map, int x, int y, TERRAIN terrain);

bool has_terrain_near(const MAP& map, int cx, int cy, TERRAIN terrain, int radius);
bool has_resource_near(const MAP& map, int cx, int cy, ResourceType resource, int radius);
bool is_near_water(const MAP& map, int x, int y, int radius);

/*
 * ============================================================
 * MOUNTAIN GENERATION
 * ============================================================
 */

void create_montain(MAP& map, std::vector<MONTAIN>& montains, const GenerationConfig& cfg);
void paint_mountain_brush(MAP& map, int cx, int cy, int thickness);

/*
 * ============================================================
 * RIVER AND LAKE GENERATION
 * ============================================================
 */

bool find_mountain_source(const MAP& map, int& x, int& y, const GenerationConfig& cfg);
void create_rivers(MAP& map, const GenerationConfig& cfg);
void draw_river(MAP& map, int x, int y, int dir, const GenerationConfig& cfg);

int paint_river_brush(MAP& map, int cx, int cy, int thickness);

int calculate_lake_area(int river_volume, int river_thickness, const GenerationConfig& cfg);
bool can_paint_lake_cell(const MAP& map, int x, int y);
void create_lake_from_river(MAP& map, int x, int y, int river_volume, int river_thickness, const GenerationConfig& cfg);
void paint_lake_area(MAP& map, int cx, int cy, int target_area);

// Legacy circular lake function.
void paint_lake(MAP& map, int cx, int cy, int radius);

/*
 * ============================================================
 * RAVINE GENERATION
 * ============================================================
 */

bool find_ravine_source(const MAP& map, int& x, int& y, int& dir, const GenerationConfig& cfg);
void create_ravines(MAP& map, const GenerationConfig& cfg);
void draw_ravine(MAP& map, int x, int y, int dir, const GenerationConfig& cfg);
void paint_ravine_tear_brush(MAP& map, int cx, int cy, int dir, int width);
void draw_ravine_branch(MAP& map, int x, int y, int dir, int length);

/*
 * ============================================================
 * FOREST AND TREE GENERATION
 * ============================================================
 */

bool can_place_tree(const MAP& map, int x, int y);

void place_tree(MAP& map, int x, int y);

bool find_forest_center(const MAP& map, int& x, int& y, const GenerationConfig& cfg);
void paint_forest_patch(MAP& map, int cx, int cy, int radius, const GenerationConfig& cfg);

void create_forests(MAP& map, const GenerationConfig& cfg);
void create_scattered_trees(MAP& map, const GenerationConfig& cfg);

/*
 * ============================================================
 * BUSH AND BERRY GENERATION
 * ============================================================
 */

bool can_place_bush(const MAP& map, int x, int y);
void place_bush(MAP& map, int x, int y, int berry_chance);

bool find_bush_center(const MAP& map, int& x, int& y, const GenerationConfig& cfg);
void paint_bush_patch(MAP& map, int cx, int cy, int radius, const GenerationConfig& cfg);

void create_bush_patches(MAP& map, const GenerationConfig& cfg);
void create_scattered_bushes(MAP& map, const GenerationConfig& cfg);
