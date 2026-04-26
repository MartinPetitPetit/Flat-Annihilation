#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>

#define X_Max 20
#define Y_Max 20
typedef enum {Plain,Montain,Lake,tree,gold}TERRAIN;// 0plain ;1 montain ;2 lake ,3 tree ; 4 gold
typedef enum {None_Struct,Usine,Production,Resource}STRUCTURE;
typedef enum {None_Unit,archer,MONK}UNIT;

typedef struct{
    TERRAIN type_terrain;
    STRUCTURE type_struct;
    UNIT type_unit;
}CARRE;

typedef struct MAP{
    CARRE tab[X_Max][Y_Max];
}MAP;

MAP *cree_map(void);
MAP *genarete_map(MAP *map);
void affiche_map(MAP *map_);
int rac_TERRAIN_size_rec(MAP *map, int x, int y, TERRAIN type_, int **visited);


int main(){
    
    MAP *map=cree_map();
    map=genarete_map(map);
    //affiche_map(map);
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

    
for(int x=0;x!=X_Max;x++){
        for(int y=0;y!=Y_Max;y++){
            map->tab[x][y].type_terrain= rand() % 5;
            /*
            if(randon==0){

            }
            if(randon==1){

            }
            if(randon==2){

            }
            if(randon==3){

            }
            if(randon==4){

            }
            */
            //map->tab[x][y].type_terrain=;

        }
    }
     affiche_map(map);
    int terreno_atual=map->tab[2][2].type_terrain; 
    printf(
        "coord x = 2 y =2 type = %d we have repeted %d times \n"
        ,terreno_atual,rac_TERRAIN_size_rec(map, 2, 2, terreno_atual,visited));
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
