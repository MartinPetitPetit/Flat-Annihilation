#include "Map.hpp"

#include <cstdlib>
#include <cmath>
#include <utility>
#include <vector>
#include <algorithm>
#include <queue>
#include <limits>

/*
 * ============================================================
 * ORGANIC SPRING-TO-RIVER NETWORK GENERATION
 * ============================================================
 *
 * Expected behavior:
 * - Every mountain can create springs.
 * - The number of springs is proportional to the size of the mountain.
 * - Springs start on the face of the mountain that looks toward another mountain.
 * - Springs grow organically.
 * - Springs prefer valley/midline cells between mountains, not straight global diagonals.
 * - Rivers stop when they hit ravines.
 * - Rivers join when they hit existing rivers or lakes.
 * - When a spring joins an existing river, its flow continues downstream and increases thickness.
 *
 * Important:
 * - This version does not create terminal lakes automatically.
 * - To make rivers stop at ravines, call create_ravines(map, cfg) before create_rivers(map, cfg).
 */

struct RiverPoint {
    int x;
    int y;
};

struct MountainComponent {
    std::vector<RiverPoint> cells;
    std::vector<RiverPoint> border_cells;
    double cx;
    double cy;
    int size;
    int face;
};

struct SourceNode {
    RiverPoint p;
    int mountain_id;
    int face;
};

struct RiverPath {
    std::vector<RiverPoint> cells;
    int join_path;
    int join_index;
    bool joins_existing;
    bool hit_ravine;
    bool hit_water;
    bool reached_source;
};

struct ValleyCell {
    int first_mountain;
    int second_mountain;
    float first_dist;
    float second_dist;
    float score;
};


struct LakeComponent {
    std::vector<RiverPoint> cells;
    std::vector<RiverPoint> border_cells;
    double cx;
    double cy;
    int size;
};

struct QueueNode {
    float d;
    int x;
    int y;
    int mountain_id;
};

struct QueueNodeCompare {
    bool operator()(const QueueNode& a, const QueueNode& b) const
    {
        return a.d > b.d;
    }
};

using ValleyField = std::vector<std::vector<ValleyCell>>;

static const RiverPoint river_dirs[8] = {
    {-1,  0}, // 0 north
    {-1,  1}, // 1 northeast
    { 0,  1}, // 2 east
    { 1,  1}, // 3 southeast
    { 1,  0}, // 4 south
    { 1, -1}, // 5 southwest
    { 0, -1}, // 6 west
    {-1, -1}  // 7 northwest
};

/*
 * ============================================================
 * PRIVATE DECLARATIONS
 * ============================================================
 */

static double rand01();
static int clamp_int_local(int value, int min_value, int max_value);
static int wrap_dir(int dir);
static double dist2_int(int ax, int ay, int bx, int by);
static double dist_int(int ax, int ay, int bx, int by);
static int direction_toward(const RiverPoint& from, const RiverPoint& to);

static bool terrain_near_local(const MAP& map, int x, int y, TERRAIN terrain, int radius);
static bool point_in_vector(const std::vector<RiverPoint>& points, int x, int y);
static bool point_in_path(const std::vector<RiverPoint>& path, int x, int y);
static bool source_near_used(const std::vector<SourceNode>& sources, int x, int y, int radius);

static bool can_step_river(const MAP& map, int x, int y);
static bool is_terminal_water(const MAP& map, int x, int y);
static void clear_water_cell(MAP& map, int x, int y);

static std::vector<MountainComponent> measure_mountains(const MAP& map);
static int side_of_point_relative_to_mountain(const MountainComponent& mountain, const RiverPoint& p);
static int choose_face_toward_nearest_mountain(
    const std::vector<MountainComponent>& mountains,
    int mountain_id
);
static std::vector<RiverPoint> candidates_on_face(const MountainComponent& mountain, int face);
static int source_count_for_mountain(const MountainComponent& mountain, const MAP& map);
static std::vector<SourceNode> build_sources_from_mountains(
    const MAP& map,
    std::vector<MountainComponent>& mountains
);

static ValleyField build_valley_field(
    const MAP& map,
    const std::vector<MountainComponent>& mountains
);

static bool accept_distance_for_cell(
    ValleyCell& cell,
    int mountain_id,
    float d
);

static int choose_first_source_index(const std::vector<SourceNode>& sources);
static int find_nearest_source_from_other_mountain(
    const std::vector<SourceNode>& sources,
    int source_index,
    double max_distance
);

static int find_nearest_network_point_index(
    const std::vector<RiverPoint>& network_cells,
    const RiverPoint& source,
    double max_distance
);

static bool find_nearby_network_join(
    const MAP& map,
    const std::vector<std::vector<int>>& network_path,
    const std::vector<std::vector<int>>& network_index,
    int cx,
    int cy,
    int radius,
    int& out_path,
    int& out_index,
    RiverPoint& out_point
);

static bool append_y_connector_to_join(
    const MAP& map,
    const std::vector<std::vector<int>>& network_path,
    const std::vector<std::vector<int>>& network_index,
    std::vector<RiverPoint>& cells,
    const RiverPoint& join_point,
    int& out_path,
    int& out_index
);

static RiverPath grow_organic_river(
    const MAP& map,
    const ValleyField& valley,
    const std::vector<SourceNode>& sources,
    int source_index,
    const RiverPoint& target,
    const std::vector<std::vector<int>>& network_path,
    const std::vector<std::vector<int>>& network_index,
    bool allow_network_join,
    const GenerationConfig& cfg
);

static void register_path_in_network(
    const MAP& map,
    const RiverPath& path,
    int path_id,
    std::vector<std::vector<int>>& network_path,
    std::vector<std::vector<int>>& network_index,
    std::vector<RiverPoint>& network_cells
);

static std::vector<RiverPoint> build_full_downstream_path(
    const std::vector<RiverPath>& paths,
    int path_id
);

static void add_flow_to_path(
    const std::vector<RiverPoint>& path,
    std::vector<std::vector<double>>& flow
);

static void boost_confluence_and_sources(
    const std::vector<RiverPath>& paths,
    std::vector<std::vector<double>>& flow
);

static int river_thickness_from_flow(double flow_value, const GenerationConfig& cfg);
static void paint_river_network(MAP& map, const std::vector<std::vector<double>>& flow, const GenerationConfig& cfg);

static std::vector<LakeComponent> measure_lakes(const MAP& map);

static bool can_lake_connector_step(const MAP& map, int x, int y);

static bool choose_best_lake_border_pair(
    const LakeComponent& from_lake,
    const LakeComponent& to_lake,
    RiverPoint& out_from,
    RiverPoint& out_to
);

static bool build_lake_connector_path(
    const MAP& map,
    const RiverPoint& from,
    const RiverPoint& to,
    std::vector<RiverPoint>& connector,
    const GenerationConfig& cfg
);

static void paint_lake_connector(
    MAP& map,
    const std::vector<RiverPoint>& connector,
    int thickness
);

static void connect_lakes_after_creation(MAP& map, const GenerationConfig& cfg);

int paint_river_brush(MAP& map, int cx, int cy, int thickness);

/*
 * ============================================================
 * BASIC HELPERS
 * ============================================================
 */

