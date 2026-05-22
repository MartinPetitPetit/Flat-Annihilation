#include "Map.hpp"

#include <cstdlib>

/*
 * ============================================================
 * BUSH AND BERRY GENERATION
 * ============================================================
 */

bool can_place_bush(const MAP& map, int x, int y)
{
    if (!in_map(map, x, y)) {
        return false;
    }

    // Bushes only on plain terrain.
    if (map[x][y].type_terrain != Plain) {
        return false;
    }

    // Avoid replacing resources.
    if (map[x][y].type_resource != None_Resource) {
        return false;
    }

    return true;
}

void place_bush(MAP& map, int x, int y, int berry_chance)
{
    if (!can_place_bush(map, x, y)) {
        return;
    }

    // The resource is a bush.
    map[x][y].type_resource = BushResource;

    // Berry is a bush state, not a separate resource.
    map[x][y].has_berry = (std::rand() % 100 < berry_chance);

    // Bushes are not trees.
    map[x][y].wood_type = No_Wood;
}

bool find_bush_center(const MAP& map, int& x, int& y, const GenerationConfig& cfg)
{
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    // Try random centers.
    for (int attempt = 0; attempt < cfg.bush_center_search_attempts; attempt++) {

        int rx = std::rand() % width;
        int ry = std::rand() % height;

        if (!can_place_bush(map, rx, ry)) {
            continue;
        }

        // Bush patches are more likely near water.
        if (is_near_water(map, rx, ry, 4)) {
            if (std::rand() % 100 < 75) {
                x = rx;
                y = ry;
                return true;
            }
        }

        // Dry bush patches are still possible.
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

void paint_bush_patch(MAP& map, int cx, int cy, int radius, const GenerationConfig& cfg)
{
    int r2 = radius * radius;

    // Browse bush patch area.
    for (int x = cx - radius; x <= cx + radius; x++) {
        for (int y = cy - radius; y <= cy + radius; y++) {

            if (!can_place_bush(map, x, y)) {
                continue;
            }

            int dx = x - cx;
            int dy = y - cy;

            int dist2 = dx * dx + dy * dy;

            // Keep circular patch.
            if (dist2 > r2) {
                continue;
            }

            // Chance decreases from center to border.
            int chance =
            cfg.bush_edge_chance +
            (cfg.bush_core_chance - cfg.bush_edge_chance) * (r2 - dist2) / r2;

            // Bushes grow better near water.
            if (is_near_water(map, x, y, 3)) {
                chance += cfg.near_water_bush_bonus;
            }

            // Ravines reduce bush density.
            if (has_terrain_near(map, x, y, ravine, 2)) {
                chance -= cfg.near_ravine_bush_penalty;
            }

            if (chance < 0) {
                chance = 0;
            }

            if (chance > 100) {
                chance = 100;
            }

            if (std::rand() % 100 < chance) {
                place_bush(map, x, y, cfg.dense_bush_berry_chance);
            }
        }
    }
}

void create_bush_patches(MAP& map, const GenerationConfig& cfg)
{
    // Generate dense bush patches.
    for (int i = 0; i < cfg.max_bush_patch_quantity; i++) {

        int x = 0;
        int y = 0;

        if (!find_bush_center(map, x, y, cfg)) {
            return;
        }

        int radius =
        cfg.bush_min_radius +
        std::rand() % (cfg.bush_max_radius - cfg.bush_min_radius + 1);

        paint_bush_patch(map, x, y, radius, cfg);
    }
}

void create_scattered_bushes(MAP& map, const GenerationConfig& cfg)
{
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    // Browse all map cells.
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {

            if (!can_place_bush(map, x, y)) {
                continue;
            }

            int chance = cfg.scattered_bush_chance;

            if (is_near_water(map, x, y, 4)) {
                chance += cfg.near_water_bush_bonus;
            }

            if (has_resource_near(map, x, y, BushResource, 3)) {
                chance += 2;
            }

            if (has_terrain_near(map, x, y, ravine, 2)) {
                chance -= cfg.near_ravine_bush_penalty;
            }

            if (chance < 0) {
                chance = 0;
            }

            if (chance > 100) {
                chance = 100;
            }

            if (std::rand() % 100 < chance) {
                place_bush(map, x, y, cfg.scattered_bush_berry_chance);
            }
        }
    }
}
