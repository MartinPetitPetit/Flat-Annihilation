# Flat Annihilation

Flat Annihilation is a small real-time strategy game prototype developed in C++ with SDL2.
The project combines procedural map generation, unit movement, resource gathering, building production, combat, enemy AI, and a graphical frontend with HUD, menus, sound, and user input handling.

## Project Overview

The game is organized around a classic RTS structure:

- a procedurally generated map;
- two players, including a human player and an enemy AI;
- units that can move, attack, gather resources, and interact with buildings;
- buildings such as the Town Center and Barracks;
- terrain generation with mountains, rivers, lakes, ravines, forests, and bushes;
- a frontend responsible for rendering, events, selection, interface, sound, and window management.

The main goal of the project is to create a playable strategy game base where the player can control units, collect resources, build structures, and fight an automated opponent.

## Main Features

### Procedural Map Generation

The map is generated automatically at launch.  
The generation system creates different types of terrain and resources, including:

- plains;
- mountains;
- rivers;
- lakes;
- ravines;
- forests;
- bushes and berries.

The generation process uses randomness with constraints. This means that the map is not completely random: each terrain type follows placement rules so that the result looks more natural and remains playable.

Examples:

- mountains are generated as organic chains with curved shapes and variable thickness;
- rivers start near mountains and can connect to lakes or other rivers;
- ravines are long terrain cuts that block movement;
- forests and bushes are placed with density rules and environmental preferences.

### Units

The project has a base `Unit` class and specialized unit behavior.

Units can:

- move across the map;
- follow a target position;
- attack enemies;
- receive damage;
- die and be removed from the map;
- be selected by the player.

The `Collector` unit extends the base behavior with a resource-gathering state machine.  
Collectors can search for resources, reserve a target resource, move to it, gather from it, return to a deposit building, and repeat the process.

### Buildings

Buildings are map entities that occupy several cells.

The current building system includes:

- `Town Center`;
- `Barracks`.

Buildings have:

- a type;
- an owner;
- hit points;
- map position;
- size;
- production queues.

The Town Center can produce collectors, while the Barracks can produce soldiers.

### Resources

Resources are placed on the map during generation.

The project currently uses resource types such as:

- food;
- wood;
- stone;
- gold.

Wood is mainly represented by trees.  
Food is represented by bushes or berries.

Resources can be gathered by collectors and decrease as they are used.

### Pathfinding and Movement

The movement system is based on grid navigation.

The project includes:

- movement validation rules;
- blocked terrain detection;
- diagonal movement rules;
- A* pathfinding;
- adjacent-goal pathfinding;
- group movement planning support.

Blocked terrain includes mountains, lakes, rivers, and ravines.  
Cells occupied by buildings, resources, or other units are also handled by the movement rules.

### Combat System

The combat system updates unit attacks during the game loop.

It is responsible for:

- checking attack range;
- selecting enemy units or buildings as targets;
- applying damage;
- handling cooldowns;
- removing dead units and buildings;
- detecting when important entities are destroyed.

### Enemy AI

The enemy AI controls the computer player.

It can:

- manage economy;
- produce collectors;
- build a barracks;
- produce soldiers;
- defend its base when attacked;
- attack the player base when it has enough army strength.

The AI does not make decisions every frame.  
Instead, it updates periodically to avoid unnecessary processing and to make its behavior more stable.

### Frontend

The frontend uses SDL2 to display and interact with the game.

It is divided into several modules:

- `Window`: creates and manages the SDL window;
- `Renderer`: draws the map, textures, shapes, text, and camera view;
- `EventManager`: reads mouse and keyboard input;
- `SelectionManager`: manages unit selection;
- `UIManager`: draws menus, HUD, buttons, selected unit information, and building controls;
- `Sound`: loads and plays sound effects and music.

## Project Architecture

The project is divided into three main parts:

```text
src/
├── Backend/
│   ├── AI/
│   ├── Building/
│   ├── Cell/
│   ├── Combat/
│   ├── Coordinate/
│   ├── Entity/
│   ├── Map/
│   ├── Pathing/
│   ├── Player/
│   ├── Resource/
│   ├── ResourceManager/
│   └── Unit/
│
├── Frontend/
│   ├── EventManager/
│   ├── Renderer/
│   ├── SelectionManager/
│   ├── Sound/
│   ├── UIManager/
│   └── Window/
│
├── Game/
│   ├── Game.cpp
│   └── Game.hpp
│
└── main.cpp
```

## Backend Modules

### Map

The map module is responsible for creating and managing the game grid.

Important files include:

- `Map.cpp`
- `Map.hpp`
- `Mountain.cpp`
- `River.cpp`
- `Ravine.cpp`
- `Forest.cpp`
- `Bush.cpp`

The map is represented as a two-dimensional grid of `Cell` objects.

Each cell can contain:

- terrain information;
- resource pointer;
- unit pointer;
- building information;
- walkability state;
- texture path.

### Pathing

The pathing module contains movement rules and pathfinding logic.

Important files include:

- `MovementRules.cpp`
- `MovementRules.hpp`
- `AIPath.cpp`
- `AIPath.hpp`
- `MassPath.cpp`
- `MassPath.hpp`
- `Pathfinding.cpp`

This module is used by units, collectors, and movement commands.

### Unit

The unit module defines mobile entities.

Important files include:

