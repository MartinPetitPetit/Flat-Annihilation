#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>

#define X_Max 20
#define Y_Max 20

// montain related 
#define Max_montain_quantity 4
#define Max_montain_size 4
#define thickness_max 4 // range off 1 to 4
#define turne_chance_max 30
#define stop_chance_max 35
#define max_size 100
//--


typedef enum {Plain,Montain,Lake,Bush,ravine}TERRAIN;// 0plain ;1 montain ;2 lake ,3 tree ; 4 gold
typedef enum {None_Struct,Usine,Production,Resource}STRUCTURE;
typedef enum {None_Resource,tree,gold,Sapling}RESOURCE;

typedef enum {None_Unit,archer,MONK}UNIT;

typedef struct{
    TERRAIN type_terrain;
    STRUCTURE type_struct;
    UNIT type_unit;
}CARRE;

typedef struct MAP{
    CARRE tab[X_Max][Y_Max];
}MAP;
// struct responsable for behavior of montains
typedef struct Montain{
    int x_init;
    int y_init;
    int size;
    int DIR;//north northeast northwest east southeast southwest west south
    int thickness;
    int turne_chance;
    int stop_chance;
    
}MONTAIN;


MAP *cree_map(void);
MAP *genarete_map(MAP *map);
void affiche_map(MAP *map_);
int rac_TERRAIN_size_rec(MAP *map, int x, int y, TERRAIN type_, int **visited);
int in_map(int x, int y);
void set_terrain(MAP *map, int x, int y, TERRAIN terrain);
MAP *crate_montain(MAP *map,MONTAIN montain[Max_montain_quantity]);

int main(){
    
    MAP *map=cree_map();
    map=genarete_map(map);
    affiche_map(map);
    return 1;
}



MAP *cree_map(void){
    MAP *map;

    map = malloc(sizeof(MAP));

    if (map == NULL) {
        return NULL;
    }

    return map;
}
void affiche_map(MAP *map_){
    printf("|");
    for(int x=0;x!=X_Max;x++){
        for(int y=0;y!=Y_Max;y++){
            printf(" %d |",map_->tab[x][y].type_terrain);
        }
        printf("\n|");
    }
    
    return;
}

MAP *genarete_map(MAP *map){
    
    srand(time(NULL));
    //int randon;
    //part 1 plain
for(int x=0;x!=X_Max;x++){
        for(int y=0;y!=Y_Max;y++){
            map->tab[x][y].type_terrain=0; 

        }
    }

    MONTAIN montain[Max_montain_quantity];
    for(int i=0;i!=Max_montain_quantity;i++){
        montain[i].x_init=rand() % X_Max;
        montain[i].y_init=rand() % Y_Max;
        montain[i].size=rand() % Max_montain_size;
        montain[i].DIR=rand() % 7;
        montain[i].thickness=rand() % thickness_max;
        montain[i].turne_chance=rand() % turne_chance_max;
        montain[i].stop_chance =rand() % stop_chance_max;
    }
    map = crate_montain(map,montain);
    // part 2 montain 
    // part 3 lakes 
    // part 4 trees 
    
    return map;

}

