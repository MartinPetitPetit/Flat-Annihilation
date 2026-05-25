#include "Cell.hpp"

Cell::Cell()
{
	this->type_terrain = Plain;
	this->type_struct = None_Struct;
	this->resource = nullptr;
	this->unit = nullptr;

	this->walkable = true;
	this->occupied = false;
}

Cell::~Cell()
{
	if (resource != nullptr) {
		delete resource;
	}
}

