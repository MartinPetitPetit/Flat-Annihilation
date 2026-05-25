#include "Map.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

/*
 * ============================================================
 * INTERNAL HELPERS
 * ============================================================
 */

static int clamp_int(int value, int min_value, int max_value)
{
    if (value < min_value) {
        return min_value;
    }

    if (value > max_value) {
        return max_value;
    }

    return value;
}

/*
 * ============================================================
 * DYNAMIC GENERATION CONFIG
 * ============================================================
 */

GenerationConfig make_generation_config(int width, int height)
{
    GenerationConfig cfg;

    cfg.map_width = width;
    cfg.map_height = height;

    int area = width * height;
    int base_area = BASE_MAP_WIDTH * BASE_MAP_HEIGHT;

    cfg.area_ratio = static_cast<double>(area) / static_cast<double>(base_area);
    cfg.linear_ratio = std::sqrt(cfg.area_ratio);

    int min_dim = width < height ? width : height;
    int max_dim = width > height ? width : height;

    /*
     * Mountains scale mostly with map length.
     *
     * Larger maps should not create too many mountain chains.
     * Instead, chains become longer and wider.
     *
     * Example:
     * 300x300 -> around 9 chains
     * 500x500 -> around 15 chains
     */
    cfg.max_montain_quantity =
    clamp_int(static_cast<int>(BASE_Max_montain_quantity * cfg.linear_ratio), 3, 30);

    cfg.max_montain_size = BASE_Max_montain_size;

    /*
     * Mountain thickness grows with map size.
     * Larger maps get wider mountain chains.
     */
    cfg.thickness_max =
    clamp_int(static_cast<int>(BASE_thickness_max * cfg.linear_ratio), 3, 12);

    /*
     * Lower turn chance creates longer and more natural ranges.
     * Very high turn chance creates tangled mountains.
     */
    cfg.turne_chance_max = BASE_turne_chance_max;

    /*
     * Lower stop chance makes mountains live longer.
     */
    cfg.stop_chance_max = BASE_stop_chance_max;

    /*
     * Minimum length prevents tiny mountain fragments.
     */
    cfg.min_montain_steps =
    clamp_int(static_cast<int>(BASE_min_montain_steps * cfg.linear_ratio), 20, max_dim);

    /*
     * Maximum length grows strongly with map size.
     * On a 500x500 map this allows long mountain chains.
     */
    cfg.max_montain_steps =
    clamp_int(static_cast<int>(BASE_max_montain_steps * cfg.linear_ratio), 80, max_dim);

    /*
     * Stop chance grows slowly.
     * This makes mountain chains longer.
     */
    cfg.montain_stop_growth = BASE_montain_stop_growth;
    /*
     * Rivers scale with linear ratio and map length.
     */
    cfg.source_search_attempts =
    clamp_int(static_cast<int>(BASE_source_search_attempts * cfg.linear_ratio), 100, 8000);

    /*
     * Fewer rivers, but with much more room to become long and important.
     */
    cfg.max_river_quantity =
    clamp_int(static_cast<int>(BASE_Max_river_quantity * cfg.linear_ratio * 0.45), 2, 40);

    /*
     * Rivers can now travel much farther across the map.
     */
    cfg.max_river_size =
    clamp_int(static_cast<int>(min_dim * 0.85), 60, max_dim * 3 / 2);

    /*
     * Larger lakes so terminal lakes become visually meaningful.
     */
    cfg.lake_min_area =
    clamp_int(static_cast<int>(area * 0.00045), 18, 600);

    cfg.lake_max_area =
    clamp_int(static_cast<int>(area * 0.01000), 80, area / 4);

    cfg.river_min_thickness = BASE_river_min_thickness;

    /*
     * Allow thicker rivers on larger maps.
     */
    cfg.river_max_thickness =
    clamp_int(static_cast<int>(BASE_river_max_thickness * cfg.linear_ratio) + 1, 2, 9);
    cfg.river_turn_chance = BASE_river_turn_chance;

    /*
     * Ravines scale mostly with linear ratio.
     */
    cfg.max_ravine_quantity =
    clamp_int(static_cast<int>(BASE_Max_ravine_quantity * cfg.linear_ratio), 1, 80);

    cfg.max_ravine_size =
    clamp_int(static_cast<int>(min_dim * 0.15), 8, max_dim);

    cfg.ravine_turn_chance = BASE_ravine_turn_chance;
    cfg.ravine_hard_turn_chance = BASE_ravine_hard_turn_chance;
    cfg.ravine_stop_chance = BASE_ravine_stop_chance;
    cfg.ravine_branch_chance = BASE_ravine_branch_chance;

    cfg.ravine_source_attempts =
    clamp_int(static_cast<int>(BASE_ravine_source_attempts * cfg.linear_ratio), 100, 10000);

    /*
     * Forest patches scale with area.
     * Forest radius scales with linear ratio.
     */
    cfg.max_forest_quantity =
    clamp_int(static_cast<int>(BASE_Max_forest_quantity * cfg.area_ratio), 1, 250);

    cfg.forest_min_radius =
    clamp_int(static_cast<int>(BASE_forest_min_radius * cfg.linear_ratio), 2, 25);

    cfg.forest_max_radius =
    clamp_int(static_cast<int>(BASE_forest_max_radius * cfg.linear_ratio), cfg.forest_min_radius + 1, 70);

    cfg.forest_center_search_attempts =
    clamp_int(static_cast<int>(BASE_forest_center_search_attempts * cfg.linear_ratio), 100, 10000);

    cfg.forest_core_chance = BASE_forest_core_chance;
    cfg.forest_edge_chance = BASE_forest_edge_chance;
    cfg.scattered_tree_chance = BASE_scattered_tree_chance;
    cfg.near_water_tree_bonus = BASE_near_water_tree_bonus;
    cfg.near_forest_tree_bonus = BASE_near_forest_tree_bonus;
    cfg.near_ravine_tree_penalty = BASE_near_ravine_tree_penalty;

    cfg.wood_type_b_chance = BASE_wood_type_b_chance;
    cfg.wood_type_c_chance = BASE_wood_type_c_chance;

    /*
     * Bush patches scale with area.
     * Bush radius scales with linear ratio.
     */
    cfg.max_bush_patch_quantity =
    clamp_int(static_cast<int>(BASE_Max_bush_patch_quantity * cfg.area_ratio), 1, 250);

    cfg.bush_min_radius =
    clamp_int(static_cast<int>(BASE_bush_min_radius * cfg.linear_ratio), 2, 20);

    cfg.bush_max_radius =
    clamp_int(static_cast<int>(BASE_bush_max_radius * cfg.linear_ratio), cfg.bush_min_radius + 1, 50);

    cfg.bush_center_search_attempts =
    clamp_int(static_cast<int>(BASE_bush_center_search_attempts * cfg.linear_ratio), 100, 10000);

    cfg.bush_core_chance = BASE_bush_core_chance;
    cfg.bush_edge_chance = BASE_bush_edge_chance;
    cfg.scattered_bush_chance = BASE_scattered_bush_chance;
    cfg.dense_bush_berry_chance = BASE_dense_bush_berry_chance;
    cfg.scattered_bush_berry_chance = BASE_scattered_bush_berry_chance;
    cfg.near_water_bush_bonus = BASE_near_water_bush_bonus;
    cfg.near_ravine_bush_penalty = BASE_near_ravine_bush_penalty;

    return cfg;
}