MAP *crate_montain(MAP *map,MONTAIN montain[Max_montain_quantity]){
    for(int i=0;i!=Max_montain_quantity;i++){
        map->tab[montain[i].x_init][montain[i].y_init].type_terrain=1;
        int x= montain[i].x_init;
        int y= montain[i].y_init;
        int half;
        int **visited;
        visited = malloc(X_Max * sizeof(int *));
        if (visited == NULL) {
            return NULL;
        }
        for (int i = 0; i < X_Max; i++) {
        visited[i] = malloc(Y_Max* sizeof(int));
            if (visited[i] == NULL) {
                return NULL;
            }
        }
        while((montain[i].stop_chance<=rand() % 100) && montain[i].size<max_size && in_map(x,y)==1){// do we keep going ?
            if(montain[i].turne_chance<=rand() % 100){// are we turning ?
                int Dir=rand() %2;//if yes, to what direction
                if(Dir==1)montain[i].DIR=montain[i].DIR-1;// we go clock direction     
                if(Dir==2)montain[i].DIR=montain[i].DIR+1;// we go anti clock direction
                if(montain[i].DIR>7)montain[i].DIR=0;//correction of negatif overflow
                if(montain[i].DIR<0)montain[i].DIR=7;//correction of positif overflow
            }
            if (montain[i].DIR == 0) { // north (+x,1)

                for (int i_ = 0; i_ < montain[i].size; i_++) {
                    x--; // north
                    half = montain[i].thickness / 2;
                    for (int t = -half; t <= half; t++) {
                            set_terrain(map, x, y + t, Montain);
                    }
                }
            }
            if(montain[i].DIR==1){// northeast  (-x,+y)
                for (int i_ = 0; i_ < montain[i].size; i_++) {
                    x--; // north
                    y++; // east
                    half = montain[i].thickness / 2;
                    for (int t = -half; t <= half; t++) {
                            set_terrain(map, x, y + t, Montain);
                    }
                }
            }
            if(montain[i].DIR==2){// east       (1,+y)
                
                for (int i_ = 0; i_ < montain[i].size; i_++) {
                    y++; // east
                    half = montain[i].thickness / 2;
                    for (int t = -half; t <= half; t++) {
                            set_terrain(map, x, y + t, Montain);
                    }
                }
            }
            if(montain[i].DIR==3){// southeast  (+x,+y)
                for (int i_ = 0; i_ < montain[i].size; i_++) {
                    x++; // south
                    y++; // east
                    half = montain[i].thickness / 2;
                    for (int t = -half; t <= half; t++) {
                            set_terrain(map, x, y + t, Montain);
                    }
                }
            }
            if(montain[i].DIR==4){// south      (+x,1 )

                for (int i_ = 0; i_ < montain[i].size; i_++) {
                    x++; // south
                    half = montain[i].thickness / 2;
                    for (int t = -half; t <= half; t++) {
                            set_terrain(map, x, y + t, Montain);
                    }
                }
            }
            if(montain[i].DIR==5){// southwest  (+x,-y )
                    for (int i_ = 0; i_ < montain[i].size; i_++) {
                    x++; // south
                    y--; // west
                    half = montain[i].thickness / 2;
                    for (int t = -half; t <= half; t++) {
                            set_terrain(map, x, y + t, Montain);
                    }
                }
            }
            if(montain[i].DIR==6){// west       (1,-y)
                
                for (int i_ = 0; i_ < montain[i].size; i_++) {
                    y--; // west
                    half = montain[i].thickness / 2;
                    for (int t = -half; t <= half; t++) {
                            set_terrain(map, x, y + t, Montain);
                    }
                }
            }
            if(montain[i].DIR==7){// northwest  (-x,-y)
                for (int i_ = 0; i_ < montain[i].size; i_++) {
                    x--; // sud
                    y--; // west
                    half = montain[i].thickness / 2;
                    for (int t = -half; t <= half; t++) {
                            set_terrain(map, x, y + t, Montain);
                    }
                }
            }
            montain[i].size=rac_TERRAIN_size_rec(map,x,y,1,visited);
        }
    }
    return map;
}

int rac_TERRAIN_size_rec(MAP *map, int x, int y, TERRAIN type_, int **visited) {
    if (x < 0 || x >= X_Max || y < 0 || y >= Y_Max) {// Im i in the map?
        return 0;
    }
    if (visited[x][y] == 1) {// have i already tested this one?
        return 0;
    }
    if (map->tab[x][y].type_terrain != type_) {// is it the same TERRAIN ?
        return 0;
    }
    visited[x][y] = 1;// check where im passing 

    int total = 1;// add currant case to total 
    // 6. check around 
    total += rac_TERRAIN_size_rec(map, x + 1, y,     type_, visited);//DOWN
    total += rac_TERRAIN_size_rec(map, x - 1, y,     type_, visited);//UP
    total += rac_TERRAIN_size_rec(map, x,     y + 1, type_, visited);//RIGHT 
    total += rac_TERRAIN_size_rec(map, x,     y - 1, type_, visited);//LEFT

    total += rac_TERRAIN_size_rec(map, x + 1, y + 1, type_, visited);//DOWN RIGHT
    total += rac_TERRAIN_size_rec(map, x + 1, y - 1, type_, visited);//DOWN LEFT 
    total += rac_TERRAIN_size_rec(map, x - 1, y + 1, type_, visited);//UP   RIGHT
    total += rac_TERRAIN_size_rec(map, x - 1, y - 1, type_, visited);//UP   LEFT 

    return total;
}

int in_map(int x, int y)// are we inside pf the map?
{
    return x >= 0 && x < X_Max && y >= 0 && y < Y_Max;
}

void set_terrain(MAP *map, int x, int y, TERRAIN terrain)// change terrain if possible 
{
    if (in_map(x, y)) {// can i?
        map->tab[x][y].type_terrain = terrain;// yes! do it 
    }
}
