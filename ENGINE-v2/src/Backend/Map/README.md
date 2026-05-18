FLAT ANNIHILATION
GLOBAL PROCEDURAL MAP GENERATION DOCUMENTATION
Provided source version: Cell.hpp, Map.hpp, Map.cpp, Mountain.cpp, Ravine.cpp, River.cpp, Forest.cpp, Bush.cpp

===============================================================================
1. PURPOSE OF THE MAP GENERATOR
===============================================================================

The procedural map generator creates a 2D natural map for Flat Annihilation.

The generator works over a matrix of cells:

    using MAP = std::vector<std::vector<Cell>>;

Each Cell contains independent layers for terrain, structures, resources, units,
and visual/resource states. The main design goal is to keep terrain and resources
separate. For example, trees and bushes do not replace the ground layer; they are
placed on top of Plain terrain as resources.

The generator builds the map in successive phases:

    1. Reset the map to a clean Plain state.
    2. Generate mountain chains.
    3. Generate ravines.
    4. Generate rivers and lakes.
    5. Generate forest patches and scattered trees.
    6. Generate bush patches and scattered bushes with optional berries.

The order matters because later systems depend on earlier terrain features.
Rivers depend on mountains and ravines. Forests and bushes react to water and
ravines.

===============================================================================
2. CELL AND MAP DATA MODEL
===============================================================================

2.1. Cell layers

Each Cell contains:

    TERRAIN type_terrain;
    STRUCTURE type_struct;
    RESOURCE type_resource;
    UNIT type_unit;
    WOOD_TYPE wood_type;
    bool has_berry;
    bool walkable;
    bool occupied;
    SDL_Texture* texture;

2.2. Terrain layer

The terrain layer is the physical ground layer:

    Plain      Basic terrain.
    Montain    Mountain terrain.
    Lake       Lake water.
    River      River water.
    Bush       Legacy terrain bush. Prefer RESOURCE::BushResource.
    ravine     Cracked terrain / ravine.

Important:
    Bush exists in the TERRAIN enum as a legacy value, but the current bush
    generation system uses RESOURCE::BushResource instead.

2.3. Structure layer

Structures are represented by:

    None_Struct
    Usine
    Production
    Resource

The current natural map generator resets this layer to None_Struct but does not
actively generate buildings.

2.4. Resource layer

Resources are placed over terrain:

    None_Resource
    tree
    stone
    gold
    iron
    Sapling
    BushResource

Trees and bushes are resources, not terrain. This is important because it allows
a cell to remain Plain while still containing a tree or bush.

2.5. Unit layer

Units are represented by:

    None_Unit
    archer
    MONK

The generator resets units to None_Unit.

2.6. Wood type

Trees can be rendered with different visual models:

    No_Wood
    Wood_A
    Wood_B
    Wood_C

2.7. Berry state

Berries are not a separate resource. They are a state of a bush:

    map[x][y].type_resource = BushResource;
    map[x][y].has_berry = true or false;

===============================================================================
3. MAIN FILE RESPONSIBILITIES
===============================================================================

Cell.hpp
    Declares terrain, structure, resource, unit, wood type enums, and the Cell
    class.

Cell.cpp
    Initializes a Cell with default values:
        terrain = Plain
        structure = None_Struct
        resource = None_Resource
        unit = None_Unit
        wood_type = No_Wood
        has_berry = false
        walkable = true
        occupied = false
        texture = nullptr

Map.hpp
    Declares the MAP type, the optional Map wrapper class, base generation
    constants, GenerationConfig, generation structs, and all public generation
    function prototypes.

Map.cpp
    Builds GenerationConfig from the map size, resets the map, and runs the full
    generation pipeline.

Mountain.cpp
    Generates distributed organic mountain chains.

Ravine.cpp
    Generates ravines as dry cracks/cuts starting near mountains.

River.cpp
    Measures mountain components, creates main rivers and tributaries from
    mountain sides, accumulates flow, paints rivers, and creates pushed-away
    terminal lakes.

