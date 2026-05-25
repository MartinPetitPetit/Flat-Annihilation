#include "Map.hpp"

#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>



/*
 * ============================================================
 * ORGANIC MOUNTAIN RANGE GENERATION WITH NATURAL JUNCTIONS
 * ============================================================
 *
 * Goals:
 * - mountain chains evolve with organic noise;
 * - no sudden 90-degree turns;
 * - direction changes happen gradually;
 * - chains may form broad curves and U-shapes;
 * - tips are thinner, center is thicker;
 * - when a range approaches an existing range, it merges naturally
 *   instead of crossing abruptly or creating a hard block;
 * - junctions become slightly wider and smoother.
 *
 * Public compatibility kept with Map.hpp:
 * - create_montain(...)
 * - paint_mountain_brush(...)
 */

struct MountainPoint {
    int x;
    int y;
};

struct MountainProfile {
    int length;
    int tip_thickness;
    int center_thickness;
    int heading_change_interval_min;
    int heading_change_interval_max;
    double max_turn_step_rad;
    int junction_search_radius;
};

using MountainMask = std::vector<std::vector<unsigned char>>;

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

static double rand_range(double a, double b)
{
    return a + rand01() * (b - a);
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

static bool is_good_mountain_start(const MAP& map, int x, int y)
{
    if (!in_map(map, x, y)) {
        return false;
    }

    if (map[x][y].type_terrain != Plain) {
        return false;
    }

    if (terrain_near_local(map, x, y, Montain, 10)) {
        return false;
    }

    if (terrain_near_local(map, x, y, Lake, 5)) {
        return false;
    }

    if (terrain_near_local(map, x, y, River, 5)) {
        return false;
    }

    if (terrain_near_local(map, x, y, ravine, 4)) {
        return false;
    }

    return true;
}

static bool choose_mountain_start_in_region(
    const MAP& map,
    int region_x0,
    int region_y0,
    int region_x1,
    int region_y1,
    int& out_x,
    int& out_y
) {
    int region_width = std::max(1, region_x1 - region_x0);
    int region_height = std::max(1, region_y1 - region_y0);

    for (int attempt = 0; attempt < 80; attempt++) {
        int x = region_x0 + std::rand() % region_width;
        int y = region_y0 + std::rand() % region_height;

        if (is_good_mountain_start(map, x, y)) {
            out_x = x;
            out_y = y;
            return true;
        }
    }

    return false;
}

static double normalize_angle(double angle)
{
    const double PI = 3.14159265358979323846;
    const double TWO_PI = 2.0 * PI;

    while (angle <= -PI) {
        angle += TWO_PI;
    }

    while (angle > PI) {
        angle -= TWO_PI;
    }

    return angle;
}

static int angle_to_dir_index(double angle)
{
    const double PI = 3.14159265358979323846;
    const double TWO_PI = 2.0 * PI;

    while (angle < 0.0) {
        angle += TWO_PI;
    }

    while (angle >= TWO_PI) {
        angle -= TWO_PI;
    }

    int index = static_cast<int>(std::floor((angle / TWO_PI) * 8.0 + 0.5)) % 8;
    return index;
}

static double choose_base_heading_away_from_border(const MAP& map, int x, int y)
{
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    double center_x = static_cast<double>(width - 1) * 0.5;
    double center_y = static_cast<double>(height - 1) * 0.5;

    double dx = center_x - static_cast<double>(x);
    double dy = center_y - static_cast<double>(y);

    double angle_to_center = std::atan2(dy, dx);

    double offset = rand_range(-1.2, 1.2);

    return angle_to_center + offset;
}

static MountainProfile choose_mountain_profile(const MAP& map, const GenerationConfig& cfg)
{
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());
    int min_dim = std::min(width, height);
    int max_dim = std::max(width, height);

    MountainProfile profile;

    int min_len = std::max(cfg.min_montain_steps, static_cast<int>(min_dim * 0.14));
    int max_len = std::max(min_len + 1, static_cast<int>(min_dim * (0.30 + rand01() * 0.18)));

    max_len = clamp_int_local(max_len, min_len, max_dim);

    profile.length = min_len + std::rand() % std::max(1, max_len - min_len + 1);

    profile.tip_thickness = clamp_int_local(1 + std::rand() % 2, 1, 3);

    profile.center_thickness = clamp_int_local(
        static_cast<int>(cfg.thickness_max * (0.80 + rand01() * 0.30)),
                                               profile.tip_thickness + 2,
                                               std::max(profile.tip_thickness + 2, cfg.thickness_max)
    );

    profile.heading_change_interval_min = 4;
    profile.heading_change_interval_max = 10;

    /*
     * About 20 to 30 degrees per update.
     */
    profile.max_turn_step_rad = rand_range(0.35, 0.52);

    /*
     * How far the range can detect a previous mountain range and merge into it.
     */
    profile.junction_search_radius = clamp_int_local(profile.center_thickness + 6, 8, 18);

    return profile;
}

