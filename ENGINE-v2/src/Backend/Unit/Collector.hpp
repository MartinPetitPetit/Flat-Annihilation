#pragma once

#include "Unit.hpp"

struct Collector {
    UnitBase base;

    int collect_amount;
    float collect_speed;

    RESOURCE current_target_resource;

    bool is_collecting;

    int carried_amount;
    int max_capacity;
};

Collector create_collector_data(int x, int y, int owner_id);