Forest.cpp
    Generates forest patches and scattered trees as resources on Plain terrain.

Bush.cpp
    Generates bush patches and scattered bushes as resources on Plain terrain,
    with optional berries.

===============================================================================
4. DYNAMIC GENERATION CONFIGURATION
===============================================================================

The generator does not rely only on fixed constants. It builds a runtime
GenerationConfig using:

    GenerationConfig make_generation_config(int width, int height);

The configuration uses:

    area_ratio   = (width * height) / (BASE_MAP_WIDTH * BASE_MAP_HEIGHT)
    linear_ratio = sqrt(area_ratio)

The base reference map is:

    BASE_MAP_WIDTH  = 300
    BASE_MAP_HEIGHT = 300

4.1. Why two ratios exist

area_ratio is used for systems that scale with map area:
    - number of forest patches
    - number of bush patches
    - lake area

linear_ratio is used for systems that scale with distance or length:
    - mountain length
    - mountain thickness
    - search attempts
    - forest radius
    - bush radius

4.2. Mountain configuration

Base values:

    BASE_Max_montain_quantity = 9
    BASE_Max_montain_size     = 1
    BASE_thickness_max        = 5
    BASE_turne_chance_max     = 18
    BASE_stop_chance_max      = 3
    BASE_min_montain_steps    = 45
    BASE_max_montain_steps    = 160
    BASE_montain_stop_growth  = 0.10f

Runtime scaling:

    max_montain_quantity = BASE_Max_montain_quantity * linear_ratio, clamped 3..30
    thickness_max        = BASE_thickness_max * linear_ratio, clamped 3..12
    min_montain_steps    = BASE_min_montain_steps * linear_ratio, clamped 20..max_dim
    max_montain_steps    = BASE_max_montain_steps * linear_ratio, clamped 80..max_dim

Design idea:
    Larger maps should not simply create an excessive number of mountain chains.
    Instead, the chains also become longer and wider.

4.3. River and lake configuration

Base values:

    BASE_source_search_attempts = 500
    BASE_Max_river_quantity     = 20
    BASE_Max_river_size         = 120
    BASE_lake_min_area          = 15
    BASE_lake_max_area          = 350
    BASE_river_min_thickness    = 1
    BASE_river_max_thickness    = 3
    BASE_river_turn_chance      = 25

Runtime scaling:

    source_search_attempts = BASE_source_search_attempts * linear_ratio
    max_river_quantity     = BASE_Max_river_quantity * linear_ratio
    max_river_size         = min_dim * 0.40, clamped 20..max_dim*2
    lake_min_area          = area * 0.00017, clamped 4..300
    lake_max_area          = area * 0.00390, clamped 12..area/8
    river_max_thickness    = BASE_river_max_thickness * linear_ratio, clamped 1..7

4.4. Ravine configuration

Base values:

    BASE_Max_ravine_quantity      = 8
    BASE_Max_ravine_size          = 45
    BASE_ravine_turn_chance       = 12
    BASE_ravine_hard_turn_chance  = 8
    BASE_ravine_stop_chance       = 2
    BASE_ravine_branch_chance     = 12
    BASE_ravine_source_attempts   = 1000

Runtime scaling:

    max_ravine_quantity = BASE_Max_ravine_quantity * linear_ratio, clamped 1..80
    max_ravine_size     = min_dim * 0.15, clamped 8..max_dim
    ravine_source_attempts = BASE_ravine_source_attempts * linear_ratio

4.5. Forest configuration

Base values:

    BASE_Max_forest_quantity          = 12
    BASE_forest_min_radius            = 4
    BASE_forest_max_radius            = 12
    BASE_forest_center_search_attempts = 1000
    BASE_forest_core_chance           = 85
    BASE_forest_edge_chance           = 20
    BASE_scattered_tree_chance        = 2
    BASE_near_water_tree_bonus        = 6
    BASE_near_forest_tree_bonus       = 3
    BASE_near_ravine_tree_penalty     = 4

