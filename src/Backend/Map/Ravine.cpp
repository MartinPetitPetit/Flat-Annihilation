#include "Map.hpp"

#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <vector>

/*
 * ============================================================
 * DIRECT CUT RAVINE GENERATION
 * ============================================================
 *
 * Goal:
 * - rare ravines
 * - long, narrow and directional
 * - looks like a direct terrain cut
 * - never makes a U-turn
 * - never uses thickness 1
 * - tips are thinner, center is wider
 * - random noise is applied to the border/width, not to the main direction
 * - if it touches a lake or river, the whole ravine is cancelled
 * - mountains do NOT block the ravine
 */

struct RavinePoint {
    int x;
    int y;
};

struct RavineProfile {
    int length;
    int tip_width;
    int center_width;
    int branch_chance;
    int branch_min_length;
    int branch_max_length;
};

static const RavinePoint ravine_dirs[8] = {
    {-1,  0}, // north
    {-1,  1}, // northeast
    { 0,  1}, // east
    { 1,  1}, // southeast
    { 1,  0}, // south
    { 1, -1}, // southwest
    { 0, -1}, // west
    {-1, -1}  // northwest
};

/*
 * ============================================================
 * HELPERS
 * ============================================================
 */

static int wrap_dir(int dir)
{
    while (dir < 0) {
        dir += 8;
    }

    while (dir >= 8) {
        dir -= 8;
    }

    return dir;
}

static int clamp_int_local(int value, int min_value, int max_value)
{
    if (value < min_value) {
        return min_value;
    }

    if (value > max_value) {
        return max_value;
    }

    return value;
}

static double rand01()
{
    return static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
}

static bool terrain_near_local(const MAP& map, int x, int y, TERRAIN terrain, int radius)
{
    for (int dx = -radius; dx <= radius; dx++) {
        for (int dy = -radius; dy <= radius; dy++) {
            int nx = x + dx;
            int ny = y + dy;

            if (!in_map(map, nx, ny)) {
                continue;
            }

            if (map[nx][ny].type_terrain == terrain) {
                return true;
            }
        }
    }

    return false;
}

static bool is_good_ravine_source(const MAP& map, int x, int y)
{
    if (!in_map(map, x, y)) {
        return false;
    }

    if (map[x][y].type_terrain != Plain) {
        return false;
    }

    if (terrain_near_local(map, x, y, River, 8)) {
        return false;
    }

    if (terrain_near_local(map, x, y, Lake, 12)) {
        return false;
    }

    if (terrain_near_local(map, x, y, ravine, 22)) {
        return false;
    }

    return true;
}

static bool centerline_can_continue(const MAP& map, int x, int y)
{
    if (!in_map(map, x, y)) {
        return false;
    }

    TERRAIN t = map[x][y].type_terrain;

    if (t == Lake) {
        return false;
    }

    if (t == River) {
        return false;
    }

    if (t == ravine) {
        return false;
    }

    // Mountain is allowed: ravine cuts through it.
    return true;
}

static bool can_paint_ravine_cell(const MAP& map, int x, int y)
{
    if (!in_map(map, x, y)) {
        return false;
    }

    TERRAIN t = map[x][y].type_terrain;

    if (t == Lake) {
        return false;
    }

    if (t == River) {
        return false;
    }

    // Mountain is allowed.
    return true;
}

static void paint_ravine_cell(MAP& map, int x, int y)
{
    if (!can_paint_ravine_cell(map, x, y)) {
        return;
    }

    set_terrain(map, x, y, ravine);
    map[x][y].type_resource = None_Resource;
    map[x][y].has_berry = false;
}

static RavineProfile choose_ravine_profile(const MAP& map)
{
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());
    int min_dim = std::min(width, height);
    int max_dim = std::max(width, height);

    RavineProfile profile;

    profile.length =
    clamp_int_local(
        static_cast<int>(min_dim * (0.24 + rand01() * 0.12)),
                    40,
                    max_dim
    );

    /*
     * Never use thickness 1.
     * The ravine starts and ends with 2 or 3.
     */
    profile.tip_width = 2 + (std::rand() % 2);

    profile.center_width =
    clamp_int_local(
        static_cast<int>(min_dim * (0.010 + rand01() * 0.004)),
                    4,
                    7
    );

    if (profile.center_width <= profile.tip_width) {
        profile.center_width = profile.tip_width + 2;
    }

    /*
     * Branches almost disabled to preserve direct cut shape.
     */
    profile.branch_chance = 0;
    profile.branch_min_length = 3;
    profile.branch_max_length = 5;

    return profile;
}

