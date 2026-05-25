#include "Cell.hpp"

Cell::Cell()
{
    type_terrain   = Plain;
    walkable       = true;
    occupied       = false;
    buildingID     = -1;
    buildingOwner  = -1;
	this->resource = nullptr;
	this->unit = nullptr;
}

Cell::~Cell() {}