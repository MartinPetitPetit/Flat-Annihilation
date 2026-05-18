#include "UnitManager.hpp"

UnitManager::UnitManager()
{
    next_unit_id = 0;
}

bool UnitManager::inside_map(const MAP& map, int x, int y) const
{
    return x >= 0 &&
    y >= 0 &&
    x < static_cast<int>(map.size()) &&
    y < static_cast<int>(map[0].size());
}

bool UnitManager::cell_is_free_for_unit(const MAP& map, int x, int y) const
{
    if (!inside_map(map, x, y))
        return false;

    const Cell& cell = map[x][y];

    if (!cell.walkable)
        return false;

    if (cell.occupied)
        return false;

    if (cell.type_unit != None_Unit)
        return false;

    if (cell.type_struct != None_Struct)
        return false;

    return true;
}

int UnitManager::create_collector(MAP& map, int x, int y, int owner_id)
{
    if (!inside_map(map, x, y))
        return -1;

    Collector collector = create_collector_data(x, y, owner_id);

    if (!collector.base.can_walk_on(map[x][y].type_terrain))
        return -1;

    if (!cell_is_free_for_unit(map, x, y))
        return -1;

    collectors.push_back(collector);

    int collector_index = static_cast<int>(collectors.size()) - 1;
    int unit_id = next_unit_id++;

    UnitRecord record;
    record.type = Collector_Unit;
    record.index = collector_index;
    record.alive = true;

    units[unit_id] = record;

    map[x][y].type_unit = Collector_Unit;
    map[x][y].unit_id = unit_id;
    map[x][y].occupied = true;

    return unit_id;
}

bool UnitManager::move_unit(MAP& map, int unit_id, int new_x, int new_y)
{
    auto it = units.find(unit_id);

    if (it == units.end())
        return false;

    UnitRecord& record = it->second;

    if (!record.alive)
        return false;

    if (!inside_map(map, new_x, new_y))
        return false;

    if (record.type == Collector_Unit)
    {
        Collector& collector = collectors[record.index];

        if (!collector.base.can_walk_on(map[new_x][new_y].type_terrain))
            return false;

        if (!cell_is_free_for_unit(map, new_x, new_y))
            return false;

        int old_x = collector.base.x;
        int old_y = collector.base.y;

        if (!inside_map(map, old_x, old_y))
            return false;

        map[old_x][old_y].type_unit = None_Unit;
        map[old_x][old_y].unit_id = -1;
        map[old_x][old_y].occupied = false;

        collector.base.x = new_x;
        collector.base.y = new_y;

        map[new_x][new_y].type_unit = Collector_Unit;
        map[new_x][new_y].unit_id = unit_id;
        map[new_x][new_y].occupied = true;

        return true;
    }

    return false;
}

bool UnitManager::remove_unit(MAP& map, int unit_id)
{
    auto it = units.find(unit_id);

    if (it == units.end())
        return false;

    UnitRecord& record = it->second;

    if (!record.alive)
        return false;

    if (record.type == Collector_Unit)
    {
        Collector& collector = collectors[record.index];

        int x = collector.base.x;
        int y = collector.base.y;

        if (inside_map(map, x, y))
        {
            map[x][y].type_unit = None_Unit;
            map[x][y].unit_id = -1;
            map[x][y].occupied = false;
        }

        record.alive = false;

        return true;
    }

    return false;
}

Collector* UnitManager::get_collector(int unit_id)
{
    auto it = units.find(unit_id);

    if (it == units.end())
        return nullptr;

    UnitRecord& record = it->second;

    if (!record.alive)
        return nullptr;

    if (record.type != Collector_Unit)
        return nullptr;

    return &collectors[record.index];
}

const Collector* UnitManager::get_collector(int unit_id) const
{
    auto it = units.find(unit_id);

    if (it == units.end())
        return nullptr;

    const UnitRecord& record = it->second;

    if (!record.alive)
        return nullptr;

    if (record.type != Collector_Unit)
        return nullptr;

    return &collectors[record.index];
}

int UnitManager::get_unit_x(int unit_id) const
{
    auto it = units.find(unit_id);

    if (it == units.end())
        return -1;

    const UnitRecord& record = it->second;

    if (!record.alive)
        return -1;

    if (record.type == Collector_Unit)
        return collectors[record.index].base.x;

    return -1;
}

int UnitManager::get_unit_y(int unit_id) const
{
    auto it = units.find(unit_id);

    if (it == units.end())
        return -1;

    const UnitRecord& record = it->second;

    if (!record.alive)
        return -1;

    if (record.type == Collector_Unit)
        return collectors[record.index].base.y;

    return -1;
}

UNIT UnitManager::get_unit_type(int unit_id) const
{
    auto it = units.find(unit_id);

    if (it == units.end())
        return None_Unit;

    const UnitRecord& record = it->second;

    if (!record.alive)
        return None_Unit;

    return record.type;
}

int UnitManager::get_collector_count() const
{
    return static_cast<int>(collectors.size());
}
