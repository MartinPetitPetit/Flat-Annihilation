#include "Base.hpp"

static bool inside_map(const MAP& map, int x, int y)
{
    return x >= 0 &&
    y >= 0 &&
    x < static_cast<int>(map.size()) &&
    y < static_cast<int>(map[0].size());
}

static bool can_place_base_on(const Cell& cell)
{
    if (cell.type_terrain != Plain)
        return false;

    if (cell.type_struct != None_Struct)
        return false;

    if (cell.type_unit != None_Unit)
        return false;

    if (cell.type_resource != None_Resource)
        return false;

    if (cell.occupied)
        return false;

    return true;
}

bool place_base(
    MAP& map,
    int x,
    int y,
    int player_id
)
{
    if (!inside_map(map, x, y))
        return false;

    Cell& cell = map[x][y];

    if (!can_place_base_on(cell))
        return false;

    cell.type_struct = Base;
    cell.struct_owner_id = player_id;

    cell.walkable = false;
    cell.occupied = true;

    return true;
}

int create_collectors_around_base(
    MAP& map,
    UnitManager& unitManager,
    int base_x,
    int base_y,
    int player_id,
    int quantity
)
{
    if (!inside_map(map, base_x, base_y))
        return 0;

    if (map[base_x][base_y].type_struct != Base)
        return 0;

    int created = 0;

    const int dx[] = {
        0,  1,  0, -1,
        -1,  1, -1,  1
    };

    const int dy[] = {
        -1,  0,  1,  0,
        -1, -1,  1,  1
    };

    const int total_positions = 8;

    for (int i = 0; i < total_positions && created < quantity; i++)
    {
        int nx = base_x + dx[i];
        int ny = base_y + dy[i];

        int id = unitManager.create_collector(
            map,
            nx,
            ny,
            player_id
        );

        if (id != -1)
            created++;
    }

    for (int radius = 2; radius <= 4 && created < quantity; radius++)
    {
        for (int ox = -radius; ox <= radius && created < quantity; ox++)
        {
            for (int oy = -radius; oy <= radius && created < quantity; oy++)
            {
                bool border =
                ox == -radius ||
                ox ==  radius ||
                oy == -radius ||
                oy ==  radius;

                if (!border)
                    continue;

                int nx = base_x + ox;
                int ny = base_y + oy;

                int id = unitManager.create_collector(
                    map,
                    nx,
                    ny,
                    player_id
                );

                if (id != -1)
                    created++;
            }
        }
    }

    return created;
}

bool create_player_base_with_collectors(
    MAP& map,
    UnitManager& unitManager,
    int base_x,
    int base_y,
    int player_id,
    int collector_quantity
)
{
    bool base_created = place_base(
        map,
        base_x,
        base_y,
        player_id
    );

    if (!base_created)
        return false;

    int collectors_created = create_collectors_around_base(
        map,
        unitManager,
        base_x,
        base_y,
        player_id,
        collector_quantity
    );

    return collectors_created == collector_quantity;
}
