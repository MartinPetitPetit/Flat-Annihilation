#pragma once

#include <SDL2/SDL_image.h>

/*
 * ============================================================
 * TERRAIN LAYER
 * ============================================================
 */

// Ground layer.
enum TERRAIN {
	Plain,      // Basic terrain.
	Montain,   // Mountain terrain.
	Lake,      // Lake water.
	River,     // River water.
	Bush,      // Legacy terrain bush. Prefer RESOURCE::BushResource.
	ravine     // Cracked terrain.
};

/*
 * ============================================================
 * STRUCTURE LAYER
 * ============================================================
 */

// Building layer.
enum STRUCTURE {
	None_Struct,
	Usine,
	Production,
	Resource
};

/*
 * ============================================================
 * RESOURCE LAYER
 * ============================================================
 */

// Resource layer placed over terrain.
enum RESOURCE {
	None_Resource,
	tree,
	stone,
	gold,
	iron,
	Sapling,
	BushResource
};

/*
 * ============================================================
 * UNIT LAYER
 * ============================================================
 */

// Unit layer.
enum UNIT {
	None_Unit,
	archer,
	MONK
};

/*
 * ============================================================
 * WOOD TYPE
 * ============================================================
 */

// Used to draw different tree models.
enum WOOD_TYPE {
	No_Wood,
	Wood_A,
	Wood_B,
	Wood_C
};

/*
 * ============================================================
 * CELL
 * ============================================================
 */

class Cell
{
public:
	Cell();
	virtual ~Cell();

	TERRAIN type_terrain;
	STRUCTURE type_struct;
	RESOURCE type_resource;
	UNIT type_unit;
	WOOD_TYPE wood_type;

	bool has_berry;

	bool walkable;
	bool occupied;

	SDL_Texture* texture;
};