static double rand01()
{
    return static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
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

static double dist2_int(int ax, int ay, int bx, int by)
{
    double dx = static_cast<double>(ax - bx);
    double dy = static_cast<double>(ay - by);

    return dx * dx + dy * dy;
}

static double dist_int(int ax, int ay, int bx, int by)
{
    return std::sqrt(dist2_int(ax, ay, bx, by));
}

static int direction_toward(const RiverPoint& from, const RiverPoint& to)
{
    double vx = static_cast<double>(to.x - from.x);
    double vy = static_cast<double>(to.y - from.y);

    double len = std::sqrt(vx * vx + vy * vy);

    if (len < 0.0001) {
        return std::rand() % 8;
    }

    vx /= len;
    vy /= len;

    double best_dot = -999999.0;
    int best_dir = 0;

    for (int d = 0; d < 8; d++) {
        double dx = static_cast<double>(river_dirs[d].x);
        double dy = static_cast<double>(river_dirs[d].y);

        double dlen = std::sqrt(dx * dx + dy * dy);

        if (dlen < 0.0001) {
            continue;
        }

        dx /= dlen;
        dy /= dlen;

        double dot = vx * dx + vy * dy;

        if (dot > best_dot) {
            best_dot = dot;
            best_dir = d;
        }
    }

    return best_dir;
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

static bool point_in_vector(const std::vector<RiverPoint>& points, int x, int y)
{
    for (const RiverPoint& p : points) {
        if (p.x == x && p.y == y) {
            return true;
        }
    }

    return false;
}

static bool point_in_path(const std::vector<RiverPoint>& path, int x, int y)
{
    for (const RiverPoint& p : path) {
        if (p.x == x && p.y == y) {
            return true;
        }
    }

    return false;
}

static bool source_near_used(const std::vector<SourceNode>& sources, int x, int y, int radius)
{
    int radius2 = radius * radius;

    for (const SourceNode& s : sources) {
        int dx = s.p.x - x;
        int dy = s.p.y - y;

        if (dx * dx + dy * dy <= radius2) {
            return true;
        }
    }

    return false;
}

static bool can_step_river(const MAP& map, int x, int y)
{
    if (!in_map(map, x, y)) {
        return false;
    }

    TERRAIN t = map[x][y].type_terrain;

    if (t == Montain) {
        return false;
    }

    return true;
}

static bool is_terminal_water(const MAP& map, int x, int y)
{
    if (!in_map(map, x, y)) {
        return false;
    }

    TERRAIN t = map[x][y].type_terrain;

    return t == River || t == Lake;
}

static void clear_water_cell(MAP& map, int x, int y)
{
    map[x][y].type_resource = None_Resource;
    map[x][y].has_berry = false;
}

/*
 * ============================================================
 * MEASURE MOUNTAINS
 * ============================================================
 */

static std::vector<MountainComponent> measure_mountains(const MAP& map)
{
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    std::vector<MountainComponent> mountains;
    std::vector<std::vector<int>> visited(width, std::vector<int>(height, 0));

    int dx[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int dy[8] = {0, 1, 1, 1, 0, -1, -1, -1};

    for (int sx = 0; sx < width; sx++) {
        for (int sy = 0; sy < height; sy++) {
            if (visited[sx][sy]) {
                continue;
            }

            if (map[sx][sy].type_terrain != Montain) {
                continue;
            }

            MountainComponent mountain;
            mountain.cx = 0.0;
            mountain.cy = 0.0;
            mountain.size = 0;
            mountain.face = 0;

            std::queue<RiverPoint> q;
            q.push({sx, sy});
            visited[sx][sy] = 1;

            while (!q.empty()) {
                RiverPoint p = q.front();
                q.pop();

                mountain.cells.push_back(p);
                mountain.cx += static_cast<double>(p.x);
                mountain.cy += static_cast<double>(p.y);

                for (int d = 0; d < 8; d++) {
                    int nx = p.x + dx[d];
                    int ny = p.y + dy[d];

                    if (!in_map(map, nx, ny)) {
                        continue;
                    }

                    if (visited[nx][ny]) {
                        continue;
                    }

                    if (map[nx][ny].type_terrain != Montain) {
                        continue;
                    }

                    visited[nx][ny] = 1;
                    q.push({nx, ny});
                }
            }

            mountain.size = static_cast<int>(mountain.cells.size());

            if (mountain.size <= 0) {
                continue;
            }

            mountain.cx /= static_cast<double>(mountain.size);
            mountain.cy /= static_cast<double>(mountain.size);

            for (const RiverPoint& m : mountain.cells) {
                for (int d = 0; d < 8; d++) {
                    int bx = m.x + dx[d];
                    int by = m.y + dy[d];

                    if (!in_map(map, bx, by)) {
                        continue;
                    }

                    if (map[bx][by].type_terrain != Plain) {
                        continue;
                    }

                    if (!point_in_vector(mountain.border_cells, bx, by)) {
                        mountain.border_cells.push_back({bx, by});
                    }
                }
            }

            if (!mountain.border_cells.empty()) {
                mountains.push_back(mountain);
            }
        }
    }

    std::sort(
        mountains.begin(),
              mountains.end(),
              [](const MountainComponent& a, const MountainComponent& b) {
                  return a.size > b.size;
              }
    );

    return mountains;
}

/*
 * face:
 * 0 north
 * 1 east
 * 2 south
 * 3 west
 */
static int side_of_point_relative_to_mountain(const MountainComponent& mountain, const RiverPoint& p)
{
    double dx = static_cast<double>(p.x) - mountain.cx;
    double dy = static_cast<double>(p.y) - mountain.cy;

    if (std::abs(dx) > std::abs(dy)) {
        if (dx < 0.0) {
            return 0;
        }

        return 2;
    }

    if (dy > 0.0) {
        return 1;
    }

    return 3;
}

static int choose_face_toward_nearest_mountain(
    const std::vector<MountainComponent>& mountains,
    int mountain_id
) {
    if (mountain_id < 0 || mountain_id >= static_cast<int>(mountains.size())) {
        return 0;
    }

    const MountainComponent& m = mountains[mountain_id];

    double best_d2 = std::numeric_limits<double>::infinity();
    int best_id = -1;

    for (int i = 0; i < static_cast<int>(mountains.size()); i++) {
        if (i == mountain_id) {
            continue;
        }

        double d2 = dist2_int(
            static_cast<int>(m.cx),
                              static_cast<int>(m.cy),
                              static_cast<int>(mountains[i].cx),
                              static_cast<int>(mountains[i].cy)
        );

        if (d2 < best_d2) {
            best_d2 = d2;
            best_id = i;
        }
    }

    if (best_id < 0) {
        return 0;
    }

    double dx = mountains[best_id].cx - m.cx;
    double dy = mountains[best_id].cy - m.cy;

    if (std::abs(dx) > std::abs(dy)) {
        if (dx < 0.0) {
            return 0;
        }

        return 2;
    }

    if (dy > 0.0) {
        return 1;
    }

    return 3;
}

static std::vector<RiverPoint> candidates_on_face(const MountainComponent& mountain, int face)
{
    std::vector<RiverPoint> candidates;

    for (const RiverPoint& p : mountain.border_cells) {
        if (side_of_point_relative_to_mountain(mountain, p) == face) {
            candidates.push_back(p);
        }
    }

    return candidates;
}

static int source_count_for_mountain(const MountainComponent& mountain, const MAP& map)
{
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());
    int min_dim = std::min(width, height);

    /*
     * More springs proportional to mountain size.
     *
     * Previous rule was conservative:
     *   count = 1 + mountain.size / (min_dim / 8), capped at 10.
     *
     * New rule:
     * - smaller divisor -> mountain size creates more springs;
     * - higher cap -> large mountain chains can feed many tributaries;
     * - minimum 3 -> every relevant mountain contributes visible water.
     */
    int divisor = std::max(6, min_dim / 14);
    int count = 2 + mountain.size / divisor;

    /*
     * Extra bonus for very large mountain masses.
     */
    if (mountain.size > min_dim * 2) {
        count += 2;
    }

    if (mountain.size > min_dim * 4) {
        count += 2;
    }

    /*
     * Avoid absurd source spam on huge maps, but allow enough tributaries
     * to build a main artery.
     */
    int max_sources = clamp_int_local(min_dim / 18, 12, 28);

    return clamp_int_local(count, 3, max_sources);
}

static std::vector<SourceNode> build_sources_from_mountains(
    const MAP& map,
    std::vector<MountainComponent>& mountains
) {
    std::vector<SourceNode> sources;

    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());
    int min_dim = std::min(width, height);

    for (int mid = 0; mid < static_cast<int>(mountains.size()); mid++) {
        mountains[mid].face = choose_face_toward_nearest_mountain(mountains, mid);

        std::vector<RiverPoint> candidates =
        candidates_on_face(mountains[mid], mountains[mid].face);

        if (candidates.empty()) {
            candidates = mountains[mid].border_cells;
        }

        for (int i = static_cast<int>(candidates.size()) - 1; i > 0; i--) {
            int j = std::rand() % (i + 1);
            std::swap(candidates[i], candidates[j]);
        }

        int needed = source_count_for_mountain(mountains[mid], map);
        int added = 0;

        /*
         * Smaller spacing means large mountain faces can actually place
         * the increased number of springs. On small maps it remains compact;
         * on large maps it avoids placing all springs in the same few cells.
         */
        int spacing = clamp_int_local(min_dim / 120, 3, 5);

        for (const RiverPoint& p : candidates) {
            if (added >= needed) {
                break;
            }

            if (source_near_used(sources, p.x, p.y, spacing)) {
                continue;
            }

            SourceNode s;
            s.p = p;
            s.mountain_id = mid;
            s.face = mountains[mid].face;

            sources.push_back(s);
            added++;
        }

        /*
         * Fallback pass:
         * If the selected face does not have enough free positions,
         * use the rest of the mountain border with tighter spacing.
         * This preserves the main-face logic but prevents large mountains
         * from getting too few springs.
         */
        if (added < needed) {
            std::vector<RiverPoint> fallback = mountains[mid].border_cells;

            for (int i = static_cast<int>(fallback.size()) - 1; i > 0; i--) {
                int j = std::rand() % (i + 1);
                std::swap(fallback[i], fallback[j]);
            }

            int fallback_spacing = std::max(2, spacing - 1);

            for (const RiverPoint& p : fallback) {
                if (added >= needed) {
                    break;
                }

                if (source_near_used(sources, p.x, p.y, fallback_spacing)) {
                    continue;
                }

                SourceNode s;
                s.p = p;
                s.mountain_id = mid;
                s.face = side_of_point_relative_to_mountain(mountains[mid], p);

                sources.push_back(s);
                added++;
            }
        }

        /*
         * Guarantee at least one source per mountain.
         */
        if (added == 0 && !candidates.empty()) {
            SourceNode s;
            s.p = candidates[0];
            s.mountain_id = mid;
            s.face = mountains[mid].face;

            sources.push_back(s);
        }
    }

    return sources;
}

/*
 * ============================================================
 * VALLEY FIELD
 * ============================================================
 */

static bool accept_distance_for_cell(
    ValleyCell& cell,
    int mountain_id,
    float d
) {
    if (mountain_id < 0) {
        return false;
    }

    if (cell.first_mountain == mountain_id) {
        if (d < cell.first_dist) {
            cell.first_dist = d;
            return true;
        }

        return false;
    }

    if (cell.second_mountain == mountain_id) {
        if (d < cell.second_dist) {
            cell.second_dist = d;
            return true;
        }

        return false;
    }

    if (d < cell.first_dist) {
        cell.second_dist = cell.first_dist;
        cell.second_mountain = cell.first_mountain;

        cell.first_dist = d;
        cell.first_mountain = mountain_id;

        return true;
    }

    if (d < cell.second_dist) {
        cell.second_dist = d;
        cell.second_mountain = mountain_id;

        return true;
    }

    return false;
}

static ValleyField build_valley_field(
    const MAP& map,
    const std::vector<MountainComponent>& mountains
) {
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());
    int min_dim = std::min(width, height);

    float max_distance = static_cast<float>(clamp_int_local(min_dim / 2, 40, 160));

    ValleyField field(width, std::vector<ValleyCell>(height));

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            field[x][y].first_mountain = -1;
            field[x][y].second_mountain = -1;
            field[x][y].first_dist = 999999.0f;
            field[x][y].second_dist = 999999.0f;
            field[x][y].score = -1000.0f;
        }
    }

    std::priority_queue<QueueNode, std::vector<QueueNode>, QueueNodeCompare> pq;

    for (int mid = 0; mid < static_cast<int>(mountains.size()); mid++) {
        for (const RiverPoint& p : mountains[mid].cells) {
            if (!in_map(map, p.x, p.y)) {
                continue;
            }

            if (accept_distance_for_cell(field[p.x][p.y], mid, 0.0f)) {
                pq.push({0.0f, p.x, p.y, mid});
            }
        }
    }

    while (!pq.empty()) {
        QueueNode node = pq.top();
        pq.pop();

        if (!in_map(map, node.x, node.y)) {
            continue;
        }

        bool known = false;

        if (field[node.x][node.y].first_mountain == node.mountain_id &&
            std::abs(field[node.x][node.y].first_dist - node.d) < 0.001f) {
            known = true;
            }

            if (field[node.x][node.y].second_mountain == node.mountain_id &&
                std::abs(field[node.x][node.y].second_dist - node.d) < 0.001f) {
                known = true;
                }

                if (!known) {
                    continue;
                }

                if (node.d > max_distance) {
                    continue;
                }

                for (int d = 0; d < 8; d++) {
                    int nx = node.x + river_dirs[d].x;
                    int ny = node.y + river_dirs[d].y;

                    if (!in_map(map, nx, ny)) {
                        continue;
                    }

                    float step_cost = (d % 2 == 0) ? 1.0f : 1.4142f;
                    float nd = node.d + step_cost;

                    if (nd > max_distance) {
                        continue;
                    }

                    if (accept_distance_for_cell(field[nx][ny], node.mountain_id, nd)) {
                        pq.push({nd, nx, ny, node.mountain_id});
                    }
                }
    }

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            ValleyCell& cell = field[x][y];

            if (cell.first_mountain >= 0 && cell.second_mountain >= 0) {
                float balance = std::abs(cell.first_dist - cell.second_dist);
                float near_dist = cell.first_dist;
                float far_dist = cell.second_dist;

                /*
                 * High score when the cell is in the middle between two mountain masses.
                 * This prevents long direct diagonals through open terrain.
                 */
                cell.score = 70.0f - balance * 5.0f;

                /*
                 * Ideal valley is not glued to the mountain and not infinitely far.
                 */
                if (near_dist < 3.0f) {
                    cell.score -= 90.0f;
                }
                else if (near_dist < 5.0f) {
                    cell.score -= 30.0f;
                }
                else if (near_dist <= 26.0f) {
                    cell.score += 16.0f;
                }
                else if (near_dist <= 45.0f) {
                    cell.score += 4.0f;
                }
                else {
                    cell.score -= 18.0f;
                }

                if (far_dist > max_distance * 0.88f) {
                    cell.score -= 25.0f;
                }

                if (terrain_near_local(map, x, y, Montain, 1)) {
                    cell.score -= 120.0f;
                }
                else if (terrain_near_local(map, x, y, Montain, 2)) {
                    cell.score -= 55.0f;
                }

                if (map[x][y].type_terrain == Montain || map[x][y].type_terrain == ravine) {
                    cell.score = -1000.0f;
                }
            }
            else {
                cell.score = -120.0f;
            }
        }
    }

    return field;
}

