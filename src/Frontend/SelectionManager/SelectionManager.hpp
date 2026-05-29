/*
 * Frontend/SelectionManager/SelectionManager.hpp
 *
 * Rôle du fichier :
 * Declares selection state, drag rectangle handling, click selection, and selected-unit accessors.
 *
 * Notes de lecture :
 * Ce module garde l'état de sélection des unités contrôlables par le joueur local.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "../Window/Window.hpp"
#include "../../Backend/Unit/Unit.hpp"

/*
 * Garde la sélection courante du joueur et le rectangle de drag.
 */
class SelectionManager {
    std::vector<Unit*> selected;
    SDL_Rect           dragRect  {};
    bool               isDragging{ false };
    Vector2i           dragStart {};

    /* Interface de sélection utilisée par EventManager, UIManager et le game loop. */
public:
    void startDrag(int x, int y);
    void updateDrag(int x, int y);

    // Sélection par drag sur la liste d'unités de la scène
    void endDrag(int x, int y, std::vector<Unit*>& units,
                 int offsetX, int offsetY, int scale);

    // Clic simple
    void tryClickSelect(int x, int y, std::vector<Unit*>& units,
                        int offsetX, int offsetY, int scale);

    // Supprime les unités sélectionnées de la scène
    void deleteSelected(std::vector<std::unique_ptr<Unit>>& units);

    std::vector<Unit*>& getSelected();
    void clearSelection();
    SDL_Rect getDragRect()   const;
    bool     getIsDragging() const;
};