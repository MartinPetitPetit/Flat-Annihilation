# Backend Documentation

This package contains the commented backend source files for the project. The original code logic was preserved; only explanatory comments were added to the files.

## Global backend structure

The backend is organized around map generation, entities, units, resources, pathfinding, players, buildings, combat, and enemy AI. The game state is mainly represented by a two-dimensional `MAP`, where each `Cell` can contain terrain, a resource, a building reference, or a unit pointer.

## Modules

### Building

- `Building.cpp`: Defines building metadata, construction data, production queues, and production timing for buildings.

- `Building.hpp`: Declares building types, building definitions, unit production kinds, and the Building class interface.


### Cell

- `Cell.cpp`: Initializes map cells and renders the cell terrain texture through the resource manager.

- `Cell.hpp`: Declares the Cell structure used by the map grid, including terrain, resources, units, buildings, and texture data.


### CombatSystem

- `CombatSystem.cpp`: Updates attack cooldowns, selects valid enemy targets, applies damage, and removes dead entities from the map.

- `CombatSystem.hpp`: Declares the static combat update and cleanup functions used by the game loop.


### Coordinate

- `Coordinate.hpp`: Declares the simple grid coordinate class used to store x and y map positions.


### EnemyAI

- `EnemyAI.cpp`: Implements the enemy strategic AI: economy, unit production, barracks construction, base defense, and attacks.

- `EnemyAI.hpp`: Declares the EnemyAI interface and the coefficients used to compare player and AI strength.


### Entity

- `Entity.cpp`: Implements common entity behavior such as health, damage, healing, identity, team, and position access.

- `Entity.hpp`: Declares the base Entity class shared by units and buildings.


### Map

- `Bush.cpp`: Generates bush and berry resource patches using terrain context, water proximity, and random density.

- `Forest.cpp`: Generates forest patches and scattered trees as wood resources.

- `Map.cpp`: Builds the procedural map, scales generation settings, exposes map helpers, and places starting entities.

- `Map.hpp`: Declares the map grid type, generation configuration, terrain helpers, and procedural generation functions.

- `Mountain.cpp`: Generates organic mountain ranges with curved direction changes, varying thickness, and natural junctions.

- `Pathfinding.cpp`: Keeps a legacy empty pathfinding file to avoid duplicate symbols when older map pathing files still exist.

- `Ravine.cpp`: Generates rare direct ravines as long terrain cuts with variable width and controlled randomness.

- `River.cpp`: Generates organic river networks from mountain springs, valley fields, confluences, and lake connections.


### Pathing

- `AIPath.cpp`: Implements A* pathfinding with exact or adjacent goal modes and step-by-step movement along computed paths.

- `AIPath.hpp`: Declares AIPath result types, goal modes, path search, and path-following functions.

- `MassPath.cpp`: Stores simple group movement plans and exposes synchronization helpers for units following mass movement orders.

- `MassPath.hpp`: Declares the group movement planning and synchronization interface.

- `MovementRules.cpp`: Centralizes terrain, building, unit, and diagonal movement validation rules.

- `MovementRules.hpp`: Declares reusable movement validation functions for units and pathfinding.

- `Pathfinding.cpp`: Implements the older A* pathfinding function and formation destination generation.


### Player

- `Player.cpp`: Manages player identity, resources, building placement, free building creation, and dead building cleanup.

- `Player.hpp`: Declares the Player class, resources, building ownership, and building management functions.


### Resource

- `Resource.cpp`: Implements resource type data, available amount, gathering logic, sprite path selection, and rendering.

- `Resource.hpp`: Declares resource types and the Resource class used by wood, food, and other map resources.


### ResourceManager

- `ResourceManager.cpp`: Implements the singleton texture manager, path resolution, compatibility search, caching, and cleanup.

- `ResourceManager.hpp`: Declares the singleton resource manager used to load and reuse SDL textures.


### Unit

- `Collector.cpp`: Implements collector AI as a state machine for searching, moving, gathering, depositing, and rendering.

- `Collector.hpp`: Declares the Collector unit class, resource state machine fields, and helper methods.

- `Unit.cpp`: Implements base unit movement, offensive behavior, attack parameters, selection, target handling, and rendering.

- `Unit.hpp`: Declares the base Unit class, movement state, combat API, offensive commands, and rendering interface.


## Main runtime flow

1. `Map` creates and configures the grid, then calls procedural generation helpers for mountains, rivers, ravines, forests, and bushes.

2. `Player` owns resources and buildings. Buildings can produce units through a queue system.

3. `Unit` handles basic movement, offensive orders, attack parameters, and rendering. `Collector` extends `Unit` with a resource-gathering state machine.

4. `Pathing` provides movement rules and A* pathfinding helpers. Collectors use these helpers for resource and deposit movement.

5. `CombatSystem` updates combat interactions and removes dead entities from the map.

6. `EnemyAI` periodically decides whether to build, train collectors, train soldiers, defend its base, or attack the player's base.


## Notes

- Terrain such as mountains, lakes, rivers, and ravines is treated as blocked for standard movement.

- `ResourceManager` centralizes texture loading and avoids repeatedly printing the same loading error.

- `Collector` uses reservations so that too many collectors do not target the same resource at the same time.

- Some files contain compatibility logic, especially the legacy map pathfinding placeholder.
