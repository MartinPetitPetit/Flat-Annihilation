#pragma once

#include <vector>
#include <unordered_map>

#include "Map.hpp"
#include "Collector.hpp"

struct UnitRecord {
    UNIT type;
    int index;
    bool alive;
};

class UnitManager {
private:
    int next_unit_id;

    std::vector<Collector> collectors;

    std::unordered_map<int, UnitRecord> units;

    bool inside_map(const MAP& map, int x, int y) const;
    bool cell_is_free_for_unit(const MAP& map, int x, int y) const;

public:
    UnitManager();

    int create_collector(MAP& map, int x, int y, int owner_id);

    bool move_unit(MAP& map, int unit_id, int new_x, int new_y);

    bool remove_unit(MAP& map, int unit_id);

    Collector* get_collector(int unit_id);
    const Collector* get_collector(int unit_id) const;

    int get_unit_x(int unit_id) const;
    int get_unit_y(int unit_id) const;

    UNIT get_unit_type(int unit_id) const;

    int get_collector_count() const;
};
