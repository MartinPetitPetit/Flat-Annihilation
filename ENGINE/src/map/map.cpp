#include "map.hpp" // Declarations, constants and map types.

#include <iostream> // Console output.
#include <cstdlib>  // rand and srand.
#include <ctime>    // time for random seed.
#include <cmath>    // sqrt and pow.


/*
 *   ============================================================
 *   BASIC MAP FUNCTIONS
 *   ============================================================
 */

MAP create_map(int width, int height)
{
    // Create a 2D map with width rows and height columns.
    return MAP(width, std::vector<CARRE>(height));
}

bool in_map(const MAP& map, int x, int y)
{
    // Reject empty maps.
    if (map.empty()) {
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

            // Print plain terrain.
            if (map[x][y].type_terrain == Plain) {
                std::cout << "🟩";
            }

            // Print mountain terrain.
            else if (map[x][y].type_terrain == Montain) {
                std::cout << "🟫";
            }

            // Print river terrain.
            else if (map[x][y].type_terrain == River) {
                std::cout << "🟦";
            }

            // Print lake terrain.
            else if (map[x][y].type_terrain == Lake) {
                std::cout << "🟦";
            }

            // Print every other terrain.
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
 *   ============================================================
 *   FULL MAP GENERATION
 *   ============================================================
 */

void generate_map(MAP& map)
{
    // Avoid reseeding random more than once.
    static bool seed_done = false;

    // Initialize random seed once.
    if (!seed_done) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        seed_done = true;
    }

    // Store map width.
    int width = static_cast<int>(map.size());

    // Store map height.
    int height = static_cast<int>(map[0].size());

    /*
     *       ----------------------------
     *       PART 1: BASE PLAIN MAP
     *       ----------------------------
     */

    // Fill all cells with default values.
    for (int x = 0; x < width; x++) {

        // Browse each column.
        for (int y = 0; y < height; y++) {

            // Default terrain.
            map[x][y].type_terrain = Plain;

            // No structure.
            map[x][y].type_struct = None_Struct;

            // No unit.
            map[x][y].type_unit = None_Unit;

            // No resource.
            map[x][y].type_resource = None_Resource;
        }
    }

    /*
     *       ----------------------------
     *       PART 2: MOUNTAINS
     *       ----------------------------
     */

    // Create mountain parameter list.
    std::vector<MONTAIN> montains(Max_montain_quantity);

    // Initialize each mountain chain.
    for (int i = 0; i < Max_montain_quantity; i++) {

        // Random start row.
        montains[i].x_init = std::rand() % width;

        // Random start column.
        montains[i].y_init = std::rand() % height;

        // Number of cells advanced per segment.
        montains[i].size = std::rand() % Max_montain_size + 1;

        // Random initial direction.
        montains[i].DIR = std::rand() % 8;

        // Random mountain thickness.
        montains[i].thickness = std::rand() % thickness_max + 2;

        // Random chance to turn.
        montains[i].turne_chance = std::rand() % turne_chance_max;

        // Random initial chance to stop.
        montains[i].stop_chance = std::rand() % stop_chance_max + 1;

        // Random sideways movement chance.
        montains[i].lateral_noise_chance = 25 + std::rand() % 30;
    }

    // Generate mountain chains.
    create_montain(map, montains);

    /*
     *       ----------------------------
     *       PART 3: RAVINES
     *       ----------------------------
     */

    // Generate ravines after mountains.
    create_ravines(map);

    /*
     *       ----------------------------
     *       PART 4: RIVERS AND LAKES
     *       ----------------------------
     */

    // Generate rivers and final lakes.
    create_rivers(map);

    /*
     *       ----------------------------
     *       PART 5: FORESTS
     *       ----------------------------
     */

    // Generate dense forest patches.
    create_forests(map);

    // Generate isolated trees.
    create_scattered_trees(map);

    /*
     *       ----------------------------
     *       PART 6: FUTURE RESOURCES
     *       ----------------------------
     */

    // Future stone, gold and iron generation can be added here.
}


/*
 *   ============================================================
 *   MOUNTAIN GENERATION
 *   ============================================================
 */

void create_montain(MAP& map, std::vector<MONTAIN>& montains)
{
    // Row movement for each direction.
    int dx[8] = {
        -1, // north
        -1, // northeast
        0, // east
        1, // southeast
        1, // south
        1, // southwest
        0, // west
        -1  // northwest
    };

    // Column movement for each direction.
    int dy[8] = {
        0, // north
        1, // northeast
        1, // east
        1, // southeast
        0, // south
        -1, // southwest
        -1, // west
        -1  // northwest
    };

    // Generate each mountain chain.
    for (int i = 0; i < static_cast<int>(montains.size()); i++) {

        // Count generated steps.
        int steps = 0;

        // Current row.
        int x = montains[i].x_init;

        // Current column.
        int y = montains[i].y_init;

        // Paint starting cell.
        set_terrain(map, x, y, Montain);

        // Continue while chain does not stop.
        while ((std::rand() % 100) >= montains[i].stop_chance &&
            steps < max_size &&
            in_map(map, x, y)) {

            // Randomly turn the chain.
            if ((std::rand() % 100) < montains[i].turne_chance) {

                // Choose left or right turn.
                int turn = (std::rand() % 2) ? 1 : -1;

                // Apply turn.
                montains[i].DIR += turn;

                // Wrap direction after northwest.
                if (montains[i].DIR > 7) {
                    montains[i].DIR = 0;
                }

                // Wrap direction before north.
                if (montains[i].DIR < 0) {
                    montains[i].DIR = 7;
                }
            }

            // Move one mountain segment.
            for (int step = 0;
                 step < montains[i].size && steps < max_size;
            step++) {

                // Current direction.
                int dir = montains[i].DIR;

                // Apply lateral noise.
                if ((std::rand() % 100) < montains[i].lateral_noise_chance) {

                    // Choose side.
                    int side = (std::rand() % 2) ? 1 : -1;

                    // Perpendicular row movement.
                    int px = -dy[dir];

                    // Perpendicular column movement.
                    int py = dx[dir];

                    // Move sideways.
                    x += side * px;

                    // Move sideways.
                    y += side * py;
                }

                // Move forward in row.
                x += dx[dir];

                // Move forward in column.
                y += dy[dir];

                // Stop segment if outside map.
                if (!in_map(map, x, y)) {
                    break;
                }

                // Paint mountain brush.
                paint_mountain_brush(map, x, y, montains[i].thickness);

                // Count one step.
                steps++;
            }

            // Increase stop chance over time.
            montains[i].stop_chance += 0.25f;
            }
    }
}

void paint_mountain_brush(MAP& map, int cx, int cy, int thickness)
{
    // Convert thickness to radius.
    int radius = thickness / 2;

    // Browse brush rows.
    for (int dx = -radius; dx <= radius; dx++) {

        // Browse brush columns.
        for (int dy = -radius; dy <= radius; dy++) {

            // Squared distance from center.
            int dist2 = dx * dx + dy * dy;

            // Squared radius.
            int r2 = radius * radius;

            // Ignore cells outside circular brush.
            if (dist2 > r2) {
                continue;
            }

            // Always paint center.
            if (dx == 0 && dy == 0) {
                set_terrain(map, cx, cy, Montain);
            }

            // Paint inner cells often.
            else if (dist2 <= 1 && std::rand() % 100 < 85) {
                set_terrain(map, cx + dx, cy + dy, Montain);
            }

            // Paint middle cells sometimes.
            else if (dist2 < r2 && std::rand() % 100 < 65) {
                set_terrain(map, cx + dx, cy + dy, Montain);
            }

            // Paint border cells rarely.
            else if (dist2 == r2 && std::rand() % 100 < 35) {
                set_terrain(map, cx + dx, cy + dy, Montain);
            }
        }
    }
}


/*
 *   ============================================================
 *   RIVER AND LAKE GENERATION
 *   ============================================================
 */

bool find_mountain_source(const MAP& map, int& x, int& y)
{
    // Store map width.
    int width = static_cast<int>(map.size());

    // Store map height.
    int height = static_cast<int>(map[0].size());

    // Try several random positions.
    for (int attempt = 0; attempt < source_search_attempts; attempt++) {

        // Random row.
        int rx = std::rand() % width;

        // Random column.
        int ry = std::rand() % height;

        // Accept mountain cell.
        if (map[rx][ry].type_terrain == Montain) {

            // Return source row.
            x = rx;

            // Return source column.
            y = ry;

            // Source found.
            return true;
        }
    }

    // No source found.
    return false;
}

void create_rivers(MAP& map)
{
    // Preferred river directions.
    int possible_dirs[5] = {
        3, // southeast
        4, // south
        5, // southwest
        2, // east
        6  // west
    };

    // Generate each river.
    for (int i = 0; i < Max_river_quantity; i++) {

        // River source row.
        int x = 0;

        // River source column.
        int y = 0;

        // Stop if no mountain source exists.
        if (!find_mountain_source(map, x, y)) {
            return;
        }

        // Choose river direction.
        int dir = possible_dirs[std::rand() % 5];

        // Draw river from mountain.
        draw_river(map, x, y, dir);
    }
}

void draw_river(MAP& map, int x, int y, int dir)
{
    // Row movement by direction.
    int dx[8] = {
        -1, -1, 0, 1, 1, 1, 0, -1
    };

    // Column movement by direction.
    int dy[8] = {
        0,  1, 1, 1, 0,-1,-1, -1
    };

    int visible_length = 0;

    // Current river length.
    int length = 0;

    // Random river thickness: 1 or 3.
    int river_thickness = 1 + 2 * (std::rand() % 2);

    // Continue until max river size.
    while (length < Max_river_size && in_map(map, x, y)) {

        // Randomly turn river.
        if ((std::rand() % 100) < river_turn_chance) {

            // Choose left or right turn.
            int turn = (std::rand() % 2) ? 1 : -1;

            // Apply turn.
            dir += turn;

            // Wrap direction above 7.
            if (dir > 7) {
                dir = 0;
            }

            // Wrap direction below 0.
            if (dir < 0) {
                dir = 7;
            }
        }

        // Calculate next row.
        int next_x = x + dx[dir];

        // Calculate next column.
        int next_y = y + dy[dir];

        // Stop if river leaves map.
        if (!in_map(map, next_x, next_y)) {
            return;
        }

        // Read next terrain.
        TERRAIN next_terrain = map[next_x][next_y].type_terrain;

        // If river reaches lake, end.
        if (next_terrain == Lake) {
            return;
        }

        // Avoid mountains after first steps.
        if (next_terrain == Montain && length > 4) {

            // Track if another path is found.
            int found_path = 0;

            // Try a few turns.
            for (int attempt = 0; attempt < 4; attempt++) {

                // Choose turn.
                int turn = (std::rand() % 2) ? 1 : -1;

                // Test new direction.
                int new_dir = dir + turn;

                // Wrap direction above 7.
                if (new_dir > 7) {
                    new_dir = 0;
                }

                // Wrap direction below 0.
                if (new_dir < 0) {
                    new_dir = 7;
                }

                // Test row.
                int test_x = x + dx[new_dir];

                // Test column.
                int test_y = y + dy[new_dir];

                // Accept path if not blocked.
                if (in_map(map, test_x, test_y) &&
                    map[test_x][test_y].type_terrain != Montain &&
                    map[test_x][test_y].type_terrain != Lake) {

                    // Use new direction.
                    dir = new_dir;

                // Update next row.
                next_x = test_x;

                // Update next column.
                next_y = test_y;

                // Mark path found.
                found_path = 1;

                // Stop searching.
                break;
                    }
            }

            // Create lake if blocked.
            if (!found_path) {
                create_lake_from_river(map, x, y, visible_length, river_thickness);
                return;
            }
        }

        // Move to next row.
        x = next_x;

        // Move to next column.
        y = next_y;

        // Paint river after leaving source mountain.
        if (map[x][y].type_terrain != Montain || length > 3) {
            paint_river_brush(map, x, y, river_thickness);
            visible_length++;
        }

        // Increase river length.
        length++;
    }

    // Create lake at river end.
    if (in_map(map, x, y)) {
        create_lake_from_river(map, x, y, visible_length, river_thickness);
    }
}

int calculate_lake_area(int river_length, int river_thickness)
{
    /*
     *       Very small rivers should not create large lakes.
     */
    if (river_length < 10) {
        return 6 + std::rand() % 8; // 6 to 13 cells
    }

    /*
     *       Convert river length into a 0.0 to 1.0 ratio.
     */
    double length_ratio =
    static_cast<double>(river_length) / static_cast<double>(Max_river_size);

    if (length_ratio < 0.0) {
        length_ratio = 0.0;
    }

    if (length_ratio > 1.0) {
        length_ratio = 1.0;
    }

    /*
     *       Strong curve:
     *       short rivers stay small,
     *       long rivers grow much more.
     */
    double length_effect = std::pow(length_ratio, 2.0);

    int area = lake_min_area +
    static_cast<int>((lake_max_area - lake_min_area) * length_effect);

    /*
     *       Thickness bonus only matters if the river is not tiny.
     */
    if (river_thickness >= 3 && river_length > Max_river_size / 3) {
        area += 25;
    }

    /*
     *       Small random variation.
     */
    area += (std::rand() % 21) - 10;

    if (area < lake_min_area) {
        area = lake_min_area;
    }

    if (area > lake_max_area) {
        area = lake_max_area;
    }

    return area;
}

void paint_lake(MAP& map, int cx, int cy, int radius)
{
    // Browse lake rows.
    for (int x = cx - radius; x <= cx + radius; x++) {

        // Browse lake columns.
        for (int y = cy - radius; y <= cy + radius; y++) {

            // Ignore invalid cells.
            if (!in_map(map, x, y)) {
                continue;
            }

            // Row offset from center.
            int dx = x - cx;

            // Column offset from center.
            int dy = y - cy;

            // Squared distance.
            int dist2 = dx * dx + dy * dy;

            // Squared radius.
            int r2 = radius * radius;

            // Ignore outside circle.
            if (dist2 > r2) {
                continue;
            }

            // Do not replace mountains.
            if (map[x][y].type_terrain == Montain) {
                continue;
            }

            // Paint lake center.
            if (dist2 <= 1) {
                set_terrain(map, x, y, Lake);
            }

            // Paint inner area often.
            else if (dist2 < r2 && std::rand() % 100 < 85) {
                set_terrain(map, x, y, Lake);
            }

            // Paint border sometimes.
            else if (dist2 == r2 && std::rand() % 100 < 55) {
                set_terrain(map, x, y, Lake);
            }
        }
    }
}

void paint_river_brush(MAP& map, int cx, int cy, int thickness)
{
    // Convert thickness to radius.
    int radius = thickness / 2;

    // Safety check.
    if (radius < 0) {
        radius = 0;
    }

    // Browse brush rows.
    for (int dx = -radius; dx <= radius; dx++) {

        // Browse brush columns.
        for (int dy = -radius; dy <= radius; dy++) {

            // Target row.
            int x = cx + dx;

            // Target column.
            int y = cy + dy;

            // Ignore invalid cells.
            if (!in_map(map, x, y)) {
                continue;
            }

            // Do not replace mountains.
            if (map[x][y].type_terrain == Montain) {
                continue;
            }

            // Always paint center.
            if (dx == 0 && dy == 0) {
                set_terrain(map, x, y, River);
            }

            // Paint surrounding cells often.
            else if (std::abs(dx) <= radius && std::abs(dy) <= radius) {
                if (std::rand() % 100 < 85) {
                    set_terrain(map, x, y, River);
                }
            }
        }
    }
}

void create_lake_from_river(MAP& map, int x, int y, int river_length, int river_thickness)
{
    // Calculate proportional lake area.
    int area = calculate_lake_area(river_length, river_thickness);

    // Paint lake using area growth.
    paint_lake_area(map, x, y, area);
}

void paint_lake_area(MAP& map, int cx, int cy, int target_area)
{
    // Reject invalid center.
    if (!in_map(map, cx, cy)) {
        return;
    }

    // Store map width.
    int width = static_cast<int>(map.size());

    // Store map height.
    int height = static_cast<int>(map[0].size());

    // Track visited cells.
    std::vector<std::vector<int>> visited(
        width,
        std::vector<int>(height, 0)
    );

    // Cells waiting to expand.
    std::vector<std::pair<int, int>> frontier;

    // Start from lake center.
    frontier.push_back(std::make_pair(cx, cy));

    // Count painted cells.
    int painted = 0;

    // Limit lake spread.
    int max_spread = static_cast<int>(std::sqrt(target_area) * 2.4) + 3;

    // Squared spread limit.
    int max_spread2 = max_spread * max_spread;

    // Row movement.
    int dx[8] = {
        -1, -1, 0, 1, 1, 1, 0, -1
    };

    // Column movement.
    int dy[8] = {
        0,  1, 1, 1, 0,-1,-1, -1
    };

    // Expand while there are cells and area is not reached.
    while (!frontier.empty() && painted < target_area) {

        // Choose random frontier cell.
        int index = std::rand() % static_cast<int>(frontier.size());

        // Get row.
        int x = frontier[index].first;

        // Get column.
        int y = frontier[index].second;

        // Remove selected frontier cell.
        frontier[index] = frontier.back();

        // Shrink frontier.
        frontier.pop_back();

        // Ignore invalid cells.
        if (!in_map(map, x, y)) {
            continue;
        }

        // Ignore visited cells.
        if (visited[x][y]) {
            continue;
        }

        // Mark as visited.
        visited[x][y] = 1;

        // Row distance from center.
        int ox = x - cx;

        // Column distance from center.
        int oy = y - cy;

        // Squared distance from center.
        int dist2 = ox * ox + oy * oy;

        // Avoid snake-shaped lake.
        if (dist2 > max_spread2) {
            continue;
        }

        // Check if lake can replace this cell.
        if (!can_paint_lake_cell(map, x, y)) {
            continue;
        }

        // Paint lake.
        set_terrain(map, x, y, Lake);

        // Remove resource under water.
        map[x][y].type_resource = None_Resource;

        // Count painted cell.
        painted++;

        // Add neighbors.
        for (int d = 0; d < 8; d++) {

            // Neighbor row.
            int nx = x + dx[d];

            // Neighbor column.
            int ny = y + dy[d];

            // Ignore invalid neighbor.
            if (!in_map(map, nx, ny)) {
                continue;
            }

            // Ignore visited neighbor.
            if (visited[nx][ny]) {
                continue;
            }

            // Cardinal directions grow easily.
            if (d == 0 || d == 2 || d == 4 || d == 6) {
                frontier.push_back(std::make_pair(nx, ny));
            }

            // Diagonals grow with chance.
            else {
                if (std::rand() % 100 < 65) {
                    frontier.push_back(std::make_pair(nx, ny));
                }
            }
        }
    }
}

bool can_paint_lake_cell(const MAP& map, int x, int y)
{
    // Reject invalid cells.
    if (!in_map(map, x, y)) {
        return false;
    }

    // Do not replace mountains.
    if (map[x][y].type_terrain == Montain) {
        return false;
    }

    // Other terrains can become lake.
    return true;
}


/*
 *   ============================================================
 *   RAVINE GENERATION
 *   ============================================================
 */

bool find_ravine_source(const MAP& map, int& x, int& y, int& dir)
{
    // Row movement.
    int dx[8] = {
        -1, -1, 0, 1, 1, 1, 0, -1
    };

    // Column movement.
    int dy[8] = {
        0,  1, 1, 1, 0,-1,-1, -1
    };

    // Store map width.
    int width = static_cast<int>(map.size());

    // Store map height.
    int height = static_cast<int>(map[0].size());

    // Try random source positions.
    for (int attempt = 0; attempt < ravine_source_attempts; attempt++) {

        // Random row.
        int rx = std::rand() % width;

        // Random column.
        int ry = std::rand() % height;

        // Ravine must start on plain.
        if (map[rx][ry].type_terrain != Plain) {
            continue;
        }

        // Check nearby terrain.
        for (int d = 0; d < 8; d++) {

            // Neighbor row.
            int nx = rx + dx[d];

            // Neighbor column.
            int ny = ry + dy[d];

            // Ignore invalid neighbor.
            if (!in_map(map, nx, ny)) {
                continue;
            }

            // Start ravine near mountains.
            if (map[nx][ny].type_terrain == Montain) {

                // Return ravine start row.
                x = rx;

                // Return ravine start column.
                y = ry;

                // Move away from mountain.
                dir = (d + 4) % 8;

                // Source found.
                return true;
            }
        }
    }

    // No source found.
    return false;
}

void create_ravines(MAP& map)
{
    // Generate each ravine.
    for (int i = 0; i < Max_ravine_quantity; i++) {

        // Ravine start row.
        int x = 0;

        // Ravine start column.
        int y = 0;

        // Ravine direction.
        int dir = 0;

        // Stop if no source is found.
        if (!find_ravine_source(map, x, y, dir)) {
            return;
        }

        // Draw one ravine.
        draw_ravine(map, x, y, dir);
    }
}

void paint_ravine_tear_brush(MAP& map, int cx, int cy, int dir, int width)
{
    // Row movement.
    int dx[8] = {
        -1, -1, 0, 1, 1, 1, 0, -1
    };

    // Column movement.
    int dy[8] = {
        0,  1, 1, 1, 0,-1,-1, -1
    };

    // Perpendicular row.
    int px = -dy[dir];

    // Perpendicular column.
    int py = dx[dir];

    // Paint ravine center if allowed.
    if (in_map(map, cx, cy) &&
        map[cx][cy].type_terrain != River &&
        map[cx][cy].type_terrain != Lake) {
        set_terrain(map, cx, cy, ravine);
        }

        // Paint broken sides.
        for (int w = 1; w <= width; w++) {

            // Lower chance for wider border.
            int chance = 65 - w * 20;

            // Minimum border chance.
            if (chance < 15) {
                chance = 15;
            }

            // Paint one side.
            if (std::rand() % 100 < chance) {

                // Side row.
                int x1 = cx + px * w;

                // Side column.
                int y1 = cy + py * w;

                // Paint if valid.
                if (in_map(map, x1, y1) &&
                    map[x1][y1].type_terrain != River &&
                    map[x1][y1].type_terrain != Lake &&
                    map[x1][y1].type_terrain != Montain) {
                    set_terrain(map, x1, y1, ravine);
                    }
            }

            // Paint opposite side.
            if (std::rand() % 100 < chance) {

                // Opposite row.
                int x2 = cx - px * w;

                // Opposite column.
                int y2 = cy - py * w;

                // Paint if valid.
                if (in_map(map, x2, y2) &&
                    map[x2][y2].type_terrain != River &&
                    map[x2][y2].type_terrain != Lake &&
                    map[x2][y2].type_terrain != Montain) {
                    set_terrain(map, x2, y2, ravine);
                    }
            }
        }

        // Add occasional broken pixel behind.
        if (std::rand() % 100 < 35) {

            // Back row.
            int bx = cx - dx[dir];

            // Back column.
            int by = cy - dy[dir];

            // Paint only on plain.
            if (in_map(map, bx, by) &&
                map[bx][by].type_terrain == Plain) {
                set_terrain(map, bx, by, ravine);
                }
        }
}

void draw_ravine_branch(MAP& map, int x, int y, int dir, int length)
{
    // Row movement.
    int dx[8] = {
        -1, -1, 0, 1, 1, 1, 0, -1
    };

    // Column movement.
    int dy[8] = {
        0,  1, 1, 1, 0,-1,-1, -1
    };

    // Draw branch cells.
    for (int i = 0; i < length; i++) {

        // Randomly turn branch.
        if ((std::rand() % 100) < 25) {

            // Choose turn.
            int turn = (std::rand() % 2) ? 1 : -1;

            // Apply turn.
            dir += turn;

            // Wrap above 7.
            if (dir > 7) {
                dir = 0;
            }

            // Wrap below 0.
            if (dir < 0) {
                dir = 7;
            }
        }

        // Move branch row.
        x += dx[dir];

        // Move branch column.
        y += dy[dir];

        // Stop outside map.
        if (!in_map(map, x, y)) {
            return;
        }

        // Stop on blocked terrain.
        if (map[x][y].type_terrain == River ||
            map[x][y].type_terrain == Lake ||
            map[x][y].type_terrain == Montain) {
            return;
            }

            // Paint branch cell.
            set_terrain(map, x, y, ravine);
    }
}

void draw_ravine(MAP& map, int x, int y, int dir)
{
    // Row movement.
    int dx[8] = {
        -1, -1, 0, 1, 1, 1, 0, -1
    };

    // Column movement.
    int dy[8] = {
        0,  1, 1, 1, 0,-1,-1, -1
    };

    // Current ravine length.
    int length = 0;

    // Dynamic stop chance.
    float stop_chance = ravine_stop_chance;

    // Continue until max size or stop.
    while (length < Max_ravine_size && in_map(map, x, y)) {

        // Slight direction change.
        if ((std::rand() % 100) < ravine_turn_chance) {

            // Choose turn.
            int turn = (std::rand() % 2) ? 1 : -1;

            // Apply turn.
            dir += turn;

            // Wrap above 7.
            if (dir > 7) {
                dir = 0;
            }

            // Wrap below 0.
            if (dir < 0) {
                dir = 7;
            }
        }

        // Hard direction change.
        if ((std::rand() % 100) < ravine_hard_turn_chance) {

            // Choose hard turn.
            int hard_turn = (std::rand() % 2) ? 2 : -2;

            // Apply hard turn.
            dir += hard_turn;

            // Wrap above 7.
            if (dir > 7) {
                dir -= 8;
            }

            // Wrap below 0.
            if (dir < 0) {
                dir += 8;
            }
        }

        // Next row.
        int next_x = x + dx[dir];

        // Next column.
        int next_y = y + dy[dir];

        // Stop outside map.
        if (!in_map(map, next_x, next_y)) {
            return;
        }

        // Read next terrain.
        TERRAIN next_terrain = map[next_x][next_y].type_terrain;

        // Stop on water.
        if (next_terrain == River || next_terrain == Lake) {
            return;
        }

        // Stop on mountains.
        if (next_terrain == Montain) {
            return;
        }

        // Move row.
        x = next_x;

        // Move column.
        y = next_y;

        // Default ravine width.
        int width = 0;

        // Sometimes widen ravine.
        if (std::rand() % 100 < 40) {
            width = 1;
        }

        // Rarely make it wider.
        if (std::rand() % 100 < 12) {
            width = 2;
        }

        // Paint ravine tear.
        paint_ravine_tear_brush(map, x, y, dir, width);

        // Sometimes create side branch.
        if ((std::rand() % 100) < ravine_branch_chance) {

            // Choose branch side.
            int side_turn = (std::rand() % 2) ? 2 : -2;

            // Calculate branch direction.
            int branch_dir = dir + side_turn;

            // Wrap above 7.
            if (branch_dir > 7) {
                branch_dir -= 8;
            }

            // Wrap below 0.
            if (branch_dir < 0) {
                branch_dir += 8;
            }

            // Random branch length.
            int branch_length = 2 + std::rand() % 5;

            // Draw branch.
            draw_ravine_branch(map, x, y, branch_dir, branch_length);
        }

        // Increase ravine length.
        length++;

        // Increase stop chance.
        stop_chance += 0.15f;

        // Randomly stop ravine.
        if ((std::rand() % 100) < stop_chance) {
            return;
        }
    }
}


/*
 *   ============================================================
 *   FOREST AND TREE GENERATION
 *   ============================================================
 */

bool can_place_tree(const MAP& map, int x, int y)
{
    // Reject invalid cells.
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

    // Tree can be placed.
    return true;
}

bool has_terrain_near(const MAP& map, int cx, int cy, TERRAIN terrain, int radius)
{
    // Browse nearby rows.
    for (int dx = -radius; dx <= radius; dx++) {

        // Browse nearby columns.
        for (int dy = -radius; dy <= radius; dy++) {

            // Target row.
            int x = cx + dx;

            // Target column.
            int y = cy + dy;

            // Ignore invalid cells.
            if (!in_map(map, x, y)) {
                continue;
            }

            // Squared distance.
            int dist2 = dx * dx + dy * dy;

            // Squared radius.
            int r2 = radius * radius;

            // Keep circular search.
            if (dist2 > r2) {
                continue;
            }

            // Found requested terrain.
            if (map[x][y].type_terrain == terrain) {
                return true;
            }
        }
    }

    // Terrain not found.
    return false;
}

bool is_near_water(const MAP& map, int x, int y, int radius)
{
    // Check river or lake nearby.
    return has_terrain_near(map, x, y, River, radius) ||
    has_terrain_near(map, x, y, Lake, radius);
}

bool find_forest_center(const MAP& map, int& x, int& y)
{
    // Store map width.
    int width = static_cast<int>(map.size());

    // Store map height.
    int height = static_cast<int>(map[0].size());

    // Try random centers.
    for (int attempt = 0; attempt < forest_center_search_attempts; attempt++) {

        // Random row.
        int rx = std::rand() % width;

        // Random column.
        int ry = std::rand() % height;

        // Skip invalid tree cells.
        if (!can_place_tree(map, rx, ry)) {
            continue;
        }

        // Prefer forests near water.
        if (is_near_water(map, rx, ry, 5)) {

            // High acceptance chance.
            if (std::rand() % 100 < 80) {

                // Return center row.
                x = rx;

                // Return center column.
                y = ry;

                // Center found.
                return true;
            }
        }

        // Also allow dry forests.
        else {

            // Lower acceptance chance.
            if (std::rand() % 100 < 35) {

                // Return center row.
                x = rx;

                // Return center column.
                y = ry;

                // Center found.
                return true;
            }
        }
    }

    // No center found.
    return false;
}

void paint_forest_patch(MAP& map, int cx, int cy, int radius)
{
    // Squared forest radius.
    int r2 = radius * radius;

    // Browse forest rows.
    for (int x = cx - radius; x <= cx + radius; x++) {

        // Browse forest columns.
        for (int y = cy - radius; y <= cy + radius; y++) {

            // Skip invalid tree cells.
            if (!can_place_tree(map, x, y)) {
                continue;
            }

            // Row offset.
            int dx = x - cx;

            // Column offset.
            int dy = y - cy;

            // Squared distance from center.
            int dist2 = dx * dx + dy * dy;

            // Keep circular forest.
            if (dist2 > r2) {
                continue;
            }

            // Compute density from center to edge.
            int chance = forest_edge_chance +
            (forest_core_chance - forest_edge_chance) * (r2 - dist2) / r2;

            // Increase chance near water.
            if (is_near_water(map, x, y, 3)) {
                chance += near_water_tree_bonus;
            }

            // Reduce chance near ravines.
            if (has_terrain_near(map, x, y, ravine, 2)) {
                chance -= near_ravine_tree_penalty;
            }

            // Clamp minimum chance.
            if (chance < 0) {
                chance = 0;
            }

            // Clamp maximum chance.
            if (chance > 100) {
                chance = 100;
            }

            // Place tree resource.
            if (std::rand() % 100 < chance) {
                map[x][y].type_resource = tree;
            }
        }
    }
}

void create_forests(MAP& map)
{
    // Generate each forest patch.
    for (int i = 0; i < Max_forest_quantity; i++) {

        // Forest center row.
        int x = 0;

        // Forest center column.
        int y = 0;

        // Stop if no center is found.
        if (!find_forest_center(map, x, y)) {
            return;
        }

        // Random forest radius.
        int radius = forest_min_radius +
        std::rand() % (forest_max_radius - forest_min_radius + 1);

        // Paint forest patch.
        paint_forest_patch(map, x, y, radius);
    }
}

void create_scattered_trees(MAP& map)
{
    // Store map width.
    int width = static_cast<int>(map.size());

    // Store map height.
    int height = static_cast<int>(map[0].size());

    // Browse all rows.
    for (int x = 0; x < width; x++) {

        // Browse all columns.
        for (int y = 0; y < height; y++) {

            // Skip invalid tree cells.
            if (!can_place_tree(map, x, y)) {
                continue;
            }

            // Base scattered tree chance.
            int chance = scattered_tree_chance;

            // Increase chance near water.
            if (is_near_water(map, x, y, 4)) {
                chance += near_water_tree_bonus;
            }

            // Increase chance near forests.
            if (has_resource_near(map, x, y, tree, 3)) {
                chance += near_forest_tree_bonus;
            }

            // Reduce chance near ravines.
            if (has_terrain_near(map, x, y, ravine, 2)) {
                chance -= near_ravine_tree_penalty;
            }

            // Clamp minimum chance.
            if (chance < 0) {
                chance = 0;
            }

            // Clamp maximum chance.
            if (chance > 100) {
                chance = 100;
            }

            // Place scattered tree.
            if (std::rand() % 100 < chance) {
                map[x][y].type_resource = tree;
            }
        }
    }
}

bool has_resource_near(const MAP& map, int cx, int cy, RESOURCE resource, int radius)
{
    // Browse nearby rows.
    for (int dx = -radius; dx <= radius; dx++) {

        // Browse nearby columns.
        for (int dy = -radius; dy <= radius; dy++) {

            // Target row.
            int x = cx + dx;

            // Target column.
            int y = cy + dy;

            // Ignore invalid cells.
            if (!in_map(map, x, y)) {
                continue;
            }

            // Squared distance.
            int dist2 = dx * dx + dy * dy;

            // Squared radius.
            int r2 = radius * radius;

            // Keep circular search.
            if (dist2 > r2) {
                continue;
            }

            // Found requested resource.
            if (map[x][y].type_resource == resource) {
                return true;
            }
        }
    }

    // Resource not found.
    return false;
}
