#include <stdio.h>
#include <vector>
#include <stdint.h>
#include <string.h>
#include "primitives.h"


int main(void) {
    cube_t solved = cube_new();
    cube_t cube = cube_new();
    string filename = "scramble.txt";
    sol_t scramble = read_scramble_from_file(filename);
    printf("scramble: ");
    print_solution_singmaster(scramble);
    cube = apply_alg(cube, &scramble);
    char buf[500];
    cube_write("LST", cube, buf);
    printf("%s\n", buf);

    // printf("oriented: %s\n", coud(cube) ? "true" : "false");
    // cube_t trans = cube_transform(cube, RUr);
    // for (int i = 0; i < CORNERS; i++) {
    //     printf("%x\t", solved.corner[i]);
    //     print_binary(solved.corner[i]);
    //     printf("\t");
    //     printf("%x\t", cube.corner[i]);
    //     print_binary(cube.corner[i]);
    //     printf("\t");
    //     printf("%x\t", trans.corner[i]);
    //     print_binary(trans.corner[i]);
    //     printf("\n");
    // }
    twoPhase(cube);
    return 0;
}
