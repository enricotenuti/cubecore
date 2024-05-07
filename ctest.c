#include "cube.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define EDGES 12
#define CORNERS 8

void print_binary(uint8_t byte) {
    for(int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
}

int eofb(const cube_t c){
    bool oriented = true;
    for(int i = 0; i < EDGES; i++){
        if(c.edge[i] & 0xF0){
            oriented = false;
            break;
        }
    }
    return oriented;
}

int eorl(const cube_t c){
    cube_t transformed = cube_transform(c, URr);
    return eofb(transformed);
}
int eoud(const cube_t c){
    cube_t transformed = cube_transform(c, FUr);
    return eofb(transformed);
}

int coud(const cube_t c){
    bool oriented = true;
    for(int i = 0; i <  CORNERS; i++){
        if(c.corner[i] & 0xF0){
            oriented = false;
            break;
        }
    }
    return oriented;
}
int corl(const cube_t c){
    cube_t transformed = cube_transform(c, RUr);
    return coud(transformed);
}
int cofb(const cube_t c){
    cube_t transformed = cube_transform(c, FUr);
    return coud(transformed);
}

cube_t apply_alg(cube_t cube, move_t moves[], int n) {
	cube_t ret = cube;

	for (int i = 0; i < n; i++)
		ret = cube_move(ret, moves[i]);

	return ret;
}


int main(void) {
    cube_t solved = cube_new();
    cube_t cube = cube_new();
    move_t moves[6] = { R, L, F, B, R, L };
    printf("\n");
    cube = apply_alg(cube, moves, 6);

    printf("oriented: %s\n", coud(cube)? "true":"false");
     cube_t trans = cube_transform(cube, RUr);
    for(int i=0; i<CORNERS; i++){
        printf("%x\t", solved.corner[i]);
        print_binary(solved.corner[i]);
        printf("\t");
        printf("%x\t", cube.corner[i]);
        print_binary(cube.corner[i]);
        printf("\t");
        printf("%x\t", trans.corner[i]);
        print_binary(trans.corner[i]);
        printf("\n");
    }
    return 0;

}
