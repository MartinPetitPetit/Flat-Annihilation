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
