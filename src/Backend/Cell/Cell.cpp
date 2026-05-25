#include "Cell.hpp"

Cell::Cell()
{
    type_terrain   = Plain;
    type_resource  = None_Resource;
    type_unit      = None_Unit;
    wood_type      = No_Wood;
    has_berry      = false;
    walkable       = true;
    occupied       = false;
    buildingID     = -1;
    buildingOwner  = -1;
    texture        = nullptr;
}

Cell::~Cell() {}