/*
 * ============================================================
 * TARGETING
 * ============================================================
 */

static int choose_first_source_index(const std::vector<SourceNode>& sources)
{
    if (sources.empty()) {
        return -1;
    }

    return std::rand() % static_cast<int>(sources.size());
}

static int find_nearest_source_from_other_mountain(
    const std::vector<SourceNode>& sources,
    int source_index,
    double max_distance
) {
    if (source_index < 0 || source_index >= static_cast<int>(sources.size())) {
        return -1;
    }

    double best_score = std::numeric_limits<double>::infinity();
    int best_index = -1;

    for (int i = 0; i < static_cast<int>(sources.size()); i++) {
        if (i == source_index) {
            continue;
        }

        if (sources[i].mountain_id == sources[source_index].mountain_id) {
            continue;
        }

        double d = dist_int(
            sources[source_index].p.x,
            sources[source_index].p.y,
            sources[i].p.x,
            sources[i].p.y
        );

        if (d > max_distance) {
            continue;
        }

        double score = d + rand01() * 12.0;

        if (score < best_score) {
            best_score = score;
            best_index = i;
        }
    }

    return best_index;
}

static int find_nearest_network_point_index(
    const std::vector<RiverPoint>& network_cells,
    const RiverPoint& source,
    double max_distance
) {
    if (network_cells.empty()) {
        return -1;
    }

    double max_d2 = max_distance * max_distance;
    double best_d2 = std::numeric_limits<double>::infinity();
    int best_index = -1;

    for (int i = 0; i < static_cast<int>(network_cells.size()); i++) {
        double d2 = dist2_int(source.x, source.y, network_cells[i].x, network_cells[i].y);

        if (d2 > max_d2) {
            continue;
        }

        if (d2 < best_d2) {
            best_d2 = d2;
            best_index = i;
        }
    }

    return best_index;
}

/*
 * ============================================================
 * Y-MERGE HELPERS
 * ============================================================
 *
 * These helpers allow two nearby rivers to merge before they literally touch.
 * The branch searches for an existing river cell inside a small radius and
 * creates a short connector, producing a Y-shaped confluence instead of
 * parallel rivers running side by side.
 */

static bool find_nearby_network_join(
    const MAP& map,
    const std::vector<std::vector<int>>& network_path,
    const std::vector<std::vector<int>>& network_index,
    int cx,
    int cy,
    int radius,
    int& out_path,
    int& out_index,
    RiverPoint& out_point
) {
    out_path = -1;
    out_index = -1;
    out_point = {cx, cy};

    int best_d2 = radius * radius + 1;
    bool found = false;

    for (int dx = -radius; dx <= radius; dx++) {
        for (int dy = -radius; dy <= radius; dy++) {
            int x = cx + dx;
            int y = cy + dy;

            if (!in_map(map, x, y)) {
                continue;
            }

            if (network_path[x][y] < 0) {
                continue;
            }

            /*
             * Do not choose a join point inside/against forbidden terrain.
             * The network itself should already avoid this, but this keeps
             * the Y connector clean.
             */
            if (map[x][y].type_terrain == Montain ||
                map[x][y].type_terrain == ravine ||
                map[x][y].type_terrain == Lake) {
                continue;
                }

                int d2 = dx * dx + dy * dy;

            if (d2 < best_d2) {
                best_d2 = d2;
                out_path = network_path[x][y];
                out_index = network_index[x][y];
                out_point = {x, y};
                found = true;
            }
        }
    }

    return found;
}

