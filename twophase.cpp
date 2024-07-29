#include "primitives.h"

int phase2search(cube_t c, int d2, sol_t s, move_t last, move_t second_last, int td, int hb, int axis, int pd) {
    if (d2 == 0) {
        if (cube_solved(c)) {
            print_solution_singmaster(s, true);
            hb = min(hb, td);
            return hb;
        }
        return INT_MAX;
    } else if (d2 > 0 && td <= pd) {
        for (int i = 0; i < DRMOVES; i++) {
            if (notRedundantMove(drmoves[axis][i],last, second_last)) {
                cube_t newcube = cube_move(c, drmoves[axis][i]);
                sol_t new_s = s;
                new_s.push_back(drmoves[axis][i]);
                hb = min(hb, phase2search(newcube, d2 - 1, new_s, drmoves[axis][i], last, td+1, hb, axis, pd));
            }
        }
    }
    return hb;
}

int phase1search(cube_t c, int d1, sol_t s, move_t last, move_t second_last, int td, int hb, int pd) {
    if (d1 == 0) {
        if (dr(c)) {
            int axis = 0;
            if (drrl(c)) {
                axis = 1;
            } else if (drfb(c)) {
                axis = 2;
            }
            for (int i = 0; i < hb-td; i++) {      
                cube_t nc = c;
                hb = min(hb,phase2search(nc, i, s, last, second_last, td, hb, axis, pd));
            }
            return hb;
        }
        return INT_MAX;
    } else if (d1 > 0 && td <= pd) {
        for (int i = 0; i < MOVES; i++) {
            if (notRedundantMove(moves[i], last, second_last)) {
                cube_t newcube = cube_move(c, moves[i]);
                sol_t new_s = s;
                new_s.push_back(moves[i]);
                hb = min(hb, phase1search(newcube, d1 - 1, new_s, moves[i], last, td+1, hb, pd));
            }
        }
    }
    return hb;
}

sol_t twoPhase(cube_t c) {
    sol_t solution;
    int hb = LIMSOL;
    for (int pd = 0; pd < hb; pd++) {
        // printf("phase depth: %d\n", pd);
        for(int d = 0; d < pd; d++) {
            hb = min(hb, phase1search(c, d, solution, move_t(), move_t(), 0, hb, pd));
        }
    }
    return solution;
}