Runtime scaling:

    max_forest_quantity = BASE_Max_forest_quantity * area_ratio
    forest_min_radius   = BASE_forest_min_radius * linear_ratio
    forest_max_radius   = BASE_forest_max_radius * linear_ratio

4.6. Wood type probabilities

Base values:

    BASE_wood_type_b_chance = 35
    BASE_wood_type_c_chance = 15

Tree type selection:
    - Wood_C if roll < wood_type_c_chance
    - Wood_B if roll < wood_type_c_chance + wood_type_b_chance
    - Wood_A otherwise

4.7. Bush and berry configuration

Base values:

    BASE_Max_bush_patch_quantity        = 10
    BASE_bush_min_radius                = 3
    BASE_bush_max_radius                = 7
    BASE_bush_center_search_attempts    = 1000
    BASE_bush_core_chance               = 80
    BASE_bush_edge_chance               = 20
    BASE_scattered_bush_chance          = 1
    BASE_dense_bush_berry_chance        = 45
    BASE_scattered_bush_berry_chance    = 25
    BASE_near_water_bush_bonus          = 4
    BASE_near_ravine_bush_penalty       = 3

===============================================================================
5. FULL GENERATION PIPELINE
===============================================================================

The full pipeline is implemented by:

    void generate_map(MAP& map);

5.1. Empty map guard

The function returns immediately if the map is empty.

5.2. Random seed

The generator seeds std::rand() only once:

    static bool seed_done = false;

This prevents reseeding during repeated generation calls.

5.3. Reset phase

Every cell is reset:

    type_terrain  = Plain
    type_struct   = None_Struct
    type_unit     = None_Unit
    type_resource = None_Resource
    wood_type     = No_Wood
    has_berry     = false
    walkable      = true
    occupied      = false

5.4. Mountain initialization

Map.cpp creates a vector of MONTAIN objects using cfg.max_montain_quantity.

Each mountain receives:
    - random x/y start, later overwritten by distributed start logic in Mountain.cpp
    - size = 1
    - random DIR from 0 to 7
    - thickness based on cfg.thickness_max
    - tip_thickness 1 or 2
    - target_steps between cfg.min_montain_steps and cfg.max_montain_steps
    - turne_chance
    - stop_chance
    - lateral_noise_chance

Then it calls:

    create_montain(map, montains, cfg);

5.5. Remaining pipeline

The rest of the generation order is:

    create_ravines(map, cfg);
    create_rivers(map, cfg);
    create_forests(map, cfg);
    create_scattered_trees(map, cfg);
    create_bush_patches(map, cfg);
    create_scattered_bushes(map, cfg);

===============================================================================
6. SHARED MAP UTILITY FUNCTIONS
===============================================================================

create_map(width, height)
    Creates a MAP with width rows and height columns.

in_map(map, x, y)
    Returns true if x/y are inside the current map.

set_terrain(map, x, y, terrain)
    Sets type_terrain if the coordinate is valid.

affiche_map(map)
    Prints the map using emojis:
        Plain   = green square
        Montain = brown square
        River   = blue square
        Lake    = blue square
        other   = black square

has_terrain_near(map, cx, cy, terrain, radius)
    Searches in a circular radius for a terrain type.

has_resource_near(map, cx, cy, resource, radius)
    Searches in a circular radius for a resource type.

is_near_water(map, x, y, radius)
    Returns true if River or Lake is near the cell.

===============================================================================
7. MOUNTAIN GENERATION
===============================================================================

File:
    Mountain.cpp

Public functions:
    create_montain(...)
    paint_mountain_brush(...)

Internal helpers:
    mountain_clamp_int(...)
    mountain_rand_between(...)
    mountain_shuffle_positions(...)
    build_mountain_start_positions(...)
    compute_mountain_thickness(...)

7.1. Mountain distribution

Mountains are not started by pure global randomness. Mountain.cpp rebuilds the
mountain starting positions using spatial regions.

