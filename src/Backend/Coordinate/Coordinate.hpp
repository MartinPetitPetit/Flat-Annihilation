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