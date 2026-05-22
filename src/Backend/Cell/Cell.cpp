#include "Cell.hpp"

Cell::Cell()
{
    type_terrain = Plain;
    type_struct = None_Struct;
    type_resource = None_Resource;
    type_unit = None_Unit;
    wood_type = No_Wood;

    has_berry = false;

    walkable = true;
    occupied = false;

    texture = nullptr;
}

Cell::~Cell()
{
}
