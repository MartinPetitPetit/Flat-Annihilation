# EnemyAI Documentation

## Purpose

`EnemyAI.hpp` and `EnemyAI.cpp` define and implement the behavior of an enemy player controlled by the computer. The AI manages two main responsibilities: economic production and military control.

## Header file: `EnemyAI.hpp`

The header declares the `EnemyAI` class. The class is static-only: it is not meant to be instantiated. Its public interface contains `updateSimpleEnemy`, which is the main update function called by the game loop, and `computeCoefficients`, which counts the player's and enemy's collectors and soldiers.

The `Coefficients` structure stores simple indicators used by the AI, such as the number of collectors, soldiers, and a basic force score. Soldiers are considered more important than collectors in the force calculation.

## Source file: `EnemyAI.cpp`

The source file contains the full decision logic of the enemy AI. Helper functions inside the anonymous namespace are private to this file and are used to keep the main AI methods readable.

The economic logic checks if the enemy has enough collectors, whether it needs to build a barracks, and when it should queue soldiers. If the enemy base is under threat, soldier production becomes the priority. If there is no immediate threat, the AI prepares an attack while still trying to maintain its economy.

The troop-control logic first looks for enemies close to the AI Town Center. If a threat exists, soldiers are sent to defend. Otherwise, the AI attacks the player's base, prioritizing enemy units near the player's Town Center, then buildings, and finally isolated units.

## Main behavior summary

1. The AI does not update every tick. It waits a short delay to avoid heavy decision-making each frame.
2. It keeps a minimum number of collectors to maintain resource production.
3. It builds a barracks when soldiers are needed.
4. It produces soldiers according to the player's army and the number of targets near the player's base.
5. It always prioritizes base defense before attacking.

## Files included in this package

- `EnemyAI.hpp`: commented class declaration.
- `EnemyAI.cpp`: commented implementation.
- `EnemyAI_documentation.md`: this simple documentation file.

## Note

No gameplay logic was changed. The files were only annotated with comments.
