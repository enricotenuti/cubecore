#include <stdio.h>
#include <vector>
#include <stdint.h>
#include <string.h>

extern "C" {
#include "cube.h"
}

#define EDGES 12
#define CORNERS 8
#define MOVES 18
#define DRMOVES 10
using namespace std;

typedef vector<move_t> sol_t;



move_t moves[] = 
{ 	U, U2, U3, D, D2, D3,
	R, R2, R3, L, L2, L3,
	F, F2, F3, B, B2, B3
};

move_t drmoves[] = 
{ 	U, U2, U3, D, D2, D3,
	R2, L2, F2, B2
};

void print_binary(uint8_t byte) {
    for(int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
}

cube_t apply_alg(cube_t cube, std::vector<move_t> *moves) {
	auto ret = cube;

	for (auto m : *moves)
		ret = cube_move(ret, m);

	return ret;
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

int drud(const cube_t c){
    return eofb(c) && eorl(c) && coud(c);
}
int drrl(const cube_t c){
    return drud(cube_transform(c, RUr));
}
int drfb(const cube_t c){
    return drud(cube_transform(c, FUr));
}

sol_t phase2search(cube_t c, int d, sol_t s) {
    if (d == 0) {
        if(cube_solved(c)){
            return s;
        }
    } else if (d > 0) {
        for (int i=0; i<DRMOVES; i++){
            cube_t newcube = cube_move(c, drmoves[i]);
            sol_t new_s = s;
            new_s.push_back(drmoves[i]);
            phase2search(newcube, d - 1, new_s);
        }
    }  
    return s;
}

sol_t phase1search(cube_t c, int d, sol_t s) {
    if (d == 0) {
        if(drud(c)){
            phase2search(c, 18, s);
        }
    } else if (d > 0) {
        for (int i=0; i<MOVES; i++){
            cube_t newcube = cube_move(c, moves[i]);
            sol_t new_s = s;
            new_s.push_back(moves[i]);
            phase1search(newcube, d - 1, new_s);
        }
    }  
    return s;
}

sol_t twoPhase(cube_t c){
    sol_t solution;
    for (int d = 0; d <= 12; d++) {
        phase1search(c, d, solution);
        printf("phase 1 depth %d\n", d);
    }
    return solution;
}

int main(void) {
    cube_t solved = cube_new();
    cube_t cube = cube_new();
    vector<move_t> moves = {U, R, L, F};
    printf("\n");
    cube = apply_alg(cube, &moves);

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
    sol_t solution = twoPhase(cube);
    for(int i = 0; i < solution.size(); i++){
        printf("%u ", solution[i]);
    }
    return 0;

}