The algorithm:
    1. Computes the map aspect ratio.
    2. Builds a region grid close to the requested mountain count.
    3. Picks one random start inside each region.
    4. Uses a margin inside each region to avoid grid-border starts.
    5. Shuffles the selected positions.
    6. Keeps only the requested amount.

Purpose:
    Avoid clustering all mountain chains in the same area.

7.2. Mountain shape

Mountain thickness is computed by:

    compute_mountain_thickness(step, target_steps, tip_thickness, center_thickness)

It uses a sine bell profile:

    progress = step / (target_steps - 1)
    center_factor = sin(progress * pi)

Result:
    - start of chain  = thin
    - middle of chain = thick
    - end of chain    = thin

7.3. Mountain movement

Each mountain uses one of eight directions:

    0 north
    1 northeast
    2 east
    3 southeast
    4 south
    5 southwest
    6 west
    7 northwest

During generation:
    - The chain may turn by +1 or -1 direction based on turne_chance.
    - The chain may receive lateral noise by moving perpendicular to its direction.
    - Random stopping is only allowed after 75% of the intended chain length.
    - stop_chance grows slowly during the final part of the chain.

7.4. Mountain painting

paint_mountain_brush(map, cx, cy, thickness)
    Converts thickness to radius.
    Paints a circular area.
    Does not overwrite River or Lake.
    Clears resources and berries on painted mountain cells.

Brush behavior:
    - Center cell always becomes Montain.
    - Inner cells are very likely to become Montain.
    - Middle cells are likely.
    - Border cells are irregular and probabilistic.

===============================================================================
8. RAVINE GENERATION
===============================================================================

File:
    Ravine.cpp

Public functions:
    find_ravine_source(...)
    create_ravines(...)
    draw_ravine(...)
    paint_ravine_tear_brush(...)
    draw_ravine_branch(...)

8.1. Ravine source

find_ravine_source(...)
    Searches random Plain cells.
    A source is accepted only if there is an adjacent Montain cell.
    The initial ravine direction is set to move away from that mountain.

8.2. Ravine creation loop

create_ravines(...)
    Tries to create cfg.max_ravine_quantity ravines.
    If no source is found, it returns.

8.3. Ravine path

draw_ravine(...)
    Advances the ravine step by step.

Behavior:
    - Slight direction changes use cfg.ravine_turn_chance.
    - Hard direction changes use cfg.ravine_hard_turn_chance.
    - The ravine stops when leaving the map.
    - The ravine stops before crossing River, Lake, or Montain.
    - Width is randomized at each step:
        width = 0 by default
        40% chance of width = 1
        12% chance of width = 2
    - Small lateral branches may be created using cfg.ravine_branch_chance.
    - stop_chance increases over time.

8.4. Ravine painting

paint_ravine_tear_brush(...)
    Paints the central ravine cell.
    Paints broken lateral borders perpendicular to direction.
    Does not paint over River, Lake, or Montain.
    Clears resources and berries.
    May also paint one broken pixel behind the current step.

8.5. Ravine branches

draw_ravine_branch(...)
    Creates short side cracks.
    Branches may randomly turn.
    Branches stop at map borders, River, Lake, or Montain.
    Branches clear resources and berries on painted cells.

===============================================================================
9. RIVER AND LAKE GENERATION
===============================================================================

File:
    River.cpp

Top-level behavior in the provided version:
    - Measure mountain components.
    - For each relevant mountain, create one main river from a mountain border.
    - Add tributaries from the same side of the mountain.
    - Tributaries try to join the existing river network.
    - Flow is accumulated through downstream paths.
    - Rivers are painted with thickness based on accumulated flow.
    - Terminal lakes are created from path endpoints.
    - Lake centers are pushed away from mountains.
    - Lake growth is biased away from mountains.

9.1. Core structures

RiverPoint
    Simple coordinate:
        int x;
        int y;

MountainComponent
    Measured connected mountain component:
        mountain_cells
        border_cells
        center cx/cy
        size

RiverPath
    A generated path:
        cells
        join_path
        join_index
        joins_existing

InfluenceField
    2D matrix of mountain influence values.

9.2. Mountain influence

