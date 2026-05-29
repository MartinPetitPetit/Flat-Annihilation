# Frontend Documentation

This package contains the commented frontend source files for the project. The original code logic was preserved; only explanatory comments were added.

## Global frontend structure

The frontend is built on SDL2. It manages the application window, rendering, user input, unit selection, menus, HUD elements, and audio playback. The frontend reads game state from backend objects such as `MAP`, `Player`, `Unit`, and `Building`, then renders or converts user input into commands that the main game loop can consume.

## Modules

### EventManager

- `EventManager.cpp`: Reads SDL events and converts mouse, keyboard, camera, build, movement, and attack-move inputs into game commands.

- `EventManager.hpp`: Declares the event manager interface, pending command flags, drag state, and input handlers.


### Renderer

- `Renderer.cpp`: Wraps SDL rendering operations, font loading, map drawing, zooming, viewport management, and simple drawing helpers.

- `Renderer.hpp`: Declares the renderer API used to draw textures, text, map cells, shapes, and access SDL renderer resources.


### SelectionManager

- `SelectionManager.cpp`: Implements unit selection through click and drag rectangles, limited to local player units.

- `SelectionManager.hpp`: Declares selection state, drag rectangle handling, click selection, and selected-unit accessors.


### Sound

- `Sound.cpp`: Loads sound effects and music with path fallbacks, controls volume, plays samples, and frees SDL_mixer resources.

- `Sound.hpp`: Declares the sound manager for SDL_mixer samples, music, playback, and volume control.


### UIManager

- `UIManager.cpp`: Draws menus, options, HUD panels, selected unit info, building controls, production queues, and building previews.

- `UIManager.hpp`: Declares the UI manager state and methods for HUD, menus, building mode, selected building, and UI click handling.


### Window

- `Window.cpp`: Creates and manages the SDL window, including size, fullscreen mode, resizing, and validation.

- `Window.hpp`: Declares display options, simple integer vector size type, and the SDL window wrapper.


## Main runtime flow

1. `Window` creates the SDL window and stores the current display options.

2. `Renderer` owns the SDL renderer, loads the font, draws the map, draws shapes/text, and applies zoom/camera offsets.

3. `EventManager` polls SDL events and stores pending actions such as movement, offensive movement, building placement, or building selection.

4. `SelectionManager` maintains the list of selected local-player units through click and drag selection.

5. `UIManager` draws menus, options, HUD panels, building controls, production queues, building ghosts, and building rectangles.

6. `Sound` loads and plays effects and background music using SDL_mixer.


## Notes

- The input system does not execute game logic directly. It exposes pending flags that the game loop can consume.

- The renderer uses level of detail when drawing the map so zoomed-out views remain lighter.

- Selection is restricted to the local player team. Enemy units are visible but not controllable.

- Audio and font loading use fallback paths to support different working directories.