/*
 * ============================================================
 * BASIC MAP FUNCTIONS
 * ============================================================
 */

MAP create_map(int width, int height)
{
    // Create a 2D map with width rows and height columns.
    return MAP(width, std::vector<Cell>(height));
}

bool in_map(const MAP& map, int x, int y)
{
    // Reject empty maps.
    if (map.empty() || map[0].empty()) {
        return false;
    }

    // Check if x and y are inside map limits.
    return x >= 0 &&
    x < static_cast<int>(map.size()) &&
    y >= 0 &&
    y < static_cast<int>(map[0].size());
}

void set_terrain(MAP& map, int x, int y, TERRAIN terrain)
{
    // Change terrain only if the cell exists.
    if (in_map(map, x, y)) {
        map[x][y].type_terrain = terrain;
    }
}

void affiche_map(const MAP& map)
{
    // Start first printed row.
    std::cout << "|";

    // Browse all map rows.
    for (int x = 0; x < static_cast<int>(map.size()); x++) {

        // Browse all cells in the current row.
        for (int y = 0; y < static_cast<int>(map[x].size()); y++) {

            if (map[x][y].type_terrain == Plain) {
                std::cout << "🟩";
            }
            else if (map[x][y].type_terrain == Montain) {
                std::cout << "🟫";
            }
            else if (map[x][y].type_terrain == River) {
                std::cout << "🟦";
            }
            else if (map[x][y].type_terrain == Lake) {
                std::cout << "🟦";
            }
            else {
                std::cout << "⬛";
            }
        }

        // Move to next printed row.
        std::cout << "\n|";
    }

    // Finish terminal output.
    std::cout << "\n";
}

/*
 * ============================================================
 * SHARED SEARCH HELPERS
 * ============================================================
 */

