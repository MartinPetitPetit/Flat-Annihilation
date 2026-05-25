#include "Map.hpp"

#include <cstdlib>

/*
 * ============================================================
 * FOREST AND TREE GENERATION
 * ============================================================
 */

bool can_place_tree(const MAP& map, int x, int y)
{
    if (!in_map(map, x, y)) {
        return false;
    }

    // Trees only on plain terrain.
    if (map[x][y].type_terrain != Plain) {
        return false;
    }

    // Avoid replacing resources.
    if (map[x][y].type_resource != None_Resource) {
        return false;
    }

    return true;
}

WOOD_TYPE choose_wood_type(const GenerationConfig& cfg)
{
    int roll = std::rand() % 100;

    if (roll < cfg.wood_type_c_chance) {
        return Wood_C;
    }

    if (roll < cfg.wood_type_c_chance + cfg.wood_type_b_chance) {
        return Wood_B;
    }

    return Wood_A;
}

void place_tree(MAP& map, int x, int y, WOOD_TYPE wood_type)
{
    if (!can_place_tree(map, x, y)) {
        return;
    }
	map[x][y].resource = new Resource(wood, 100, 0);
}

bool find_forest_center(const MAP& map, int& x, int& y, const GenerationConfig& cfg)
{
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    // Try random centers.
    for (int attempt = 0; attempt < cfg.forest_center_search_attempts; attempt++) {

        int rx = std::rand() % width;
        int ry = std::rand() % height;

        if (!can_place_tree(map, rx, ry)) {
            continue;
        }

        // Prefer forests near water.
        if (is_near_water(map, rx, ry, 5)) {
            if (std::rand() % 100 < 80) {
                x = rx;
                y = ry;
                return true;
            }
        }

        // Also allow dry forests.
        else {
            if (std::rand() % 100 < 35) {
                x = rx;
                y = ry;
                return true;
            }
        }
    }

    return false;
}

void paint_forest_patch(MAP& map, int cx, int cy, int radius, WOOD_TYPE wood_type, const GenerationConfig& cfg)
{
    int r2 = radius * radius;

    // Browse forest area.
    for (int x = cx - radius; x <= cx + radius; x++) {
        for (int y = cy - radius; y <= cy + radius; y++) {

            if (!can_place_tree(map, x, y)) {
                continue;
            }

            int dx = x - cx;
            int dy = y - cy;

            int dist2 = dx * dx + dy * dy;

            // Keep circular forest.
            if (dist2 > r2) {
                continue;
            }

            // Chance decreases from center to border.
            int chance =
            cfg.forest_edge_chance +
            (cfg.forest_core_chance - cfg.forest_edge_chance) * (r2 - dist2) / r2;

            // More trees near water.
            if (is_near_water(map, x, y, 3)) {
                chance += cfg.near_water_tree_bonus;
            }

            // Ravines reduce tree density.
            if (has_terrain_near(map, x, y, ravine, 2)) {
                chance -= cfg.near_ravine_tree_penalty;
            }

            if (chance < 0) {
                chance = 0;
            }

            if (chance > 100) {
                chance = 100;
            }

            if (std::rand() % 100 < chance) {
                place_tree(map, x, y, wood_type);
            }
        }
    }
}

void create_forests(MAP& map, const GenerationConfig& cfg)
{
    // Generate each forest patch.
    for (int i = 0; i < cfg.max_forest_quantity; i++) {

        int x = 0;
        int y = 0;

        if (!find_forest_center(map, x, y, cfg)) {
            return;
        }

        int radius =
        cfg.forest_min_radius +
        std::rand() % (cfg.forest_max_radius - cfg.forest_min_radius + 1);

        // One wood type per forest.
        WOOD_TYPE wood_type = choose_wood_type(cfg);

        paint_forest_patch(map, x, y, radius, wood_type, cfg);
    }
}

void create_scattered_trees(MAP& map, const GenerationConfig& cfg)
{
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    // Browse all map cells.
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {

            if (!can_place_tree(map, x, y)) {
                continue;
            }

            int chance = cfg.scattered_tree_chance;

            if (is_near_water(map, x, y, 4)) {
                chance += cfg.near_water_tree_bonus;
            }

            if (has_resource_near(map, x, y, tree, 3)) {
                chance += cfg.near_forest_tree_bonus;
            }

            if (has_terrain_near(map, x, y, ravine, 2)) {
                chance -= cfg.near_ravine_tree_penalty;
            }

            if (chance < 0) {
                chance = 0;
            }

            if (chance > 100) {
                chance = 100;
            }

            if (std::rand() % 100 < chance) {
                place_tree(map, x, y, choose_wood_type(cfg));
            }
        }
    }
}