static bool append_y_connector_to_join(
    const MAP& map,
    const std::vector<std::vector<int>>& network_path,
    const std::vector<std::vector<int>>& network_index,
    std::vector<RiverPoint>& cells,
    const RiverPoint& join_point,
    int& out_path,
    int& out_index
) {
    if (cells.empty()) {
        return false;
    }

    RiverPoint current = cells.back();

    int max_steps =
    static_cast<int>(std::ceil(dist_int(current.x, current.y, join_point.x, join_point.y))) + 4;

    for (int step = 0; step <= max_steps; step++) {
        if (current.x == join_point.x && current.y == join_point.y) {
            if (!in_map(map, current.x, current.y)) {
                return false;
            }

            out_path = network_path[current.x][current.y];
            out_index = network_index[current.x][current.y];

            return out_path >= 0 && out_index >= 0;
        }

        double best_score = std::numeric_limits<double>::infinity();
        RiverPoint best_next = current;

        for (int d = 0; d < 8; d++) {
            int nx = current.x + river_dirs[d].x;
            int ny = current.y + river_dirs[d].y;

            if (!in_map(map, nx, ny)) {
                continue;
            }

            bool is_join_point = (nx == join_point.x && ny == join_point.y);

            if (!is_join_point && point_in_path(cells, nx, ny)) {
                continue;
            }

            TERRAIN t = map[nx][ny].type_terrain;

            if (t == Montain || t == ravine || t == Lake) {
                continue;
            }

            /*
             * The connector should be short and direct.
             * It is only used after the river is already close to the network.
             */
            double score = dist2_int(nx, ny, join_point.x, join_point.y);

            if (terrain_near_local(map, nx, ny, Montain, 1)) {
                score += 40.0;
            }

            score += rand01() * 0.50;

            if (score < best_score) {
                best_score = score;
                best_next = {nx, ny};
            }
        }

        if (best_next.x == current.x && best_next.y == current.y) {
            return false;
        }

        cells.push_back(best_next);
        current = best_next;
    }

    return false;
}

/*
 * ============================================================
 * ORGANIC PATH GROWTH
 * ============================================================
 */

static RiverPath grow_organic_river(
    const MAP& map,
    const ValleyField& valley,
    const std::vector<SourceNode>& sources,
    int source_index,
    const RiverPoint& target,
    const std::vector<std::vector<int>>& network_path,
    const std::vector<std::vector<int>>& network_index,
    bool allow_network_join,
    const GenerationConfig& cfg
) {
    RiverPath path;
    path.join_path = -1;
    path.join_index = -1;
    path.joins_existing = false;
    path.hit_ravine = false;
    path.hit_water = false;
    path.reached_source = false;

    if (source_index < 0 || source_index >= static_cast<int>(sources.size())) {
        return path;
    }

    RiverPoint current = sources[source_index].p;
    int current_dir = direction_toward(current, target);

    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());
    int min_dim = std::min(width, height);

    double initial_target_distance = dist_int(current.x, current.y, target.x, target.y);

    int max_steps = clamp_int_local(
        static_cast<int>(initial_target_distance * 1.75),
                                    22,
                                    std::max(60, min_dim)
    );

    int straight_streak = 0;
    int meander_bias = (std::rand() % 2) ? 1 : -1;
    int meander_timer = 5 + std::rand() % 10;

    path.cells.push_back(current);

    for (int step = 0; step < max_steps; step++) {
        if (dist2_int(current.x, current.y, target.x, target.y) <= 4.0) {
            path.reached_source = true;
            break;
        }

        int target_dir = direction_toward(current, target);

        std::vector<int> candidates;

        /*
         * Candidate set is deliberately local.
         * This is the main anti-diagonal rule: the river cannot suddenly aim
         * across the whole map by taking large angle corrections.
         */
        candidates.push_back(wrap_dir(current_dir - 2));
        candidates.push_back(wrap_dir(current_dir - 1));
        candidates.push_back(current_dir);
        candidates.push_back(wrap_dir(current_dir + 1));
        candidates.push_back(wrap_dir(current_dir + 2));

        /*
         * Add target direction only as a soft suggestion, not as the dominant rule.
         */
        candidates.push_back(wrap_dir(target_dir - 1));
        candidates.push_back(target_dir);
        candidates.push_back(wrap_dir(target_dir + 1));

        meander_timer--;

        if (meander_timer <= 0) {
            meander_bias = (std::rand() % 2) ? 1 : -1;
            meander_timer = 5 + std::rand() % 10;
        }

        candidates.push_back(wrap_dir(current_dir + meander_bias));

        std::sort(candidates.begin(), candidates.end());
        candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

        double best_score = -std::numeric_limits<double>::infinity();
        RiverPoint best_next = current;
        int best_dir = current_dir;
        bool best_join = false;
        int best_join_path = -1;
        int best_join_index = -1;
        RiverPoint best_join_point = {-1, -1};
        bool best_hit_ravine = false;
        bool best_hit_water = false;

        for (int dir : candidates) {
            int nx = current.x + river_dirs[dir].x;
            int ny = current.y + river_dirs[dir].y;

            if (!in_map(map, nx, ny)) {
                continue;
            }

            if (map[nx][ny].type_terrain == Montain) {
                continue;
            }

            if (point_in_path(path.cells, nx, ny)) {
                continue;
            }

            bool hit_ravine = map[nx][ny].type_terrain == ravine;
            bool hit_water = is_terminal_water(map, nx, ny);

            bool joins_network = false;
            int jp = -1;
            int ji = -1;
            RiverPoint join_point = {nx, ny};

            if (allow_network_join && step > 3 && network_path[nx][ny] >= 0) {
                joins_network = true;
                jp = network_path[nx][ny];
                ji = network_index[nx][ny];
                join_point = {nx, ny};
            }

            /*
             * Y-merge rule:
             * if this branch gets close to an existing river, do not keep
             * running parallel. Bend into the network through a short connector.
             */
            if (allow_network_join && !joins_network && step > 7) {
                int merge_radius = clamp_int_local(min_dim / 90, 3, 6);

                if (find_nearby_network_join(
                    map,
                    network_path,
                    network_index,
                    nx,
                    ny,
                    merge_radius,
                    jp,
                    ji,
                    join_point
                )) {
                    joins_network = true;
                }
            }

            if (!hit_ravine && !hit_water && !joins_network && !can_step_river(map, nx, ny)) {
                continue;
            }

            double d_now = dist_int(current.x, current.y, target.x, target.y);
            double d_next = dist_int(nx, ny, target.x, target.y);
            double gain = d_now - d_next;

            int diff = std::abs(dir - current_dir);

            if (diff > 4) {
                diff = 8 - diff;
            }

            double score = 0.0;

            /*
             * Target attraction is intentionally weaker than the valley field.
             * This prevents long straight lines across the map.
             */
            score += gain * 8.0;
            score -= d_next * 0.025;

            /*
             * The valley matrix is the dominant force.
             */
            score += static_cast<double>(valley[nx][ny].score) * 2.2;

            /*
             * Curve and continuity.
             */
            if (diff == 0) {
                score += 3.2;
            }
            else if (diff == 1) {
                score += 1.6;
            }
            else if (diff == 2) {
                score -= 2.8;
            }
            else {
                score -= 30.0;
            }

            /*
             * Avoid perfectly straight geometric segments.
             */
            if (straight_streak >= 5 && dir == current_dir) {
                score -= static_cast<double>(straight_streak - 4) * 4.5;
            }

            /*
             * Organic local noise.
             */
            if (dir == wrap_dir(current_dir + meander_bias)) {
                score += 2.4;
            }

            score += rand01() * 6.0;

            /*
             * Avoid scraping mountains.
             */
            if (step > 4 && terrain_near_local(map, nx, ny, Montain, 1)) {
                score -= 100.0;
            }
            else if (step > 7 && terrain_near_local(map, nx, ny, Montain, 2)) {
                score -= 35.0;
            }

            /*
             * Strongly discourage open-field diagonals.
             */
            if (step > 10 && valley[nx][ny].score < -20.0f) {
                score -= 70.0;
            }
            else if (step > 10 && valley[nx][ny].score < 0.0f) {
                score -= 25.0;
            }

            /*
             * Existing water/network is a valid endpoint.
             */
            if (joins_network) {
                double join_distance = dist_int(nx, ny, join_point.x, join_point.y);

                /*
                 * Exact contact is great, but nearby contact is also good:
                 * this produces Y-shaped confluences instead of parallel streams.
                 */
                score += 260.0;
                score -= join_distance * 18.0;
            }

            if (hit_water) {
                score += 150.0;
            }

            /*
             * Ravine is a hard stop, not an attraction.
             */
            if (hit_ravine) {
                score -= 25.0;
            }

            if (score > best_score) {
                best_score = score;
                best_next = {nx, ny};
                best_dir = dir;
                best_join = joins_network;
                best_join_path = jp;
                best_join_index = ji;
                best_join_point = join_point;
                best_hit_ravine = hit_ravine;
                best_hit_water = hit_water;
            }
        }

        if (best_next.x == current.x && best_next.y == current.y) {
            break;
        }

        if (best_hit_ravine) {
            path.hit_ravine = true;
            break;
        }

        if (best_dir == current_dir) {
            straight_streak++;
        }
        else {
            straight_streak = 0;
        }

        current = best_next;
        current_dir = best_dir;
        path.cells.push_back(current);

        /*
         * Do not stop only because the path temporarily left the valley field.
         * The previous version used open_field_streak to break here, which made
         * visible rivers end before reaching another river/ravine.
         *
         * Now the river keeps advancing toward the selected target or network.
         * The flow model later tapers terminal parts, so if it still ends, it
         * should end thin instead of as a thick unfinished river.
         */

        if (best_join) {
            /*
             * If the selected candidate is close to the network but not on it,
             * append a short connector to create a visible Y.
             */
            if (!(current.x == best_join_point.x && current.y == best_join_point.y)) {
                bool connected = append_y_connector_to_join(
                    map,
                    network_path,
                    network_index,
                    path.cells,
                    best_join_point,
                    best_join_path,
                    best_join_index
                );

                if (!connected) {
                    /*
                     * If connector failed, stop this branch without declaring
                     * a downstream join. This avoids fake parallel rivers.
                     */
                    path.hit_water = true;
                    break;
                }
            }

            path.joins_existing = true;
            path.join_path = best_join_path;
            path.join_index = best_join_index;
            break;
        }

        if (best_hit_water) {
            path.hit_water = true;
            break;
        }
    }

    if (
        static_cast<int>(path.cells.size()) < 6 &&
        !path.joins_existing &&
        !path.reached_source &&
        !path.hit_water
    ) {
        path.cells.clear();
    }

    (void)cfg;

    return path;
}