mountain_influence_at(map, x, y, radius)
    Computes local mountain influence by summing nearby mountain cells.
    Contribution is stronger when the mountain cell is closer.

build_mountain_influence_field(map)
    Builds the influence field:
        - Montain cells get high influence value 120.
        - Other cells receive influence based on nearby Montain cells.

Purpose:
    Rivers can follow mountain flanks without entering mountains.

9.3. Mountain measurement

measure_mountains(map)
    Uses BFS/flood-fill to find connected Montain components.
    For each component:
        - stores all mountain cells
        - computes center cx/cy
        - collects border cells

Border cells are Plain cells adjacent to the mountain component.
Border cells are rejected if:
        - not Plain
        - near Lake within radius 5
        - near River within radius 4

Mountain components are sorted by descending size.

9.4. Mountain side selection

The code divides the mountain surroundings into four logical sides:

    0 north
    1 east
    2 south
    3 west

side_of_point_relative_to_mountain(...)
    Determines a border cell side based on its position relative to the mountain
    center.

choose_best_side_for_mountain(...)
    Chooses the side with the most valid border cells.

candidates_on_side(...)
    Extracts all valid source candidates on the chosen side.

choose_tangent_direction_for_side(...)
    Determines a tangent flow direction:
        - north/south side -> flow east or west
        - east/west side   -> flow north or south

choose_main_source_on_side(...)
    Chooses a main source on the selected side, biased toward the beginning of
    the side according to the tangent direction.

9.5. Main river path

build_main_river_from_mountain(...)
    Generates the main river path for one mountain.

Main path principles:
    - Start from a selected mountain border source.
    - Follow the mountain flank.
    - Avoid entering Montain, ravine, or Lake.
    - Avoid existing nearby lakes and rivers.
    - Avoid wrapping around the entire mountain.
    - Once the river leaves the mountain system, it mostly continues straight.
    - Do not move back toward stronger mountain influence after leaving.
    - Reject paths shorter than a minimum length.

Anti-contour logic:
    If the river starts on one mountain side but begins moving to another side
    for several steps, it is considered to be wrapping around the mountain.
    In that case, the river switches into "left_mountain" mode.

left_mountain mode:
    - Candidates are restricted around the exit direction.
    - Cells too close to mountains are rejected.
    - Cells with increasing mountain influence are rejected.
    - Straight continuation is strongly preferred.

9.6. River scoring

mountain_follow_score(...)
    Scores candidate cells while the river is still following the mountain.

It rewards:
    - mountain flank at medium distance
    - useful mountain influence

It penalizes:
    - being too close to mountain
    - flowing directly into a mountain wall
    - too little mountain context
    - too much mountain influence
    - cells adjacent to mountains

9.7. Tributaries

extra_springs_for_mountain(...)
    Computes additional spring count based on mountain size and map size.

build_tributary_to_network(...)
    Generates a tributary from the same mountain side toward the existing river
    network.

Tributary behavior:
    - Uses current_dir - 1, current_dir, current_dir + 1 candidates.
    - Moves toward the nearest river network cell.
    - Joins only if it reaches existing network.
    - If it never joins, the tributary is discarded.

9.8. River network registration

register_path_in_network(...)
    Stores the path id and local index of every river cell in network_path and
    network_index.

These matrices allow tributaries to know where they joined and allow flow to
continue downstream through the receiving path.

9.9. Downstream flow

build_full_downstream_path(paths, path_id)
    Builds the complete downstream path for a river or tributary.

If a path joins another path:
    - It appends its own cells.
    - Then it appends cells from the joined path after the join point.
    - It continues recursively through joined paths.

This is the mechanism that makes tributary flow continue into the main river.

add_flow_to_path(...)
    Adds +1 flow to every cell in the full downstream path.

river_thickness_from_flow(...)
    Converts accumulated integer flow to thickness:
        flow <= 1  -> thickness 1
        flow == 2  -> thickness 3
        flow <= 4  -> thickness 5
        flow > 4   -> thickness 7

