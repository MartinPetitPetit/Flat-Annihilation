# Game Module Documentation

## Overview

The `Game` module is the central coordinator of the project. It connects the frontend systems, the backend simulation, the map, the players, the units, the enemy AI, combat, production, audio, and rendering.

`Game.cpp` contains the implementation of the main game flow. It initializes SDL and all managers, starts the menu, creates the map and starting bases, runs the main loop, processes player commands, updates the simulation, spawns produced units, removes destroyed entities, and renders each frame.

`Game.hpp` declares the `Game` class, the tick/FPS constants, and all private helper methods used to separate the game loop into clear responsibilities.

## Main responsibilities

- Initialize and shut down SDL, SDL_ttf, SDL_image, the window, renderer, UI, sound, selection, and event managers.
- Start a new game from the main menu.
- Create the map and the initial player/AI bases.
- Process mouse, keyboard, HUD, building, production, attack-move, and movement commands.
- Run the fixed-tick simulation loop.
- Update enemy AI, unit AI, movement, combat, destroyed entities, and building production.
- Spawn units from buildings when production is complete.
- Render the world, units, buildings, building ghost, selection rectangle, and HUD.

## Game loop structure

The main loop is split into three major parts:

1. `processEventsAndCommands()` reads SDL events and transforms pending UI/event flags into gameplay actions.
2. `processFixedTicks()` updates simulation logic at a stable tick rate.
3. `renderFrameIfNeeded()` renders frames according to the FPS cap.

`updateStatsIfNeeded()` updates FPS, TPS, tick number, and game time once per second.

## Production and spawning

Production requests come from the UI. Soldiers are queued from barracks and collectors are queued from the Town Center. When a building finishes production, `spawnPendingUnitFromBuilding()` searches for a free cell around the building and creates either a `Collector` or a normal `Unit`.

The spawn sound is only played for the local player. Enemy AI unit production stays silent so the player does not hear feedback for actions they did not directly trigger.

## Movement commands

Normal move orders are passed to `MassPath::requestGroupMove()`. If the clicked target is occupied by a building or unit, `resolveManualMoveTarget()` tries to redirect the destination to a nearby free cell.

Offensive move orders are applied only to local units that can attack. These units receive an offensive destination and then use their own combat-oriented behavior to engage nearby enemies.

## Rendering

Rendering is divided into layers:

- `renderWorld()` draws the map.
- `renderUnits()` draws all units.
- `renderBuildingsLayer()` draws buildings.
- `renderUI()` draws selection, building placement preview, and HUD.

## Files

- `Game.cpp`: implementation of initialization, game loop, commands, simulation update, spawning, and rendering.
- `Game.hpp`: declaration of the `Game` class and its constants, managers, state, and helper methods.