static int width_for_progress(const RavineProfile& profile, double progress)
{
    const double PI = 3.14159265358979323846;

    double factor = std::sin(progress * PI);

    if (factor < 0.0) {
        factor = 0.0;
    }

    int width =
    profile.tip_width +
    static_cast<int>((profile.center_width - profile.tip_width) * factor);

    if (width < 2) {
        width = 2;
    }

    return width;
}

static int local_dir_from_points(const RavinePoint& a, const RavinePoint& b, int fallback_dir)
{
    int dx = b.x - a.x;
    int dy = b.y - a.y;

    for (int d = 0; d < 8; d++) {
        if (ravine_dirs[d].x == dx && ravine_dirs[d].y == dy) {
            return d;
        }
    }

    return fallback_dir;
}

/*
 * ============================================================
 * PATH BUILDING
 * ============================================================
 *
 * The centerline is mostly direct.
 * Randomness is sparse and local:
 * - most steps go in base_dir
 * - sometimes one step goes base_dir - 1 or base_dir + 1
 * - there is no accumulated drift, so it does not spiral or zigzag wildly
 */

static bool build_ravine_path(
    const MAP& map,
    int start_x,
    int start_y,
    int base_dir,
    const RavineProfile& profile,
    std::vector<RavinePoint>& out_path
)
{
    out_path.clear();

    int current_x = start_x;
    int current_y = start_y;

    for (int step = 0; step < profile.length; step++) {
        if (!centerline_can_continue(map, current_x, current_y)) {
            return false;
        }

        out_path.push_back({current_x, current_y});

        int local_dir = base_dir;

        /*
         * Random noise in the path, but not persistent.
         * This creates a direct cut with small irregularity.
         */
        int roll = std::rand() % 100;

        if (roll < 12) {
            local_dir = wrap_dir(base_dir - 1);
        }
        else if (roll < 24) {
            local_dir = wrap_dir(base_dir + 1);
        }

        int next_x = current_x + ravine_dirs[local_dir].x;
        int next_y = current_y + ravine_dirs[local_dir].y;

        if (!in_map(map, next_x, next_y)) {
            break;
        }

        TERRAIN next_t = map[next_x][next_y].type_terrain;

        /*
         * If the ravine would touch water, cancel the whole ravine.
         */
        if (next_t == Lake || next_t == River) {
            return false;
        }

        if (next_t == ravine) {
            break;
        }

        current_x = next_x;
        current_y = next_y;
    }

    if (static_cast<int>(out_path.size()) < std::max(20, profile.length / 2)) {
        return false;
    }

    return true;
}

/*
 * ============================================================
 * PUBLIC SOURCE SEARCH
 * ============================================================
 */

bool find_ravine_source(const MAP& map, int& x, int& y, int& dir, const GenerationConfig& cfg)
{
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    for (int attempt = 0; attempt < cfg.ravine_source_attempts; attempt++) {
        int rx = std::rand() % width;
        int ry = std::rand() % height;

        if (!is_good_ravine_source(map, rx, ry)) {
            continue;
        }

        x = rx;
        y = ry;

        /*
         * Use readable grid directions.
         */
        int preferred_dirs[8] = {0, 2, 4, 6, 1, 3, 5, 7};
        dir = preferred_dirs[std::rand() % 8];

        return true;
    }

    return false;
}

/*
 * ============================================================
 * CREATE RAVINES
 * ============================================================
 */

void create_ravines(MAP& map, const GenerationConfig& cfg)
{
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());
    int min_dim = std::min(width, height);

    int desired_ravines = 0;

    if (std::rand() % 100 < 38) {
        desired_ravines = 1;
    }

    if (min_dim >= 500 && std::rand() % 100 < 18) {
        desired_ravines = 2;
    }

    desired_ravines = std::min(desired_ravines, cfg.max_ravine_quantity);

    for (int i = 0; i < desired_ravines; i++) {
        int x = 0;
        int y = 0;
        int dir = 0;

        if (!find_ravine_source(map, x, y, dir, cfg)) {
            continue;
        }

        draw_ravine(map, x, y, dir, cfg);
    }
}