bool has_terrain_near(const MAP& map, int cx, int cy, TERRAIN terrain, int radius)
{
    // Browse nearby rows.
    for (int dx = -radius; dx <= radius; dx++) {

        // Browse nearby columns.
        for (int dy = -radius; dy <= radius; dy++) {

            int x = cx + dx;
            int y = cy + dy;

            if (!in_map(map, x, y)) {
                continue;
            }

            int dist2 = dx * dx + dy * dy;
            int r2 = radius * radius;

            if (dist2 > r2) {
                continue;
            }

            if (map[x][y].type_terrain == terrain) {
                return true;
            }
        }
    }

    return false;
}

bool has_resource_near(const MAP& map, int cx, int cy, RESOURCE resource, int radius)
{
    // Browse nearby rows.
    for (int dx = -radius; dx <= radius; dx++) {

        // Browse nearby columns.
        for (int dy = -radius; dy <= radius; dy++) {

            int x = cx + dx;
            int y = cy + dy;

            if (!in_map(map, x, y)) {
                continue;
            }

            int dist2 = dx * dx + dy * dy;
            int r2 = radius * radius;

            if (dist2 > r2) {
                continue;
            }

            if (map[x][y].type_resource == resource) {
                return true;
            }
        }
    }

    return false;
}

bool is_near_water(const MAP& map, int x, int y, int radius)
{
    // Check river or lake nearby.
    return has_terrain_near(map, x, y, River, radius) ||
    has_terrain_near(map, x, y, Lake, radius);
}

/*
 * ============================================================
 * FULL MAP GENERATION
 * ============================================================
 */

void generate_map(MAP& map)
{
    // Reject empty maps.
    if (map.empty() || map[0].empty()) {
        return;
    }

    // Avoid reseeding random more than once.
    static bool seed_done = false;

    if (!seed_done) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        seed_done = true;
    }

    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    // Dynamic generation values based on map size.
    GenerationConfig cfg = make_generation_config(width, height);

    /*
     * ----------------------------
     * PART 1: BASE PLAIN MAP
     * ----------------------------
     */

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            map[x][y].type_terrain  = Plain;
            map[x][y].type_unit     = None_Unit;
            map[x][y].type_resource = None_Resource;
            map[x][y].wood_type     = No_Wood;
            map[x][y].has_berry     = false;
            map[x][y].walkable      = true;
            map[x][y].occupied      = false;
            map[x][y].buildingID    = -1;
            map[x][y].buildingOwner = -1;
        }
    }

    /*
     * ----------------------------
     * PART 2: MOUNTAINS
     * ----------------------------
     */

    std::vector<MONTAIN> montains(cfg.max_montain_quantity);

    // Initialize each mountain chain.
    for (int i = 0; i < cfg.max_montain_quantity; i++) {
        montains[i].x_init = std::rand() % width;   // Random start row.
        montains[i].y_init = std::rand() % height;  // Random start column.

        // Number of cells advanced per segment.
        montains[i].size = 1;

        // Random initial direction.
        montains[i].DIR = std::rand() % 8;

        /*
         * Peak thickness used near the middle of the chain.
         * Larger maps allow thicker mountain bodies.
         */
        montains[i].thickness = 3 + std::rand() % std::max(1, cfg.thickness_max);

        /*
         * Thin tips.
         * Usually 1 or 2 cells.
         */
        montains[i].tip_thickness = 1 + std::rand() % 2;

        /*
         * Intended chain length.
         * Larger maps create longer ranges.
         */
        montains[i].target_steps =
        cfg.min_montain_steps +
        std::rand() % std::max(1, cfg.max_montain_steps - cfg.min_montain_steps + 1);

        // Moderate turn chance.
        montains[i].turne_chance = 5 + std::rand() % std::max(1, cfg.turne_chance_max);

        // Small initial stop chance.
        montains[i].stop_chance = 1 + std::rand() % std::max(1, cfg.stop_chance_max);

        // Small lateral noise keeps the chain organic.
        montains[i].lateral_noise_chance = 10 + std::rand() % 20;
    }

    create_montain(map, montains, cfg);

    /*
     * ----------------------------
     * PART 3: RAVINES
     * ----------------------------
     */

    create_ravines(map, cfg);

    /*
     * ----------------------------
     * PART 4: RIVERS AND LAKES
     * ----------------------------
     */

    create_rivers(map, cfg);

    /*
     * ----------------------------
     * PART 5: FORESTS
     * ----------------------------
     */

    create_forests(map, cfg);
    create_scattered_trees(map, cfg);

    /*
     * ----------------------------
     * PART 6: BUSHES AND BERRIES
     * ----------------------------
     */

    create_bush_patches(map, cfg);
    create_scattered_bushes(map, cfg);
}