/*
 * ============================================================
 * NETWORK / FLOW
 * ============================================================
 */

static void register_path_in_network(
    const MAP& map,
    const RiverPath& path,
    int path_id,
    std::vector<std::vector<int>>& network_path,
    std::vector<std::vector<int>>& network_index,
    std::vector<RiverPoint>& network_cells
) {
    for (int i = 0; i < static_cast<int>(path.cells.size()); i++) {
        int x = path.cells[i].x;
        int y = path.cells[i].y;

        if (!in_map(map, x, y)) {
            continue;
        }

        if (network_path[x][y] < 0) {
            network_path[x][y] = path_id;
            network_index[x][y] = i;
            network_cells.push_back({x, y});
        }
    }
}

static std::vector<RiverPoint> build_full_downstream_path(
    const std::vector<RiverPath>& paths,
    int path_id
) {
    std::vector<RiverPoint> full;

    if (path_id < 0 || path_id >= static_cast<int>(paths.size())) {
        return full;
    }

    const RiverPath& p = paths[path_id];

    for (const RiverPoint& cell : p.cells) {
        full.push_back(cell);
    }

    if (p.joins_existing && p.join_path >= 0) {
        int current_path = p.join_path;
        int current_index = p.join_index;
        int guard = 0;

        while (
            current_path >= 0 &&
            current_path < static_cast<int>(paths.size()) &&
            guard < 100
        ) {
            const RiverPath& joined = paths[current_path];

            for (int i = current_index + 1; i < static_cast<int>(joined.cells.size()); i++) {
                full.push_back(joined.cells[i]);
            }

            if (!joined.joins_existing) {
                break;
            }

            current_path = joined.join_path;
            current_index = joined.join_index;
            guard++;
        }
    }

    return full;
}

static void add_flow_to_path(
    const std::vector<RiverPoint>& path,
    std::vector<std::vector<double>>& flow
) {
    /*
     * Artery logic:
     * each spring contributes water to the path, but this contribution
     * slowly loses strength with distance. Therefore:
     *
     * - near a single spring, the channel remains thin;
     * - after confluences, several source contributions overlap and the river widens;
     * - far downstream, if no new springs enter, the accumulated flow tapers again.
     */
    int len = static_cast<int>(path.size());

    if (len <= 0) {
        return;
    }

    int width = static_cast<int>(flow.size());
    int height = static_cast<int>(flow[0].size());

    for (int i = 0; i < len; i++) {
        const RiverPoint& p = path[i];

        if (p.x < 0 || p.x >= width) {
            continue;
        }

        if (p.y < 0 || p.y >= height) {
            continue;
        }

        double progress =
        static_cast<double>(i) /
        static_cast<double>(std::max(1, len - 1));

        /*
         * A spring starts thin, gains body shortly after the source,
         * then slowly fades toward the terminal part of the river.
         */
        double source_ramp = std::min(1.0, static_cast<double>(i + 1) / 8.0);
        double downstream_decay = 1.0 - std::pow(progress, 1.65) * 0.82;

        if (downstream_decay < 0.08) {
            downstream_decay = 0.08;
        }

        double contribution = source_ramp * downstream_decay;

        flow[p.x][p.y] += contribution;
    }
}

static void boost_confluence_and_sources(
    const std::vector<RiverPath>& paths,
    std::vector<std::vector<double>>& flow
) {
    int width = static_cast<int>(flow.size());
    int height = static_cast<int>(flow[0].size());

    /*
     * Local confluence emphasis:
     * keep springs thin, but make the exact Y-junction and the first cells
     * after the join visibly stronger. The downstream widening mostly comes
     * from build_full_downstream_path() + add_flow_to_path().
     */
    for (const RiverPath& path : paths) {
        if (!path.joins_existing || path.cells.empty()) {
            continue;
        }

        RiverPoint join = path.cells.back();

        if (join.x < 0 || join.x >= width || join.y < 0 || join.y >= height) {
            continue;
        }

        flow[join.x][join.y] += 1.70;

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int nx = join.x + dx;
                int ny = join.y + dy;

                if (nx < 0 || nx >= width || ny < 0 || ny >= height) {
                    continue;
                }

                int d2 = dx * dx + dy * dy;

                if (d2 <= 1) {
                    flow[nx][ny] += 0.65;
                }
            }
        }
    }
}

static int river_thickness_from_flow(double flow_value, const GenerationConfig& cfg)
{
    /*
     * Artery hierarchy:
     * low flow      = thin spring
     * medium flow   = river
     * high flow     = main artery after several tributaries
     *
     * Because flow is now double and decays with distance, the river can
     * naturally taper near its terminal part.
     */
    int thickness = 1;

    if (flow_value < 1.30) {
        thickness = 1;
    }
    else if (flow_value < 2.40) {
        thickness = 3;
    }
    else if (flow_value < 4.10) {
        thickness = 5;
    }
    else {
        thickness = 7;
    }

    int max_thickness = std::max(1, cfg.river_max_thickness);

    if (thickness > max_thickness) {
        thickness = max_thickness;
    }

    if (thickness < 1) {
        thickness = 1;
    }

    if (thickness > 1 && thickness % 2 == 0) {
        thickness++;
    }

    return thickness;
}

static void paint_river_network(
    MAP& map,
    const std::vector<std::vector<double>>& flow,
    const GenerationConfig& cfg
) {
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (flow[x][y] <= 0) {
                continue;
            }

            int thickness = river_thickness_from_flow(flow[x][y], cfg);
            paint_river_brush(map, x, y, thickness);
        }
    }
}


/*
 * ============================================================
 * LAKE CHAIN CONNECTION
 * ============================================================
 *
 * Goal:
 * After lakes have been created, connect them as a chain:
 * one lake finds the nearest already connected lake, then the next lake
 * joins that connected water system, until all reachable lakes belong to
 * the same connected hydrographic system.
 *
 * This avoids a star pattern. It does not connect every lake to the center.
 * It connects nearest lake-to-lake, progressively.
 */

