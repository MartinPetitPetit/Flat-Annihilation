#include "Collector.hpp"

Collector create_collector_data(int x, int y, int owner_id)
{
    Collector collector;

    collector.base.type = Collector_Unit;

    collector.base.x = x;
    collector.base.y = y;

    collector.base.hp = 50;
    collector.base.max_hp = 50;

    collector.base.movement_speed = 1.0f;

    collector.base.owner_id = owner_id;

    collector.base.allowed_terrains = {
        Plain
    };

    collector.collect_amount = 5;
    collector.collect_speed = 1.0f;

    collector.current_target_resource = None_Resource;

    collector.is_collecting = false;

    collector.carried_amount = 0;
    collector.max_capacity = 20;

    return collector;
}
