/*
 * Frontend/UIManager/UIManager.hpp
 *
 * Rôle du fichier :
 * Declares the UI manager state and methods for HUD, menus, building mode, selected building, and UI click handling.
 *
 * Notes de lecture :
 * Ce module dessine les menus, le HUD et les contrôles liés aux bâtiments et à la production.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#pragma once

#include "../Renderer/Renderer.hpp"
#include "../SelectionManager/SelectionManager.hpp"
#include "../../Backend/Map/Map.hpp"
#include "../../Backend/Building/Building.hpp"
#include "../Sound/Sound.hpp"
#include "../../Backend/Player/Player.hpp"
#include "../../Backend/Unit/Unit.hpp"

/*
 * Gestionnaire de l'interface graphique : menus, HUD, boutons, mode bâtiment et sélection bâtiment.
 */
class UIManager {
    Renderer* renderer { nullptr };
    Window*   window   { nullptr };

    // État HUD
    bool   gamePaused { false };
    int    currentFPS { 0     };
    int    currentTPS { 0     };
    Uint64 gameTick   { 0     };
    int    tickRate   { 10    };

    // Mode placement bâtiment
    bool         inBuildingMode       { false };
    BuildingType selectedBuildingType { BuildingType::TownCenter };

    // Bâtiment sélectionné
    Building*    selectedBuilding     { nullptr };
    /* Interface appelée par le game loop et les gestionnaires d'entrée. */
public:
    UIManager(Renderer& renderer, Window& window);

    void setHUDStats(int fps, int tps, Uint64 tick, int tickRateVal);
    bool isGamePaused()     const;
    bool isInBuildingMode() const;
    BuildingType getSelectedBuildingType() const;
    void cancelBuildingMode();
    Building*    getSelectedBuilding() const;
    void         selectBuilding(Building* b);
    void         clearBuildingSelection();
    bool pendingProduceUnit { false };
    bool pendingProduceCollector {false};

void renderHUD(const Player* localPlayer,
               const std::vector<Unit*>& selectedUnits = {});
    void renderBuildingGhost(int mouseX, int mouseY, BuildingType type,
                             int scale, int offsetX, int offsetY);
    void renderBuildings(const MAP& map, const std::vector<Player*>& players,
                         int scale, int offsetX, int offsetY);

    void renderMinimap(MAP& map, int MAP_W, int MAP_H);
    void renderSelectionPanel();
    void renderDragRect(SelectionManager& sel);

    bool handleHUDClick(int x, int y);

    int  showMainMenu(DISPLAY_OPTIONS& options, Sound& sound);
    void showOptionsMenu(DISPLAY_OPTIONS& options, Sound& sound);

    /* Helpers internes de dessin et de positionnement du HUD. */
private:
    void drawButton(const char* label, SDL_Rect rect, bool selected, bool disabled);
    void applyResolution(DISPLAY_OPTIONS& options, int w, int h, bool fullscreen);
    SDL_Texture* makeTextTexture(const char* text, SDL_Color color);

    void drawHUDRect(SDL_Rect r, SDL_Color fill, SDL_Color border);
    void drawHUDText(const char* text, int x, int y, SDL_Color color);
    SDL_Rect getHUDPauseRect()     const;
    SDL_Rect getHUDStopRect()      const;
    SDL_Rect getHUDSelectionRect() const;
    SDL_Rect getHUDBuildingRect()  const;
};
