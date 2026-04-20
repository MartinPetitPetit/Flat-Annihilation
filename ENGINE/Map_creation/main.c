#define X_Max 100 
#define Y_Max 100
typedef enum {Plain,Montain,Lake,tree,gold}TERRAIN;
typedef enum {None,Usine,Production,Resource}STRUCTURE;
typedef enum {None,soldier,archer,MONK}UNIT;

typedef struct{
    TERRAIN type_terrain;
    Structure type_struct;
    UNIT type_unit;
}CARRE;

typedef struct MAP{
    CARRE tab[X_Max][Y_Max];
}MAP;

MAP cree_map(){
    MAP *pte_map;
    pte_map=(CARRE);
    for(int x=0;x!=X_Max;x++){
        for(int y=0;y!=Y_Max;y++){
            
        }
    }
    return MAP;
}


int main(){
    cree_map();
    affiche_map();
    return 1;
}