static std::vector<LakeComponent> measure_lakes(const MAP& map)
{
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    std::vector<LakeComponent> lakes;
    std::vector<std::vector<int>> visited(width, std::vector<int>(height, 0));

    int dx[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int dy[8] = {0, 1, 1, 1, 0, -1, -1, -1};

    for (int sx = 0; sx < width; sx++) {
        for (int sy = 0; sy < height; sy++) {
            if (visited[sx][sy]) {
                continue;
            }

            if (map[sx][sy].type_terrain != Lake) {
                continue;
            }

            LakeComponent lake;
            lake.cx = 0.0;
            lake.cy = 0.0;
            lake.size = 0;

            std::queue<RiverPoint> q;
            q.push({sx, sy});
            visited[sx][sy] = 1;

            while (!q.empty()) {
                RiverPoint p = q.front();
                q.pop();

                lake.cells.push_back(p);
                lake.cx += static_cast<double>(p.x);
                lake.cy += static_cast<double>(p.y);

                bool is_border = false;

                for (int d = 0; d < 8; d++) {
                    int nx = p.x + dx[d];
                    int ny = p.y + dy[d];

                    if (!in_map(map, nx, ny)) {
                        continue;
                    }

                    if (map[nx][ny].type_terrain == Lake) {
                        if (!visited[nx][ny]) {
                            visited[nx][ny] = 1;
                            q.push({nx, ny});
                        }
                    }
                    else {
                        is_border = true;
                    }
                }

                if (is_border) {
                    lake.border_cells.push_back(p);
                }
            }

            lake.size = static_cast<int>(lake.cells.size());

            if (lake.size <= 0) {
                continue;
            }

            lake.cx /= static_cast<double>(lake.size);
            lake.cy /= static_cast<double>(lake.size);

            if (lake.border_cells.empty()) {
                lake.border_cells = lake.cells;
            }

            lakes.push_back(lake);
        }
    }

    std::sort(
        lakes.begin(),
              lakes.end(),
              [](const LakeComponent& a, const LakeComponent& b) {
                  return a.size > b.size;
              }
    );

    return lakes;
}

static bool can_lake_connector_step(const MAP& map, int x, int y)
{
    if (!in_map(map, x, y)) {
        return false;
    }

    TERRAIN t = map[x][y].type_terrain;

    if (t == Montain) {
        return false;
    }

    if (t == ravine) {
        return false;
    }

    return true;
}

static bool choose_best_lake_border_pair(
    const LakeComponent& from_lake,
    const LakeComponent& to_lake,
    RiverPoint& out_from,
    RiverPoint& out_to
) {
    if (from_lake.border_cells.empty() || to_lake.border_cells.empty()) {
        return false;
    }

    double best_d2 = std::numeric_limits<double>::infinity();
    bool found = false;

    /*
     * Full search is okay for small borders. For very large lakes, sample
     * through the border lists to keep this stage cheap.
     */
    int step_a = std::max(1, static_cast<int>(from_lake.border_cells.size()) / 180);
    int step_b = std::max(1, static_cast<int>(to_lake.border_cells.size()) / 180);

    for (int i = 0; i < static_cast<int>(from_lake.border_cells.size()); i += step_a) {
        const RiverPoint& a = from_lake.border_cells[i];

        for (int j = 0; j < static_cast<int>(to_lake.border_cells.size()); j += step_b) {
            const RiverPoint& b = to_lake.border_cells[j];

            double d2 = dist2_int(a.x, a.y, b.x, b.y);

            if (d2 < best_d2) {
                best_d2 = d2;
                out_from = a;
                out_to = b;
                found = true;
            }
        }
    }

    return found;
}

static bool build_lake_connector_path(
    const MAP& map,
    const RiverPoint& from,
    const RiverPoint& to,
    std::vector<RiverPoint>& connector,
    const GenerationConfig& cfg
) {
    (void)cfg;

    connector.clear();

    RiverPoint current = from;
    int current_dir = direction_toward(from, to);

    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());
    int min_dim = std::min(width, height);

    int direct_distance = static_cast<int>(dist_int(from.x, from.y, to.x, to.y));

    int max_steps = clamp_int_local(
        static_cast<int>(direct_distance * 1.75),
                                    12,
                                    std::max(40, min_dim)
    );

    int meander_bias = (std::rand() % 2) ? 1 : -1;
    int meander_timer = 4 + std::rand() % 8;

    for (int step = 0; step < max_steps; step++) {
        if (dist2_int(current.x, current.y, to.x, to.y) <= 4.0) {
            connector.push_back(to);
            return connector.size() >= 2;
        }

        int target_dir = direction_toward(current, to);

        std::vector<int> candidates;

        candidates.push_back(wrap_dir(current_dir - 1));
        candidates.push_back(current_dir);
        candidates.push_back(wrap_dir(current_dir + 1));
        candidates.push_back(wrap_dir(target_dir - 1));
        candidates.push_back(target_dir);
        candidates.push_back(wrap_dir(target_dir + 1));

        meander_timer--;

        if (meander_timer <= 0) {
            meander_bias = (std::rand() % 2) ? 1 : -1;
            meander_timer = 4 + std::rand() % 8;
        }

        candidates.push_back(wrap_dir(current_dir + meander_bias));

        std::sort(candidates.begin(), candidates.end());
        candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

        double best_score = -std::numeric_limits<double>::infinity();
        RiverPoint best_next = current;
        int best_dir = current_dir;

        for (int dir : candidates) {
            int nx = current.x + river_dirs[dir].x;
            int ny = current.y + river_dirs[dir].y;

            if (!can_lake_connector_step(map, nx, ny)) {
                continue;
            }

            if (point_in_path(connector, nx, ny)) {
                continue;
            }

            double d_now = dist_int(current.x, current.y, to.x, to.y);
            double d_next = dist_int(nx, ny, to.x, to.y);
            double gain = d_now - d_next;

            int diff = std::abs(dir - current_dir);

            if (diff > 4) {
                diff = 8 - diff;
            }

            double score = 0.0;

            /*
             * Main goal: move toward the next lake.
             */
            score += gain * 18.0;
            score -= d_next * 0.10;

            /*
             * Organicity: slight random side bias.
             */
            if (dir == wrap_dir(current_dir + meander_bias)) {
                score += 2.0;
            }

            /*
             * Keep curves small. Lake connectors should behave like natural
             * outlets, not zigzag channels.
             */
            if (diff == 0) {
                score += 3.5;
            }
            else if (diff == 1) {
                score += 1.4;
            }
            else {
                score -= 15.0;
            }

            /*
             * Prefer using existing rivers if they are encountered, because
             * they already represent the drainage system.
             */
            if (map[nx][ny].type_terrain == River) {
                score += 22.0;
            }

            /*
             * Do not scrape mountains while connecting lakes.
             */
            if (terrain_near_local(map, nx, ny, Montain, 1)) {
                score -= 45.0;
            }
            else if (terrain_near_local(map, nx, ny, Montain, 2)) {
                score -= 20.0;
            }

            /*
             * Lakes may be the terminal target. Passing through a third lake
             * is also acceptable because it means the chain connected earlier.
             */
            if (map[nx][ny].type_terrain == Lake && dist2_int(nx, ny, to.x, to.y) > 9.0) {
                score += 45.0;
            }

            score += rand01() * 3.0;

            if (score > best_score) {
                best_score = score;
                best_next = {nx, ny};
                best_dir = dir;
            }
        }

        if (best_next.x == current.x && best_next.y == current.y) {
            break;
        }

        connector.push_back(best_next);

        if (map[best_next.x][best_next.y].type_terrain == Lake &&
            dist2_int(best_next.x, best_next.y, from.x, from.y) > 9.0) {
            return connector.size() >= 2;
            }

            current = best_next;
        current_dir = best_dir;
    }

    return false;
}

static void paint_lake_connector(
    MAP& map,
    const std::vector<RiverPoint>& connector,
    int thickness
) {
    for (const RiverPoint& p : connector) {
        if (!in_map(map, p.x, p.y)) {
            continue;
        }

        if (map[p.x][p.y].type_terrain == Montain ||
            map[p.x][p.y].type_terrain == ravine ||
            map[p.x][p.y].type_terrain == Lake) {
            continue;
            }

            paint_river_brush(map, p.x, p.y, thickness);
    }
}