static int thickness_for_progress(const MountainProfile& profile, double progress)
{
    const double PI = 3.14159265358979323846;

    double factor = std::sin(progress * PI);

    if (factor < 0.0) {
        factor = 0.0;
    }

    int thickness =
    profile.tip_thickness +
    static_cast<int>((profile.center_thickness - profile.tip_thickness) * factor);

    if (thickness < 1) {
        thickness = 1;
    }

    return thickness;
}

static MountainMask snapshot_existing_mountains(const MAP& map)
{
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    MountainMask mask(width, std::vector<unsigned char>(height, 0));

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (map[x][y].type_terrain == Montain) {
                mask[x][y] = 1;
            }
        }
    }

    return mask;
}

static bool find_existing_mountain_near(
    const MAP& map,
    const MountainMask& existing,
    int cx,
    int cy,
    int radius,
    int& out_x,
    int& out_y
) {
    int best_dist2 = 99999999;
    bool found = false;

    for (int dx = -radius; dx <= radius; dx++) {
        for (int dy = -radius; dy <= radius; dy++) {
            int x = cx + dx;
            int y = cy + dy;

            if (!in_map(map, x, y)) {
                continue;
            }

            if (!existing[x][y]) {
                continue;
            }

            int dist2 = dx * dx + dy * dy;

            if (dist2 < best_dist2) {
                best_dist2 = dist2;
                out_x = x;
                out_y = y;
                found = true;
            }
        }
    }

    return found;
}

static void paint_mountain_junction(
    MAP& map,
    int from_x,
    int from_y,
    int to_x,
    int to_y,
    int thickness
) {
    double dx = static_cast<double>(to_x - from_x);
    double dy = static_cast<double>(to_y - from_y);

    double dist = std::sqrt(dx * dx + dy * dy);

    if (dist < 1.0) {
        dist = 1.0;
    }

    int steps = static_cast<int>(std::ceil(dist));

    for (int i = 0; i <= steps; i++) {
        double t = static_cast<double>(i) / static_cast<double>(std::max(1, steps));

        int x = static_cast<int>(std::round(static_cast<double>(from_x) + dx * t));
        int y = static_cast<int>(std::round(static_cast<double>(from_y) + dy * t));

        /*
         * Slightly thicker in the middle of the connection.
         */
        double bulge = std::sin(t * 3.14159265358979323846);
        int local_thickness = thickness + static_cast<int>(bulge * 2.0);

        paint_mountain_brush(map, x, y, local_thickness);
    }

    /*
     * Soft node at the actual meeting point.
     * Not too big, only enough to avoid a hard seam.
     */
    paint_mountain_brush(map, to_x, to_y, thickness + 1);
}

/*
 * ============================================================
 * PUBLIC GENERATION
 * ============================================================
 */

