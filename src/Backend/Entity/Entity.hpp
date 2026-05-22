#pragma  once


#include <vector>
#include <SDL2/SDL_image.h>

#include "../Coordinate/Coordinate.hpp"

class Entity
{
    public:
        Entity();
        
        virtual ~Entity();


        void update();
        // void render(r *Renderer);
        void takeDamage(int amount);
        void heal(int amount);
        bool isAlive();
        bool isSelected();

        Coordinate getPos();

        int getId();

        int getTeam();


    private:


    protected:
        int id;
        Coordinate position;
        int health;
        SDL_Texture* texture;
        int team;




};