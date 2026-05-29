/*
 * Backend/Coordinate/Coordinate.hpp
 *
 * Rôle du fichier :
 * Declares the simple grid coordinate class used to store x and y map positions.
 *
 * Notes de lecture :
 * Ce fichier appartient au module Coordinate. Il fournit une petite classe utilitaire pour les positions x/y sur la grille.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#pragma once

class Coordinate
{
public:
    Coordinate() : x(0), y(0) {}
    Coordinate(int x, int y) : x(x), y(y) {}

    int getX() const { return x; }
    int getY() const { return y; }
    void setX(int v) { x = v; }
    void setY(int v) { y = v; }

private:
    int x;
    int y;
};