static void connect_lakes_after_creation(MAP& map, const GenerationConfig& cfg)
{
    std::vector<LakeComponent> lakes = measure_lakes(map);

    if (lakes.size() < 2) {
        return;
    }

    int lake_count = static_cast<int>(lakes.size());
    std::vector<int> connected(lake_count, 0);

    /*
     * Start from the largest lake. Every next lake connects to the closest
     * already connected lake. This creates a chain/tree instead of a star.
     */
    connected[0] = 1;
    int connected_count = 1;

    int max_attempts = lake_count * lake_count;
    int attempts = 0;

    while (connected_count < lake_count && attempts < max_attempts) {
        attempts++;

        double best_d2 = std::numeric_limits<double>::infinity();
        int best_unconnected = -1;
        int best_connected = -1;
        RiverPoint best_from = {0, 0};
        RiverPoint best_to = {0, 0};

        for (int i = 0; i < lake_count; i++) {
            if (connected[i]) {
                continue;
            }

            for (int j = 0; j < lake_count; j++) {
                if (!connected[j]) {
                    continue;
                }

                RiverPoint from;
                RiverPoint to;

                if (!choose_best_lake_border_pair(lakes[i], lakes[j], from, to)) {
                    continue;
                }

                double d2 = dist2_int(from.x, from.y, to.x, to.y);

                if (d2 < best_d2) {
                    best_d2 = d2;
                    best_unconnected = i;
                    best_connected = j;
                    best_from = from;
                    best_to = to;
                }
            }
        }

        if (best_unconnected < 0 || best_connected < 0) {
            break;
        }

        std::vector<RiverPoint> connector;

        bool connected_now = build_lake_connector_path(
            map,
            best_from,
            best_to,
            connector,
            cfg
        );

        if (connected_now) {
            int connector_thickness = clamp_int_local(cfg.river_max_thickness, 1, 3);
            paint_lake_connector(map, connector, connector_thickness);

            connected[best_unconnected] = 1;
            connected_count++;
        }
        else {
            /*
             * If this lake cannot be connected without crossing mountains/ravines,
             * mark it as processed to avoid an infinite loop. It will remain isolated
             * because forcing a connector would create an artificial straight line.
             */
            connected[best_unconnected] = 1;
            connected_count++;
        }
    }
}

/*
 * Public wrapper.
 * Add its prototype to Map.hpp if you want to call this after any separate
 * lake-generation stage:
 *
 *     void connect_lakes(MAP& map, const GenerationConfig& cfg);
 */
void connect_lakes(MAP& map, const GenerationConfig& cfg)
{
    connect_lakes_after_creation(map, cfg);
}


/*
 * ============================================================
 * PUBLIC GENERATION
 * ============================================================
 */

void create_rivers(MAP& map, const GenerationConfig& cfg)
{
    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());
    int min_dim = std::min(width, height);

    std::vector<MountainComponent> mountains = measure_mountains(map);

    if (mountains.size() < 2) {
        return;
    }

    ValleyField valley = build_valley_field(map, mountains);
    std::vector<SourceNode> sources = build_sources_from_mountains(map, mountains);

    if (sources.size() < 2) {
        return;
    }

    std::vector<RiverPath> paths;
    std::vector<RiverPoint> network_cells;

    std::vector<std::vector<int>> network_path(width, std::vector<int>(height, -1));
    std::vector<std::vector<int>> network_index(width, std::vector<int>(height, -1));

    /*
     * Track which mountain effectively produced at least one visible river.
     * It is not enough to create a source object: if every path from that
     * source fails or is skipped, the mountain appears without springs.
     */
    std::vector<int> mountain_has_river(static_cast<int>(mountains.size()), 0);

    /*
     * First river: it creates the initial network between nearby mountain springs.
     */
    int first_source = choose_first_source_index(sources);

    double max_source_target_distance =
    static_cast<double>(clamp_int_local(min_dim / 2, 35, 140));

    int first_target = find_nearest_source_from_other_mountain(
        sources,
        first_source,
        max_source_target_distance
    );

    /*
     * If no nearby target exists, still create the initial artery using a
     * farther spring. This prevents the whole hydrology from failing on maps
     * where mountains are spaced far apart.
     */
    if (first_target < 0) {
        first_target = find_nearest_source_from_other_mountain(
            sources,
            first_source,
            static_cast<double>(min_dim * 2)
        );
    }

    if (first_source >= 0 && first_target >= 0) {
        RiverPath first_path = grow_organic_river(
            map,
            valley,
            sources,
            first_source,
            sources[first_target].p,
            network_path,
            network_index,
            false,
            cfg
        );

        if (!first_path.cells.empty()) {
            int path_id = static_cast<int>(paths.size());
            paths.push_back(first_path);

            if (sources[first_source].mountain_id >= 0 &&
                sources[first_source].mountain_id < static_cast<int>(mountain_has_river.size())) {
                mountain_has_river[sources[first_source].mountain_id] = 1;
                }

                register_path_in_network(
                    map,
                    first_path,
                    path_id,
                    network_path,
                    network_index,
                    network_cells
                );
        }
    }

    /*
     * All springs try to reach the existing river network.
     * If the network is too far, they try a spring from another mountain.
     */
    std::vector<int> order;

    for (int i = 0; i < static_cast<int>(sources.size()); i++) {
        order.push_back(i);
    }

    for (int i = static_cast<int>(order.size()) - 1; i > 0; i--) {
        int j = std::rand() % (i + 1);
        std::swap(order[i], order[j]);
    }

    /*
     * Do not let cfg.max_river_quantity suppress source coverage.
     * We want every mountain to have at least one visible spring/river.
     */
    int max_paths = static_cast<int>(sources.size()) + static_cast<int>(mountains.size()) + 8;
    int created = static_cast<int>(paths.size());

    /*
     * Larger join search distance prevents rivers from stopping just because
     * the nearest existing network is slightly outside a conservative radius.
     * Y-merge still uses a small local radius, so this does not force long
     * straight connectors.
     */
    double max_network_distance =
    static_cast<double>(clamp_int_local(min_dim / 2, 45, 160));

    for (int idx : order) {
        if (created >= max_paths) {
            break;
        }

        RiverPoint target;
        bool allow_join = false;

        if (!network_cells.empty()) {
            int nearest_network = find_nearest_network_point_index(
                network_cells,
                sources[idx].p,
                max_network_distance
            );

            if (nearest_network >= 0) {
                target = network_cells[nearest_network];
                allow_join = true;
            }
            else {
                int target_source = find_nearest_source_from_other_mountain(
                    sources,
                    idx,
                    max_source_target_distance
                );

                if (target_source < 0) {
                    target_source = find_nearest_source_from_other_mountain(
                        sources,
                        idx,
                        static_cast<double>(min_dim * 2)
                    );
                }

                if (target_source < 0) {
                    continue;
                }

                target = sources[target_source].p;
            }
        }
        else {
            int target_source = find_nearest_source_from_other_mountain(
                sources,
                idx,
                max_source_target_distance
            );

            if (target_source < 0) {
                target_source = find_nearest_source_from_other_mountain(
                    sources,
                    idx,
                    static_cast<double>(min_dim * 2)
                );
            }

            if (target_source < 0) {
                continue;
            }

            target = sources[target_source].p;
        }

        RiverPath path = grow_organic_river(
            map,
            valley,
            sources,
            idx,
            target,
            network_path,
            network_index,
            allow_join,
            cfg
        );

        if (path.cells.empty()) {
            continue;
        }

        int path_id = static_cast<int>(paths.size());
        paths.push_back(path);

        if (sources[idx].mountain_id >= 0 &&
            sources[idx].mountain_id < static_cast<int>(mountain_has_river.size())) {
            mountain_has_river[sources[idx].mountain_id] = 1;
            }

            register_path_in_network(
                map,
                path,
                path_id,
                network_path,
                network_index,
                network_cells
            );

            created++;
    }

    /*
     * Guarantee pass:
     * after the normal generation, any mountain still without a visible river
     * gets one forced attempt from one of its own sources.
     */
    for (int mid = 0; mid < static_cast<int>(mountains.size()); mid++) {
        if (mountain_has_river[mid]) {
            continue;
        }

        int attempts_for_mountain = 0;

        for (int sidx = 0; sidx < static_cast<int>(sources.size()); sidx++) {
            if (sources[sidx].mountain_id != mid) {
                continue;
            }

            if (attempts_for_mountain >= 3) {
                break;
            }

            attempts_for_mountain++;

            RiverPoint target;
            bool allow_join = false;

            if (!network_cells.empty()) {
                int nearest_network = find_nearest_network_point_index(
                    network_cells,
                    sources[sidx].p,
                    static_cast<double>(min_dim * 2)
                );

                if (nearest_network >= 0) {
                    target = network_cells[nearest_network];
                    allow_join = true;
                }
                else {
                    int target_source = find_nearest_source_from_other_mountain(
                        sources,
                        sidx,
                        static_cast<double>(min_dim * 2)
                    );

                    if (target_source < 0) {
                        continue;
                    }

                    target = sources[target_source].p;
                }
            }
            else {
                int target_source = find_nearest_source_from_other_mountain(
                    sources,
                    sidx,
                    static_cast<double>(min_dim * 2)
                );

                if (target_source < 0) {
                    continue;
                }

                target = sources[target_source].p;
            }

            RiverPath forced_path = grow_organic_river(
                map,
                valley,
                sources,
                sidx,
                target,
                network_path,
                network_index,
                allow_join,
                cfg
            );

            if (forced_path.cells.empty()) {
                continue;
            }

            int path_id = static_cast<int>(paths.size());
            paths.push_back(forced_path);
            mountain_has_river[mid] = 1;

            register_path_in_network(
                map,
                forced_path,
                    path_id,
                    network_path,
                    network_index,
                    network_cells
            );

            break;
        }
    }

    if (paths.empty()) {
        return;
    }

    std::vector<std::vector<double>> flow(width, std::vector<double>(height, 0.0));

    for (int i = 0; i < static_cast<int>(paths.size()); i++) {
        std::vector<RiverPoint> full_path = build_full_downstream_path(paths, i);

        if (full_path.empty()) {
            continue;
        }

        add_flow_to_path(full_path, flow);
    }

    boost_confluence_and_sources(paths, flow);
    paint_river_network(map, flow, cfg);

    /*
     * Safe automatic pass:
     * If lakes already exist at this moment, connect them progressively.
     * If there are no lakes yet, this does nothing.
     */
    connect_lakes_after_creation(map, cfg);
}


