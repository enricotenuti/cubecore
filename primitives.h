#ifndef PRIMITIVES_H
#define PRIMITIVES_H
#include <stdbool.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "cube.h"



#define EDGES 12
#define CORNERS 8
#define MOVES 18
#define DRMOVES 10
#define LIMSOL 20

using namespace std;
typedef vector<move_t> sol_t;


extern unordered_map<string, move_t> singmaster_map;
extern const sol_t moves;
extern const sol_t drmoves;


cube_t apply_alg(cube_t cube, vector<move_t> *moves);

void print_binary(uint8_t byte);
void print_solution_H48(sol_t s);
void print_solution_enum(sol_t s);
void print_solution_singmaster(sol_t s, bool l=false);


int eofb(const cube_t c);
int eorl(const cube_t c);
int eoud(const cube_t c);
int coud(const cube_t c);
int corl(const cube_t c);
int cofb(const cube_t c);

int drud(const cube_t c);
int drrl(const cube_t c);
int drfb(const cube_t c);
int dr(const cube_t c);

bool isRedundantMove(move_t new_move, move_t last_move, move_t second_last_move);
sol_t read_scramble_from_file(const string& filename);

int phase2search(cube_t c, int d2, sol_t s, move_t last, move_t second_last, int td, int tdm);
int phase1search(cube_t c, int d1, int d2, sol_t s, move_t last, move_t second_last, int td, int tdm);
sol_t twoPhase(cube_t c);

#endif