- `Unit.cpp`
- `Unit.hpp`
- `Collector.cpp`
- `Collector.hpp`

The base `Unit` class handles common movement and combat behavior.  
The `Collector` class handles resource gathering.

### Player

The player module stores player data.

It manages:

- player identity;
- resources;
- buildings;
- building placement;
- free building creation;
- cleanup of destroyed buildings.

### Building

The building module defines building types and production behavior.

It stores:

- building definitions;
- size;
- cost;
- maximum hit points;
- production queue;
- pending unit spawn.

### Combat

The combat module controls attacks and entity death.

It checks whether units can attack, applies damage, and removes destroyed entities from the map.

### Enemy AI

The AI module controls the enemy player.  
It evaluates the state of the game and decides whether to expand economy, create soldiers, defend, or attack.

### Resource and ResourceManager

The `Resource` module represents gatherable resources on the map.  
The `ResourceManager` module loads and caches SDL textures so that the same image is not loaded multiple times.

## Frontend Modules

### Window

Creates the SDL window and manages display options such as width, height, and fullscreen mode.

### Renderer

Draws the map and game objects.  
It also handles zoom, camera offset, text rendering, rectangles, circles, and texture drawing.

### EventManager

Reads SDL events and converts them into game actions.

It handles:

- mouse clicks;
- drag selection;
- camera dragging;
- mouse wheel zoom;
- keyboard shortcuts;
- build placement requests;
- attack-move orders.

### SelectionManager

Stores the list of selected units.  
It supports both click selection and rectangle selection.

### UIManager

Draws the graphical interface.

It includes:

- main menu;
- options menu;
- HUD;
- resource display;
- selected unit panel;
- building buttons;
- production queue display;
- building placement preview.

### Sound

Loads and plays audio effects and background music using SDL_mixer.

## Game Module

The `Game` module connects the backend and frontend.

It is responsible for:

- initializing SDL libraries;
- creating managers;
- creating the map;
- creating players and starting bases;
- updating the game loop;
- processing player input;
- updating units;
- updating AI;
- updating combat;
- rendering each frame;
- shutting down SDL correctly.

In practice, `Game` is the central coordinator of the project.

## Build Requirements

The project uses C++17 and SDL2.

Required libraries:

- SDL2;
- SDL2_image;
- SDL2_ttf;
- SDL2_mixer.

On an Arch-based Linux distribution, the dependencies can usually be installed with:

```bash
sudo pacman -S sdl2 sdl2_image sdl2_ttf sdl2_mixer
```

On Debian/Ubuntu-based systems:

```bash
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
```

## Build

From the project root, compile with:

```bash
make
```

To clean generated object files and binaries:

```bash
make clean
```

If `make clean` is not available, remove the build directory manually:

```bash
rm -rf build
make
```

## Run

After compilation, run the generated executable from the project root.

Depending on the Makefile configuration, the binary may be located in:

```bash
./build/bin/flat_annihilation
```

or another output path defined by the Makefile.

## Controls

The exact controls may depend on the current implementation, but the game supports:

- mouse click to select units;
- drag rectangle to select multiple units;
- right click to move selected units;
- attack-move modifier using `A` or `Q`;
- mouse wheel to zoom;
- right mouse drag or camera movement logic depending on the event manager;
- HUD buttons for building and game actions.

## Assets

The project uses external assets such as:

- terrain sprites;
- building sprites;
- unit sprites;
- resource sprites;
- sounds;
- fonts;
- background images.

The asset loading system supports several fallback paths so that the game can still find files when launched from different working directories.

Typical asset folders include:

```text
assets/
├── terrain/
├── sounds/
├── fonts/
└── images/
```

## Important Implementation Notes

### Comments and Documentation

The source files contain comments explaining the main logic.  
These comments are intended to make the code easier to understand without changing its behavior.

### Duplicate Source Files

Be careful not to keep duplicate versions of the same module in different folders.

For example, compiling both:

```text
src/Backend/AI/EnemyAI.cpp
src/Backend/EnemyAI/EnemyAI.cpp
```

or both:

```text
src/Backend/Combat/CombatSystem.cpp
src/Backend/CombatSystem/CombatSystem.cpp
```

can produce linker errors such as `multiple definition`.

Only one version of each module should be compiled.

### Path and Asset Errors

If a texture or sound cannot be loaded, verify:

- the file exists;
- the path matches the folder structure;
- the game is launched from the expected working directory;
- the asset is located in one of the fallback folders supported by the loader.

## Suggested Report Explanation

When presenting the project, the clearest explanation is:

1. `Game` is the central coordinator.
2. `Backend` stores the game state and rules.
3. `Map` creates the procedural world.
4. `Pathing` allows units to move intelligently.
5. `Unit`, `Collector`, `Building`, `Player`, and `Resource` define the gameplay objects.
6. `CombatSystem` resolves fights.
7. `EnemyAI` controls the opponent.
8. `Frontend` displays the result and converts user input into game commands.

## Current Status

The project already contains the base systems required for a playable RTS prototype:

- procedural terrain;
- controllable units;
- collector behavior;
- resources;
- buildings;
- production;
- combat;
- enemy AI;
- rendering;
- sound;
- UI;
- event management.

Further improvements could include:

- better animations;
- more unit types;
- more buildings;
- improved AI strategy;
- minimap implementation;
- save/load system;
- multiplayer support;
- more polished menus and balancing.