void create_montain(MAP& map, std::vector<MONTAIN>& montains, const GenerationConfig& cfg)
{
    montains.clear();

    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    int desired = cfg.max_montain_quantity;

    if (desired < 1) {
        return;
    }

    int grid_cols = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(desired))));
    int grid_rows = static_cast<int>(std::ceil(static_cast<double>(desired) / static_cast<double>(grid_cols)));

    std::vector<int> region_ids;
    for (int i = 0; i < grid_rows * grid_cols; i++) {
        region_ids.push_back(i);
    }

    for (int i = static_cast<int>(region_ids.size()) - 1; i > 0; i--) {
        int j = std::rand() % (i + 1);
        std::swap(region_ids[i], region_ids[j]);
    }

    int created = 0;

    for (int index = 0; index < static_cast<int>(region_ids.size()) && created < desired; index++) {
        int region_id = region_ids[index];
        int row = region_id / grid_cols;
        int col = region_id % grid_cols;

        int x0 = (width * row) / grid_rows;
        int x1 = (width * (row + 1)) / grid_rows;

        int y0 = (height * col) / grid_cols;
        int y1 = (height * (col + 1)) / grid_cols;

        int sx = 0;
        int sy = 0;

        if (!choose_mountain_start_in_region(map, x0, y0, x1, y1, sx, sy)) {
            continue;
        }

        /*
         * Snapshot only the mountains that already existed before this new chain.
         * This prevents the current chain from detecting itself as a junction.
         */
        MountainMask existing_mountains = snapshot_existing_mountains(map);

        double base_heading = choose_base_heading_away_from_border(map, sx, sy);

        MONTAIN m;
        m.x_init = sx;
        m.y_init = sy;
        m.size = cfg.max_montain_size;
        m.DIR = angle_to_dir_index(base_heading);
        m.thickness = cfg.thickness_max;
        m.tip_thickness = 1;
        m.target_steps = cfg.max_montain_steps;
        m.turne_chance = cfg.turne_chance_max;
        m.stop_chance = static_cast<float>(cfg.stop_chance_max);
        m.lateral_noise_chance = 20;

        montains.push_back(m);

        MountainProfile profile = choose_mountain_profile(map, cfg);

        double current_heading = base_heading;
        double target_heading = base_heading;

        int change_timer =
        profile.heading_change_interval_min +
        std::rand() %
        std::max(1, profile.heading_change_interval_max - profile.heading_change_interval_min + 1);

        double fx = static_cast<double>(sx);
        double fy = static_cast<double>(sy);

        int last_ix = sx;
        int last_iy = sy;

        bool merging = false;
        int merge_x = 0;
        int merge_y = 0;

        int min_merge_step = std::max(12, profile.length / 4);

        for (int step = 0; step < profile.length; step++) {
            if (!in_map(map, last_ix, last_iy)) {
                break;
            }

            double progress =
            static_cast<double>(step) /
            static_cast<double>(std::max(1, profile.length - 1));

            int thickness = thickness_for_progress(profile, progress);

            paint_mountain_brush(map, last_ix, last_iy, thickness);

            /*
             * Detect previous mountain ranges and begin a controlled merge.
             */
            if (!merging && step > min_merge_step) {
                if (find_existing_mountain_near(
                    map,
                    existing_mountains,
                    last_ix,
                    last_iy,
                    profile.junction_search_radius,
                    merge_x,
                    merge_y
                )) {
                    merging = true;
                }
            }

            if (merging) {
                double dx = static_cast<double>(merge_x) - fx;
                double dy = static_cast<double>(merge_y) - fy;
                double dist = std::sqrt(dx * dx + dy * dy);

                if (dist <= static_cast<double>(std::max(2, thickness / 2 + 2))) {
                    paint_mountain_junction(map, last_ix, last_iy, merge_x, merge_y, thickness + 1);
                    break;
                }

                /*
                 * Steer gently toward the existing range.
                 * This creates a natural junction instead of a crossing.
                 */
                target_heading = std::atan2(dy, dx);
            }
            else {
                change_timer--;

                if (change_timer <= 0) {
                    double local_turn = rand_range(-0.55, 0.55);
                    target_heading = current_heading + local_turn;

                    change_timer =
                    profile.heading_change_interval_min +
                    std::rand() %
                    std::max(1, profile.heading_change_interval_max - profile.heading_change_interval_min + 1);
                }
            }

            double delta = normalize_angle(target_heading - current_heading);

            if (delta > profile.max_turn_step_rad) {
                delta = profile.max_turn_step_rad;
            }
            else if (delta < -profile.max_turn_step_rad) {
                delta = -profile.max_turn_step_rad;
            }

            current_heading = normalize_angle(current_heading + delta);

            if (!merging) {
                current_heading = normalize_angle(current_heading + rand_range(-0.06, 0.06));
            }
            else {
                /*
                 * Less noise while merging, so the connection stays clean.
                 */
                current_heading = normalize_angle(current_heading + rand_range(-0.025, 0.025));
            }

            fx += std::cos(current_heading);
            fy += std::sin(current_heading);

            int nx = static_cast<int>(std::round(fx));
            int ny = static_cast<int>(std::round(fy));

            if (!in_map(map, nx, ny)) {
                break;
            }

            if (nx == last_ix && ny == last_iy) {
                continue;
            }

            /*
             * If the new cell is already a previous mountain, finalize a junction.
             */
            if (existing_mountains[nx][ny] && step > min_merge_step) {
                paint_mountain_junction(map, last_ix, last_iy, nx, ny, thickness + 1);
                break;
            }

            last_ix = nx;
            last_iy = ny;
        }

        created++;
    }
}

/*
 * ============================================================
 * MOUNTAIN BRUSH
 * ============================================================
 */

void paint_mountain_brush(MAP& map, int cx, int cy, int thickness)
{
    int radius = thickness / 2;

    if (radius < 0) {
        radius = 0;
    }

    for (int dx = -radius; dx <= radius; dx++) {
        for (int dy = -radius; dy <= radius; dy++) {
            int x = cx + dx;
            int y = cy + dy;

            if (!in_map(map, x, y)) {
                continue;
            }

            int dist2 = dx * dx + dy * dy;
            int radius2 = radius * radius;

            bool should_paint = false;

            if (dx == 0 && dy == 0) {
                should_paint = true;
            }
            else if (radius == 0) {
                should_paint = false;
            }
            else if (dist2 <= radius2) {
                should_paint = true;
            }
            else if (dist2 <= radius2 + 1) {
                should_paint = (std::rand() % 100 < 40);
            }

            if (!should_paint) {
                continue;
            }

            TERRAIN t = map[x][y].type_terrain;

            if (t == River || t == Lake || t == ravine) {
                continue;
            }

            set_terrain(map, x, y, Montain);
            map[x][y].resource = nullptr;
            map[x][y].has_berry = false;
        }
    }
}