/*
 * ============================================================
 * MAIN RAVINE
 * ============================================================
 */

void draw_ravine(MAP& map, int x, int y, int dir, const GenerationConfig& cfg)
{
    (void)cfg;

    RavineProfile profile = choose_ravine_profile(map);

    std::vector<RavinePoint> path;

    if (!build_ravine_path(map, x, y, dir, profile, path)) {
        return;
    }

    /*
     * Validate the whole brush against water before painting.
     * If any part of the future cut touches a lake or river, cancel it.
     */
    for (int i = 0; i < static_cast<int>(path.size()); i++) {
        double progress =
        static_cast<double>(i) /
        static_cast<double>(std::max(1, static_cast<int>(path.size()) - 1));

        int width_here = width_for_progress(profile, progress);


        int bound = std::max(2, width_here);

        for (int ox = -bound; ox <= bound; ox++) {
            for (int oy = -bound; oy <= bound; oy++) {
                int tx = path[i].x + ox;
                int ty = path[i].y + oy;

                if (!in_map(map, tx, ty)) {
                    continue;
                }

                if (map[tx][ty].type_terrain == Lake || map[tx][ty].type_terrain == River) {
                    return;
                }
            }
        }
    }

    for (int i = 0; i < static_cast<int>(path.size()); i++) {
        double progress =
        static_cast<double>(i) /
        static_cast<double>(std::max(1, static_cast<int>(path.size()) - 1));

        int width_here = width_for_progress(profile, progress);

        int local_dir = dir;

        if (i + 1 < static_cast<int>(path.size())) {
            local_dir = local_dir_from_points(path[i], path[i + 1], dir);
        }
        else if (i > 0) {
            local_dir = local_dir_from_points(path[i - 1], path[i], dir);
        }

        paint_ravine_tear_brush(map, path[i].x, path[i].y, local_dir, width_here);
    }
}

/*
 * ============================================================
 * BRUSH
 * ============================================================
 *
 * Random noise is placed on the border, not on the centerline.
 * This preserves the direct cut quality while avoiding perfect straight edges.
 */

void paint_ravine_tear_brush(MAP& map, int cx, int cy, int dir, int width)
{
    double vx = static_cast<double>(ravine_dirs[dir].x);
    double vy = static_cast<double>(ravine_dirs[dir].y);

    double len = std::sqrt(vx * vx + vy * vy);

    if (len == 0.0) {
        len = 1.0;
    }

    vx /= len;
    vy /= len;

    double radius = std::max(1.0, static_cast<double>(width) * 0.45);
    int bound = std::max(2, width);

    for (int ox = -bound; ox <= bound; ox++) {
        for (int oy = -bound; oy <= bound; oy++) {
            int x = cx + ox;
            int y = cy + oy;

            if (!can_paint_ravine_cell(map, x, y)) {
                continue;
            }

            double along = std::abs(vx * ox + vy * oy);
            double perp  = std::abs(-vy * ox + vx * oy);

            if (along > 1.18) {
                continue;
            }

            /*
             * Random edge roughness.
             * The core is always painted.
             * The edge is slightly irregular.
             */
            double random_edge = (rand01() - 0.5) * 0.35;
            double effective_radius = radius + random_edge;

            if (perp > effective_radius) {
                continue;
            }

            paint_ravine_cell(map, x, y);
        }
    }
}

/*
 * ============================================================
 * BRANCHES
 * ============================================================
 *
 * Kept for Map.hpp compatibility, but create_ravines uses branch_chance = 0.
 */

void draw_ravine_branch(MAP& map, int x, int y, int dir, int length)
{
    int current_x = x;
    int current_y = y;
    int current_dir = wrap_dir(dir);

    for (int step = 0; step < length; step++) {
        current_x += ravine_dirs[current_dir].x;
        current_y += ravine_dirs[current_dir].y;

        if (!in_map(map, current_x, current_y)) {
            break;
        }

        TERRAIN t = map[current_x][current_y].type_terrain;

        if (t == Lake || t == River || t == ravine) {
            break;
        }

        paint_ravine_tear_brush(map, current_x, current_y, current_dir, 2);
    }
}