The final value is clamped by cfg.river_max_thickness and forced to be odd when
greater than 1.

paint_river_network(...)
    Paints every cell with flow > 0 using paint_river_brush(...).

9.10. River painting

paint_river_brush(map, cx, cy, thickness)
    Paints a circular river brush.
    Does not paint over:
        - Montain
        - ravine
        - Lake

Brush behavior:
    - Center is always painted.
    - Inner area is painted with high probability.
    - Border cells are irregular.
    - Resources and berries are cleared.

9.11. Lake endpoint grouping

create_rivers(...) groups all full downstream paths by final endpoint:

    lake_volume_by_end[end] += full_path.size()
    lake_sources_by_end[end] += 1

Then it creates one lake for each endpoint.

Lake thickness input is based on how many sources contribute to that endpoint.

9.12. Lake area

calculate_lake_area(...)
    Computes lake area from:
        - river_volume
        - river_thickness
        - cfg.max_river_size
        - cfg.river_max_thickness
        - cfg.lake_min_area
        - cfg.lake_max_area

The ratio is clamped between 0.22 and 3.20.
The size factor uses pow(ratio, 1.12).
The area also receives:
        river_thickness * river_thickness * 32
        random variation between -15 and +15

The hard cap is max_area * 2.

9.13. Lake position correction

push_lake_center_away_from_mountains(...)
    Moves the lake center away from nearby mountains before painting.

It uses:
    safe_radius = min_dim / 35, clamped 10..20
    max_push_steps = min_dim / 16, clamped 18..42

If cfg.lake_max_area > 300, safe_radius receives +2.

The function computes a vector pointing away from nearby mountain cells and
moves the lake center along that vector while it is still inside mountain
proximity.

9.14. Lake connector

paint_river_connector_to_lake(...)
    If the lake center is moved, this function paints a short river connector
    from the original river endpoint to the shifted lake center.

9.15. Mountain-away lake growth

mountain_away_vector(...)
    Computes a normalized vector pointing away from mountain cells around the
    lake center.

paint_lake_area_away_from_mountain(...)
    Paints the lake using frontier expansion, but rejects cells that grow
    backward toward mountains.

Additional protections:
    - uses can_paint_lake_cell(...)
    - rejects cells too far from the lake center
    - avoids expanding through existing River cells except the lake center
    - rejects growth with negative dot product against the away vector

9.16. can_paint_lake_cell(...)

A lake cell is rejected if:
    - outside map
    - terrain is Montain
    - terrain is ravine
    - too close to mountain, using radius 8

9.17. Fallback lake painting

paint_lake_area(...)
    Uses randomized frontier expansion without the mountain-away vector.
    Still uses can_paint_lake_cell(...).

paint_lake(...)
    Legacy circular API that converts radius to area and calls paint_lake_area.

===============================================================================
10. FOREST AND TREE GENERATION
===============================================================================

File:
    Forest.cpp

Public functions:
    can_place_tree(...)
    choose_wood_type(...)
    place_tree(...)
    find_forest_center(...)
    paint_forest_patch(...)
    create_forests(...)
    create_scattered_trees(...)

10.1. Placement rule

A tree can be placed only if:
    - the coordinate is inside the map
    - type_terrain == Plain
    - type_resource == None_Resource

10.2. Tree placement

place_tree(...)
    Sets:
        type_resource = tree
        wood_type = selected wood type
        has_berry = false

10.3. Forest center search

find_forest_center(...)
    Tries random positions.

A valid center must be a valid tree placement cell.

Acceptance probability:
    - near water within radius 5: 80%
    - dry location: 35%

10.4. Forest patch painting

paint_forest_patch(...)
    Paints a circular forest patch.

Chance is interpolated from:
    forest_core_chance at the center
    forest_edge_chance near the edge

Modifiers:
    - near water within radius 3: +near_water_tree_bonus
    - near ravine within radius 2: -near_ravine_tree_penalty

The final chance is clamped to 0..100.

One wood type is selected per forest patch, so a patch has visual consistency.

