#include "primitives.h"

int phase2search(cube_t c, int d2, sol_t s, move_t last, move_t second_last, int td, int tdm) {
    if (d2 == 0) {
        if (cube_solved(c)) {
            print_solution_singmaster(s, true);
            tdm = min(tdm, td);
            return tdm;
        }
        return INT_MAX;
    } else if (d2 > 0 && d2 < tdm-td) {
        for (int i = 0; i < DRMOVES; i++) {
            if (isRedundantMove(drmoves[i],last, second_last)) {
                cube_t newcube = cube_move(c, drmoves[i]);
                sol_t new_s = s;
                new_s.push_back(drmoves[i]);
                tdm = min(tdm, phase2search(newcube, d2 - 1, new_s, drmoves[i], last, td+1, tdm));
            }
        }
    }
    return tdm;
}

int phase1search(cube_t c, int d1, int d2, sol_t s, move_t last, move_t second_last, int td, int tdm) {
    if (d1 == 0) {
        if (drud(c)) {
            for (int i = 0; i < d2; i++) {
                cube_t nc = c;
                tdm = min(tdm,phase2search(nc, i, s, last, second_last, td, tdm));
            }
            return tdm;
        }
        return INT_MAX;
    } else if (d1 > 0 && d1 < tdm-td) {
        for (int i = 0; i < MOVES; i++) {
            if (isRedundantMove(moves[i], last, second_last)) {
                cube_t newcube = cube_move(c, moves[i]);
                sol_t new_s = s;
                new_s.push_back(moves[i]);
                tdm = min(tdm, phase1search(newcube, d1 - 1, d2, new_s, moves[i], last, td+1, tdm));
            }
        }
    }
    return tdm;
}

sol_t twoPhase(cube_t c) {
    sol_t solution;
    int tdm = LIMSOL;
    for (int d = 0; d <= tdm; d++) {
        tdm = min(tdm, phase1search(c, d, LIMSOL - d, solution, move_t(), move_t(), 0, tdm));
    }
    return solution;
}