void draw_river(MAP& map, int x, int y, int dir, const GenerationConfig& cfg)
{
    (void)dir;

    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    std::vector<MountainComponent> mountains = measure_mountains(map);

    if (mountains.size() < 2) {
        return;
    }

    ValleyField valley = build_valley_field(map, mountains);
    std::vector<SourceNode> sources = build_sources_from_mountains(map, mountains);

    if (sources.empty()) {
        return;
    }

    SourceNode manual;
    manual.p = {x, y};
    manual.mountain_id = 0;
    manual.face = 0;
    sources.push_back(manual);

    int source_index = static_cast<int>(sources.size()) - 1;
    int target_index = find_nearest_source_from_other_mountain(sources, source_index, 999999.0);

    if (target_index < 0) {
        return;
    }

    std::vector<std::vector<int>> network_path(width, std::vector<int>(height, -1));
    std::vector<std::vector<int>> network_index(width, std::vector<int>(height, -1));

    RiverPath path = grow_organic_river(
        map,
        valley,
        sources,
        source_index,
        sources[target_index].p,
        network_path,
        network_index,
        false,
        cfg
    );

    if (path.cells.empty()) {
        return;
    }

    std::vector<std::vector<double>> flow(width, std::vector<double>(height, 0.0));
    add_flow_to_path(path.cells, flow);
    paint_river_network(map, flow, cfg);
}

/*
 * ============================================================
 * RIVER PAINTING
 * ============================================================
 */

int paint_river_brush(MAP& map, int cx, int cy, int thickness)
{
    int painted = 0;
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

            TERRAIN t = map[x][y].type_terrain;

            if (t == Montain || t == ravine || t == Lake) {
                continue;
            }

            int d2 = dx * dx + dy * dy;
            int r2 = radius * radius;

            bool should_paint = false;

            if (dx == 0 && dy == 0) {
                should_paint = true;
            }
            else if (radius == 0) {
                should_paint = false;
            }
            else if (d2 <= r2) {
                should_paint = true;
            }
            else if (d2 <= r2 + 1) {
                should_paint = (std::rand() % 100 < 35);
            }

            if (!should_paint) {
                continue;
            }

            if (map[x][y].type_terrain != River) {
                painted++;
            }

            set_terrain(map, x, y, River);
            clear_water_cell(map, x, y);
        }
    }

    return painted;
}

/*
 * ============================================================
 * LEGACY SOURCE SEARCH
 * ============================================================
 */

bool find_mountain_source(const MAP& map, int& x, int& y, const GenerationConfig& cfg)
{
    (void)cfg;

    std::vector<MountainComponent> mountains = measure_mountains(map);

    if (mountains.empty()) {
        return false;
    }

    std::vector<SourceNode> sources = build_sources_from_mountains(map, mountains);

    if (sources.empty()) {
        return false;
    }

    SourceNode s = sources[std::rand() % static_cast<int>(sources.size())];

    x = s.p.x;
    y = s.p.y;

    return true;
}

/*
 * ============================================================
 * LAKE FUNCTIONS KEPT FOR MAP.HPP COMPATIBILITY
 * ============================================================
 */

int calculate_lake_area(int river_volume, int river_thickness, const GenerationConfig& cfg)
{
    int min_area = cfg.lake_min_area;
    int max_area = cfg.lake_max_area;

    double reference_volume =
    static_cast<double>(cfg.max_river_size) *
    static_cast<double>(std::max(1, cfg.river_max_thickness));

    if (reference_volume < 1.0) {
        reference_volume = 1.0;
    }

    double ratio = static_cast<double>(river_volume) / reference_volume;

    if (ratio < 0.20) {
        ratio = 0.20;
    }

    if (ratio > 2.50) {
        ratio = 2.50;
    }

    int area =
    min_area +
    static_cast<int>((max_area - min_area) * std::pow(ratio, 1.05));

    area += river_thickness * river_thickness * 20;

    if (area < min_area) {
        area = min_area;
    }

    if (area > max_area * 2) {
        area = max_area * 2;
    }

    return area;
}

bool can_paint_lake_cell(const MAP& map, int x, int y)
{
    if (!in_map(map, x, y)) {
        return false;
    }

    TERRAIN t = map[x][y].type_terrain;

    if (t == Montain || t == ravine) {
        return false;
    }

    if (terrain_near_local(map, x, y, Montain, 6)) {
        return false;
    }

    return true;
}

void paint_lake_area(MAP& map, int cx, int cy, int target_area)
{
    if (!in_map(map, cx, cy)) {
        return;
    }

    int width = static_cast<int>(map.size());
    int height = static_cast<int>(map[0].size());

    std::vector<std::vector<int>> visited(width, std::vector<int>(height, 0));
    std::vector<std::pair<int, int>> frontier;

    frontier.push_back(std::make_pair(cx, cy));

    int painted = 0;
    int max_spread = static_cast<int>(std::sqrt(static_cast<double>(target_area)) * 2.0) + 4;
    int max_spread2 = max_spread * max_spread;

    int dx[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int dy[8] = {0, 1, 1, 1, 0, -1, -1, -1};

    while (!frontier.empty() && painted < target_area) {
        int index = std::rand() % static_cast<int>(frontier.size());

        int x = frontier[index].first;
        int y = frontier[index].second;

        frontier[index] = frontier.back();
        frontier.pop_back();

        if (!in_map(map, x, y)) {
            continue;
        }

        if (visited[x][y]) {
            continue;
        }

        visited[x][y] = 1;

        int ox = x - cx;
        int oy = y - cy;
        int d2 = ox * ox + oy * oy;

        if (d2 > max_spread2) {
            continue;
        }

        if (!can_paint_lake_cell(map, x, y)) {
            continue;
        }

        set_terrain(map, x, y, Lake);
        clear_water_cell(map, x, y);
        painted++;

        for (int d = 0; d < 8; d++) {
            int nx = x + dx[d];
            int ny = y + dy[d];

            if (!in_map(map, nx, ny)) {
                continue;
            }

            if (visited[nx][ny]) {
                continue;
            }

            if (d == 0 || d == 2 || d == 4 || d == 6) {
                frontier.push_back(std::make_pair(nx, ny));
            }
            else if (std::rand() % 100 < 55) {
                frontier.push_back(std::make_pair(nx, ny));
            }
        }
    }
}

void paint_lake(MAP& map, int cx, int cy, int radius)
{
    int target_area =
    static_cast<int>(3.14159265358979323846 * radius * radius);

    if (target_area < 1) {
        target_area = 1;
    }

    paint_lake_area(map, cx, cy, target_area);
}

void create_lake_from_river(
    MAP& map,
    int x,
    int y,
    int river_volume,
    int river_thickness,
    const GenerationConfig& cfg
) {
    int area = calculate_lake_area(river_volume, river_thickness, cfg);

    /*
     * Compatibility only.
     * create_rivers() does not call this in this organic-network version.
     */
    paint_lake_area(map, x, y, area);
}