10.5. Forest patch creation

create_forests(...)
    Repeats up to cfg.max_forest_quantity.
    If no center is found, it returns.
    Radius is random between cfg.forest_min_radius and cfg.forest_max_radius.

10.6. Scattered trees

create_scattered_trees(...)
    Scans the full map.
    For every valid tree cell:
        base chance = cfg.scattered_tree_chance

Modifiers:
    - near water within radius 4: +near_water_tree_bonus
    - near existing tree within radius 3: +near_forest_tree_bonus
    - near ravine within radius 2: -near_ravine_tree_penalty

The final chance is clamped to 0..100.

===============================================================================
11. BUSH AND BERRY GENERATION
===============================================================================

File:
    Bush.cpp

Public functions:
    can_place_bush(...)
    place_bush(...)
    find_bush_center(...)
    paint_bush_patch(...)
    create_bush_patches(...)
    create_scattered_bushes(...)

11.1. Placement rule

A bush can be placed only if:
    - the coordinate is inside the map
    - type_terrain == Plain
    - type_resource == None_Resource

11.2. Bush placement

place_bush(...)
    Sets:
        type_resource = BushResource
        has_berry = random roll against berry_chance
        wood_type = No_Wood

Important:
    Berry is a property of the bush, not a separate resource.

11.3. Bush center search

find_bush_center(...)
    Tries random positions.

A valid center must be a valid bush placement cell.

Acceptance probability:
    - near water within radius 4: 75%
    - dry location: 35%

11.4. Bush patch painting

paint_bush_patch(...)
    Paints a circular bush patch.

Chance is interpolated from:
    bush_core_chance at the center
    bush_edge_chance near the edge

Modifiers:
    - near water within radius 3: +near_water_bush_bonus
    - near ravine within radius 2: -near_ravine_bush_penalty

The final chance is clamped to 0..100.

Dense patches use:

    cfg.dense_bush_berry_chance

as the berry probability.

11.5. Bush patch creation

create_bush_patches(...)
    Repeats up to cfg.max_bush_patch_quantity.
    If no center is found, it returns.
    Radius is random between cfg.bush_min_radius and cfg.bush_max_radius.

11.6. Scattered bushes

create_scattered_bushes(...)
    Scans the full map.
    For every valid bush cell:
        base chance = cfg.scattered_bush_chance

Modifiers:
    - near water within radius 4: +near_water_bush_bonus
    - near existing BushResource within radius 3: +2
    - near ravine within radius 2: -near_ravine_bush_penalty

Scattered bushes use:

    cfg.scattered_bush_berry_chance

as the berry probability.

===============================================================================
12. OVERWRITE AND LAYERING RULES
===============================================================================

12.1. Terrain overwrites resources

When major terrain features are painted, they clear resources:

    type_resource = None_Resource
    has_berry = false

This happens for:
    - mountains
    - ravines
    - rivers
    - lakes

12.2. Resources do not overwrite terrain

Trees and bushes only use type_resource.
They require type_terrain == Plain.
They do not modify type_terrain.

12.3. Water protection

Mountains do not overwrite River or Lake.
Ravines do not cross River, Lake, or Montain.
Rivers do not paint over Montain, ravine, or Lake.
Lakes do not paint over Montain or ravine and avoid mountain proximity.

12.4. One resource per cell

Trees and bushes both require:

    type_resource == None_Resource

Therefore, a cell cannot contain both a tree and a bush.

===============================================================================
13. GENERATION DEPENDENCIES
===============================================================================

Mountains must be generated before ravines and rivers because:
    - ravines start near mountains
    - rivers use mountains as hydrological sources
    - river mountain influence depends on existing mountain cells

Ravines are generated before rivers because:
    - rivers must avoid ravines
    - ravines represent dry cracks that water should not overwrite

Rivers are generated before forests and bushes because:
    - vegetation is influenced by nearby water
    - trees and bushes avoid non-Plain terrain

Forests are generated before bushes because:
    - both use the resource layer
    - forest generation has priority over bush placement in shared valid cells

===============================================================================
14. IMPORTANT LIMITATIONS OF THE PROVIDED VERSION
===============================================================================

The supplied River.cpp uses a mountain-side network model with terminal lakes.
It does not yet implement all later experimental ideas such as:
    - artery-style double flow with gradual downstream decay
    - guaranteed springs on every mountain after failed path attempts
    - Y-merge between close rivers using post-processing
    - lake-chain connection after lake creation

Those behaviors may exist in newer experimental River.cpp files, but they are not
part of the supplied River.cpp documented here.

The supplied version does contain:
    - main rivers from mountain components
    - extra tributaries from the same side
    - downstream flow through joined paths
    - variable river thickness from integer flow
    - lake pushing away from mountains
    - mountain-away lake expansion

===============================================================================
15. DEBUGGING CHECKLIST
===============================================================================

15.1. A generation feature does not appear

Check:
    - Is the function called in generate_map(...)?
    - Is the corresponding .cpp file compiled in the makefile?
    - Is the function declared in Map.hpp?
    - Does the function return early because no valid source/center is found?
    - Are cfg values large enough for the current map size?

15.2. Undefined reference

Usually means:
    - the function is declared in Map.hpp but not implemented
    - or the .cpp file containing the implementation is not compiled

15.3. Multiple definition

Usually means:
    - the same non-static function was implemented in two .cpp files

Examples to avoid:
    - paint_river_brush in both River.cpp and Ravine.cpp
    - create_rivers in both River.cpp and another module
    - paint_lake_area in both River.cpp and another module

15.4. Trees or bushes appear inside water/mountains

Check:
    - can_place_tree(...)
    - can_place_bush(...)
    - type_terrain must be Plain
    - type_resource must be None_Resource

15.5. Rivers create unnatural lakes around mountains

Relevant functions:
    - push_lake_center_away_from_mountains(...)
    - lake_cell_is_too_close_to_mountain(...)
    - paint_lake_area_away_from_mountain(...)

Increase mountain buffer if lakes still hug mountains.

15.6. Mountains cluster too much

Relevant function:
    - build_mountain_start_positions(...)

Check target mountain count and region distribution.

15.7. Rivers are too thin or too thick

Relevant function:
    - river_thickness_from_flow(...)

Also check:
    - cfg.river_max_thickness
    - number of tributaries successfully joining
    - build_full_downstream_path(...)

===============================================================================
16. EXTENSION IDEAS
===============================================================================

16.1. Height map

Add a dedicated elevation matrix:
    - mountains create high elevation
    - rivers follow descending gradient
    - lakes form in local basins

16.2. Improved hydrology

Extend River.cpp with:
    - guaranteed springs per mountain
    - artery-like flow accumulation
    - downstream decay
    - Y-merge for nearby rivers
    - lake-to-lake connection

16.3. Post-processing pass

Add a final cleanup phase:
    - remove tiny isolated river fragments
    - connect near river ends
    - smooth mountain edges
    - remove resources trapped in invalid terrain
    - ensure every mountain has at least one visible water source if desired

16.4. Biomes

Use water, ravine, and mountain proximity to create:
    - wet forests near rivers
    - sparse vegetation near ravines
    - different tree types by region
    - dry bushlands far from water

===============================================================================
17. SUMMARY
===============================================================================

The supplied map generator is modular and layer-based.

The central architecture is:

    MAP = vector<vector<Cell>>

Each Cell separates:
    - terrain
    - resource
    - structure
    - unit
    - visual/resource states

The generation order is:

    Plain reset
    -> Mountains
    -> Ravines
    -> Rivers and Lakes
    -> Forests and Scattered Trees
    -> Bush Patches and Scattered Bushes

The most important design principle is separation of concerns:

    Terrain features change type_terrain.
    Natural resources use type_resource.
    Visual resource states use wood_type and has_berry.

This separation keeps the generator stable and prevents common procedural map
bugs such as trees replacing mountains, bushes becoming terrain, or water
overwriting key dry features incorrectly.
